#include "wtap.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <windows.h>

static void release_w_key()
{
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 'W';
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

static void repress_w_key()
{
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 'W';
	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));
}

static bool is_sprinting()
{
	jobject player = sdk::instance->get_player();
	if (!player) return false;

	auto env = enhance::instance->get_env();
	if (!env)
	{
		return false;
	}

	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class)
	{
		env->DeleteLocalRef(player);
		return false;
	}

	jmethodID is_sprinting_mid = env->GetMethodID(entity_class, sdk::mappings::is_sprinting_name, sdk::mappings::is_sprinting_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	if (!is_sprinting_mid)
	{
		env->DeleteLocalRef(player);
		return false;
	}

	jboolean sprinting = env->CallBooleanMethod(player, is_sprinting_mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(player);
	
	return sprinting == JNI_TRUE;
}

void enhance::modules::wtap::on_hit()
{
	if (!globals::wtap_enabled) return;
	if (globals::wtap_is_active) return;
	
	bool w_held = (GetAsyncKeyState('W') & 0x8000) != 0;
	if (!w_held) return;
	
	if (is_sprinting())
	{
		globals::wtap_is_active = true;
		globals::wtap_start_time = GetTickCount64();
		
		release_w_key();
	}
}

void enhance::modules::wtap::run()
{
	if (!globals::wtap_enabled)
	{
		if (globals::wtap_is_active)
		{
			repress_w_key();
			globals::wtap_is_active = false;
		}
		return;
	}

	if (globals::show_gui)
	{
		if (globals::wtap_is_active)
		{
			repress_w_key();
			globals::wtap_is_active = false;
		}
		return;
	}

	if (globals::wtap_is_active)
	{
		ULONGLONG current_time = GetTickCount64();
		if (current_time - globals::wtap_start_time >= (ULONGLONG)globals::wtap_duration_ms)
		{
			repress_w_key();
			globals::wtap_is_active = false;
		}
	}
}
