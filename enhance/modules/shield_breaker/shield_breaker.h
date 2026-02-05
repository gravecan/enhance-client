#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class shield_breaker
		{
		public:
			static void run();

		private:
			static int find_item_in_hotbar(jobject player, const char* item_name);
			static bool is_item(jobject item_stack, const char* item_name);
			static void swap_to_slot(int slot);
			static int get_current_slot(jobject player);
			static void aim_at_entity(jobject player, jobject target_entity);
			static bool is_using_shield(jobject target_entity);
			static double calculate_distance(double x1, double y1, double z1, double x2, double y2, double z2);
			static void send_double_attack();
		};
	}
}

