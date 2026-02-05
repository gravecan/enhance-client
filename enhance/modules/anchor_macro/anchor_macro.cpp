#include "anchor_macro.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <cstring>
#include <windows.h>

int enhance::modules::anchor_macro::find_item_in_hotbar(jobject player, const char* item_name)
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

bool enhance::modules::anchor_macro::is_item(jobject item_stack, const char* item_name)
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

void enhance::modules::anchor_macro::swap_to_slot(int slot)
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

int enhance::modules::anchor_macro::get_current_slot(jobject player)
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

bool enhance::modules::anchor_macro::is_holding_anchor()
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

	jfieldID selected_slot_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_selected_slot_name, sdk::mappings::inventory_selected_slot_sig);
	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	
	if (!selected_slot_fid || !get_stack_mid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		env->DeleteLocalRef(player);
		return false;
	}

	int current_slot = env->GetIntField(inventory, selected_slot_fid);
	jobject stack = env->CallObjectMethod(inventory, get_stack_mid, current_slot);
	
	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
	env->DeleteLocalRef(player);
	
	if (!stack) return false;

	bool is_anchor = is_item(stack, "respawn_anchor");
	env->DeleteLocalRef(stack);

	return is_anchor;
}

void enhance::modules::anchor_macro::send_right_click()
{
	INPUT inputs[2] = {};
	
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	
	SendInput(1, &inputs[0], sizeof(INPUT));
	Sleep(20);
	SendInput(1, &inputs[1], sizeof(INPUT));
}

void enhance::modules::anchor_macro::send_left_click()
{
	INPUT inputs[2] = {};
	
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	
	SendInput(1, &inputs[0], sizeof(INPUT));
	Sleep(20);
	SendInput(1, &inputs[1], sizeof(INPUT));
}

static bool last_key_state = false;

static bool is_keybind_active()
{
	// If module not enabled, not active
	if (!globals::anchor_macro_enabled)
	{
		return false;
	}
	
	// If no keybind set, always active when enabled
	if (globals::anchor_macro_keybind == 0)
	{
		return true;
	}
	
	bool key_now = (GetAsyncKeyState(globals::anchor_macro_keybind) & 0x8000) != 0;
	bool key_pressed = key_now && !last_key_state;
	last_key_state = key_now;
	
	// Mode 0 = Hold: active while key is held
	if (globals::anchor_macro_mode == 0)
	{
		return key_now;
	}
	
	// Mode 1 = Toggle: toggle on key press
	if (globals::anchor_macro_mode == 1)
	{
		if (key_pressed)
		{
			globals::anchor_macro_toggled = !globals::anchor_macro_toggled;
		}
		return globals::anchor_macro_toggled;
	}
	
	// Mode 2 = Always: always active if enabled (and keybind is set)
	if (globals::anchor_macro_mode == 2)
	{
		return true;
	}
	
	return false;
}

void enhance::modules::anchor_macro::run()
{
	if (!globals::anchor_macro_enabled)
	{
		globals::anchor_macro_executing = false;
		globals::anchor_macro_step = 0;
		globals::anchor_macro_saved_slot = -1;
		globals::anchor_macro_toggled = false;
		last_key_state = false;
		return;
	}

	if (globals::show_gui)
	{
		last_key_state = false;
		return;
	}

	HWND window = Hook::get_window();
	if (!window) return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window)
	{
		globals::anchor_macro_executing = false;
		globals::anchor_macro_step = 0;
		last_key_state = false;
		return;
	}

	auto env = enhance::instance->get_env();
	if (!env) return;

	bool keybind_active = is_keybind_active();
	
	if (!globals::anchor_macro_executing)
	{
		if (keybind_active && globals::anchor_macro_keybind != 0)
		{
			jobject player = sdk::instance->get_player();
			if (player)
			{
				int anchor_slot = find_item_in_hotbar(player, "respawn_anchor");
				int glowstone_slot = find_item_in_hotbar(player, "glowstone");
				
				if (anchor_slot == -1 || glowstone_slot == -1)
				{
					env->DeleteLocalRef(player);
					globals::anchor_macro_toggled = false;
					return;
				}
				
				globals::anchor_macro_saved_slot = get_current_slot(player);
				env->DeleteLocalRef(player);
			}
			
			globals::anchor_macro_executing = true;
			globals::anchor_macro_step = 0;
			globals::anchor_macro_last_action = GetTickCount64();
		}
		return;
	}

	jobject player = sdk::instance->get_player();
	if (!player)
	{
		globals::anchor_macro_executing = false;
		globals::anchor_macro_step = 0;
		globals::anchor_macro_toggled = false;
		return;
	}

	ULONGLONG current_time = GetTickCount64();
	ULONGLONG time_since_last = current_time - globals::anchor_macro_last_action;

	switch (globals::anchor_macro_step)
	{
		case 0:
		{
			int anchor_slot = find_item_in_hotbar(player, "respawn_anchor");
			if (anchor_slot == -1)
			{
				globals::anchor_macro_executing = false;
				globals::anchor_macro_step = 0;
				globals::anchor_macro_toggled = false;
				env->DeleteLocalRef(player);
				return;
			}
			swap_to_slot(anchor_slot);
			globals::anchor_macro_step = 1;
			globals::anchor_macro_last_action = current_time;
			break;
		}
		case 1:
		{
			if (time_since_last < (ULONGLONG)globals::anchor_macro_swap_delay_ms)
			{
				env->DeleteLocalRef(player);
				return;
			}
			send_right_click();
			globals::anchor_macro_step = 2;
			globals::anchor_macro_last_action = current_time;
			break;
		}
		case 2:
		{
			if (time_since_last < (ULONGLONG)globals::anchor_macro_charge_delay_ms)
			{
				env->DeleteLocalRef(player);
				return;
			}
			int glowstone_slot = find_item_in_hotbar(player, "glowstone");
			if (glowstone_slot == -1)
			{
				globals::anchor_macro_executing = false;
				globals::anchor_macro_step = 0;
				globals::anchor_macro_toggled = false;
				env->DeleteLocalRef(player);
				return;
			}
			swap_to_slot(glowstone_slot);
			globals::anchor_macro_step = 3;
			globals::anchor_macro_last_action = current_time;
			break;
		}
		case 3:
		{
			if (time_since_last < (ULONGLONG)globals::anchor_macro_swap_delay_ms)
			{
				env->DeleteLocalRef(player);
				return;
			}
			send_right_click();
			globals::anchor_macro_step = 4;
			globals::anchor_macro_last_action = current_time;
			break;
		}
		case 4:
		{
			if (time_since_last < (ULONGLONG)globals::anchor_macro_swap_delay_ms)
			{
				env->DeleteLocalRef(player);
				return;
			}
			
			if (!globals::anchor_macro_break_anchor)
			{
				globals::anchor_macro_step = 6;
				globals::anchor_macro_last_action = current_time;
				break;
			}
			
			if (globals::anchor_macro_saved_slot != -1)
			{
				swap_to_slot(globals::anchor_macro_saved_slot);
			}
			else
			{
				int weapon_slot = find_item_in_hotbar(player, "sword");
				if (weapon_slot == -1) weapon_slot = find_item_in_hotbar(player, "axe");
				if (weapon_slot == -1) weapon_slot = find_item_in_hotbar(player, "pickaxe");
				if (weapon_slot != -1) swap_to_slot(weapon_slot);
			}
			globals::anchor_macro_step = 5;
			globals::anchor_macro_last_action = current_time;
			break;
		}
		case 5:
		{
			if (time_since_last < (ULONGLONG)globals::anchor_macro_break_delay_ms)
			{
				env->DeleteLocalRef(player);
				return;
			}
			send_right_click();
			globals::anchor_macro_step = 6;
			globals::anchor_macro_last_action = current_time;
			break;
		}
		case 6:
		{
			if (time_since_last < 100)
			{
				env->DeleteLocalRef(player);
				return;
			}
			globals::anchor_macro_executing = false;
			globals::anchor_macro_step = 0;
			globals::anchor_macro_saved_slot = -1;
			globals::anchor_macro_toggled = false;
			break;
		}
	}

	env->DeleteLocalRef(player);
}
