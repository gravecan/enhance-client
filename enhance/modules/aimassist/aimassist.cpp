#include "aimassist.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../hooks/Hook.h"
#include "../../utils/logger.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <cmath>
#include <windows.h>
#include <sstream>

static bool last_key_state = false;
static bool aimassist_toggled = false;

static bool is_keybind_active()
{
	// If module not enabled, not active
	if (!globals::aimassist_enabled)
	{
		return false;
	}
	
	// If no keybind set, always active when enabled
	if (globals::aimassist_keybind == 0)
	{
		return true;
	}
	
	bool key_now = (GetAsyncKeyState(globals::aimassist_keybind) & 0x8000) != 0;
	bool key_pressed = key_now && !last_key_state;
	last_key_state = key_now;
	
	// Mode 0 = Hold: active while key is held
	if (globals::aimassist_mode == 0)
	{
		return key_now;
	}
	
	// Mode 1 = Toggle: toggle on key press
	if (globals::aimassist_mode == 1)
	{
		if (key_pressed)
		{
			aimassist_toggled = !aimassist_toggled;
		}
		return aimassist_toggled;
	}
	
	// Mode 2 = Always: always active if enabled (and keybind is set)
	if (globals::aimassist_mode == 2)
	{
		return true;
	}
	
	return false;
}

void enhance::modules::aimassist::run()
{
	if (!globals::aimassist_enabled)
	{
		aimassist_toggled = false;
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
	if (foreground_window != window) return;

	// Check keybind based on mode (hold/toggle/always)
	bool keybind_active = is_keybind_active();
	if (!keybind_active)
	{
		return;
	}

	jobject world = sdk::instance->get_world();
	if (!world)
		return;

	jobject local_player = sdk::instance->get_player();
	if (!local_player)
	{
		auto env = enhance::instance->get_env();
		if (env && world)
			env->DeleteLocalRef(world);
		return;
	}

	sdk::entity_client local_entity(local_player);
	double px = local_entity.get_x();
	double py = local_entity.get_y() + 1.62;
	double pz = local_entity.get_z();
	float current_yaw = local_entity.get_yaw();
	float current_pitch = local_entity.get_pitch();

	sdk::world_client world_client(world);
	std::vector<jobject> players = world_client.get_players();

	jobject closest_entity = nullptr;
	double max_distance = globals::aimassist_max_distance;
	double closest_distance = max_distance;

	for (size_t i = 0; i < players.size(); i++)
	{
		jobject player = players[i];
		if (!player)
			continue;

		sdk::entity_client entity_client(player);
		if (entity_client.is_same_object(local_player))
			continue;

		double ex = entity_client.get_x();
		double ey = entity_client.get_y();
		double ez = entity_client.get_z();

		double distance = calculate_distance(px, py, pz, ex, ey, ez);
		if (distance < closest_distance && distance <= max_distance)
		{
			closest_distance = distance;
			closest_entity = player;
		}
	}

	if (!closest_entity)
	{
		for (jobject player : players)
		{
			if (player)
			{
				auto env = enhance::instance->get_env();
				if (env)
					env->DeleteLocalRef(player);
			}
		}
		auto env = enhance::instance->get_env();
		if (env && world)
			env->DeleteLocalRef(world);
		if (env && local_player)
			env->DeleteLocalRef(local_player);
		return;
	}

	sdk::entity_client target_entity(closest_entity);
	double ex = target_entity.get_x();
	double ey = target_entity.get_y() + 1.62;
	double ez = target_entity.get_z();

	float target_yaw, target_pitch;
	calculate_angles(px, py, pz, ex, ey, ez, target_yaw, target_pitch);

	float yaw_diff = target_yaw - current_yaw;
	if (yaw_diff > 180.0f)
		yaw_diff -= 360.0f;
	else if (yaw_diff < -180.0f)
		yaw_diff += 360.0f;

	float pitch_diff = target_pitch - current_pitch;

	if (std::abs(yaw_diff) < 0.5f && std::abs(pitch_diff) < 0.5f)
	{
		for (jobject player : players)
		{
			if (player)
			{
				auto env = enhance::instance->get_env();
				if (env)
					env->DeleteLocalRef(player);
			}
		}
		auto env = enhance::instance->get_env();
		if (env && world)
			env->DeleteLocalRef(world);
		if (env && local_player)
			env->DeleteLocalRef(local_player);
		return;
	}

	float smoothed_yaw = smooth_angle(current_yaw, target_yaw, globals::aimassist_smoothing);
	float smoothed_pitch = smooth_angle(current_pitch, target_pitch, globals::aimassist_smoothing);

	// Apply based on which axes are enabled
	float final_yaw = current_yaw;
	float final_pitch = current_pitch;
	
	if (globals::aimassist_horizontal)
		final_yaw = smoothed_yaw;
	if (globals::aimassist_vertical)
		final_pitch = smoothed_pitch;

	if (globals::aimassist_use_mouse_input)
	{
		float yaw_delta = globals::aimassist_horizontal ? (final_yaw - current_yaw) : 0.0f;
		while (yaw_delta > 180.0f) yaw_delta -= 360.0f;
		while (yaw_delta < -180.0f) yaw_delta += 360.0f;
		
		float pitch_delta = globals::aimassist_vertical ? (final_pitch - current_pitch) : 0.0f;
		
		float mc_sensitivity = globals::aimassist_mouse_sensitivity * 0.6f + 0.2f;
		float degrees_per_pixel = mc_sensitivity * 0.15f;
		
		if (degrees_per_pixel < 0.001f) degrees_per_pixel = 0.001f;
		
		float mouse_x = yaw_delta / degrees_per_pixel;
		float mouse_y = pitch_delta / degrees_per_pixel;
		
		long mouse_delta_x = static_cast<long>(mouse_x);
		long mouse_delta_y = static_cast<long>(mouse_y);

		if (mouse_delta_x != 0 || mouse_delta_y != 0)
		{
			move_mouse(mouse_delta_x, mouse_delta_y);
		}
	}
	else
	{
		if (globals::aimassist_horizontal)
			local_entity.set_yaw(final_yaw);
		if (globals::aimassist_vertical)
			local_entity.set_pitch(final_pitch);
	}


	for (jobject player : players)
	{
		if (player)
		{
			auto env = enhance::instance->get_env();
			if (env)
				env->DeleteLocalRef(player);
		}
	}

	auto env = enhance::instance->get_env();
	if (env && world)
		env->DeleteLocalRef(world);
	if (env && local_player)
		env->DeleteLocalRef(local_player);
}

void enhance::modules::aimassist::calculate_angles(double px, double py, double pz, double ex, double ey, double ez, float& yaw, float& pitch)
{
	double dx = ex - px;
	double dy = ey - py;
	double dz = ez - pz;

	double horizontal_distance = std::sqrt(dx * dx + dz * dz);

	if (horizontal_distance < 0.0001)
	{
		yaw = 0.0f;
		pitch = (dy > 0.0) ? -90.0f : 90.0f;
		return;
	}

	yaw = static_cast<float>(std::atan2(-dx, dz) * 180.0 / 3.14159265358979323846);
	pitch = static_cast<float>(std::atan2(-dy, horizontal_distance) * 180.0 / 3.14159265358979323846);

	if (pitch > 90.0f) pitch = 90.0f;
	if (pitch < -90.0f) pitch = -90.0f;
}

float enhance::modules::aimassist::smooth_angle(float current, float target, float smoothing)
{
	float diff = target - current;

	while (diff > 180.0f) diff -= 360.0f;
	while (diff < -180.0f) diff += 360.0f;

	float smooth_factor = smoothing;
	if (smooth_factor < 0.0f) smooth_factor = 0.0f;
	if (smooth_factor > 0.99f) smooth_factor = 0.99f;

	float lerp_amount = 1.0f - smooth_factor;
	
	float step = diff * lerp_amount;

	if (std::abs(step) < 0.01f)
		return current;

	float result = current + step;
	
	while (result > 180.0f) result -= 360.0f;
	while (result < -180.0f) result += 360.0f;

	return result;
}

double enhance::modules::aimassist::calculate_distance(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double dz = z2 - z1;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

void enhance::modules::aimassist::move_mouse(long dx, long dy)
{
	HWND window = Hook::get_window();
	if (!window)
		return;

	HWND foreground_window = GetForegroundWindow();
	if (foreground_window != window)
		return;

	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = dx;
	input.mi.dy = dy;
	
	if (SendInput(1, &input, sizeof(INPUT)) == 0)
	{
		mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(dx), static_cast<DWORD>(dy), 0, 0);
	}
}
