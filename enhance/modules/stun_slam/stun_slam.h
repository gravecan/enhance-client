#pragma once

#include <jni.h>

namespace enhance
{
	namespace modules
	{
		namespace stun_slam
		{
			void run();
			int find_item_in_hotbar(jobject player, const char* item_name);
			bool is_item(jobject item_stack, const char* item_name);
			void swap_to_slot(int slot);
			int get_current_slot(jobject player);
			bool is_using_shield(jobject target_entity);
			void send_attack();
			bool should_activate(); // Random chance check
		}
	}
}

