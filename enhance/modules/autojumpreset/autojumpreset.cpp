#include "autojumpreset.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>

static void simulate_jump()
{
	HWND window = Hook::get_window();
	if (!window) return;
	
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = VK_SPACE;
	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));
	
	// Release immediately (quick jump press)
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

void enhance::modules::autojumpreset::run()
{
	if (!globals::autojumpreset_enabled)
	{
		// Reset health tracking when disabled
		globals::autojumpreset_last_health = 0.0f;
		return;
	}

	jobject player = sdk::instance->get_player();
	if (!player) return;

	auto env = enhance::instance->get_env();
	if (!env)
	{
		return;
	}

	// Get current health
	jclass living_entity_class = sdk::classloader::find_class(env, sdk::mappings::living_entity_class_sig);
	if (!living_entity_class)
	{
		env->DeleteLocalRef(player);
		return;
	}

	jmethodID get_health_mid = env->GetMethodID(living_entity_class, sdk::mappings::living_entity_get_health_name, sdk::mappings::living_entity_get_health_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(living_entity_class);

	if (!get_health_mid)
	{
		env->DeleteLocalRef(player);
		return;
	}

	jfloat current_health = env->CallFloatMethod(player, get_health_mid);
	if (env->ExceptionCheck()) env->ExceptionClear();

	// Initialize last health if not set
	if (globals::autojumpreset_last_health == 0.0f)
	{
		globals::autojumpreset_last_health = current_health;
		env->DeleteLocalRef(player);
		return;
	}

	// Check if health decreased (player took damage)
	if (current_health < globals::autojumpreset_last_health)
	{
		ULONGLONG current_time = GetTickCount64();
		if (current_time - globals::autojumpreset_last_jump >= globals::autojumpreset_cooldown_ms)
		{
			simulate_jump();
			globals::autojumpreset_last_jump = current_time;
		}
	}

	// Update last health
	globals::autojumpreset_last_health = current_health;

	env->DeleteLocalRef(player);
}

void enhance::modules::autojumpreset::on_hit()
{
	if (!globals::autojumpreset_enabled) return;
	
	ULONGLONG current_time = GetTickCount64();
	if (current_time - globals::autojumpreset_last_jump >= globals::autojumpreset_cooldown_ms)
	{
		simulate_jump();
		globals::autojumpreset_last_jump = current_time;
	}
}

