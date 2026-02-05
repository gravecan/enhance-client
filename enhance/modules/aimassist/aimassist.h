#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class aimassist
		{
		public:
			static void run();

		private:
			static void calculate_angles(double px, double py, double pz, double ex, double ey, double ez, float& yaw, float& pitch);
			static float smooth_angle(float current, float target, float smoothing);
			static double calculate_distance(double x1, double y1, double z1, double x2, double y2, double z2);
			static void move_mouse(long dx, long dy);
		};
	}
}
