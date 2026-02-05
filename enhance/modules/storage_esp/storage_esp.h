#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class storage_esp
		{
		public:
			static void run();
			static void draw_boxes();
		};
	}
}

struct storage_block_data
{
	double x, y, z;  // Block position (center)
	int type;        // 0 = chest, 1 = ender chest, 2 = shulker box
};

struct storage_esp_camera_data
{
	double cam_x, cam_y, cam_z;
	float yaw, pitch;
	float fov;
};
