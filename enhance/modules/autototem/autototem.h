#pragma once

#include <sdk/includes.h>
#include <Windows.h>

namespace enhance
{
	namespace modules
	{
		class autototem
		{
		public:
			static void run();

	private:
		static bool has_totem_in_offhand(jobject player);
		static int find_totem_in_inventory(jobject player);
		static bool is_item(jobject item_stack, const char* item_name);
		static void swap_to_offhand_blatant(int slot);
		static void swap_to_offhand_legit(int slot);
		static void open_inventory();
		static void close_inventory();
		static void move_mouse_to_slot(int slot);
		static void click_slot(int slot);
		static void send_update_selected_slot_packet(int slot);
		static void send_player_action_packet(jobject block_pos, int action);
		static void send_pick_from_inventory_packet(int slot);
		static jobject get_block_pos_origin();
		};
	}
}

