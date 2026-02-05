#include "mace.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/util/box.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstring>
#include <cmath>
#include <windows.h>

int enhance::modules::mace::find_item_in_hotbar(jobject player, const char* item_name)
{
	auto env = enhance::instance->get_env();
	if (!env || !player) return -1;

	jclass player_class = env->GetObjectClass(player);
	if (!player_class) return -1;

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		return -1;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	if (!inventory) return -1;

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		return -1;
	}

	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	if (!get_stack_mid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		return -1;
	}

	for (int slot = 0; slot < 9; slot++)
	{
		jobject stack = env->CallObjectMethod(inventory, get_stack_mid, slot);
		if (stack)
		{
			jclass itemstack_class = env->GetObjectClass(stack);
			if (itemstack_class)
			{
				jmethodID is_empty_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_is_empty_name, sdk::mappings::itemstack_is_empty_sig);
				env->DeleteLocalRef(itemstack_class);
				
				if (is_empty_mid)
				{
					jboolean empty = env->CallBooleanMethod(stack, is_empty_mid);
					if (empty == JNI_FALSE && is_item(stack, item_name))
					{
						env->DeleteLocalRef(stack);
						env->DeleteLocalRef(inventory_class);
						env->DeleteLocalRef(inventory);
						return slot;
					}
				}
			}
			env->DeleteLocalRef(stack);
		}
	}

	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
	return -1;
}


bool enhance::modules::mace::is_item(jobject item_stack, const char* item_name)
{
	if (!item_stack) return false;
	auto env = enhance::instance->get_env();
	if (!env) return false;

	jclass itemstack_class = env->GetObjectClass(item_stack);
	if (!itemstack_class) return false;

	jmethodID get_item_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_get_item_name, sdk::mappings::itemstack_get_item_sig);
	if (!get_item_mid)
	{
		env->DeleteLocalRef(itemstack_class);
		return false;
	}

	jobject item = env->CallObjectMethod(item_stack, get_item_mid);
	env->DeleteLocalRef(itemstack_class);
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

	bool matches = (strstr(key_cstr, item_name) != nullptr);
	env->ReleaseStringUTFChars(translation_key, key_cstr);
	env->DeleteLocalRef(translation_key);

	return matches;
}


void enhance::modules::mace::swap_to_slot(int slot)
{
	if (slot < 0 || slot > 8) return;
	jobject player = sdk::instance->get_player();
	if (!player) return;
	
	auto env = enhance::instance->get_env();
	if (!env) return;

	jclass player_class = env->GetObjectClass(player);
	if (!player_class)
	{
		env->DeleteLocalRef(player);
		return;
	}

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		env->DeleteLocalRef(player);
		return;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	env->DeleteLocalRef(player);
	if (!inventory) return;

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		return;
	}

	jfieldID selected_slot_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_selected_slot_name, sdk::mappings::inventory_selected_slot_sig);
	if (selected_slot_fid)
	{
		env->SetIntField(inventory, selected_slot_fid, slot);
	}

	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
}

int enhance::modules::mace::get_current_slot(jobject player)
{
	auto env = enhance::instance->get_env();
	if (!env || !player) return -1;

	jclass player_class = env->GetObjectClass(player);
	if (!player_class) return -1;

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		return -1;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	if (!inventory) return -1;

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		return -1;
	}

	jfieldID selected_slot_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_selected_slot_name, sdk::mappings::inventory_selected_slot_sig);
	if (!selected_slot_fid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		return -1;
	}

	int slot = env->GetIntField(inventory, selected_slot_fid);
	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);

	return slot;
}


void enhance::modules::mace::aim_at_entity(jobject player, jobject target_entity)
{
	if (!player || !target_entity) return;

	sdk::entity_client player_entity(player);
	sdk::entity_client target_entity_client(target_entity);

	double px = player_entity.get_x();
	double py = player_entity.get_y() + 1.62;
	double pz = player_entity.get_z();

	double ex = target_entity_client.get_x();
	double ey = target_entity_client.get_y() + 1.6;
	double ez = target_entity_client.get_z();

	double dx = ex - px;
	double dy = ey - py;
	double dz = ez - pz;

	double horizontal_distance = sqrt(dx * dx + dz * dz);
	double yaw = atan2(dz, dx) * 180.0 / 3.14159265358979323846 - 90.0;
	double pitch = -atan2(dy, horizontal_distance) * 180.0 / 3.14159265358979323846;

	player_entity.set_yaw(static_cast<float>(yaw));
	player_entity.set_pitch(static_cast<float>(pitch));
}

bool enhance::modules::mace::has_elytra(jobject player)
{
	auto env = enhance::instance->get_env();
	if (!env || !player) return false;

	jclass player_class = env->GetObjectClass(player);
	if (!player_class) return false;

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		return false;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	if (!inventory) return false;

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		return false;
	}

	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	if (!get_stack_mid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		return false;
	}

	jobject chestplate_stack = env->CallObjectMethod(inventory, get_stack_mid, 38);
	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
	
	if (!chestplate_stack) return false;

	bool has_elytra_item = is_item(chestplate_stack, "elytra");
	env->DeleteLocalRef(chestplate_stack);
	
	return has_elytra_item;
}

void enhance::modules::mace::swap_elytra_to_chestplate(jobject player)
{
	auto env = enhance::instance->get_env();
	if (!env || !player) return;

	jclass player_class = env->GetObjectClass(player);
	if (!player_class) return;

	jfieldID inventory_fid = env->GetFieldID(player_class, sdk::mappings::player_inventory_name, sdk::mappings::player_inventory_sig);
	if (!inventory_fid)
	{
		env->DeleteLocalRef(player_class);
		return;
	}

	jobject inventory = env->GetObjectField(player, inventory_fid);
	env->DeleteLocalRef(player_class);
	if (!inventory) return;

	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class)
	{
		env->DeleteLocalRef(inventory);
		return;
	}

	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	if (!get_stack_mid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		return;
	}

	int chestplate_slot = -1;
	for (int slot = 0; slot < 9; slot++)
	{
		jobject stack = env->CallObjectMethod(inventory, get_stack_mid, slot);
		if (stack)
		{
			if (is_item(stack, "chestplate"))
			{
				chestplate_slot = slot;
				env->DeleteLocalRef(stack);
				break;
			}
			env->DeleteLocalRef(stack);
		}
	}

	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);

	if (chestplate_slot != -1)
	{
		swap_to_slot(chestplate_slot);
		Sleep(50);
		use_item();
		Sleep(50);
	}
}

void enhance::modules::mace::use_item()
{
	HWND window = Hook::get_window();
	if (!window) return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window) return;

	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	
	PostMessageA(window, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	PostMessageA(window, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

static void send_attack()
{
	HWND window = Hook::get_window();
	if (!window) return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window) return;

	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	PostMessageA(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	PostMessageA(window, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

static bool is_entity_hit_result(jobject hit_result)
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

static jobject get_entity_from_hit_result(jobject hit_result)
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

void enhance::modules::mace::run()
{
	if (!globals::mace_enabled)
	{
		if (globals::mace_saved_slot != -1 && globals::mace_switch_back)
		{
			swap_to_slot(globals::mace_saved_slot);
			globals::mace_saved_slot = -1;
		}
		globals::mace_elytra_swapped_this_fall = false;
		globals::mace_was_in_fall_distance = false;
		return;
	}

	if (globals::show_gui) return;

	HWND window = Hook::get_window();
	if (!window) return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window) return;

	jobject local_player = sdk::instance->get_player();
	if (!local_player) return;

	sdk::entity_client local_entity(local_player);
	double fall_distance = local_entity.get_fall_distance();
	bool is_falling = !local_entity.is_on_ground();

	if (!is_falling)
	{
		if (globals::mace_saved_slot != -1 && globals::mace_switch_back)
			swap_to_slot(globals::mace_saved_slot);
		globals::mace_saved_slot = -1;
		globals::mace_last_attack = 0;
		globals::mace_elytra_swapped_this_fall = false;
		globals::mace_was_in_fall_distance = false;
		auto env = enhance::instance->get_env();
		if (env) env->DeleteLocalRef(local_player);
		return;
	}

	bool in_fall_distance = (fall_distance >= globals::mace_min_fall_distance);
	if (!in_fall_distance)
	{
		if (globals::mace_was_in_fall_distance && globals::mace_saved_slot != -1 && globals::mace_switch_back)
		{
			ULONGLONG current_time = GetTickCount64();
			if (current_time - globals::mace_last_attack >= 500)
			{
				swap_to_slot(globals::mace_saved_slot);
				globals::mace_saved_slot = -1;
			}
		}
		globals::mace_was_in_fall_distance = false;
		globals::mace_elytra_swapped_this_fall = false;
		auto env = enhance::instance->get_env();
		if (env) env->DeleteLocalRef(local_player);
		return;
	}
	globals::mace_was_in_fall_distance = true;

	ULONGLONG current_time = GetTickCount64();
	jobject world = sdk::instance->get_world();
	if (!world)
	{
		auto env = enhance::instance->get_env();
		if (env) env->DeleteLocalRef(local_player);
		return;
	}

	sdk::entity_client local_entity_pos(local_player);
	double player_x = local_entity_pos.get_x();
	double player_y = local_entity_pos.get_y();
	double player_z = local_entity_pos.get_z();

	sdk::world_client world_client(world);
	std::vector<jobject> players = world_client.get_players();
	jobject target_entity = nullptr;
	double closest_distance = 100.0;
	
	double MAX_HORIZONTAL = globals::mace_fall_hitbox_width * 20.0;
	double MAX_VERTICAL = globals::mace_fall_hitbox_height * 20.0;
	if (MAX_HORIZONTAL < 1.0) MAX_HORIZONTAL = 6.0;
	if (MAX_VERTICAL < 1.0) MAX_VERTICAL = 15.0;

	for (jobject player : players)
	{
		if (!player) continue;
		sdk::entity_client entity_client(player);
		if (entity_client.is_same_object(local_player)) continue;

		double ex = entity_client.get_x();
		double ey = entity_client.get_y();
		double ez = entity_client.get_z();
		double horizontal = sqrt((ex - player_x) * (ex - player_x) + (ez - player_z) * (ez - player_z));
		if (horizontal > MAX_HORIZONTAL) continue;
		double vertical = fabs(player_y - ey);
		if (vertical > MAX_VERTICAL) continue;

		double dx = ex - player_x, dy = ey - player_y, dz = ez - player_z;
		double distance = sqrt(dx * dx + dy * dy + dz * dz);
		if (distance < closest_distance)
		{
			closest_distance = distance;
			target_entity = player;
		}
	}

	if (!target_entity)
	{
		globals::mace_elytra_swapped_this_fall = false;
		for (jobject player : players)
		{
			auto env = enhance::instance->get_env();
			if (env && player) env->DeleteLocalRef(player);
		}
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(world);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	sdk::entity_client target_entity_client(target_entity);
	double height_above = player_y - target_entity_client.get_y();
	if (height_above < globals::mace_height_above_target)
	{
		globals::mace_elytra_swapped_this_fall = false;
		for (jobject player : players)
		{
			auto env = enhance::instance->get_env();
			if (env && player) env->DeleteLocalRef(player);
		}
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(world);
			env->DeleteLocalRef(local_player);
			env->DeleteLocalRef(target_entity);
		}
		return;
	}

	if (globals::mace_saved_slot == -1)
		globals::mace_saved_slot = get_current_slot(local_player);

	if (globals::mace_remove_elytra && !globals::mace_elytra_swapped_this_fall && has_elytra(local_player))
	{
		swap_elytra_to_chestplate(local_player);
		globals::mace_elytra_swapped_this_fall = true;
	}

	int mace_slot = find_item_in_hotbar(local_player, "mace");
	if (mace_slot == -1)
	{
		for (jobject player : players)
		{
			auto env = enhance::instance->get_env();
			if (env && player) env->DeleteLocalRef(player);
		}
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(world);
			env->DeleteLocalRef(local_player);
			env->DeleteLocalRef(target_entity);
		}
		return;
	}

	if (get_current_slot(local_player) != mace_slot)
	{
		swap_to_slot(mace_slot);
		Sleep(50);
	}

	if (globals::mace_look)
		aim_at_entity(local_player, target_entity);

	// Check if we're close enough to attack using the target we already found
	sdk::entity_client local_entity_check(local_player);
	double px_check = local_entity_check.get_x();
	double py_check = local_entity_check.get_y();
	double pz_check = local_entity_check.get_z();

	double ex_check = target_entity_client.get_x();
	double ey_check = target_entity_client.get_y();
	double ez_check = target_entity_client.get_z();
	double dx_check = ex_check - px_check, dy_check = ey_check - py_check, dz_check = ez_check - pz_check;
	double distance = sqrt(dx_check * dx_check + dy_check * dy_check + dz_check * dz_check);

	// Attack if within 5.5 blocks (increased for earlier hits) and cooldown allows
	// Allow attacking even with slight cooldown for faster response
	bool can_attack = (sdk::instance->get_attack_cooldown() <= 0.1f) && 
	                  (distance <= 5.5) && 
	                  (current_time - globals::mace_last_attack >= 100);

	if (can_attack)
	{
		send_attack();
		globals::mace_last_attack = current_time;
		
		if (globals::mace_switch_back && globals::mace_saved_slot != -1)
		{
			Sleep(50);
			swap_to_slot(globals::mace_saved_slot);
		}
	}

	for (jobject player : players)
	{
		auto env = enhance::instance->get_env();
		if (env && player) env->DeleteLocalRef(player);
	}
	auto env = enhance::instance->get_env();
	if (env)
	{
		env->DeleteLocalRef(world);
		env->DeleteLocalRef(target_entity);
		env->DeleteLocalRef(local_player);
	}
}
