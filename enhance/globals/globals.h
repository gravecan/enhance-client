#pragma once

namespace globals
{
	inline bool show_gui = false;
	inline bool flight_enabled = false;
	inline HWND mc_window = nullptr;
	
	inline bool hitbox_enabled = false;
	inline double hitbox_expand_width = 0.3;
	inline double hitbox_expand_height = 0.0;
	inline int hitbox_keybind = 0;
	inline int hitbox_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	
	inline bool sprint_enabled = false;
	inline bool triggerbot_enabled = false;
	inline int triggerbot_mode = 0; // 0 = Custom Delay, 1 = Weapon Cooldown, 2 = Combo
	inline int triggerbot_delay_ms = 50;
	inline int triggerbot_min_delay_ms = 40;
	inline int triggerbot_max_delay_ms = 80;
	inline int triggerbot_crit_mode = 0; // 0 = Off, 1 = Crit Only, 2 = Priority Crit
	inline bool triggerbot_hit_select = true; // Only hit when cooldown is ready
	inline int triggerbot_keybind = 0;
	inline int triggerbot_keybind_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline bool triggerbot_weapon_only = false; // Only trigger when holding weapon
	
	inline bool reach_enabled = false;
	inline double reach_distance = 3.0;
	inline int reach_keybind = 0;
	inline int reach_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	
	inline bool aimassist_enabled = false;
	inline float aimassist_smoothing = 0.3f;
	inline double aimassist_max_distance = 10.0;
	inline int aimassist_keybind = 0;
	inline int aimassist_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline bool aimassist_horizontal = true; // Yaw aiming
	inline bool aimassist_vertical = true; // Pitch aiming
	
	inline int pearl_catch_keybind = 0;
	inline int pearl_catch_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline int pearl_catch_aim_mode = 0; // 0 = Silent (Detected), 1 = Visible
	
	inline bool box_enabled = false;
	inline int esp_keybind = 0;
	inline int esp_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline bool esp_health_bar = false;
	
	inline bool mace_enabled = false;
	inline bool mace_look = false;
	inline bool mace_switch_back = false;
	inline bool mace_remove_elytra = false;
	inline double mace_min_fall_distance = 3.0;
	inline double mace_height_above_target = 0.0;
	inline double mace_fall_hitbox_width = 0.3;
	inline double mace_fall_hitbox_height = 0.0;
	inline int mace_keybind = 0;
	inline int mace_keybind_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	
	// Mace state tracking
	inline int mace_saved_slot = -1;
	inline ULONGLONG mace_last_attack = 0;
	inline bool mace_elytra_swapped_this_fall = false;
	inline bool mace_was_in_fall_distance = false;
	
	// Shield breaker
	inline bool shield_breaker_enabled = false;
	inline bool shield_breaker_aim = false;
	inline bool shield_breaker_switch_back = false;
	inline int shield_breaker_saved_slot = -1;
	inline ULONGLONG shield_breaker_last_attack = 0;
	inline int shield_breaker_delay_ms = 0;
	inline int shield_breaker_keybind = 0;
	inline int shield_breaker_keybind_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	
	// Aimassist
	inline bool aimassist_use_mouse_input = false;
	inline float aimassist_mouse_sensitivity = 0.5f; // Mouse sensitivity multiplier
	
	// Triggerbot Shield Use (use shield after tbot hit)
	inline bool triggerbot_use_shield = false;
	inline int triggerbot_shield_duration_ms = 300; // How long to hold shield
	inline ULONGLONG triggerbot_shield_start_time = 0;
	inline bool triggerbot_shield_holding = false; // Are we currently holding shield
	
	// Triggerbot Shield Check (check if target is using shield)
	inline bool triggerbot_check_shield = false;
	inline int triggerbot_shield_action = 0; // 0 = Don't click, 1 = Spam click
	
	// Server Rotation / Silent Aim
	inline bool server_rotation_enabled = false;
	inline int server_rotation_mode = 0; // 0 = Look Up, 1 = Look Down, 2 = Look Behind, 3 = Custom
	inline float server_rotation_custom_yaw = 0.0f;
	inline float server_rotation_custom_pitch = -90.0f;
	
	// Stun Slam (shield break + mace combo when falling)
	inline bool stun_slam_enabled = false;
	inline float stun_slam_chance = 100.0f; // Chance percentage
	inline int stun_slam_swap_delay_ms = 10; // Delay for swapping items (fast)
	inline int stun_slam_axe_delay_ms = 20; // Delay after axe hit
	inline int stun_slam_mace_delay_ms = 20; // Delay after mace hit
	inline double stun_slam_min_fall = 1.5; // Lower default for easier triggering
	
	// S-Tap (briefly press S to reset sprint) - uses actual key simulation
	inline bool stap_enabled = false;
	inline int stap_duration_ms = 50; // How long to press S (50-100ms typical)
	inline ULONGLONG stap_start_time = 0;
	inline bool stap_is_active = false;
	
	// W-Tap (briefly release W to reset sprint) - uses actual key simulation
	inline bool wtap_enabled = false;
	inline int wtap_duration_ms = 50; // How long to release W (50-100ms typical)
	inline ULONGLONG wtap_start_time = 0;
	inline bool wtap_is_active = false;
	
	// Anchor Macro (keybind to place, charge, and explode anchor)
	inline bool anchor_macro_enabled = false;
	inline int anchor_macro_keybind = 0; // 0 = none, VK code otherwise
	inline int anchor_macro_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline bool anchor_macro_break_anchor = true; // Whether to break the anchor at the end
	inline int anchor_macro_swap_delay_ms = 80; // Delay for swapping items
	inline int anchor_macro_charge_delay_ms = 120; // Delay before charging
	inline int anchor_macro_break_delay_ms = 80; // Delay before breaking
	inline bool anchor_macro_executing = false;
	inline bool anchor_macro_toggled = false; // For toggle mode
	inline int anchor_macro_step = 0;
	inline ULONGLONG anchor_macro_last_action = 0;
	inline int anchor_macro_saved_slot = -1;
	
	// Storage ESP
	inline bool storage_esp_enabled = false;
	inline int storage_esp_keybind = 0;
	inline int storage_esp_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline bool storage_esp_chest = true;
	inline bool storage_esp_ender_chest = true;
	inline bool storage_esp_shulker = true;
	
	// AutoCrystal
	inline bool autocrystal_enabled = false;
	inline int autocrystal_keybind = 0;
	inline int autocrystal_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline int autocrystal_delay_ms = 50; // Delay between place and break
	inline bool autocrystal_debug_enabled = false; // Debug logging
	
	// Auto Totem
	inline bool autototem_enabled = false;
	inline int autototem_keybind = 0;
	inline int autototem_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline bool autototem_rage_mode = false; // Rage mode (uses different packet sequence)
	
	// Auto Jump Reset
	inline bool autojumpreset_enabled = false;
	inline int autojumpreset_keybind = 0;
	inline int autojumpreset_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline int autojumpreset_cooldown_ms = 100; // Cooldown between jumps
	inline ULONGLONG autojumpreset_last_jump = 0;
	inline float autojumpreset_last_health = 0.0f;
	
	// Backtrack
	inline bool backtrack_enabled = false;
	inline int backtrack_keybind = 0;
	inline int backtrack_mode = 2; // 0 = Hold, 1 = Toggle, 2 = Always
	inline double backtrack_min_distance = 1.0;
	inline double backtrack_max_distance = 4.0;
	inline int backtrack_max_delay_ms = 200; // Maximum delay in milliseconds
	inline float backtrack_cooldown_seconds = 0.0f; // Cooldown before reactivation
	inline int backtrack_max_hurt_time_ms = 0; // Maximum hurt time (i-frames) to activate
	inline bool backtrack_disable_on_hit = false; // Disable when taking knockback
	inline bool backtrack_visualization_enabled = true;
	inline ImVec4 backtrack_visualization_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red by default
	inline float backtrack_visualization_line_width = 1.0f;
	inline bool backtrack_visualization_filled = false; // Outline by default
	inline bool backtrack_visualization_show_head_rotation = false;
}
