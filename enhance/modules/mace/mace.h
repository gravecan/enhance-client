#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class mace
		{
		public:
			static void run();

		private:
			static int find_item_in_hotbar(jobject player, const char* item_name);
			static bool is_item(jobject item_stack, const char* item_name);
			static void swap_to_slot(int slot);
			static int get_current_slot(jobject player);
			static void aim_at_entity(jobject player, jobject target_entity);
			static bool has_elytra(jobject player);
			static void swap_elytra_to_chestplate(jobject player);
			static void use_item();
		};
	}
}

