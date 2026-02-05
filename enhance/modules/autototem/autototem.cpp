#include "autototem.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstring>
#include <string>

static ULONGLONG last_check_time = 0;
static bool inventory_open = false;

bool enhance::modules::autototem::has_totem_in_offhand(jobject player)
{
	if (!player) return false;
	
	auto env = enhance::instance->get_env();
	if (!env) return false;
	
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
	
	// Try using mappings first
	jobject offhand_stack = nullptr;
	
	if (sdk::mappings::inventory_offhand_name && sdk::mappings::inventory_offhand_sig)
	{
		jfieldID offhand_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_offhand_name, sdk::mappings::inventory_offhand_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (offhand_fid)
		{
			jobject offhand_list = env->GetObjectField(inventory, offhand_fid);
			if (offhand_list)
			{
				jclass list_class = env->GetObjectClass(offhand_list);
				if (list_class)
				{
					jmethodID get_mid = env->GetMethodID(list_class, "get", "(I)Ljava/lang/Object;");
					if (env->ExceptionCheck()) env->ExceptionClear();
					
					if (get_mid)
					{
						offhand_stack = env->CallObjectMethod(offhand_list, get_mid, 0);
						if (env->ExceptionCheck()) env->ExceptionClear();
					}
					env->DeleteLocalRef(list_class);
				}
				env->DeleteLocalRef(offhand_list);
			}
		}
	}
	
	// Fallback to slot 40
	if (!offhand_stack)
	{
		jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
		if (get_stack_mid)
		{
			offhand_stack = env->CallObjectMethod(inventory, get_stack_mid, 40);
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
	}
	
	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
	
	if (!offhand_stack) return false;
	
	// Check if it's empty
	jclass itemstack_class = env->GetObjectClass(offhand_stack);
	if (!itemstack_class)
	{
		env->DeleteLocalRef(offhand_stack);
		return false;
	}
	
	jmethodID is_empty_mid = env->GetMethodID(itemstack_class, sdk::mappings::itemstack_is_empty_name, sdk::mappings::itemstack_is_empty_sig);
	env->DeleteLocalRef(itemstack_class);
	
	if (!is_empty_mid)
	{
		env->DeleteLocalRef(offhand_stack);
		return false;
	}
	
	jboolean empty = env->CallBooleanMethod(offhand_stack, is_empty_mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (empty == JNI_TRUE)
	{
		env->DeleteLocalRef(offhand_stack);
		return false;
	}
	
	// Check if it's a totem
	bool is_totem = is_item(offhand_stack, "totem");
	env->DeleteLocalRef(offhand_stack);
	
	return is_totem;
}

int enhance::modules::autototem::find_totem_in_inventory(jobject player)
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
	
	// Search all inventory slots (0-39, excluding offhand slot 40 to avoid "ghost totem")
	for (int slot = 0; slot < 40; slot++)
	{
		// Skip offhand slot (40) - we don't want to find the totem we just placed
		if (slot == 40) continue;
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
					if (empty == JNI_FALSE && is_item(stack, "totem"))
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

bool enhance::modules::autototem::is_item(jobject item_stack, const char* item_name)
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

void enhance::modules::autototem::swap_to_offhand_blatant(int slot)
{
	if (slot < 0 || slot >= 40) return;
	
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
	
	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	jmethodID set_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_set_stack_name, sdk::mappings::inventory_set_stack_sig);
	
	if (!get_stack_mid || !set_stack_mid)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		return;
	}
	
	// Get totem stack from source slot
	jobject totem_stack = env->CallObjectMethod(inventory, get_stack_mid, slot);
	if (!totem_stack)
	{
		env->DeleteLocalRef(inventory_class);
		env->DeleteLocalRef(inventory);
		return;
	}
	
	// Get offhand stack (slot 40)
	jobject offhand_stack = env->CallObjectMethod(inventory, get_stack_mid, 40);
	
	// For hotbar items (0-8), use F key to swap (server-synced)
	// For main inventory items (9-39), use set_stack (client-side only - may appear as "ghost")
	if (slot < 9)
	{
		// Select the slot containing the totem
		jfieldID selected_slot_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_selected_slot_name, sdk::mappings::inventory_selected_slot_sig);
		if (selected_slot_fid)
		{
			env->SetIntField(inventory, selected_slot_fid, slot);
		}
		
		// Simulate F key press to swap to offhand (this actually syncs with server)
		HWND window = Hook::get_window();
		if (window)
		{
			INPUT input = {};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = 'F';
			input.ki.dwFlags = 0;
			SendInput(1, &input, sizeof(INPUT));
			Sleep(5);
			
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
		}
	}
	else
	{
		// For main inventory items, use set_stack (client-side only, may not sync perfectly)
		// This is why it might appear as "ghost totem" - it's client-side only
		// For proper server sync, use legit mode instead
		env->CallVoidMethod(inventory, set_stack_mid, 40, totem_stack);
		if (offhand_stack)
		{
			env->CallVoidMethod(inventory, set_stack_mid, slot, offhand_stack);
		}
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	env->DeleteLocalRef(totem_stack);
	if (offhand_stack) env->DeleteLocalRef(offhand_stack);
	env->DeleteLocalRef(inventory_class);
	env->DeleteLocalRef(inventory);
}

void enhance::modules::autototem::open_inventory()
{
	HWND window = Hook::get_window();
	if (!window) return;
	
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 'E';
	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));
	
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
	
	inventory_open = true;
	Sleep(100); // Wait for inventory to open
}

void enhance::modules::autototem::close_inventory()
{
	HWND window = Hook::get_window();
	if (!window) return;
	
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 'E';
	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));
	
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
	
	inventory_open = false;
	Sleep(50); // Wait for inventory to close
}

void enhance::modules::autototem::move_mouse_to_slot(int slot)
{
	// Calculate inventory slot position on screen
	// This is a simplified calculation - actual positions depend on screen resolution and GUI scale
	HWND window = Hook::get_window();
	if (!window) return;
	
	RECT window_rect;
	GetClientRect(window, &window_rect);
	
	// Approximate inventory slot positions (these would need calibration)
	// Main inventory: 9x3 grid starting around center-left of screen
	int inv_start_x = window_rect.right / 2 - 200;
	int inv_start_y = window_rect.bottom / 2 - 50;
	
	// Calculate slot position
	int row = slot / 9;
	int col = slot % 9;
	
	int slot_x = inv_start_x + col * 18; // 18 pixels per slot
	int slot_y = inv_start_y + row * 18;
	
	// Convert to screen coordinates
	POINT client_point = { slot_x, slot_y };
	ClientToScreen(window, &client_point);
	
	SetCursorPos(client_point.x, client_point.y);
}

void enhance::modules::autototem::click_slot(int slot)
{
	move_mouse_to_slot(slot);
	Sleep(10);
	
	HWND window = Hook::get_window();
	if (!window) return;
	
	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	
	PostMessageA(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	PostMessageA(window, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	
	Sleep(10);
	
	// Click offhand slot (slot 40, which is the offhand slot in inventory)
	move_mouse_to_slot(40);
	Sleep(10);
	
	GetCursorPos(&cursorPos);
	ScreenToClient(window, &cursorPos);
	
	PostMessageA(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	PostMessageA(window, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
}

jobject enhance::modules::autototem::get_block_pos_origin()
{
	auto env = enhance::instance->get_env();
	if (!env) return nullptr;
	
	// Get BlockPos.ORIGIN static field using mappings
	jclass block_pos_class = nullptr;
	if (sdk::mappings::block_pos_class_sig)
	{
		block_pos_class = sdk::classloader::find_class(env, sdk::mappings::block_pos_class_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	if (!block_pos_class) return nullptr;
	
	// Try ORIGIN static field using mappings
	jfieldID origin_fid = nullptr;
	if (sdk::mappings::block_pos_origin_name && sdk::mappings::block_pos_origin_sig)
	{
		origin_fid = env->GetStaticFieldID(block_pos_class, sdk::mappings::block_pos_origin_name, sdk::mappings::block_pos_origin_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	if (!origin_fid)
	{
		env->DeleteLocalRef(block_pos_class);
		return nullptr;
	}
	
	jobject origin = env->GetStaticObjectField(block_pos_class, origin_fid);
	env->DeleteLocalRef(block_pos_class);
	
	return origin;
}

void enhance::modules::autototem::send_update_selected_slot_packet(int slot)
{
	if (slot < 0 || slot > 8) return; // Only hotbar slots
	
	auto env = enhance::instance->get_env();
	if (!env) return;
	
	jobject network_handler = sdk::instance->get_network_handler();
	if (!network_handler) return;
	
	// Find UpdateSelectedSlotC2SPacket class using mappings
	jclass packet_class = nullptr;
	if (sdk::mappings::update_selected_slot_c2s_packet_class_sig)
	{
		packet_class = sdk::classloader::find_class(env, sdk::mappings::update_selected_slot_c2s_packet_class_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	if (!packet_class)
	{
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Find constructor: UpdateSelectedSlotC2SPacket(int slot)
	jmethodID constructor = env->GetMethodID(packet_class, "<init>", "(I)V");
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (!constructor)
	{
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Create packet instance
	jobject packet = env->NewObject(packet_class, constructor, static_cast<jint>(slot));
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (!packet)
	{
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Get sendPacket method using mappings
	jclass handler_class = env->GetObjectClass(network_handler);
	if (!handler_class)
	{
		env->DeleteLocalRef(packet);
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	jmethodID send_packet_mid = nullptr;
	if (sdk::mappings::send_packet_name && sdk::mappings::send_packet_sig)
	{
		send_packet_mid = env->GetMethodID(handler_class, sdk::mappings::send_packet_name, sdk::mappings::send_packet_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	env->CallVoidMethod(network_handler, send_packet_mid, packet);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	env->DeleteLocalRef(packet);
	env->DeleteLocalRef(packet_class);
	env->DeleteLocalRef(handler_class);
	env->DeleteLocalRef(network_handler);
}

void enhance::modules::autototem::send_player_action_packet(jobject block_pos, int action)
{
	auto env = enhance::instance->get_env();
	if (!env || !block_pos) return;
	
	jobject network_handler = sdk::instance->get_network_handler();
	if (!network_handler) return;
	
	// Find PlayerActionC2SPacket class using mappings
	jclass packet_class = nullptr;
	if (sdk::mappings::player_action_c2s_packet_class_sig)
	{
		packet_class = sdk::classloader::find_class(env, sdk::mappings::player_action_c2s_packet_class_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	if (!packet_class)
	{
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Find Action enum class using mappings
	jclass action_class = nullptr;
	if (sdk::mappings::player_action_c2s_packet_action_class_sig)
	{
		action_class = sdk::classloader::find_class(env, sdk::mappings::player_action_c2s_packet_action_class_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	if (!action_class)
	{
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Get SWAP_ITEM_WITH_OFFHAND action using mappings
	jobject action_enum = nullptr;
	if (sdk::mappings::swap_item_with_offhand_action_name && sdk::mappings::swap_item_with_offhand_action_sig)
	{
		jfieldID swap_action_fid = env->GetStaticFieldID(action_class, sdk::mappings::swap_item_with_offhand_action_name, sdk::mappings::swap_item_with_offhand_action_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (swap_action_fid)
		{
			action_enum = env->GetStaticObjectField(action_class, swap_action_fid);
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
	}
	
	// Get Direction.DOWN using mappings
	jclass direction_class = nullptr;
	jobject direction_enum = nullptr;
	if (sdk::mappings::direction_class_sig && sdk::mappings::direction_down_name && sdk::mappings::direction_down_sig)
	{
		direction_class = sdk::classloader::find_class(env, sdk::mappings::direction_class_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		if (direction_class)
		{
			jfieldID down_fid = env->GetStaticFieldID(direction_class, sdk::mappings::direction_down_name, sdk::mappings::direction_down_sig);
			if (env->ExceptionCheck()) env->ExceptionClear();
			
			if (down_fid)
			{
				direction_enum = env->GetStaticObjectField(direction_class, down_fid);
				if (env->ExceptionCheck()) env->ExceptionClear();
			}
			env->DeleteLocalRef(direction_class);
		}
	}
	
	if (!action_enum || !direction_enum)
	{
		if (action_enum) env->DeleteLocalRef(action_enum);
		if (direction_enum) env->DeleteLocalRef(direction_enum);
		env->DeleteLocalRef(action_class);
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Find constructor: PlayerActionC2SPacket(Action action, BlockPos pos, Direction direction)
	// Construct signature from mappings - all must be available
	if (!sdk::mappings::player_action_c2s_packet_action_class_sig || 
	    !sdk::mappings::block_pos_class_sig || 
	    !sdk::mappings::direction_class_sig)
	{
		env->DeleteLocalRef(action_enum);
		env->DeleteLocalRef(direction_enum);
		env->DeleteLocalRef(action_class);
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	std::string constructor_sig = "(L";
	constructor_sig += sdk::mappings::player_action_c2s_packet_action_class_sig;
	constructor_sig += ";L";
	constructor_sig += sdk::mappings::block_pos_class_sig;
	constructor_sig += ";L";
	constructor_sig += sdk::mappings::direction_class_sig;
	constructor_sig += ";)V";
	
	jmethodID constructor = env->GetMethodID(packet_class, "<init>", constructor_sig.c_str());
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (!constructor)
	{
		env->DeleteLocalRef(action_enum);
		env->DeleteLocalRef(direction_enum);
		env->DeleteLocalRef(action_class);
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Create packet instance
	jobject packet = env->NewObject(packet_class, constructor, action_enum, block_pos, direction_enum);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	env->DeleteLocalRef(action_enum);
	env->DeleteLocalRef(direction_enum);
	env->DeleteLocalRef(action_class);
	
	if (!packet)
	{
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	// Get sendPacket method using mappings
	jclass handler_class = env->GetObjectClass(network_handler);
	if (!handler_class)
	{
		env->DeleteLocalRef(packet);
		env->DeleteLocalRef(packet_class);
		env->DeleteLocalRef(network_handler);
		return;
	}
	
	jmethodID send_packet_mid = nullptr;
	if (sdk::mappings::send_packet_name && sdk::mappings::send_packet_sig)
	{
		send_packet_mid = env->GetMethodID(handler_class, sdk::mappings::send_packet_name, sdk::mappings::send_packet_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	env->CallVoidMethod(network_handler, send_packet_mid, packet);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	env->DeleteLocalRef(packet);
	env->DeleteLocalRef(packet_class);
	env->DeleteLocalRef(handler_class);
	env->DeleteLocalRef(network_handler);
}

void enhance::modules::autototem::send_pick_from_inventory_packet(int slot)
{
	// Function not implemented - using swap_to_offhand_blatant instead
	(void)slot; // Suppress unused parameter warning
}

void enhance::modules::autototem::swap_to_offhand_legit(int slot)
{
	if (slot < 0 || slot >= 40) return;
	
	if (!inventory_open)
	{
		open_inventory();
	}
	
	// Always use blatant mode (legit mode removed)
	swap_to_offhand_blatant(slot);
	
	close_inventory();
}

void enhance::modules::autototem::run()
{
	if (!globals::autototem_enabled) return;
	
	if (globals::show_gui) return;
	
	HWND window = Hook::get_window();
	if (!window) return;
	
	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window) return;
	
	// Throttle checks to avoid spam
	ULONGLONG current_time = GetTickCount64();
	if (current_time - last_check_time < 100) // Check every 100ms
	{
		return;
	}
	last_check_time = current_time;
	
	jobject player = sdk::instance->get_player();
	if (!player) return;
	
	auto env = enhance::instance->get_env();
	if (!env)
	{
		env->DeleteLocalRef(player);
		return;
	}
	
	// Check if player already has totem in offhand
	if (has_totem_in_offhand(player))
	{
		env->DeleteLocalRef(player);
		return;
	}
	
	// Find totem in inventory
	int totem_slot = find_totem_in_inventory(player);
	env->DeleteLocalRef(player);
	
	if (totem_slot == -1) return; // No totem found
	
	// Use blatant mode for swapping
	swap_to_offhand_blatant(totem_slot);
}

