#include "stap.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <windows.h>

static bool s_key_pressed = false;

static void press_s_key()
{
	if (s_key_pressed) return;
	
	HWND window = Hook::get_window();
	if (!window) return;
	
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 'S';
	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));
	
	s_key_pressed = true;
}

static void release_s_key()
{
	if (!s_key_pressed) return;
	
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 'S';
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
	
	s_key_pressed = false;
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

static bool is_on_ground()
{
	jobject player = sdk::instance->get_player();
	if (!player) return false;

	sdk::entity_client entity(player);
	bool on_ground = entity.is_on_ground();
	
	auto env = enhance::instance->get_env();
	if (env) env->DeleteLocalRef(player);
	
	return on_ground;
}

void enhance::modules::stap::on_hit()
{
	if (!globals::stap_enabled) return;
	if (globals::stap_is_active) return;
	
	if (is_sprinting() && is_on_ground())
	{
		globals::stap_is_active = true;
		globals::stap_start_time = GetTickCount64();
		
		press_s_key();
	}
}

void enhance::modules::stap::run()
{
	if (!globals::stap_enabled)
	{
		if (globals::stap_is_active)
		{
			release_s_key();
			globals::stap_is_active = false;
		}
		return;
	}

	if (globals::show_gui)
	{
		if (globals::stap_is_active)
		{
			release_s_key();
			globals::stap_is_active = false;
		}
		return;
	}

	if (globals::stap_is_active)
	{
		ULONGLONG current_time = GetTickCount64();
		if (current_time - globals::stap_start_time >= (ULONGLONG)globals::stap_duration_ms)
		{
			release_s_key();
			globals::stap_is_active = false;
		}
	}
}
