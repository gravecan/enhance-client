#pragma once

#include <sdk/includes.h>
#include <Windows.h>

namespace enhance
{
	namespace modules
	{
		class autocrystal
		{
		public:
			static void run();

		private:
			static int find_item_in_inventory(jobject player, const char* item_name);
			static void swap_to_slot(int slot);
			static int get_current_slot(jobject player);
			static bool is_item(jobject item_stack, const char* item_name);
			static void right_click();
			static void left_click();
		};
	}
}

