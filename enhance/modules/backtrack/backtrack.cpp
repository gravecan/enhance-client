#include "backtrack.h"
#include "backtrack_hook.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../gui/GUI.h"
#include "../../utils/logger.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/minecraft/util/box.h>
#include <sdk/classloader.h>
#include <windows.h>
#include <cmath>
#include <cfloat>
#include <limits>
#include <vector>
#include <deque>
#include <map>
#include <mutex>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static bool hook_initialized = false;
static bool last_key_state = false;
static bool backtrack_toggled = false;
static ULONGLONG last_cooldown_end = 0;
static bool is_active = false;
static double last_local_velocity_magnitude = 0.0;
static ULONGLONG knockback_disable_until = 0;

// Position history storage
static std::map<jobject, std::deque<backtrack_position_record>> player_position_history;
static std::mutex position_history_mutex;

// Visualization data
static std::vector<backtrack_player_data> backtrack_players;
static std::mutex backtrack_players_mutex;

// View matrix for world-to-screen (reuse ESP's approach)
struct view_matrix_t
{
	float right[3];
	float up[3];
	float forward[3];
	float cam_pos[3];
	float fov_x, fov_y;
	float half_w, half_h;
	bool valid;
};

static view_matrix_t g_view_matrix;

static void compute_view_matrix(float cam_x, float cam_y, float cam_z, float yaw, float pitch, float fov, int screen_width, int screen_height)
{
	float yaw_rad = yaw * (float)(M_PI / 180.0);
	float pitch_rad = pitch * (float)(M_PI / 180.0);

	float cos_yaw = cosf(yaw_rad);
	float sin_yaw = sinf(yaw_rad);
	float cos_pitch = cosf(pitch_rad);
	float sin_pitch = sinf(pitch_rad);

	// Forward vector
	g_view_matrix.forward[0] = -sin_yaw * cos_pitch;
	g_view_matrix.forward[1] = -sin_pitch;
	g_view_matrix.forward[2] = cos_yaw * cos_pitch;

	// Right vector (cross product of forward and world up)
	g_view_matrix.right[0] = g_view_matrix.forward[1] * 0.0f - g_view_matrix.forward[2] * 1.0f;
	g_view_matrix.right[1] = g_view_matrix.forward[2] * 0.0f - g_view_matrix.forward[0] * 0.0f;
	g_view_matrix.right[2] = g_view_matrix.forward[0] * 1.0f - g_view_matrix.forward[1] * 0.0f;

	// Normalize right
	float right_len = sqrtf(g_view_matrix.right[0] * g_view_matrix.right[0] + 
	                        g_view_matrix.right[1] * g_view_matrix.right[1] + 
	                        g_view_matrix.right[2] * g_view_matrix.right[2]);
	if (right_len > 1e-6f) {
		g_view_matrix.right[0] /= right_len;
		g_view_matrix.right[1] /= right_len;
		g_view_matrix.right[2] /= right_len;
	}

	// Up vector (cross product of right and forward)
	g_view_matrix.up[0] = g_view_matrix.right[1] * g_view_matrix.forward[2] - g_view_matrix.right[2] * g_view_matrix.forward[1];
	g_view_matrix.up[1] = g_view_matrix.right[2] * g_view_matrix.forward[0] - g_view_matrix.right[0] * g_view_matrix.forward[2];
	g_view_matrix.up[2] = g_view_matrix.right[0] * g_view_matrix.forward[1] - g_view_matrix.right[1] * g_view_matrix.forward[0];

	// Camera position
	g_view_matrix.cam_pos[0] = cam_x;
	g_view_matrix.cam_pos[1] = cam_y;
	g_view_matrix.cam_pos[2] = cam_z;

	// FOV projection factors
	float fov_rad = fov * (float)(M_PI / 180.0);
	float aspect = (float)screen_width / (float)screen_height;
	float tan_half_fov = tanf(fov_rad * 0.5f);
	g_view_matrix.fov_x = 1.0f / (tan_half_fov * aspect);
	g_view_matrix.fov_y = 1.0f / tan_half_fov;

	// Screen half dimensions
	g_view_matrix.half_w = screen_width * 0.5f;
	g_view_matrix.half_h = screen_height * 0.5f;

	g_view_matrix.valid = true;
}

static inline bool world_to_screen_fast(float x, float y, float z, float& out_x, float& out_y)
{
	// Local space coordinates
	float lx = x - g_view_matrix.cam_pos[0];
	float ly = y - g_view_matrix.cam_pos[1];
	float lz = z - g_view_matrix.cam_pos[2];

	// Project onto view axes
	float px = lx * g_view_matrix.right[0] + ly * g_view_matrix.right[1] + lz * g_view_matrix.right[2];
	float py = lx * g_view_matrix.up[0] + ly * g_view_matrix.up[1] + lz * g_view_matrix.up[2];
	float pz = lx * g_view_matrix.forward[0] + ly * g_view_matrix.forward[1] + lz * g_view_matrix.forward[2];

	if (pz < 0.01f) return false;
	
	float inv_z = 1.0f / pz;
	out_x = g_view_matrix.half_w + (px * inv_z * g_view_matrix.fov_x) * g_view_matrix.half_w;
	out_y = g_view_matrix.half_h - (py * inv_z * g_view_matrix.fov_y) * g_view_matrix.half_h;
	
	return true;
}

static double calculate_distance(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double dz = z2 - z1;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

static double get_velocity_magnitude(jobject velocity_vec)
{
	if (!velocity_vec) return 0.0;
	
	auto env = enhance::instance->get_env();
	if (!env) return 0.0;
	
	jclass vec3d_class = sdk::classloader::find_class(env, sdk::mappings::vec3d_class_sig);
	if (!vec3d_class) return 0.0;
	
	jfieldID x_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_x_name, sdk::mappings::vec3d_x_sig);
	jfieldID y_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_y_name, sdk::mappings::vec3d_y_sig);
	jfieldID z_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_z_name, sdk::mappings::vec3d_z_sig);
	
	if (!x_fid || !y_fid || !z_fid)
	{
		env->DeleteLocalRef(vec3d_class);
		return 0.0;
	}
	
	double vx = env->GetDoubleField(velocity_vec, x_fid);
	double vy = env->GetDoubleField(velocity_vec, y_fid);
	double vz = env->GetDoubleField(velocity_vec, z_fid);
	
	env->DeleteLocalRef(vec3d_class);
	
	return sqrt(vx * vx + vy * vy + vz * vz);
}

static int get_hurt_time(jobject entity)
{
	if (!entity) return 999;
	
	// Check if mappings are available
	if (!sdk::mappings::living_entity_hurt_time_name || !sdk::mappings::living_entity_hurt_time_sig)
	{
		return 999; // Mappings not loaded yet
	}
	
	auto env = enhance::instance->get_env();
	if (!env) return 999;
	
	jclass living_entity_class = sdk::classloader::find_class(env, sdk::mappings::living_entity_class_sig);
	if (!living_entity_class) return 999;
	
	jfieldID hurt_time_fid = env->GetFieldID(living_entity_class, sdk::mappings::living_entity_hurt_time_name, sdk::mappings::living_entity_hurt_time_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (!hurt_time_fid)
	{
		env->DeleteLocalRef(living_entity_class);
		return 999;
	}
	
	jint hurt_time = env->GetIntField(entity, hurt_time_fid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	env->DeleteLocalRef(living_entity_class);
	
	return hurt_time;
}

static bool is_keybind_active()
{
	// If module not enabled, not active
	if (!globals::backtrack_enabled)
	{
		return false;
	}
	
	// If no keybind set, always active when enabled
	if (globals::backtrack_keybind == 0)
	{
		return true;
	}
	
	bool key_now = (GetAsyncKeyState(globals::backtrack_keybind) & 0x8000) != 0;
	bool key_pressed = key_now && !last_key_state;
	last_key_state = key_now;
	
	// Mode 0 = Hold: active while key is held
	if (globals::backtrack_mode == 0)
	{
		return key_now;
	}
	
	// Mode 1 = Toggle: toggle on key press
	if (globals::backtrack_mode == 1)
	{
		if (key_pressed)
		{
			backtrack_toggled = !backtrack_toggled;
		}
		return backtrack_toggled;
	}
	
	// Mode 2 = Always: always active if enabled (and keybind is set)
	if (globals::backtrack_mode == 2)
	{
		return true;
	}
	
	return false;
}

void enhance::modules::backtrack::run()
{
	// Only initialize hook when game is ready
		if (!hook_initialized)
		{
			// Wait for Minecraft instance to be ready before initializing hook
			if (sdk::instance && enhance::instance && enhance::instance->get_env())
			{
				// Try to get a player to ensure game is loaded
				jobject test_player = sdk::instance->get_player();
				if (test_player)
				{
					enhance::instance->get_env()->DeleteLocalRef(test_player);
					if (backtrack_hook::init())
					{
						hook_initialized = true;
						logger::log_debug("[Backtrack] Hook initialized successfully");
					}
					else
					{
						logger::log_error("[Backtrack] Failed to initialize hook");
					}
				}
			}
		}
		
		if (!globals::backtrack_enabled)
		{
			reset();
			backtrack_toggled = false;
			last_key_state = false;
			is_active = false;
			backtrack_hook::set_delay_enabled(false);
			return;
		}
		
		// Safety: Don't enable delay if hook isn't initialized
		if (!hook_initialized)
		{
			return;
		}

	// Check keybind based on mode (hold/toggle/always)
	bool keybind_active = is_keybind_active();
	if (!keybind_active)
	{
		reset();
		// Reset toggle state when keybind becomes inactive in hold mode
		if (globals::backtrack_mode == 0)
		{
			backtrack_toggled = false;
		}
		is_active = false;
		backtrack_hook::set_delay_enabled(false);
		return;
	}

	// Check cooldown
	ULONGLONG current_time = GetTickCount64();
	if (current_time < last_cooldown_end)
	{
		is_active = false;
		backtrack_hook::set_delay_enabled(false);
		return;
	}

	// Check knockback disable
	if (globals::backtrack_disable_on_hit && current_time < knockback_disable_until)
	{
		is_active = false;
		backtrack_hook::set_delay_enabled(false);
		return;
	}

	// Get local player for distance calculations and knockback detection
	jobject local_player = sdk::instance->get_player();
	if (!local_player)
	{
		is_active = false;
		backtrack_hook::set_delay_enabled(false);
		return;
	}

	sdk::entity_client local_entity_client(local_player);
	double local_x = local_entity_client.get_x();
	double local_y = local_entity_client.get_y();
	double local_z = local_entity_client.get_z();

	// Detect knockback by monitoring velocity
	if (globals::backtrack_disable_on_hit)
	{
		jobject velocity = local_entity_client.get_velocity();
		double current_velocity_magnitude = get_velocity_magnitude(velocity);
		
		// If velocity suddenly increased significantly, we took knockback
		if (last_local_velocity_magnitude > 0.0 && current_velocity_magnitude > last_local_velocity_magnitude * 1.5)
		{
			knockback_disable_until = current_time + 500; // Disable for 500ms
		}
		
		last_local_velocity_magnitude = current_velocity_magnitude;
	}

	// Get world and players
	jobject world = sdk::instance->get_world();
	if (!world)
	{
		is_active = false;
		backtrack_hook::set_delay_enabled(false);
		return;
	}

	sdk::world_client world_client(world);
	std::vector<jobject> players = world_client.get_players();

	auto env = enhance::instance->get_env();
	if (!env)
	{
		for (jobject player : players)
		{
			if (player) env->DeleteLocalRef(player);
		}
		if (world) env->DeleteLocalRef(world);
		if (local_player) env->DeleteLocalRef(local_player);
		return;
	}

	ULONGLONG current_timestamp = GetTickCount64();
	ULONGLONG max_age_ms = globals::backtrack_max_delay_ms;
	
	std::vector<backtrack_player_data> players_data;
	bool found_valid_target = false;

	// Clean up old position history and track current players
	{
		std::lock_guard<std::mutex> lock(position_history_mutex);
		
		// Remove players that are no longer in the world
		std::vector<jobject> players_to_remove;
		for (auto& pair : player_position_history)
		{
			bool found = false;
			for (jobject player : players)
			{
				if (env->IsSameObject(pair.first, player))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				players_to_remove.push_back(pair.first);
			}
		}
		
		for (jobject player_to_remove : players_to_remove)
		{
			player_position_history.erase(player_to_remove);
			env->DeleteGlobalRef(player_to_remove);
		}
		
		// Process current players
		for (jobject player : players)
		{
			if (!player) continue;

			sdk::entity_client entity_client(player);
			if (entity_client.is_same_object(local_player))
				continue;

			double player_x = entity_client.get_x();
			double player_y = entity_client.get_y();
			double player_z = entity_client.get_z();
			float player_yaw = entity_client.get_yaw();
			float player_pitch = entity_client.get_pitch();

			// Calculate distance
			double distance = calculate_distance(local_x, local_y, local_z, player_x, player_y, player_z);

			// Check if within target distance range
			if (distance < globals::backtrack_min_distance || distance > globals::backtrack_max_distance)
			{
				continue;
			}

			// Check hurt time (i-frames)
			int hurt_time = get_hurt_time(player);
			if (hurt_time > 0 && hurt_time * 50 > globals::backtrack_max_hurt_time_ms) // 50ms per tick
			{
				continue;
			}

			// Create or update position record
			backtrack_position_record record;
			record.x = player_x;
			record.y = player_y;
			record.z = player_z;
			record.yaw = player_yaw;
			record.pitch = player_pitch;
			record.timestamp_ms = current_timestamp;

			// Get or create history for this player
			jobject global_player = nullptr;
			for (auto& pair : player_position_history)
			{
				if (env->IsSameObject(pair.first, player))
				{
					global_player = pair.first;
					break;
				}
			}

			if (!global_player)
			{
				global_player = env->NewGlobalRef(player);
				if (!global_player) continue;
			}

			player_position_history[global_player].push_back(record);

			// Remove old records
			while (!player_position_history[global_player].empty())
			{
				ULONGLONG age = current_timestamp - player_position_history[global_player].front().timestamp_ms;
				if (age > max_age_ms)
				{
					player_position_history[global_player].pop_front();
				}
				else
				{
					break;
				}
			}

			// Store visualization data
			jobject bounding_box_obj = entity_client.get_bounding_box();
			if (bounding_box_obj)
			{
				sdk::box_client box(bounding_box_obj);

				backtrack_player_data data;
				data.x = player_x;
				data.y = player_y;
				data.z = player_z;
				data.min_x = box.get_min_x();
				data.min_y = box.get_min_y();
				data.min_z = box.get_min_z();
				data.max_x = box.get_max_x();
				data.max_y = box.get_max_y();
				data.max_z = box.get_max_z();
				data.yaw = player_yaw;
				data.pitch = player_pitch;
				data.is_valid = true;

				players_data.push_back(data);
				found_valid_target = true;

				env->DeleteLocalRef(bounding_box_obj);
			}
		}
	}

	// Update visualization data
	{
		std::lock_guard<std::mutex> lock(backtrack_players_mutex);
		backtrack_players = players_data;
	}

	// Activate backtrack if we found a valid target
	if (found_valid_target)
	{
		is_active = true;
		backtrack_hook::set_delay_enabled(true);
		backtrack_hook::set_delay_ms(globals::backtrack_max_delay_ms);
	}
	else
	{
		is_active = false;
		backtrack_hook::set_delay_enabled(false);
	}

	// Cleanup
	for (jobject player : players)
	{
		if (player) env->DeleteLocalRef(player);
	}
	if (world) env->DeleteLocalRef(world);
	if (local_player) env->DeleteLocalRef(local_player);
}

void enhance::modules::backtrack::draw_indicators()
{
	try
	{
		if (!globals::backtrack_visualization_enabled) return;
		if (!GUI::get_is_init()) return;
		if (!is_active) return;
		if (!sdk::instance) return;

		ImGuiIO& io = ImGui::GetIO();
		int screen_width = (int)io.DisplaySize.x;
		int screen_height = (int)io.DisplaySize.y;
		if (screen_width <= 0 || screen_height <= 0 || screen_width > 10000 || screen_height > 10000) return;

		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		if (!draw_list) return;

		// Get camera data
		sdk::camera_data cam = sdk::instance->get_camera();
		if (!cam.valid) return;
		
		// Validate camera data
		if (!std::isfinite(cam.x) || !std::isfinite(cam.y) || !std::isfinite(cam.z) ||
		    !std::isfinite(cam.yaw) || !std::isfinite(cam.pitch) || !std::isfinite(cam.fov) ||
		    cam.fov <= 0 || cam.fov > 180) return;

		// Copy data under lock
		std::vector<backtrack_player_data> players_copy;
		{
			std::lock_guard<std::mutex> lock(backtrack_players_mutex);
			if (backtrack_players.empty()) return;
			players_copy = backtrack_players;
		}

	// Pre-compute view matrix
	compute_view_matrix((float)cam.x, (float)cam.y, (float)cam.z,
	                    cam.yaw, cam.pitch, cam.fov, screen_width, screen_height);

	// Get visualization color
	ImU32 indicator_color = IM_COL32(
		(int)(globals::backtrack_visualization_color.x * 255),
		(int)(globals::backtrack_visualization_color.y * 255),
		(int)(globals::backtrack_visualization_color.z * 255),
		(int)(globals::backtrack_visualization_color.w * 255)
	);

	float line_width = globals::backtrack_visualization_line_width;

		for (const auto& player : players_copy)
		{
			try
			{
				if (!player.is_valid) continue;

				// Validate player position data
				if (!std::isfinite(player.x) || !std::isfinite(player.y) || !std::isfinite(player.z) ||
				    !std::isfinite(player.min_x) || !std::isfinite(player.min_y) || !std::isfinite(player.min_z) ||
				    !std::isfinite(player.max_x) || !std::isfinite(player.max_y) || !std::isfinite(player.max_z))
				{
					continue; // Skip invalid data
				}

				// Get the real position from history (most recent position record)
				// The player.x/y/z are the positions we're tracking, which should be the real server positions
				// since we're reading directly from entities before packet delay affects them
				double real_x = player.x;
				double real_y = player.y;
				double real_z = player.z;
				
				// Try to get the most recent position from history if available
				// This gives us the actual server position while the client shows delayed position
				{
					try
					{
						std::lock_guard<std::mutex> lock(position_history_mutex);
						// Find this player in history
						for (const auto& pair : player_position_history)
						{
							// We can't easily match by jobject here, so we use the stored position
							// The history contains the real positions we tracked
							if (!pair.second.empty())
							{
								const auto& latest = pair.second.back();
								// Validate latest position
								if (std::isfinite(latest.x) && std::isfinite(latest.y) && std::isfinite(latest.z))
								{
									// Use latest position if it's close (same player)
									double dist = calculate_distance(real_x, real_y, real_z, latest.x, latest.y, latest.z);
									if (dist < 0.1) // Very close, likely same player
									{
										real_x = latest.x;
										real_y = latest.y;
										real_z = latest.z;
										break;
									}
								}
							}
						}
					}
					catch (...)
					{
						// If history access fails, just use player position
					}
				}

				// Validate real position
				if (!std::isfinite(real_x) || !std::isfinite(real_y) || !std::isfinite(real_z))
				{
					continue;
				}

				// 8 corners of bounding box at real position
				float corners[8][3] = {
					{(float)(real_x + player.min_x - player.x), (float)(real_y + player.min_y - player.y), (float)(real_z + player.min_z - player.z)},
					{(float)(real_x + player.max_x - player.x), (float)(real_y + player.min_y - player.y), (float)(real_z + player.min_z - player.z)},
					{(float)(real_x + player.max_x - player.x), (float)(real_y + player.min_y - player.y), (float)(real_z + player.max_z - player.z)},
					{(float)(real_x + player.min_x - player.x), (float)(real_y + player.min_y - player.y), (float)(real_z + player.max_z - player.z)},
					{(float)(real_x + player.min_x - player.x), (float)(real_y + player.max_y - player.y), (float)(real_z + player.min_z - player.z)},
					{(float)(real_x + player.max_x - player.x), (float)(real_y + player.max_y - player.y), (float)(real_z + player.min_z - player.z)},
					{(float)(real_x + player.max_x - player.x), (float)(real_y + player.max_y - player.y), (float)(real_z + player.max_z - player.z)},
					{(float)(real_x + player.min_x - player.x), (float)(real_y + player.max_y - player.y), (float)(real_z + player.max_z - player.z)}
				};

				// Validate corners
				for (int i = 0; i < 8; i++)
				{
					if (!std::isfinite(corners[i][0]) || !std::isfinite(corners[i][1]) || !std::isfinite(corners[i][2]))
					{
						continue; // Skip this player if corners are invalid
					}
				}

				// Project corners to screen
				float screen_x[8], screen_y[8];
				int valid_count = 0;
				float min_screen_x = FLT_MAX, max_screen_x = -FLT_MAX;
				float min_screen_y = FLT_MAX, max_screen_y = -FLT_MAX;

				for (int i = 0; i < 8; i++)
				{
					if (world_to_screen_fast(corners[i][0], corners[i][1], corners[i][2], screen_x[i], screen_y[i]))
					{
						// Validate screen coordinates
						if (std::isfinite(screen_x[i]) && std::isfinite(screen_y[i]) &&
						    screen_x[i] >= -1000 && screen_x[i] <= screen_width + 1000 &&
						    screen_y[i] >= -1000 && screen_y[i] <= screen_height + 1000)
						{
							if (screen_x[i] < min_screen_x) min_screen_x = screen_x[i];
							if (screen_x[i] > max_screen_x) max_screen_x = screen_x[i];
							if (screen_y[i] < min_screen_y) min_screen_y = screen_y[i];
							if (screen_y[i] > max_screen_y) max_screen_y = screen_y[i];
							valid_count++;
						}
					}
				}

				if (valid_count < 4) continue;
				
				// Validate screen bounds before drawing
				if (!std::isfinite(min_screen_x) || !std::isfinite(max_screen_x) ||
				    !std::isfinite(min_screen_y) || !std::isfinite(max_screen_y) ||
				    min_screen_x < -10000 || max_screen_x > 20000 ||
				    min_screen_y < -10000 || max_screen_y > 20000)
				{
					continue; // Skip drawing if coordinates are invalid
				}

				// Draw box
				try
				{
					if (globals::backtrack_visualization_filled)
					{
						draw_list->AddRectFilled(ImVec2(min_screen_x, min_screen_y), ImVec2(max_screen_x, max_screen_y), indicator_color);
					}
					else
					{
						draw_list->AddRect(ImVec2(min_screen_x, min_screen_y), ImVec2(max_screen_x, max_screen_y), indicator_color, 0.0f, 0, line_width);
					}
				}
				catch (...)
				{
					// Skip drawing on error
				}
			}
			catch (...)
			{
				// Skip this player on error, continue with next
			}
		}
	}
	catch (...)
	{
		// Silent catch - don't crash on drawing errors
	}
}

void enhance::modules::backtrack::reset()
{
	is_active = false;
	backtrack_hook::set_delay_enabled(false);
	
	// Clear position history
	{
		std::lock_guard<std::mutex> lock(position_history_mutex);
		auto env = enhance::instance->get_env();
		if (env)
		{
			for (auto& pair : player_position_history)
			{
				env->DeleteGlobalRef(pair.first);
			}
		}
		player_position_history.clear();
	}
	
	// Clear visualization data
	{
		std::lock_guard<std::mutex> lock(backtrack_players_mutex);
		backtrack_players.clear();
	}
	
	// Update cooldown
	if (is_active)
	{
		last_cooldown_end = GetTickCount64() + (ULONGLONG)(globals::backtrack_cooldown_seconds * 1000.0f);
	}
}

void enhance::modules::backtrack::reset_hook_state()
{
	hook_initialized = false;
}
