#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class pearl_catch
		{
		public:
			static void run();

		private:
			static int find_item_in_hotbar(jobject player, const char* item_name);
			static bool is_item(jobject item_stack, const char* item_name);
			static void swap_to_slot(int slot);
		};
	}
}
