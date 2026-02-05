#include "triggerbot.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include "../stap/stap.h"
#include "../wtap/wtap.h"
#include "../autojumpreset/autojumpreset.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstring>
#include <random>

jobject enhance::modules::triggerbot::last_targeted_entity = nullptr;
ULONGLONG enhance::modules::triggerbot::last_attack_time = 0;
int enhance::modules::triggerbot::current_delay = 0;

static std::random_device rd;
static std::mt19937 gen(rd());

static int get_random_delay(int min_delay, int max_delay)
{
	if (min_delay >= max_delay)
		return min_delay;
	std::uniform_int_distribution<> dis(min_delay, max_delay);
	return dis(gen);
}

static void send_click()
{
	HWND window = Hook::get_window();
	if (!window)
		return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window)
		return;

	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	PostMessageA(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	PostMessageA(window, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

static void hold_right_click_down()
{
	HWND window = Hook::get_window();
	if (!window) return;

	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	PostMessageA(window, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

static void release_right_click()
{
	HWND window = Hook::get_window();
	if (!window) return;

	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	PostMessageA(window, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

static bool has_shield_in_offhand()
{
	jobject player = sdk::instance->get_player();
	if (!player) return false;

	auto env = enhance::instance->get_env();
	if (!env)
	{
		return false;
	}

	jclass player_class = env->GetObjectClass(player);
	if (!player_class)
	{
		env->DeleteLocalRef(player);
		return false;
	}

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		env->DeleteLocalRef(player);
		return false;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	if (!inventory)
	{
		env->DeleteLocalRef(player);
		return false;
	}

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		env->DeleteLocalRef(player);
		return false;
	}

	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	if (!get_stack_mid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		env->DeleteLocalRef(player);
		return false;
	}

	jobject offhand_stack = env->CallObjectMethod(inventory, get_stack_mid, 40);
	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
	env->DeleteLocalRef(player);

	if (!offhand_stack) return false;

	jclass itemstack_class = env->GetObjectClass(offhand_stack);
	if (!itemstack_class)
	{
		env->DeleteLocalRef(offhand_stack);
		return false;
	}

	jmethodID is_empty_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_is_empty_name, sdk::mappings::itemstack_is_empty_sig);
	if (!is_empty_mid)
	{
		env->DeleteLocalRef(itemstack_class);
		env->DeleteLocalRef(offhand_stack);
		return false;
	}

	jboolean is_empty = env->CallBooleanMethod(offhand_stack, is_empty_mid);
	if (is_empty == JNI_TRUE)
	{
		env->DeleteLocalRef(itemstack_class);
		env->DeleteLocalRef(offhand_stack);
		return false;
	}

	jmethodID get_item_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_get_item_name, sdk::mappings::itemstack_get_item_sig);
	env->DeleteLocalRef(itemstack_class);
	if (!get_item_mid)
	{
		env->DeleteLocalRef(offhand_stack);
		return false;
	}

	jobject item = env->CallObjectMethod(offhand_stack, get_item_mid);
	env->DeleteLocalRef(offhand_stack);
	if (!item) return false;

	jclass item_class = env->GetObjectClass(item);
	if (!item_class)
	{
		env->DeleteLocalRef(item);
		return false;
	}

	jmethodID get_translation_key_mid = env->GetMethodID(item_class, sdk::mappings::item_get_translation_key_name, sdk::mappings::item_get_translation_key_sig);
	if (!get_translation_key_mid)
	{
		env->DeleteLocalRef(item_class);
		env->DeleteLocalRef(item);
		return false;
	}

	jstring translation_key = (jstring)env->CallObjectMethod(item, get_translation_key_mid);
	env->DeleteLocalRef(item_class);
	env->DeleteLocalRef(item);
	if (!translation_key) return false;

	const char* key_cstr = env->GetStringUTFChars(translation_key, nullptr);
	if (!key_cstr)
	{
		env->DeleteLocalRef(translation_key);
		return false;
	}

	bool is_shield = (strstr(key_cstr, "shield") != nullptr);
	env->ReleaseStringUTFChars(translation_key, key_cstr);
	env->DeleteLocalRef(translation_key);

	return is_shield;
}

static bool is_entity_blocking(jobject target_entity)
{
	if (!target_entity) return false;
	auto env = enhance::instance->get_env();
	if (!env) return false;

	jclass player_class_check = sdk::classloader::find_class(env, sdk::mappings::player_entity_class_sig);
	if (!player_class_check) return false;
	
	jboolean is_player = env->IsInstanceOf(target_entity, player_class_check);
	env->DeleteLocalRef(player_class_check);
	
	if (!is_player) return false;

	jclass living_entity_class = sdk::classloader::find_class(env, sdk::mappings::living_entity_class_sig);
	if (!living_entity_class) return false;
	
	jmethodID is_blocking_mid = env->GetMethodID(living_entity_class, sdk::mappings::living_entity_is_blocking_name, sdk::mappings::living_entity_is_blocking_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(living_entity_class);
	
	if (!is_blocking_mid) return false;
	
	jboolean blocking = env->CallBooleanMethod(target_entity, is_blocking_mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	return blocking == JNI_TRUE;
}

// Check if player is holding a weapon (sword, axe, or mace)
static bool is_holding_weapon()
{
	jobject player = sdk::instance->get_player();
	if (!player) return false;

	auto env = enhance::instance->get_env();
	if (!env)
	{
		return false;
	}

	jclass player_class = env->GetObjectClass(player);
	if (!player_class)
	{
		env->DeleteLocalRef(player);
		return false;
	}

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		env->DeleteLocalRef(player);
		return false;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	if (!inventory)
	{
		env->DeleteLocalRef(player);
		return false;
	}

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		env->DeleteLocalRef(player);
		return false;
	}

	// Get selected slot
	jfieldID selected_slot_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_selected_slot_name, sdk::mappings::inventory_selected_slot_sig);
	if (!selected_slot_fid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		env->DeleteLocalRef(player);
		return false;
	}

	int selected_slot = env->GetIntField(inventory, selected_slot_fid);

	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	env->DeleteLocalRef(inventory_class);
	if (!get_stack_mid)
	{
		env->DeleteLocalRef(inventory);
		env->DeleteLocalRef(player);
		return false;
	}

	jobject stack = env->CallObjectMethod(inventory, get_stack_mid, selected_slot);
	env->DeleteLocalRef(inventory);
	env->DeleteLocalRef(player);

	if (!stack) return false;

	jclass itemstack_class = env->GetObjectClass(stack);
	if (!itemstack_class)
	{
		env->DeleteLocalRef(stack);
		return false;
	}

	jmethodID is_empty_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_is_empty_name, sdk::mappings::itemstack_is_empty_sig);
	if (!is_empty_mid)
	{
		env->DeleteLocalRef(itemstack_class);
		env->DeleteLocalRef(stack);
		return false;
	}

	jboolean is_empty = env->CallBooleanMethod(stack, is_empty_mid);
	if (is_empty == JNI_TRUE)
	{
		env->DeleteLocalRef(itemstack_class);
		env->DeleteLocalRef(stack);
		return false;
	}

	jmethodID get_item_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_get_item_name, sdk::mappings::itemstack_get_item_sig);
	env->DeleteLocalRef(itemstack_class);
	if (!get_item_mid)
	{
		env->DeleteLocalRef(stack);
		return false;
	}

	jobject item = env->CallObjectMethod(stack, get_item_mid);
	env->DeleteLocalRef(stack);
	if (!item) return false;

	jclass item_class = env->GetObjectClass(item);
	if (!item_class)
	{
		env->DeleteLocalRef(item);
		return false;
	}

	jmethodID get_translation_key_mid = env->GetMethodID(item_class, sdk::mappings::item_get_translation_key_name, sdk::mappings::item_get_translation_key_sig);
	if (!get_translation_key_mid)
	{
		env->DeleteLocalRef(item_class);
		env->DeleteLocalRef(item);
		return false;
	}

	jstring translation_key = (jstring)env->CallObjectMethod(item, get_translation_key_mid);
	env->DeleteLocalRef(item_class);
	env->DeleteLocalRef(item);
	if (!translation_key) return false;

	const char* key_cstr = env->GetStringUTFChars(translation_key, nullptr);
	if (!key_cstr)
	{
		env->DeleteLocalRef(translation_key);
		return false;
	}

	// Check for weapon types: sword, axe, mace
	bool is_weapon = (strstr(key_cstr, "sword") != nullptr ||
	                  strstr(key_cstr, "axe") != nullptr ||
	                  strstr(key_cstr, "mace") != nullptr ||
	                  strstr(key_cstr, "trident") != nullptr);
	env->ReleaseStringUTFChars(translation_key, key_cstr);
	env->DeleteLocalRef(translation_key);

	return is_weapon;
}

static void handle_shield_use()
{
	if (!globals::triggerbot_use_shield) return;
	
	if (!has_shield_in_offhand()) return;

	if (!globals::triggerbot_shield_holding)
	{
		globals::triggerbot_shield_holding = true;
		globals::triggerbot_shield_start_time = GetTickCount64();
		hold_right_click_down();
	}
	else
	{
		globals::triggerbot_shield_start_time = GetTickCount64();
	}
}

static void update_shield_state()
{
	if (!globals::triggerbot_use_shield)
	{
		if (globals::triggerbot_shield_holding)
		{
			release_right_click();
			globals::triggerbot_shield_holding = false;
		}
		return;
	}

	if (!globals::triggerbot_shield_holding) return;

	ULONGLONG current_time = GetTickCount64();
	if (current_time - globals::triggerbot_shield_start_time >= (ULONGLONG)globals::triggerbot_shield_duration_ms)
	{
		release_right_click();
		globals::triggerbot_shield_holding = false;
	}
}

void enhance::modules::triggerbot::run()
{
	update_shield_state();
	
	if (!globals::triggerbot_enabled)
	{
		cleanup();
		return;
	}

	if (globals::show_gui)
		return;

	HWND window = Hook::get_window();
	if (!window)
		return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window)
		return;

	jobject minecraft = sdk::instance->get_minecraft();
	if (!minecraft)
		return;

	auto env = enhance::instance->get_env();
	if (!env)
		return;

	jobject crosshair_target = sdk::instance->get_crosshair_target();
	if (!crosshair_target)
	{
		cleanup_refs(env, minecraft, nullptr, nullptr);
		return;
	}

	if (!is_entity_hit_result(crosshair_target))
	{
		cleanup_refs(env, minecraft, crosshair_target, nullptr);
		return;
	}

	jobject target_entity = get_entity_from_hit_result(crosshair_target);
	if (!target_entity)
	{
		cleanup_refs(env, minecraft, crosshair_target, nullptr);
		return;
	}

	jobject local_player = sdk::instance->get_player();
	if (local_player)
	{
		sdk::entity_client entity_client(target_entity);
		if (entity_client.is_same_object(local_player))
		{
			env->DeleteLocalRef(local_player);
			cleanup_refs(env, minecraft, crosshair_target, target_entity);
			return;
		}
		env->DeleteLocalRef(local_player);
	}

	if (globals::triggerbot_check_shield)
	{
		bool target_blocking = is_entity_blocking(target_entity);
		if (target_blocking)
		{
			if (globals::triggerbot_shield_action == 0)
			{
				cleanup_refs(env, minecraft, crosshair_target, target_entity);
				return;
			}
		}
	}

	// Check if weapon only mode is enabled
	if (globals::triggerbot_weapon_only && !is_holding_weapon())
	{
		cleanup_refs(env, minecraft, crosshair_target, target_entity);
		return;
	}

	// CRIT MODE CHECK - Different crit behaviors
	if (globals::triggerbot_crit_mode > 0)
	{
		jobject local_player_crit = sdk::instance->get_player();
		if (local_player_crit)
		{
			sdk::entity_client local_entity(local_player_crit);
			bool is_on_ground = local_entity.is_on_ground();
			
			// Get Y velocity to determine if falling
			double y_velocity = 0.0;
			bool has_velocity = false;
			jobject velocity = local_entity.get_velocity();
			if (velocity)
			{
				jclass vec3d_class = sdk::classloader::find_class(env, sdk::mappings::vec3d_class_sig);
				if (vec3d_class)
				{
					jfieldID y_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_y_name, sdk::mappings::vec3d_y_sig);
					if (y_fid)
					{
						y_velocity = env->GetDoubleField(velocity, y_fid);
						if (env->ExceptionCheck()) env->ExceptionClear();
						has_velocity = true;
					}
					env->DeleteLocalRef(vec3d_class);
				}
				env->DeleteLocalRef(velocity);
			}
			
			bool is_falling = false;
			if (has_velocity)
			{
				// Falling = not on ground AND negative Y velocity
				is_falling = !is_on_ground && (y_velocity < -0.1);
			}
			else
			{
				// Fallback: just check if not on ground
				is_falling = !is_on_ground;
			}
			
			bool is_in_air = !is_on_ground;
			bool is_going_up = has_velocity && (y_velocity > 0.1);
			
			env->DeleteLocalRef(local_player_crit);
			
			// Mode 1 = Crit Only: Only attack when falling
			if (globals::triggerbot_crit_mode == 1)
			{
				if (!is_falling)
				{
					cleanup_refs(env, minecraft, crosshair_target, target_entity);
					return;
				}
			}
			// Mode 2 = Priority Crit: Wait if in air but not falling, only attack when falling
			else if (globals::triggerbot_crit_mode == 2)
			{
				// If in air but going up (knocked into air), wait until falling
				if (is_in_air && is_going_up)
				{
					cleanup_refs(env, minecraft, crosshair_target, target_entity);
					return;
				}
				// Only attack when actually falling (negative Y velocity)
				if (!is_falling)
				{
					cleanup_refs(env, minecraft, crosshair_target, target_entity);
					return;
				}
			}
		}
		else
		{
			cleanup_refs(env, minecraft, crosshair_target, target_entity);
			return;
		}
	}

	player_client player;
	float cooldown_progress = player.get_attack_cooldown_progress(0.5f);
	bool cooldown_ready = (cooldown_progress >= 1.0f);

	if (globals::triggerbot_hit_select && !cooldown_ready)
	{
		cleanup_refs(env, minecraft, crosshair_target, target_entity);
		return;
	}

	ULONGLONG current_time = GetTickCount64();
	ULONGLONG time_since_last_attack = current_time - last_attack_time;

	bool should_attack = false;
	int required_delay = 0;

	switch (globals::triggerbot_mode)
	{
		case 0:
			required_delay = globals::triggerbot_delay_ms;
			should_attack = (time_since_last_attack >= static_cast<ULONGLONG>(required_delay));
			break;

		case 1:
			if (cooldown_ready && time_since_last_attack >= 50)
			{
				should_attack = true;
			}
			break;

		case 2:
			{
				if (current_delay == 0)
				{
					current_delay = get_random_delay(globals::triggerbot_min_delay_ms, globals::triggerbot_max_delay_ms);
				}
				
				if (time_since_last_attack >= static_cast<ULONGLONG>(current_delay))
				{
					if (!globals::triggerbot_hit_select || cooldown_ready)
					{
						should_attack = true;
						current_delay = get_random_delay(globals::triggerbot_min_delay_ms, globals::triggerbot_max_delay_ms);
					}
				}
			}
			break;

		default:
			required_delay = globals::triggerbot_delay_ms;
			should_attack = (time_since_last_attack >= static_cast<ULONGLONG>(required_delay));
			break;
	}

	if (should_attack)
	{
		send_click();
		last_attack_time = current_time;
		
		enhance::modules::stap::on_hit();
		
		enhance::modules::wtap::on_hit();
		
		enhance::modules::autojumpreset::on_hit();
		
		handle_shield_use();
		
		if (last_targeted_entity)
			env->DeleteGlobalRef(last_targeted_entity);
		last_targeted_entity = env->NewGlobalRef(target_entity);
	}

	cleanup_refs(env, minecraft, crosshair_target, target_entity);
}

void enhance::modules::triggerbot::cleanup()
{
	auto env = enhance::instance->get_env();
	if (env && last_targeted_entity)
	{
		env->DeleteGlobalRef(last_targeted_entity);
		last_targeted_entity = nullptr;
	}
	last_attack_time = 0;
	current_delay = 0;
}

void enhance::modules::triggerbot::cleanup_refs(JNIEnv* env, jobject minecraft, jobject crosshair, jobject entity)
{
	if (env)
	{
		if (entity) env->DeleteLocalRef(entity);
		if (crosshair) env->DeleteLocalRef(crosshair);
		if (minecraft) env->DeleteLocalRef(minecraft);
	}
}

bool enhance::modules::triggerbot::is_entity_hit_result(jobject hit_result)
{
	if (!hit_result) 
		return false;

	auto env = enhance::instance->get_env();
	if (!env) 
		return false;

	// Use mappings from mappings.hpp
	if (sdk::mappings::entity_hit_result_class_sig)
	{
		jclass entity_hit_result_class = sdk::classloader::find_class(env, sdk::mappings::entity_hit_result_class_sig);
		if (entity_hit_result_class)
		{
			jboolean is_entity = env->IsInstanceOf(hit_result, entity_hit_result_class);
			env->DeleteLocalRef(entity_hit_result_class);
			return is_entity == JNI_TRUE;
		}
	}
	
	// Fallback if mappings not available
		return false;
}

jobject enhance::modules::triggerbot::get_entity_from_hit_result(jobject hit_result)
{
	if (!hit_result) 
		return nullptr;

	auto env = enhance::instance->get_env();
	if (!env) 
		return nullptr;

	// Use mappings from mappings.hpp
	if (sdk::mappings::entity_hit_result_class_sig && sdk::mappings::entity_hit_result_get_entity_name && sdk::mappings::entity_hit_result_get_entity_sig)
	{
		jclass hit_result_class = sdk::classloader::find_class(env, sdk::mappings::entity_hit_result_class_sig);
	if (!hit_result_class) 
		return nullptr;

		// Try as method first
		jmethodID mid = env->GetMethodID(hit_result_class, sdk::mappings::entity_hit_result_get_entity_name, sdk::mappings::entity_hit_result_get_entity_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (mid)
	{
			jobject entity = env->CallObjectMethod(hit_result, mid);
			if (env->ExceptionCheck()) env->ExceptionClear();
			env->DeleteLocalRef(hit_result_class);
			return entity;
		}
		
		// Try as field if method failed
		jfieldID fid = env->GetFieldID(hit_result_class, sdk::mappings::entity_hit_result_get_entity_name, sdk::mappings::entity_hit_result_get_entity_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (fid)
		{
			jobject entity = env->GetObjectField(hit_result, fid);
			if (env->ExceptionCheck()) env->ExceptionClear();
		env->DeleteLocalRef(hit_result_class);
		return entity;
	}

	env->DeleteLocalRef(hit_result_class);
	}

	// Fallback if mappings not available
	return nullptr;
}
