#include "pearl_catch.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstring>
#include <windows.h>

static int state = 0;
static ULONGLONG state_start_time = 0;
static float saved_yaw = 0.0f;
static float saved_pitch = 0.0f;
static bool last_key_state = false;
static bool pearl_catch_toggled = false;

static bool is_keybind_active()
{
	// If no keybind set, not active (requires keybind to be set)
	if (globals::pearl_catch_keybind == 0)
	{
		return false;
	}
	
	bool key_now = (GetAsyncKeyState(globals::pearl_catch_keybind) & 0x8000) != 0;
	bool key_pressed = key_now && !last_key_state;
	last_key_state = key_now;
	
	// Mode 0 = Hold: active while key is held
	if (globals::pearl_catch_mode == 0)
	{
		return key_now;
	}
	
	// Mode 1 = Toggle: toggle on key press
	if (globals::pearl_catch_mode == 1)
	{
		if (key_pressed)
		{
			pearl_catch_toggled = !pearl_catch_toggled;
		}
		return pearl_catch_toggled;
	}
	
	// Mode 2 = Always: always active if keybind is set (but sequence only starts on key press)
	if (globals::pearl_catch_mode == 2)
	{
		return true;
	}
	
	return false;
}

static const ULONGLONG SWAP_DELAY = 50;
static const ULONGLONG AFTER_USE_DELAY = 100;

static void get_rotation(jobject player, float& yaw, float& pitch)
{
	sdk::entity_client entity(player);
	yaw = entity.get_yaw();
	pitch = entity.get_pitch();
}

static void set_rotation(jobject player, float yaw, float pitch)
{
	sdk::entity_client entity(player);
	entity.set_yaw(yaw);
	entity.set_pitch(pitch);
}

static bool use_item_silent(jobject player, float target_pitch)
{
	auto env = enhance::instance->get_env();
	if (!env || !player) return false;

	float s_yaw, s_pitch;
	get_rotation(player, s_yaw, s_pitch);

	jclass mc_class = sdk::classloader::find_class(env, sdk::mappings::minecraftclass_sig);
	if (!mc_class) return false;

	jfieldID instance_fid = env->GetStaticFieldID(mc_class, sdk::mappings::minecraftclient_name, sdk::mappings::minecraftclient_sig);
	if (!instance_fid)
	{
		env->DeleteLocalRef(mc_class);
		return false;
	}

	jobject mc_instance = env->GetStaticObjectField(mc_class, instance_fid);
	if (!mc_instance)
	{
		env->DeleteLocalRef(mc_class);
		return false;
	}

	jfieldID interaction_manager_fid = env->GetFieldID(mc_class, sdk::mappings::interaction_manager_name, sdk::mappings::interaction_manager_sig);
	env->DeleteLocalRef(mc_class);
	if (!interaction_manager_fid)
	{
		env->DeleteLocalRef(mc_instance);
		return false;
	}

	jobject interaction_manager = env->GetObjectField(mc_instance, interaction_manager_fid);
	env->DeleteLocalRef(mc_instance);
	if (!interaction_manager) return false;

	jclass hand_class = sdk::classloader::find_class(env, sdk::mappings::hand_class_sig);
	if (!hand_class)
	{
		env->DeleteLocalRef(interaction_manager);
		return false;
	}

	jfieldID main_hand_fid = env->GetStaticFieldID(hand_class, sdk::mappings::hand_main_hand_name, sdk::mappings::hand_main_hand_sig);
	if (!main_hand_fid)
	{
		env->DeleteLocalRef(hand_class);
		env->DeleteLocalRef(interaction_manager);
		return false;
	}

	jobject main_hand = env->GetStaticObjectField(hand_class, main_hand_fid);
	env->DeleteLocalRef(hand_class);
	if (!main_hand)
	{
		env->DeleteLocalRef(interaction_manager);
		return false;
	}

	jclass interaction_manager_class = env->GetObjectClass(interaction_manager);
	if (!interaction_manager_class)
	{
		env->DeleteLocalRef(main_hand);
		env->DeleteLocalRef(interaction_manager);
		return false;
	}

	jmethodID interact_item_mid = env->GetMethodID(interaction_manager_class, sdk::mappings::interact_item_name, sdk::mappings::interact_item_sig);
	env->DeleteLocalRef(interaction_manager_class);
	if (!interact_item_mid)
	{
		env->DeleteLocalRef(main_hand);
		env->DeleteLocalRef(interaction_manager);
		return false;
	}

	set_rotation(player, s_yaw, target_pitch);
	env->CallObjectMethod(interaction_manager, interact_item_mid, player, main_hand);
	set_rotation(player, s_yaw, s_pitch);

	env->DeleteLocalRef(main_hand);
	env->DeleteLocalRef(interaction_manager);

	return true;
}

static void send_right_click()
{
	INPUT input = {};
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1, &input, sizeof(INPUT));
	Sleep(10);
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1, &input, sizeof(INPUT));
}

int enhance::modules::pearl_catch::find_item_in_hotbar(jobject player, const char* item_name)
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
		jobject item_stack = env->CallObjectMethod(inventory, get_stack_mid, slot);
		if (!item_stack) continue;

		if (is_item(item_stack, item_name))
		{
			env->DeleteLocalRef(item_stack);
			env->DeleteLocalRef(inventory_class);
			env->DeleteLocalRef(inventory);
			return slot;
		}

		env->DeleteLocalRef(item_stack);
	}

	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
	return -1;
}

bool enhance::modules::pearl_catch::is_item(jobject item_stack, const char* item_name)
{
	if (!item_stack) return false;

	auto env = enhance::instance->get_env();
	if (!env) return false;

	jclass itemstack_class_temp = env->GetObjectClass(item_stack);
	if (!itemstack_class_temp) return false;
	
	jmethodID is_empty_mid = env->GetMethodID(itemstack_class_temp, sdk::mappings::itemstack_is_empty_name, sdk::mappings::itemstack_is_empty_sig);
	env->DeleteLocalRef(itemstack_class_temp);
	
	if (is_empty_mid)
	{
		jboolean is_empty = env->CallBooleanMethod(item_stack, is_empty_mid);
		if (is_empty == JNI_TRUE)
			return false;
	}

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
	if (!translation_key)
	{
		env->DeleteLocalRef(item_class);
		env->DeleteLocalRef(item);
		return false;
	}

	const char* key_str = env->GetStringUTFChars(translation_key, nullptr);
	bool is_match = (strstr(key_str, item_name) != nullptr);
	env->ReleaseStringUTFChars(translation_key, key_str);

	env->DeleteLocalRef(translation_key);
	env->DeleteLocalRef(item_class);
	env->DeleteLocalRef(item);

	return is_match;
}

void enhance::modules::pearl_catch::swap_to_slot(int slot)
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

void enhance::modules::pearl_catch::run()
{
	// Only start sequence on key press when state is 0
	if (state == 0)
	{
		// Require keybind to be set
		if (globals::pearl_catch_keybind == 0)
		{
			return;
		}
		
		bool key_now = (GetAsyncKeyState(globals::pearl_catch_keybind) & 0x8000) != 0;
		bool key_pressed = key_now && !last_key_state;
		last_key_state = key_now;
		
		// Check if we should start based on mode
		bool should_start = false;
		
		if (globals::pearl_catch_mode == 0)
		{
			// Hold mode: start on key press
			should_start = key_pressed;
		}
		else if (globals::pearl_catch_mode == 1)
		{
			// Toggle mode: toggle on key press, start sequence when toggled on
			if (key_pressed)
			{
				pearl_catch_toggled = !pearl_catch_toggled;
				// Start sequence when toggled ON
				should_start = pearl_catch_toggled;
			}
		}
		else if (globals::pearl_catch_mode == 2)
		{
			// Always mode: start on key press (sequence runs once per press)
			should_start = key_pressed;
		}
		
		if (!should_start)
		{
			return;
		}
		
		// Start the sequence
		jobject player = sdk::instance->get_player();
		if (player)
		{
			get_rotation(player, saved_yaw, saved_pitch);
			enhance::instance->get_env()->DeleteLocalRef(player);
		}
		state = 1;
		state_start_time = GetTickCount64();
		return;
	}
	
	// If sequence is running, continue it (don't check keybind again)

	jobject player = sdk::instance->get_player();
	if (!player)
	{
		state = 0;
		return;
	}

	auto env = enhance::instance->get_env();
	if (!env)
	{
		state = 0;
		return;
	}

	ULONGLONG now = GetTickCount64();
	ULONGLONG elapsed = now - state_start_time;

	bool silent_mode = (globals::pearl_catch_aim_mode == 0);

	switch (state)
	{
		case 1:
		{
			int pearl_slot = find_item_in_hotbar(player, "ender_pearl");
			if (pearl_slot == -1)
			{
				state = 4;
				state_start_time = now;
				break;
			}
			
			swap_to_slot(pearl_slot);
			state = 2;
			state_start_time = now;
			break;
		}
		
		case 2:
		{
			if (elapsed < SWAP_DELAY) 
			{
				if (!silent_mode) set_rotation(player, saved_yaw, -90.0f);
				break;
			}
			
			if (silent_mode)
			{
				use_item_silent(player, -90.0f);
			}
			else
			{
				set_rotation(player, saved_yaw, -90.0f);
				send_right_click();
			}
			
			state = 3;
			state_start_time = now;
			break;
		}
		
		case 3:
		{
			if (!silent_mode) set_rotation(player, saved_yaw, -90.0f);
			if (elapsed < AFTER_USE_DELAY) break;
			
			state = 4;
			state_start_time = now;
			break;
			}
			
		case 4:
		{
			int windcharge_slot = find_item_in_hotbar(player, "wind_charge");
			if (windcharge_slot == -1)
			{
				if (!silent_mode) set_rotation(player, saved_yaw, saved_pitch);
				state = 0;
				break;
			}
			
			swap_to_slot(windcharge_slot);
			state = 5;
			state_start_time = now;
			break;
		}
		
		case 5:
		{
			if (elapsed < SWAP_DELAY) 
			{
				if (!silent_mode) set_rotation(player, saved_yaw, -90.0f);
				break;
			}
			
			if (silent_mode)
			{
				use_item_silent(player, -90.0f);
			}
			else
			{
				set_rotation(player, saved_yaw, -90.0f);
				send_right_click();
			}
			
			state = 6;
			state_start_time = now;
			break;
		}
		
		case 6:
		{
			if (!silent_mode) set_rotation(player, saved_yaw, -90.0f);
			if (elapsed < AFTER_USE_DELAY) break;
			
			if (!silent_mode) set_rotation(player, saved_yaw, saved_pitch);
			state = 0;
			break;
		}
	}

	env->DeleteLocalRef(player);
}
