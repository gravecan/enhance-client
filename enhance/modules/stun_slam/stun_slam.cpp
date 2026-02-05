#include "stun_slam.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstring>
#include <cmath>
#include <random>
#include <windows.h>

static std::random_device rd;
static std::mt19937 gen(rd());
static ULONGLONG last_stun_slam_time = 0;
static bool stun_slam_executing = false;
static int stun_slam_step = 0;
static ULONGLONG stun_slam_last_action = 0;
static int stun_slam_saved_slot = -1;
static bool stun_slam_target_blocking = false;

int enhance::modules::stun_slam::find_item_in_hotbar(jobject player, const char* item_name)
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

bool enhance::modules::stun_slam::is_item(jobject item_stack, const char* item_name)
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

void enhance::modules::stun_slam::swap_to_slot(int slot)
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

int enhance::modules::stun_slam::get_current_slot(jobject player)
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

bool enhance::modules::stun_slam::is_using_shield(jobject target_entity)
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
	env->DeleteLocalRef(living_entity_class);
	
	if (!is_blocking_mid) return false;
	
	jboolean blocking = env->CallBooleanMethod(target_entity, is_blocking_mid);
	return blocking == JNI_TRUE;
}

void enhance::modules::stun_slam::send_attack()
{
	HWND window = Hook::get_window();
	if (!window) return;
	if (GetForegroundWindow() != window) return;

	INPUT inputs[2] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(2, inputs, sizeof(INPUT));
}

bool enhance::modules::stun_slam::should_activate()
{
	if (globals::stun_slam_chance >= 100.0f) return true;
	if (globals::stun_slam_chance <= 0.0f) return false;
	
	std::uniform_real_distribution<float> dis(0.0f, 100.0f);
	return dis(gen) < globals::stun_slam_chance;
}

static bool is_entity_hit_result(jobject hit_result)
{
	if (!hit_result) return false;

	auto env = enhance::instance->get_env();
	if (!env) return false;

	// Try to get EntityHitResult class using classloader (works on both Fabric and vanilla)
	// Fabric intermediary: net.minecraft.class_1298
	// Vanilla obfuscated: foe (but we'll use IsInstanceOf for better compatibility)
	jclass entity_hit_result_class = sdk::classloader::find_class(env, "net/minecraft/class_1298");
	if (!entity_hit_result_class)
	{
		// Fallback: try vanilla obfuscated name
		entity_hit_result_class = env->FindClass("foe");
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	if (!entity_hit_result_class)
	{
		// Last resort: check class name
		jclass hit_result_class = env->GetObjectClass(hit_result);
		if (!hit_result_class) return false;

		jclass class_class = env->GetObjectClass(hit_result_class);
		jmethodID get_name_method = env->GetMethodID(class_class, "getName", "()Ljava/lang/String;");
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (!get_name_method)
		{
			env->DeleteLocalRef(class_class);
			env->DeleteLocalRef(hit_result_class);
			return false;
		}

		jstring class_name = (jstring)env->CallObjectMethod(hit_result_class, get_name_method);
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (!class_name)
		{
			env->DeleteLocalRef(class_class);
			env->DeleteLocalRef(hit_result_class);
			return false;
		}

		const char* name_str = env->GetStringUTFChars(class_name, nullptr);
		bool is_entity = (strcmp(name_str, "foe") == 0 || strstr(name_str, "class_1298") != nullptr || strstr(name_str, "EntityHitResult") != nullptr);
		env->ReleaseStringUTFChars(class_name, name_str);

		env->DeleteLocalRef(class_name);
		env->DeleteLocalRef(class_class);
		env->DeleteLocalRef(hit_result_class);

		return is_entity;
	}

	// Use IsInstanceOf for reliable checking
	jboolean is_entity = env->IsInstanceOf(hit_result, entity_hit_result_class);
	env->DeleteLocalRef(entity_hit_result_class);
	
	return is_entity == JNI_TRUE;
}

static jobject get_entity_from_hit_result(jobject hit_result)
{
	if (!hit_result) return nullptr;

	auto env = enhance::instance->get_env();
	if (!env) return nullptr;

	jclass hit_result_class = env->GetObjectClass(hit_result);
	if (!hit_result_class) return nullptr;

	// Try vanilla obfuscated names first (for vanilla compatibility)
	jmethodID get_entity_mid = env->GetMethodID(hit_result_class, "a", "()Lcdv;");
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	jfieldID entity_fid = nullptr;
	if (!get_entity_mid)
	{
		entity_fid = env->GetFieldID(hit_result_class, "b", "Lcdv;");
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	// If vanilla names failed, try Fabric intermediary names
	if (!get_entity_mid && !entity_fid)
	{
		const char* entity_sig = "Lnet/minecraft/class_1297;"; // Entity class on Fabric
		
		// Try common Fabric intermediary method patterns
		const char* method_names[] = {"method_375", "method_376", "getEntity", "entity"};
		for (int i = 0; i < 4 && !get_entity_mid; i++)
		{
			get_entity_mid = env->GetMethodID(hit_result_class, method_names[i], entity_sig);
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		// Try common Fabric intermediary field patterns
		const char* field_names[] = {"field_63037", "field_63038", "entity", "field_entity"};
		for (int i = 0; i < 4 && !entity_fid; i++)
		{
			entity_fid = env->GetFieldID(hit_result_class, field_names[i], entity_sig);
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
	}
	
	if (!get_entity_mid && !entity_fid)
	{
		env->DeleteLocalRef(hit_result_class);
		return nullptr;
	}
	
	jobject entity = nullptr;
	if (get_entity_mid)
	{
		entity = env->CallObjectMethod(hit_result, get_entity_mid);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	else if (entity_fid)
	{
		entity = env->GetObjectField(hit_result, entity_fid);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	env->DeleteLocalRef(hit_result_class);
	return entity;
}

void enhance::modules::stun_slam::run()
{
	if (!globals::stun_slam_enabled)
	{
		if (stun_slam_saved_slot != -1)
		{
			swap_to_slot(stun_slam_saved_slot);
			stun_slam_saved_slot = -1;
		}
		stun_slam_executing = false;
		stun_slam_step = 0;
		return;
	}

	if (globals::show_gui) return;

	auto env = enhance::instance->get_env();
	if (!env) return;

	ULONGLONG current_time = GetTickCount64();

	if (stun_slam_executing)
	{
		jobject player = sdk::instance->get_player();
		if (!player)
		{
			stun_slam_executing = false;
			stun_slam_step = 0;
			return;
		}

		ULONGLONG time_since_last = current_time - stun_slam_last_action;

		switch (stun_slam_step)
		{
			case 1: // Axe attack
			{
				// Reduced delay for faster execution - minimum 5ms
				ULONGLONG min_delay = (ULONGLONG)(globals::stun_slam_swap_delay_ms > 5 ? globals::stun_slam_swap_delay_ms : 5);
				if (time_since_last < min_delay)
				{
					env->DeleteLocalRef(player);
					return;
				}
				send_attack();
				stun_slam_step = 2;
				stun_slam_last_action = current_time;
				break;
			}
			case 2: // Swap to mace
			{
				// Reduced delay for faster execution - minimum 5ms
				ULONGLONG min_delay = (ULONGLONG)(globals::stun_slam_axe_delay_ms > 5 ? globals::stun_slam_axe_delay_ms : 5);
				if (time_since_last < min_delay)
				{
					env->DeleteLocalRef(player);
					return;
				}
				int mace_slot = find_item_in_hotbar(player, "mace");
				if (mace_slot != -1)
				{
					swap_to_slot(mace_slot);
					Sleep(50); // Delay for swap to register (same as mace/autocrystal)
				}
				stun_slam_step = 3;
				stun_slam_last_action = current_time;
				break;
			}
			case 3: // Mace attack
			{
				// Reduced delay for faster execution - minimum 5ms
				ULONGLONG min_delay = (ULONGLONG)(globals::stun_slam_swap_delay_ms > 5 ? globals::stun_slam_swap_delay_ms : 5);
				if (time_since_last < min_delay)
				{
					env->DeleteLocalRef(player);
					return;
				}
				send_attack();
				stun_slam_step = 4;
				stun_slam_last_action = current_time;
				break;
			}
			case 4: // Switch back and finish
			{
				// Reduced delay for faster execution - minimum 5ms
				ULONGLONG min_delay = (ULONGLONG)(globals::stun_slam_mace_delay_ms > 5 ? globals::stun_slam_mace_delay_ms : 5);
				if (time_since_last < min_delay)
				{
					env->DeleteLocalRef(player);
					return;
				}
				if (stun_slam_saved_slot != -1)
				{
					swap_to_slot(stun_slam_saved_slot);
					Sleep(50); // Delay for swap to register (same as mace/autocrystal)
					stun_slam_saved_slot = -1;
				}
				stun_slam_executing = false;
				stun_slam_step = 0;
				break;
			}
		}

		env->DeleteLocalRef(player);
		return;
	}

	// Cooldown between stun slam attempts
	if (current_time - last_stun_slam_time < 300) return;

	jobject local_player = sdk::instance->get_player();
	if (!local_player) return;

	sdk::entity_client local_entity(local_player);
	double fall_distance = local_entity.get_fall_distance();
	bool is_falling = !local_entity.is_on_ground();

	if (!is_falling || fall_distance < globals::stun_slam_min_fall)
	{
		env->DeleteLocalRef(local_player);
		return;
	}

	int mace_slot = find_item_in_hotbar(local_player, "mace");
	if (mace_slot == -1)
	{
		env->DeleteLocalRef(local_player);
		return;
	}

	jobject crosshair_target = sdk::instance->get_crosshair_target();
	if (!crosshair_target || !is_entity_hit_result(crosshair_target))
	{
		if (crosshair_target) env->DeleteLocalRef(crosshair_target);
		env->DeleteLocalRef(local_player);
		return;
	}

	jobject target_entity = get_entity_from_hit_result(crosshair_target);
	env->DeleteLocalRef(crosshair_target);
	if (!target_entity)
	{
		env->DeleteLocalRef(local_player);
		return;
	}

	if (local_entity.is_same_object(target_entity))
	{
		env->DeleteLocalRef(target_entity);
		env->DeleteLocalRef(local_player);
		return;
	}

	if (!should_activate())
	{
		env->DeleteLocalRef(target_entity);
		env->DeleteLocalRef(local_player);
		return;
	}

	stun_slam_target_blocking = is_using_shield(target_entity);
	env->DeleteLocalRef(target_entity);

	stun_slam_saved_slot = get_current_slot(local_player);

	stun_slam_executing = true;
	last_stun_slam_time = current_time;

	// Execute immediately without waiting
	if (stun_slam_target_blocking)
	{
		int axe_slot = find_item_in_hotbar(local_player, "axe");
		if (axe_slot != -1)
		{
			swap_to_slot(axe_slot);
			Sleep(50); // Delay for swap to register (same as mace/autocrystal)
			stun_slam_step = 1;
			stun_slam_last_action = current_time;
		}
		else
		{
			swap_to_slot(mace_slot);
			Sleep(50); // Delay for swap to register (same as mace/autocrystal)
			stun_slam_step = 3;
			stun_slam_last_action = current_time;
		}
	}
	else
	{
		swap_to_slot(mace_slot);
		Sleep(50); // Delay for swap to register (same as mace/autocrystal)
		stun_slam_step = 3;
		stun_slam_last_action = current_time;
	}

	env->DeleteLocalRef(local_player);
}
