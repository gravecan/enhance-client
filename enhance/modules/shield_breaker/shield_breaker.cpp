#include "shield_breaker.h"
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
#include <windows.h>

int enhance::modules::shield_breaker::find_item_in_hotbar(jobject player, const char* item_name)
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

bool enhance::modules::shield_breaker::is_item(jobject item_stack, const char* item_name)
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

void enhance::modules::shield_breaker::swap_to_slot(int slot)
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

int enhance::modules::shield_breaker::get_current_slot(jobject player)
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

void enhance::modules::shield_breaker::aim_at_entity(jobject player, jobject target_entity)
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

// Helper to get offhand stack from player inventory
static jobject get_offhand_stack(JNIEnv* env, jobject inventory)
{
	if (!env || !inventory) return nullptr;
	
	jclass inventory_class = env->GetObjectClass(inventory);
	if (!inventory_class) return nullptr;
	
	// Get the offHand field (DefaultedList<ItemStack>)
	jfieldID offhand_fid = env->GetFieldID(inventory_class, sdk::mappings::inventory_offhand_name, sdk::mappings::inventory_offhand_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (offhand_fid)
	{
		// Get the offHand list
		jobject offhand_list = env->GetObjectField(inventory, offhand_fid);
		env->DeleteLocalRef(inventory_class);
		
		if (offhand_list)
		{
			// Get first item from list using get(0)
			jclass list_class = env->GetObjectClass(offhand_list);
			if (list_class)
			{
				jmethodID get_mid = env->GetMethodID(list_class, "get", "(I)Ljava/lang/Object;");
				if (env->ExceptionCheck()) env->ExceptionClear();
				
				if (get_mid)
				{
					jobject stack = env->CallObjectMethod(offhand_list, get_mid, 0);
					if (env->ExceptionCheck()) env->ExceptionClear();
					env->DeleteLocalRef(list_class);
					env->DeleteLocalRef(offhand_list);
					return stack;
				}
				env->DeleteLocalRef(list_class);
			}
			env->DeleteLocalRef(offhand_list);
		}
		return nullptr;
	}
	
	// Fallback: try getStack with slot 40 (vanilla convention)
	jmethodID get_stack_mid = env->GetMethodID(inventory_class, sdk::mappings::inventory_get_stack_name, sdk::mappings::inventory_get_stack_sig);
	env->DeleteLocalRef(inventory_class);
	
	if (!get_stack_mid)
	{
		if (env->ExceptionCheck()) env->ExceptionClear();
		return nullptr;
	}
	
	jobject stack = env->CallObjectMethod(inventory, get_stack_mid, 40);
	if (env->ExceptionCheck()) env->ExceptionClear();
	return stack;
}

bool enhance::modules::shield_breaker::is_using_shield(jobject target_entity)
{
	if (!target_entity) return false;
	auto env = enhance::instance->get_env();
	if (!env) return false;

	// Check if target is a player (only players can use shields)
	jclass player_class_check = sdk::classloader::find_class(env, sdk::mappings::player_entity_class_sig);
	if (!player_class_check) return false;
	
	jboolean is_player = env->IsInstanceOf(target_entity, player_class_check);
	env->DeleteLocalRef(player_class_check);
	
	if (!is_player) return false;

	// Check if target is actually blocking (using shield)
	jclass living_entity_class = sdk::classloader::find_class(env, sdk::mappings::living_entity_class_sig);
	if (!living_entity_class) return false;
	
	jmethodID is_blocking_mid = env->GetMethodID(living_entity_class, sdk::mappings::living_entity_is_blocking_name, sdk::mappings::living_entity_is_blocking_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(living_entity_class);
	
	if (!is_blocking_mid) return false;
	
	jboolean blocking = env->CallBooleanMethod(target_entity, is_blocking_mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	// Only return true if actually blocking
	return blocking == JNI_TRUE;
}

double enhance::modules::shield_breaker::calculate_distance(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double dz = z2 - z1;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

void enhance::modules::shield_breaker::send_double_attack()
{
	HWND window = Hook::get_window();
	if (!window) return;
	if (GetForegroundWindow() != window) return;

	// Use SendInput for more reliable input simulation
	// First attack
	INPUT inputs[2] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(2, inputs, sizeof(INPUT));
	
	// Small delay between attacks if configured
	if (globals::shield_breaker_delay_ms > 0)
	{
		Sleep(globals::shield_breaker_delay_ms);
	}
	
	// Second attack
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(2, inputs, sizeof(INPUT));
}

static bool is_entity_hit_result(jobject hit_result)
{
	if (!hit_result) 
		return false;

	auto env = enhance::instance->get_env();
	if (!env) 
		return false;

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
	if (!hit_result) 
		return nullptr;

	auto env = enhance::instance->get_env();
	if (!env) 
		return nullptr;

	jclass hit_result_class = env->GetObjectClass(hit_result);
	if (!hit_result_class) 
		return nullptr;

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

void enhance::modules::shield_breaker::run()
{
	if (!globals::shield_breaker_enabled)
	{
		if (globals::shield_breaker_saved_slot != -1 && globals::shield_breaker_switch_back)
		{
			jobject player = sdk::instance->get_player();
			if (player)
			{
				swap_to_slot(globals::shield_breaker_saved_slot);
				globals::shield_breaker_saved_slot = -1;
				auto env = enhance::instance->get_env();
				if (env) env->DeleteLocalRef(player);
			}
		}
		return;
	}

	jobject local_player = sdk::instance->get_player();
	if (!local_player)
	{
		return;
	}

	int axe_slot = find_item_in_hotbar(local_player, "axe");
	if (axe_slot == -1)
	{
		auto env = enhance::instance->get_env();
		if (env) env->DeleteLocalRef(local_player);
		return;
	}

	jobject minecraft = sdk::instance->get_minecraft();
	if (!minecraft)
	{
		auto env = enhance::instance->get_env();
		if (env) env->DeleteLocalRef(local_player);
		return;
	}

	jobject crosshair_target = sdk::instance->get_crosshair_target();
	if (!crosshair_target || !is_entity_hit_result(crosshair_target))
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			if (crosshair_target) env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	jobject target_entity = get_entity_from_hit_result(crosshair_target);
	if (!target_entity)
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	sdk::entity_client target_entity_check(target_entity);
	if (target_entity_check.is_same_object(local_player))
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(target_entity);
			env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	bool is_blocking = is_using_shield(target_entity);
	if (!is_blocking)
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(target_entity);
			env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	sdk::entity_client local_entity(local_player);
	sdk::entity_client target_entity_client(target_entity);

	double px = local_entity.get_x();
	double py = local_entity.get_y() + 1.62;
	double pz = local_entity.get_z();

	double ex = target_entity_client.get_x();
	double ey = target_entity_client.get_y() + 1.6;
	double ez = target_entity_client.get_z();

	double distance = calculate_distance(px, py, pz, ex, ey, ez);
	double max_reach = globals::reach_enabled ? globals::reach_distance : 3.0;
	if (distance > max_reach)
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(target_entity);
			env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	// Allow small cooldown values for faster response (similar to mace module)
	if (sdk::instance->get_attack_cooldown() > 0.1f)
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(target_entity);
			env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	ULONGLONG current_time = GetTickCount64();
	if (current_time - globals::shield_breaker_last_attack < 200)
	{
		auto env = enhance::instance->get_env();
		if (env)
		{
			env->DeleteLocalRef(target_entity);
			env->DeleteLocalRef(crosshair_target);
			env->DeleteLocalRef(minecraft);
			env->DeleteLocalRef(local_player);
		}
		return;
	}

	auto env = enhance::instance->get_env();
	if (!env)
	{
		return;
	}

	if (globals::shield_breaker_saved_slot == -1)
	{
		globals::shield_breaker_saved_slot = get_current_slot(local_player);
	}

	if (globals::shield_breaker_aim)
	{
		aim_at_entity(local_player, target_entity);
	}

	int current_slot = get_current_slot(local_player);
	if (current_slot != axe_slot)
	{
		swap_to_slot(axe_slot);
		Sleep(50); // Delay for swap to register (same as mace/autocrystal)
	}

	send_double_attack();
	globals::shield_breaker_last_attack = current_time;

	if (globals::shield_breaker_switch_back && globals::shield_breaker_saved_slot != -1)
	{
		// Reduced delay for faster execution
		Sleep(20);
		swap_to_slot(globals::shield_breaker_saved_slot);
		globals::shield_breaker_saved_slot = -1;
	}

	env->DeleteLocalRef(target_entity);
	env->DeleteLocalRef(crosshair_target);
	env->DeleteLocalRef(minecraft);
	env->DeleteLocalRef(local_player);
}
