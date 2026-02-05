#include "reach.h"
#include "reach_hook.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../utils/logger.h"
#include <sdk/minecraft/minecraft.h>
#include <windows.h>
#include <sstream>

static bool hook_initialized = false;
static bool last_key_state = false;
static bool reach_toggled = false;

static bool is_keybind_active()
{
	// If module not enabled, not active
	if (!globals::reach_enabled)
	{
		return false;
	}
	
	// If no keybind set, always active when enabled
	if (globals::reach_keybind == 0)
	{
		return true;
	}
	
	bool key_now = (GetAsyncKeyState(globals::reach_keybind) & 0x8000) != 0;
	bool key_pressed = key_now && !last_key_state;
	last_key_state = key_now;
	
	// Mode 0 = Hold: active while key is held
	if (globals::reach_mode == 0)
	{
		return key_now;
	}
	
	// Mode 1 = Toggle: toggle on key press
	if (globals::reach_mode == 1)
	{
		if (key_pressed)
		{
			reach_toggled = !reach_toggled;
		}
		return reach_toggled;
	}
	
	// Mode 2 = Always: always active if enabled (and keybind is set)
	if (globals::reach_mode == 2)
	{
		return true;
	}
	
	return false;
}

void enhance::modules::reach::run()
{
	if (!hook_initialized)
	{
		if (reach_hook::init())
		{
			hook_initialized = true;
		}
	}
	
	if (!globals::reach_enabled)
	{
		reset();
		reach_toggled = false;
		last_key_state = false;
		return;
	}

	// Check keybind based on mode (hold/toggle/always)
	bool keybind_active = is_keybind_active();
	if (!keybind_active)
	{
		reset();
		// Reset toggle state when keybind becomes inactive in hold mode
		if (globals::reach_mode == 0)
		{
			reach_toggled = false;
		}
		return;
	}

	reach_hook::set_reach(globals::reach_distance);
}

void enhance::modules::reach::reset_hook_state()
{
	hook_initialized = false;
}

void enhance::modules::reach::reset()
{
	reach_hook::set_reach(3.0);
}
