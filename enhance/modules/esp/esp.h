#pragma once

#include <sdk/includes.h>
#include <sdk/minecraft/util/box.h>

namespace enhance
{
	namespace modules
	{
		class esp
		{
		public:
			static void run();
			static void draw_boxes();
		};
	}
}

struct esp_player_data
{
	double x, y, z;
	double min_x, min_y, min_z;
	double max_x, max_y, max_z;
	float health;
	float max_health;
};

struct esp_camera_data
{
	double cam_x, cam_y, cam_z;
	float yaw, pitch;
	float fov;
};

enum class ESPMode
{
	GLOW = 0,
	OUTLINE = 1,
	BOX = 2
};


