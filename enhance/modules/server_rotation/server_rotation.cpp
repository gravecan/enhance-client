#include "server_rotation.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/player/player.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/mappings/mappings.hpp>
#include <windows.h>

static float g_saved_yaw = 0.0f;
static float g_saved_pitch = 0.0f;
static bool g_rotation_saved = false;

static bool get_rotation(float& yaw, float& pitch)
{
	jobject player = sdk::instance->get_player();
	if (!player) return false;

	sdk::entity_client entity(player);
	yaw = entity.get_yaw();
	pitch = entity.get_pitch();

	auto env = enhance::instance->get_env();
	if (env) env->DeleteLocalRef(player);
	return true;
}

static void set_rotation(float yaw, float pitch)
{
	jobject player = sdk::instance->get_player();
	if (!player) return;

	sdk::entity_client entity(player);
	entity.set_yaw(yaw);
	entity.set_pitch(pitch);

	auto env = enhance::instance->get_env();
	if (env) env->DeleteLocalRef(player);
}

void enhance::modules::server_rotation::set_server_rotation(float yaw, float pitch)
{
	if (!g_rotation_saved)
	{
		get_rotation(g_saved_yaw, g_saved_pitch);
		g_rotation_saved = true;
	}
	set_rotation(yaw, pitch);
}

void enhance::modules::server_rotation::restore_rotation()
{
	if (g_rotation_saved)
	{
		set_rotation(g_saved_yaw, g_saved_pitch);
		g_rotation_saved = false;
	}
}

void enhance::modules::server_rotation::run()
{
	if (g_rotation_saved && !globals::server_rotation_enabled)
	{
		restore_rotation();
	}
}
