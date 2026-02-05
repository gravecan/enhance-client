#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class backtrack
		{
		public:
			static void run();
			static void draw_indicators();
			static void reset();
			static void reset_hook_state();
		};
	}
}

struct backtrack_position_record
{
	double x, y, z;
	float yaw, pitch;
	ULONGLONG timestamp_ms;
};

struct backtrack_player_data
{
	double x, y, z;
	double min_x, min_y, min_z;
	double max_x, max_y, max_z;
	float yaw, pitch;
	bool is_valid;
};
