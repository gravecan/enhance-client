#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/classloader.h>
#include "enhance.h"
#include "globals/globals.h"
#include "hooks/Hook.h"
#include "utils/logger.h"
#include "gui/GUI.h"
#include "modules/hitbox/hitbox.h"
#include "modules/aimassist/aimassist.h"
#include "modules/triggerbot/triggerbot.h"
#include "modules/pearl_catch/pearl_catch.h"
#include "modules/reach/reach.h"
#include "modules/reach/reach_hook.h"
#include "modules/esp/esp.h"
#include "modules/mace/mace.h"
#include "modules/shield_breaker/shield_breaker.h"
#include "modules/stun_slam/stun_slam.h"
#include "modules/server_rotation/server_rotation.h"
#include "modules/stap/stap.h"
#include "modules/wtap/wtap.h"
#include "modules/anchor_macro/anchor_macro.h"
#include "modules/storage_esp/storage_esp.h"
#include "modules/autocrystal/autocrystal.h"
#include "modules/autototem/autototem.h"
#include "modules/autojumpreset/autojumpreset.h"
// #include "modules/backtrack/backtrack.h" // Disabled
// #include "modules/backtrack/backtrack_hook.h" // Disabled
#include <chrono>

bool enhance::enhance_client::attach()
{
	HMODULE jvm = GetModuleHandleA("jvm.dll");
	if (!jvm)
	{
		return false;
	}

	using t_createdvms = jint(__stdcall*)(JavaVM**, jsize, jsize*);

	FARPROC process_address = GetProcAddress(reinterpret_cast<HMODULE>(jvm), "JNI_GetCreatedJavaVMs");
	if (!process_address)
	{
		return false;
	}
	
	t_createdvms created_java_vms = reinterpret_cast<t_createdvms>(process_address);

	auto ret = created_java_vms(&vm, 1, nullptr);

	if (ret != JNI_OK)
	{
		return false;
	}

	ret = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);

	if (ret != JNI_OK)
	{
		return false;
	}

	if (!sdk::classloader::init(env))
	{
		return false;
	}

	if (sdk::classloader::is_fabric())
	{
		printf("[ENHANCE] Detected: Fabric\n");
	}
	else
	{
		printf("[ENHANCE] Detected: Vanilla\n");
	}

	if (Hook::init())
	{
		return false;
	}

	return true;
}

void enhance::enhance_client::run()
{
	sdk::instance = std::make_unique<sdk::minecraft_client>();
	auto local_player = std::make_unique<player_client>();

	while (true)
	{
		if (globals::flight_enabled)
		{
			jobject player = local_player->get_player();
			if (player)
			{
				local_player->set_flying(true);
			}
		}

		if (globals::sprint_enabled)
		{
			jobject player = local_player->get_player();
			if (player)
			{
				local_player->set_sprinting(true);
			}
		}

		enhance::modules::hitbox_expander::run();
		enhance::modules::aimassist::run();
		enhance::modules::triggerbot::run();
		enhance::modules::pearl_catch::run();
		enhance::modules::reach::run();
		enhance::modules::esp::run();
		enhance::modules::mace::run();
		enhance::modules::shield_breaker::run();
		enhance::modules::stun_slam::run();
		enhance::modules::stap::run();
		enhance::modules::wtap::run();
		enhance::modules::anchor_macro::run();
		enhance::modules::server_rotation::run();
		enhance::modules::storage_esp::run();
		enhance::modules::autocrystal::run();
		enhance::modules::autototem::run();
		enhance::modules::autojumpreset::run();
		// enhance::modules::backtrack::run(); // Disabled

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void enhance::enhance_client::unload()
{
	sdk::instance.reset();
	
	try
	{
		enhance::modules::reach::reset_hook_state();
	}
	catch (...)
	{
	}
	
	try
	{
		enhance::modules::reach_hook::shutdown();
	}
	catch (...)
	{
	}
	
	// Backtrack module disabled
	/*
	try
	{
		enhance::modules::backtrack::reset_hook_state();
	}
	catch (...)
	{
	}
	
	try
	{
		enhance::modules::backtrack_hook::shutdown();
	}
	catch (...)
	{
	}
	*/
	
	if (vm)
	{
		try
		{
			vm->DetachCurrentThread();
		}
		catch (...)
		{
		}
	}
	
	try
	{
		if (env)
		{
			sdk::classloader::cleanup(env);
		}
	}
	catch (...)
	{
	}
	
	env = nullptr;
	vm = nullptr;
	
	try
	{
		Hook::shutdown();
	}
	catch (...)
	{
	}
}

std::unique_ptr<enhance::enhance_client> enhance::instance;
