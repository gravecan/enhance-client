#include "autocrystal.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstring>
#include <windows.h>

static ULONGLONG last_action_time = 0;
static bool last_key_state = false;
static bool autocrystal_toggled = false;
static bool obsidian_placed = false;
static int crystal_slot_cached = -1;

int enhance::modules::autocrystal::find_item_in_inventory(jobject player, const char* item_name)
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
	
	// Search hotbar (slots 0-8) first
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

void enhance::modules::autocrystal::swap_to_slot(int slot)
{
	if (slot < 0 || slot > 8) return; // Only hotbar slots
	
	auto env = enhance::instance->get_env();
	if (!env) return;
	
	jobject player = sdk::instance->get_player();
	if (!player) return;
	
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

int enhance::modules::autocrystal::get_current_slot(jobject player)
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

bool enhance::modules::autocrystal::is_item(jobject item_stack, const char* item_name)
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

void enhance::modules::autocrystal::right_click()
{
	HWND window = Hook::get_window();
	if (!window) return;
	
	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	PostMessageA(window, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	PostMessageA(window, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

void enhance::modules::autocrystal::left_click()
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

static bool is_keybind_active()
{
	// If module not enabled, not active
	if (!globals::autocrystal_enabled)
	{
		return false;
	}
	
	// If no keybind set, always active when enabled
	if (globals::autocrystal_keybind == 0)
	{
		return true;
	}
	
	bool key_now = (GetAsyncKeyState(globals::autocrystal_keybind) & 0x8000) != 0;
	bool key_pressed = key_now && !last_key_state;
	last_key_state = key_now;
	
	// Mode 0 = Hold: active while key is held
	if (globals::autocrystal_mode == 0)
	{
		return key_now;
	}
	
	// Mode 1 = Toggle: toggle on key press
	if (globals::autocrystal_mode == 1)
	{
		if (key_pressed)
		{
			autocrystal_toggled = !autocrystal_toggled;
		}
		return autocrystal_toggled;
	}
	
	// Mode 2 = Always: always active if enabled
	if (globals::autocrystal_mode == 2)
	{
		return true;
	}
	
	return false;
}

void enhance::modules::autocrystal::run()
{
	if (!globals::autocrystal_enabled) 
	{
		obsidian_placed = false;
		last_action_time = 0;
		autocrystal_toggled = false;
		last_key_state = false;
		crystal_slot_cached = -1;
		return;
	}

	if (globals::show_gui) 
	{
		obsidian_placed = false;
		last_action_time = 0;
		last_key_state = false;
		crystal_slot_cached = -1;
		return;
	}

	HWND window = Hook::get_window();
	if (!window) 
	{
		obsidian_placed = false;
		last_action_time = 0;
		crystal_slot_cached = -1;
		return;
	}

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window) 
	{
		obsidian_placed = false;
		last_action_time = 0;
		crystal_slot_cached = -1;
		return;
	}

	// Check keybind based on mode (hold/toggle/always)
	bool keybind_active = is_keybind_active();
	if (!keybind_active)
	{
		obsidian_placed = false;
		last_action_time = 0;
		crystal_slot_cached = -1;
		return;
	}

	// Require holding right-click to run the macro
	bool right_down = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	if (!right_down)
	{
		obsidian_placed = false;
		last_action_time = 0;
		crystal_slot_cached = -1;
		return;
	}

	auto env = enhance::instance->get_env();
	if (!env) return;
	
	jobject player = sdk::instance->get_player();
	if (!player) return;

	ULONGLONG current_time = GetTickCount64();
	ULONGLONG delay_ms = static_cast<ULONGLONG>(globals::autocrystal_delay_ms);
	if (delay_ms < 50) delay_ms = 50; // Minimum delay of 50ms

	// Check if we need to wait before next action
	if (last_action_time > 0 && (current_time - last_action_time) < delay_ms)
	{
		env->DeleteLocalRef(player);
		return;
	}

	// Step 1: Place obsidian once (only at the start)
	if (!obsidian_placed)
	{
		int obsidian_slot = find_item_in_inventory(player, "obsidian");
		if (obsidian_slot >= 0 && obsidian_slot <= 8)
		{
			swap_to_slot(obsidian_slot);
			Sleep(100); // Small delay for slot swap
			right_click(); // Place obsidian
			obsidian_placed = true;
			last_action_time = current_time;
			env->DeleteLocalRef(player);
			return;
		}
		else
		{
			// No obsidian, skip to crystal spam
			obsidian_placed = true;
		}
	}

	// Step 2: Find and swap to crystal (only once or when needed)
	if (crystal_slot_cached == -1)
	{
		crystal_slot_cached = find_item_in_inventory(player, "end_crystal");
		if (crystal_slot_cached >= 0 && crystal_slot_cached <= 8)
		{
			swap_to_slot(crystal_slot_cached);
			Sleep(100); // Small delay for slot swap
			last_action_time = current_time;
			env->DeleteLocalRef(player);
			return;
		}
		else
		{
			// No crystal found
			env->DeleteLocalRef(player);
			return;
		}
	}

	// Step 3: Spam right and left clicks with crystal
	right_click(); // Place crystal
	Sleep(20);
	left_click();  // Break crystal
	last_action_time = current_time;

	env->DeleteLocalRef(player);
}
