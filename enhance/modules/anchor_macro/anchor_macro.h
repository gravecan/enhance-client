#pragma once

#include <jni.h>

namespace enhance
{
	namespace modules
	{
		namespace anchor_macro
		{
			void run();
			int find_item_in_hotbar(jobject player, const char* item_name);
			bool is_item(jobject item_stack, const char* item_name);
			void swap_to_slot(int slot);
			int get_current_slot(jobject player);
			bool is_holding_anchor();
			void send_right_click();
			void send_left_click();
		}
	}
}

