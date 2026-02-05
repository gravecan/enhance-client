#include "esp.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../gui/GUI.h"
#include "../../utils/logger.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/minecraft/util/box.h>
#include <sdk/classloader.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <vector>
#include <mutex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::vector<esp_player_data> esp_players;
static esp_camera_data esp_camera;
static std::mutex esp_players_mutex;

void enhance::modules::esp::run()
{
	try
	{
		jobject world = sdk::instance->get_world();
		if (!world) return;

		jobject local_player = sdk::instance->get_player();
		if (!local_player)
		{
			auto env = enhance::instance->get_env();
			if (env && world)
				env->DeleteLocalRef(world);
			return;
		}

		sdk::world_client world_client(world);
		std::vector<jobject> players = world_client.get_players();

		auto env = enhance::instance->get_env();
		if (!env) return;

		jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
		if (!entity_class)
		{
			for (jobject player : players)
			{
				if (player)
					env->DeleteLocalRef(player);
			}
			if (world)
				env->DeleteLocalRef(world);
			if (local_player)
				env->DeleteLocalRef(local_player);
			return;
		}

		jmethodID set_flag_method = env->GetMethodID(entity_class, sdk::mappings::entity_set_flag_name, sdk::mappings::entity_set_flag_sig);
		if (env->ExceptionCheck()) env->ExceptionClear();
		env->DeleteLocalRef(entity_class);

		if (!set_flag_method)
		{
			for (jobject player : players)
			{
				if (player)
					env->DeleteLocalRef(player);
			}
			if (world)
				env->DeleteLocalRef(world);
			if (local_player)
				env->DeleteLocalRef(local_player);
			return;
		}

		for (jobject player : players)
		{
			if (!player) continue;

			sdk::entity_client entity_client(player);
			if (entity_client.is_same_object(local_player))
				continue;

			// Disable glow effect (flag 6) - we only use 2D box ESP now
				env->CallVoidMethod(player, set_flag_method, 6, JNI_FALSE);
		}

		if (globals::box_enabled || globals::esp_health_bar)
		{
			std::vector<esp_player_data> players_data;

			// Get actual camera data from the game
			sdk::camera_data cam = sdk::instance->get_camera();
			if (!cam.valid)
			{
				// Fallback to player position if camera not available
				sdk::entity_client local_entity_client(local_player);
				cam.x = local_entity_client.get_x();
				cam.y = local_entity_client.get_y() + 1.62;
				cam.z = local_entity_client.get_z();
				cam.yaw = local_entity_client.get_yaw();
				cam.pitch = local_entity_client.get_pitch();
				cam.fov = 70.0f;
				}

			esp_camera_data camera;
			camera.cam_x = cam.x;
			camera.cam_y = cam.y;
			camera.cam_z = cam.z;
			camera.yaw = cam.yaw;
			camera.pitch = cam.pitch;
			camera.fov = cam.fov;

			for (jobject player : players)
			{
				if (!player) continue;

				try
				{
					sdk::entity_client entity_client(player);
					if (entity_client.is_same_object(local_player))
						continue;

					jobject bounding_box_obj = entity_client.get_bounding_box();
					if (!bounding_box_obj) continue;

					sdk::box_client box(bounding_box_obj);

					esp_player_data data;
					data.x = entity_client.get_x();
					data.y = entity_client.get_y();
					data.z = entity_client.get_z();
					data.min_x = box.get_min_x();
					data.min_y = box.get_min_y();
					data.min_z = box.get_min_z();
					data.max_x = box.get_max_x();
					data.max_y = box.get_max_y();
					data.max_z = box.get_max_z();
					
					// Get health data if health bar is enabled
					data.health = 20.0f;
					data.max_health = 20.0f;
					if (globals::esp_health_bar)
					{
						jclass living_entity_class = sdk::classloader::find_class(env, sdk::mappings::living_entity_class_sig);
						if (living_entity_class)
						{
							jmethodID get_health_mid = env->GetMethodID(living_entity_class, sdk::mappings::living_entity_get_health_name, sdk::mappings::living_entity_get_health_sig);
							if (env->ExceptionCheck()) env->ExceptionClear();
							
							jmethodID get_max_health_mid = env->GetMethodID(living_entity_class, sdk::mappings::living_entity_get_max_health_name, sdk::mappings::living_entity_get_max_health_sig);
							if (env->ExceptionCheck()) env->ExceptionClear();
							
							if (get_health_mid && get_max_health_mid)
							{
								data.health = env->CallFloatMethod(player, get_health_mid);
								if (env->ExceptionCheck()) { env->ExceptionClear(); data.health = 20.0f; }
								
								data.max_health = env->CallFloatMethod(player, get_max_health_mid);
								if (env->ExceptionCheck()) { env->ExceptionClear(); data.max_health = 20.0f; }
							}
							env->DeleteLocalRef(living_entity_class);
						}
					}

					players_data.push_back(data);
					env->DeleteLocalRef(bounding_box_obj);
				}
				catch (...) {}
			}

			{
				std::lock_guard<std::mutex> lock(esp_players_mutex);
				esp_players = players_data;
				esp_camera = camera;
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(esp_players_mutex);
			esp_players.clear();
		}

		for (jobject player : players)
		{
			if (player)
				env->DeleteLocalRef(player);
		}

		if (world)
			env->DeleteLocalRef(world);
		if (local_player)
			env->DeleteLocalRef(local_player);
	}
	catch (const std::exception& e)
	{
	}
	catch (...)
	{
	}
}

// Pre-computed view matrix for fast world-to-screen
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

// Fast world-to-screen using pre-computed matrix
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

void enhance::modules::esp::draw_boxes()
	{
		if (!globals::box_enabled && !globals::esp_health_bar) return;
		if (!GUI::get_is_init()) return;

		ImGuiIO& io = ImGui::GetIO();
		int screen_width = (int)io.DisplaySize.x;
		int screen_height = (int)io.DisplaySize.y;
		if (screen_width <= 0 || screen_height <= 0) return;

		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		if (!draw_list) return;

	// Copy data under lock
	std::vector<esp_player_data> players_copy;
	esp_camera_data camera;
	{
		std::lock_guard<std::mutex> lock(esp_players_mutex);
		if (esp_players.empty()) return;
		players_copy = esp_players;
		camera = esp_camera;
	}

	// Pre-compute view matrix once per frame
	compute_view_matrix((float)camera.cam_x, (float)camera.cam_y, (float)camera.cam_z,
	                    camera.yaw, camera.pitch, camera.fov, screen_width, screen_height);

	// Pre-define colors
	const ImU32 white_color = IM_COL32(255, 255, 255, 255);
	const ImU32 black_color = IM_COL32(0, 0, 0, 255);

		for (const auto& player : players_copy)
		{
		// 8 corners of bounding box
		float corners[8][3] = {
			{(float)player.min_x, (float)player.min_y, (float)player.min_z},
			{(float)player.max_x, (float)player.min_y, (float)player.min_z},
			{(float)player.max_x, (float)player.min_y, (float)player.max_z},
			{(float)player.min_x, (float)player.min_y, (float)player.max_z},
			{(float)player.min_x, (float)player.max_y, (float)player.min_z},
			{(float)player.max_x, (float)player.max_y, (float)player.min_z},
			{(float)player.max_x, (float)player.max_y, (float)player.max_z},
			{(float)player.min_x, (float)player.max_y, (float)player.max_z}
				};

		// Project corners to screen
		float screen_x[8], screen_y[8];
		int valid_count = 0;
		float min_x = FLT_MAX, max_x = -FLT_MAX;
		float min_y = FLT_MAX, max_y = -FLT_MAX;

				for (int i = 0; i < 8; i++)
				{
			if (world_to_screen_fast(corners[i][0], corners[i][1], corners[i][2], screen_x[i], screen_y[i]))
					{
				if (screen_x[i] < min_x) min_x = screen_x[i];
				if (screen_x[i] > max_x) max_x = screen_x[i];
				if (screen_y[i] < min_y) min_y = screen_y[i];
				if (screen_y[i] > max_y) max_y = screen_y[i];
				valid_count++;
				}
		}

		if (valid_count < 4) continue;

		// Draw box with outlines (only if box ESP is enabled)
		if (globals::box_enabled) {
			draw_list->AddRect(ImVec2(min_x - 1, min_y - 1), ImVec2(max_x + 1, max_y + 1), black_color, 0.0f, 0, 1.0f);
			draw_list->AddRect(ImVec2(min_x, min_y), ImVec2(max_x, max_y), white_color, 0.0f, 0, 1.0f);
			draw_list->AddRect(ImVec2(min_x + 1, min_y + 1), ImVec2(max_x - 1, max_y - 1), black_color, 0.0f, 0, 1.0f);
		}
		
		// Draw health bar on the left side of the box
		if (globals::esp_health_bar && player.max_health > 0)
		{
			float health_percent = player.health / player.max_health;
			if (health_percent > 1.0f) health_percent = 1.0f;
			if (health_percent < 0.0f) health_percent = 0.0f;
			
			float bar_width = 3.0f;
			float bar_height = max_y - min_y;
			float bar_x = min_x - bar_width - 3.0f;
			float filled_height = bar_height * health_percent;
			
			// Health color gradient: green to yellow to red
			ImU32 health_color;
			if (health_percent > 0.5f)
			{
				// Green to Yellow
				float t = (health_percent - 0.5f) * 2.0f;
				health_color = IM_COL32((int)(255 * (1.0f - t)), 255, 0, 255);
			}
			else
			{
				// Yellow to Red
				float t = health_percent * 2.0f;
				health_color = IM_COL32(255, (int)(255 * t), 0, 255);
			}
			
			// Draw background
			draw_list->AddRectFilled(ImVec2(bar_x - 1, min_y - 1), ImVec2(bar_x + bar_width + 1, max_y + 1), black_color);
			// Draw empty bar
			draw_list->AddRectFilled(ImVec2(bar_x, min_y), ImVec2(bar_x + bar_width, max_y), IM_COL32(50, 50, 50, 200));
			// Draw filled health
			draw_list->AddRectFilled(ImVec2(bar_x, max_y - filled_height), ImVec2(bar_x + bar_width, max_y), health_color);
		}
			}
}
