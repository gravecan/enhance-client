#define IMGUI_DEFINE_MATH_OPERATORS

#include "GUI.h"
#include "../globals/globals.h"
#include "../modules/esp/esp.h"
#include "../modules/gambling/gambling.hpp"
// #include "../modules/backtrack/backtrack.h" // Disabled
#include "../utils/logger.h"
#include "../utils/http_client.h"
#include <stdio.h>

void UpdateWolfMenuDPI();

#define STB_IMAGE_IMPLEMENTATION

#define WOLF_MENU_INCLUDED_MODE
#include "wolf_menu.cpp"
#undef WOLF_MENU_INCLUDED_MODE

static bool is_init{};
static bool do_draw{false};


bool GUI::init(HWND wnd_handle)
{
	if (is_init)
		return false;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	ImGui::StyleColorsDark();
	
	ImGui_ImplWin32_Init(wnd_handle);
	ImGui_ImplOpenGL3_Init();

	http_client::Initialize();

	is_init = true;

	return false;
}

void GUI::shutdown()
{
	if (!is_init)
		return;

	try
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		
		http_client::Cleanup();
	}
	catch (...)
	{
	}

	is_init = false;
	do_draw = false;
}

bool IsLoggedIn();
void RenderLoginScreen();
void RenderGamblingWindow();

void GUI::draw()
{
	bool should_draw_gui = do_draw || globals::flight_enabled;
	bool should_draw_box = globals::box_enabled || globals::esp_health_bar;
	
	if (!should_draw_gui && !should_draw_box)
		return;

	if (should_draw_gui)
		UpdateWolfMenuDPI();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = do_draw;

	if (should_draw_box)
	{
		try
		{
			enhance::modules::esp::draw_boxes();
		}
		catch (...)
		{
		}
	}
	
	// Backtrack visualization disabled

	if (do_draw)
	{
		if (!IsLoggedIn())
		{
            RenderWolfMenu();
			//RenderLoginScreen();
		}
		else
		{
			RenderWolfMenu();
		}
		
		if (enhance::modules::gambling::g_mines_game.isWindowOpen()) {
			RenderGamblingWindow();
		}
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RenderWolfMenu() {
    extern bool g_child_consumed_scroll;
    g_child_consumed_scroll = false;
    
    if (!g_clr.is_valid()) {
        g_clr.reset_to_defaults();
    }
    
    ApplyTheme(g_theme);
    
    for (auto& popup : g_popup_storage) {
        easing(popup.alpha, popup.open ? 1.f : 0.f, 9.f, static_easing);
    }
    
    c_vec2 window_size = s_(g_elem.window.size);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | 
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c_vec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(8, 8));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, c_vec4(0, 0, 0, 0));
    
    if (ImGui::Begin(g_elem.window.name.c_str(), nullptr, flags)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        c_vec2 win_pos = ImGui::GetWindowPos();
        c_vec2 win_size = ImGui::GetWindowSize();
        
        rect_filled(dl, win_pos, win_pos + win_size, get_clr(g_clr.layout), s_(g_elem.window.rounding), 0);
        
        ImGui::SetCursorPos(c_vec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c_vec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, c_vec2(0, 0));
        ImGui::BeginChild("Sidebar", s_(g_elem.sidebar.size), false, 
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysUseWindowPadding |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
        {
            rect_filled(ImGui::GetWindowDrawList(), ImGui::GetWindowPos(), 
                       ImGui::GetWindowPos() + ImGui::GetWindowSize(), 
                       get_clr(g_clr.child), s_(g_elem.window.rounding), ImDrawFlags_RoundCornersLeft);
            
            Logo();
            static int logo_click_count = 0;
            static float logo_click_reset_time = 0.0f;
            if (ImGui::IsItemClicked(0)) {
                float current_time = ImGui::GetTime();
                if (current_time - logo_click_reset_time > 2.0f) {
                    logo_click_count = 0;
                }
                logo_click_count++;
                logo_click_reset_time = current_time;
                if (logo_click_count >= 5) {
                    enhance::modules::gambling::g_mines_game.openWindow();
                    logo_click_count = 0;
                }
            }
            if (TabButton("ASSIST", "H", g_tab_stored == 0)) g_tab_stored = 0;
            if (TabButton("VISUALS", "B", g_tab_stored == 1)) g_tab_stored = 1;
            if (TabButton("MISC", "A", g_tab_stored == 2)) g_tab_stored = 2;
            if (TabButton("CONFIG", "J", g_tab_stored == 3)) g_tab_stored = 3;
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        
        float scrollbar_w = s_(6);
        float content_width = win_size.x - s_(g_elem.sidebar.size.x) - s_(g_elem.settings_bar.size.x) - scrollbar_w;
        c_vec2 content_pos = win_pos + c_vec2(s_(g_elem.sidebar.size.x), 0);
        c_vec2 content_size_full = c_vec2(content_width + scrollbar_w, win_size.y);
        
        c_rect scrollbar_track(c_vec2(content_pos.x + content_width + s_(2), content_pos.y + s_(4)),
                               c_vec2(content_pos.x + content_width + scrollbar_w - s_(1), content_pos.y + win_size.y - s_(4)));
        
        ImGui::SetCursorPos(c_vec2(s_(g_elem.sidebar.size.x), 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, s_(g_elem.content.padding));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(g_elem.content.padding));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0, 0, 0, 0));
        
        ImGui::BeginChild("Content", c_vec2(content_width, win_size.y), false,
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysUseWindowPadding |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoScrollbar);
        {
            ImGuiWindow* content_win = ImGui::GetCurrentWindow();
            
            if (g_tab_stored == 0) {
                float child_w = (content_avail_x() - s_(g_elem.content.padding.x)) / 2;
                float child_h = s_(250);
                
                c_vec2 row1_start = ImGui::GetCursorPos();
                
                if (BeginChildWindow("Combat", c_vec2(child_w, child_h))) {
                    Checkbox("Aimassist", "Auto aim at targets", &globals::aimassist_enabled);
                    if (globals::aimassist_enabled) {
                        Keybind("Keybind", "Activation key", &globals::aimassist_keybind, &globals::aimassist_mode);
                        static std::vector<bool> aimassist_axes = {globals::aimassist_horizontal, globals::aimassist_vertical};
                        static std::vector<std::string> axis_names = {"Horizontal (Yaw)", "Vertical (Pitch)"};
                        MultiDropdown("Axes", "Which axes to aim", &aimassist_axes, axis_names);
                        globals::aimassist_horizontal = aimassist_axes[0];
                        globals::aimassist_vertical = aimassist_axes[1];
                        Checkbox("Use Mouse Input", "Move via mouse input", &globals::aimassist_use_mouse_input);
                        if (globals::aimassist_use_mouse_input) {
                            Slider("MC Sensitivity", "Your in-game sensitivity (0-2)", &globals::aimassist_mouse_sensitivity, 0.01f, 2.0f, "%.2f");
                        }
                        float smoothing = globals::aimassist_smoothing;
                        Slider("Smoothing", "Aim interpolation", &smoothing, 0.0f, 1.0f, "%.2f");
                        globals::aimassist_smoothing = smoothing;
                        float max_dist = static_cast<float>(globals::aimassist_max_distance);
                        Slider("Max Distance", "Target range", &max_dist, 1.0f, 50.0f, "%.1f");
                        globals::aimassist_max_distance = static_cast<double>(max_dist);
					}
                    
                    Checkbox("Triggerbot", "Auto attack on target", &globals::triggerbot_enabled);
                    if (globals::triggerbot_enabled) {
                        Keybind("Keybind", "Activation key", &globals::triggerbot_keybind, &globals::triggerbot_keybind_mode);
                        static std::vector<std::string> trigger_modes = {"Custom Delay", "Weapon Cooldown", "Combo"};
                        Dropdown("Mode", "Timing mode", &globals::triggerbot_mode, trigger_modes);
                        Checkbox("Weapon Only", "Only with weapon in hand", &globals::triggerbot_weapon_only);
                        
                        if (globals::triggerbot_mode == 0) {
                            float delay_f = static_cast<float>(globals::triggerbot_delay_ms);
                            Slider("Delay", "Attack delay (ms)", &delay_f, 0.0f, 500.0f, "%.0f ms");
                            globals::triggerbot_delay_ms = static_cast<int>(delay_f);
                        }
                        else if (globals::triggerbot_mode == 2) {
                            float min_f = static_cast<float>(globals::triggerbot_min_delay_ms);
                            float max_f = static_cast<float>(globals::triggerbot_max_delay_ms);
                            Slider("Min Delay", "Minimum delay (ms)", &min_f, 0.0f, 1500.0f, "%.0f ms");
                            Slider("Max Delay", "Maximum delay (ms)", &max_f, 0.0f, 1500.0f, "%.0f ms");
                            globals::triggerbot_min_delay_ms = static_cast<int>(min_f);
                            globals::triggerbot_max_delay_ms = static_cast<int>(max_f);
                            if (globals::triggerbot_min_delay_ms > globals::triggerbot_max_delay_ms)
                                globals::triggerbot_max_delay_ms = globals::triggerbot_min_delay_ms;
                        }
                        
                        Checkbox("Hit Select", "Only hit at full damage", &globals::triggerbot_hit_select);
                        static std::vector<std::string> crit_modes = {"Off", "Crit Only", "Priority Crit"};
                        Dropdown("Crit Mode", "Critical hit behavior", &globals::triggerbot_crit_mode, crit_modes);
                        // Shield Use setting
                        Checkbox("Use Shield", "Use shield after hit", &globals::triggerbot_use_shield);
                        if (globals::triggerbot_use_shield) {
                            float shield_dur = static_cast<float>(globals::triggerbot_shield_duration_ms);
                            Slider("Shield Time", "Shield duration (ms)", &shield_dur, 10.0f, 1000.0f, "%.0f ms");
                            globals::triggerbot_shield_duration_ms = static_cast<int>(shield_dur);
                        }
                        
                        // Shield Check setting
                        Checkbox("Check Shield", "Check if target blocking", &globals::triggerbot_check_shield);
                        if (globals::triggerbot_check_shield) {
                            static std::vector<std::string> shield_actions = {"Don't Click", "Spam Click"};
                            Dropdown("Shield Action", "When target blocking", &globals::triggerbot_shield_action, shield_actions);
                        }
                    }
                    
                    Checkbox("Reach", "Extended attack reach", &globals::reach_enabled);
                    if (globals::reach_enabled) {
                        Keybind("Keybind", "Activation key", &globals::reach_keybind, &globals::reach_mode);
                        float reach = static_cast<float>(globals::reach_distance);
                        Slider("Distance", "Reach distance", &reach, 3.0f, 6.0f, "%.2f");
                        globals::reach_distance = static_cast<double>(reach);
                    }
                    
                    Checkbox("Hitbox Expander", "Expand target hitboxes", &globals::hitbox_enabled);
                    if (globals::hitbox_enabled) {
                        Keybind("Keybind", "Activation key", &globals::hitbox_keybind, &globals::hitbox_mode);
                        float x_expand = static_cast<float>(globals::hitbox_expand_width);
                        float y_expand = static_cast<float>(globals::hitbox_expand_height);
                        Slider("X/Z Expand", "Horizontal expansion", &x_expand, 0.0f, 1.0f, "%.2f");
                        globals::hitbox_expand_width = static_cast<double>(x_expand);
                        Slider("Y Expand", "Vertical expansion", &y_expand, 0.0f, 1.0f, "%.2f");
                        globals::hitbox_expand_height = static_cast<double>(y_expand);
                    }
                    
                    // Backtrack module disabled
                    
                    Checkbox("Shield Breaker", "Auto break shields", &globals::shield_breaker_enabled);
                    if (globals::shield_breaker_enabled) {
                        Keybind("Keybind", "Activation key", &globals::shield_breaker_keybind, &globals::shield_breaker_keybind_mode);
                        Checkbox("Aim", "Aim at target", &globals::shield_breaker_aim);
                        Checkbox("Switch Back", "Return to prev slot", &globals::shield_breaker_switch_back);
                        float delay_f = static_cast<float>(globals::shield_breaker_delay_ms);
                        Slider("Delay", "Break delay (ms)", &delay_f, 0.0f, 200.0f, "%.0f ms");
                        globals::shield_breaker_delay_ms = static_cast<int>(delay_f);
                    }
                }
                EndChildWindow();
                
                ImGui::SetCursorPos(c_vec2(row1_start.x + child_w + s_(g_elem.content.padding.x), row1_start.y));
                
                if (BeginChildWindow("Movement", c_vec2(child_w, child_h))) {
                    Checkbox("Auto Mace", "Auto mace on fall", &globals::mace_enabled);
                    if (globals::mace_enabled) {
                        Keybind("Keybind", "Activation key", &globals::mace_keybind, &globals::mace_keybind_mode);
                        Checkbox("Look", "Look at target", &globals::mace_look);
                        Checkbox("Switch Back", "Return to prev slot", &globals::mace_switch_back);
                        Checkbox("Remove Elytra", "Remove elytra on attack", &globals::mace_remove_elytra);
                        float min_fall = static_cast<float>(globals::mace_min_fall_distance);
                        Slider("Min Fall", "Min fall distance", &min_fall, 0.0f, 20.0f, "%.1f");
                        globals::mace_min_fall_distance = static_cast<double>(min_fall);
                        float height_above = static_cast<float>(globals::mace_height_above_target);
                        Slider("Height Above", "Height above target", &height_above, 0.0f, 50.0f, "%.1f");
                        globals::mace_height_above_target = static_cast<double>(height_above);
                        float fall_hitbox_w = static_cast<float>(globals::mace_fall_hitbox_width);
                        Slider("Hitbox X", "Fall hitbox width", &fall_hitbox_w, 0.0f, 1.0f, "%.2f");
                        globals::mace_fall_hitbox_width = static_cast<double>(fall_hitbox_w);
                        float fall_hitbox_h = static_cast<float>(globals::mace_fall_hitbox_height);
                        Slider("Hitbox Y", "Fall hitbox height", &fall_hitbox_h, 0.0f, 1.0f, "%.2f");
                        globals::mace_fall_hitbox_height = static_cast<double>(fall_hitbox_h);
                    }
                    
                    Checkbox("Stun Slam", "Shield break + mace combo", &globals::stun_slam_enabled);
                    if (globals::stun_slam_enabled) {
                        Slider("Chance", "Activation chance %%", &globals::stun_slam_chance, 0.0f, 100.0f, "%.0f%%");
                        float swap_delay = static_cast<float>(globals::stun_slam_swap_delay_ms);
                        Slider("Swap Delay", "Item swap delay", &swap_delay, 0.0f, 50.0f, "%.0f ms");
                        globals::stun_slam_swap_delay_ms = static_cast<int>(swap_delay);
                        float axe_delay = static_cast<float>(globals::stun_slam_axe_delay_ms);
                        Slider("Axe Delay", "After axe hit", &axe_delay, 0.0f, 100.0f, "%.0f ms");
                        globals::stun_slam_axe_delay_ms = static_cast<int>(axe_delay);
                        float mace_delay = static_cast<float>(globals::stun_slam_mace_delay_ms);
                        Slider("Mace Delay", "After mace hit", &mace_delay, 0.0f, 100.0f, "%.0f ms");
                        globals::stun_slam_mace_delay_ms = static_cast<int>(mace_delay);
                        float stun_fall = static_cast<float>(globals::stun_slam_min_fall);
                        Slider("Min Fall", "Min fall distance", &stun_fall, 0.5f, 10.0f, "%.1f");
                        globals::stun_slam_min_fall = static_cast<double>(stun_fall);
                    }
                    
                    Checkbox("S-Tap", "Press S on hit", &globals::stap_enabled);
                    if (globals::stap_enabled) {
                        float stap_dur = static_cast<float>(globals::stap_duration_ms);
                        Slider("S Duration", "How long to press S", &stap_dur, 20.0f, 1500.0f, "%.0f ms");
                        globals::stap_duration_ms = static_cast<int>(stap_dur);
                    }
                    
                    Checkbox("W-Tap", "Release W on hit", &globals::wtap_enabled);
                    if (globals::wtap_enabled) {
                        float wtap_dur = static_cast<float>(globals::wtap_duration_ms);
                        Slider("W Release", "How long to release W", &wtap_dur, 20.0f, 1500.0f, "%.0f ms");
                        globals::wtap_duration_ms = static_cast<int>(wtap_dur);
                    }
                    
                    Checkbox("Auto Jump Reset", "Jump when taking damage", &globals::autojumpreset_enabled);
                    if (globals::autojumpreset_enabled) {
                        Keybind("Keybind", "Activation key", &globals::autojumpreset_keybind, &globals::autojumpreset_mode);
                        float cooldown_f = static_cast<float>(globals::autojumpreset_cooldown_ms);
                        Slider("Cooldown", "Jump cooldown (ms)", &cooldown_f, 0.0f, 1000.0f, "%.0f ms");
                        globals::autojumpreset_cooldown_ms = static_cast<int>(cooldown_f);
                    }
                }
                EndChildWindow();
                
                c_vec2 row2_start = c_vec2(row1_start.x, row1_start.y + child_h + s_(g_elem.content.padding.y));
                ImGui::SetCursorPos(row2_start);
                
                if (BeginChildWindow("Utility", c_vec2(child_w, child_h))) {
                    Checkbox("AutoCrystal", "Auto place and break crystals", &globals::autocrystal_enabled);
                    if (globals::autocrystal_enabled) {
                        Keybind("Keybind", "Activation key", &globals::autocrystal_keybind, &globals::autocrystal_mode);
                        float delay_f = static_cast<float>(globals::autocrystal_delay_ms);
                        Slider("Delay", "Place/break delay (ms)", &delay_f, 0.0f, 500.0f, "%.0f ms");
                        globals::autocrystal_delay_ms = static_cast<int>(delay_f);
                        Checkbox("Debug", "Show debug logs", &globals::autocrystal_debug_enabled);
                    }
                    
                    Checkbox("Auto Totem", "Auto equip totem in offhand", &globals::autototem_enabled);
                    if (globals::autototem_enabled) {
                        Keybind("Keybind", "Activation key", &globals::autototem_keybind, &globals::autototem_mode);
                        Checkbox("Rage Mode", "Use rage mode", &globals::autototem_rage_mode);
                    }
                }
                EndChildWindow();
                
                ImGui::SetCursorPos(c_vec2(row2_start.x + child_w + s_(g_elem.content.padding.x), row2_start.y));
                
                if (BeginChildWindow("Macros", c_vec2(child_w, child_h))) {
                    Checkbox("Anchor Macro", "Auto anchor combo", &globals::anchor_macro_enabled);
                    if (globals::anchor_macro_enabled) {
                        Keybind("Keybind", "Activation key", &globals::anchor_macro_keybind, &globals::anchor_macro_mode);
                        Checkbox("Explode", "Explode anchor at end", &globals::anchor_macro_break_anchor);
                        float swap_delay = static_cast<float>(globals::anchor_macro_swap_delay_ms);
                        Slider("Swap Delay", "Item swap delay", &swap_delay, 40.0f, 200.0f, "%.0f ms");
                        globals::anchor_macro_swap_delay_ms = static_cast<int>(swap_delay);
                        float charge_delay = static_cast<float>(globals::anchor_macro_charge_delay_ms);
                        Slider("Charge Delay", "Before charging", &charge_delay, 80.0f, 300.0f, "%.0f ms");
                        globals::anchor_macro_charge_delay_ms = static_cast<int>(charge_delay);
                        float explode_delay = static_cast<float>(globals::anchor_macro_break_delay_ms);
                        Slider("Explode Delay", "Before exploding", &explode_delay, 40.0f, 200.0f, "%.0f ms");
                        globals::anchor_macro_break_delay_ms = static_cast<int>(explode_delay);
                    }
                }
                EndChildWindow();
            }
            
            if (g_tab_stored == 1) {
                float child_w = (content_avail_x() - s_(g_elem.content.padding.x)) / 2;
                float child_h = s_(200);
                
                c_vec2 row1_start = ImGui::GetCursorPos();
                
                if (BeginChildWindow("ESP", c_vec2(child_w, child_h))) {
                    Checkbox("Box ESP", "Draw boxes around players", &globals::box_enabled);
                    if (globals::box_enabled) {
                        Keybind("Keybind", "Activation key", &globals::esp_keybind, &globals::esp_mode);
                    }
                    Checkbox("Health Bar", "Show health bars", &globals::esp_health_bar);
                }
                EndChildWindow();
            }
            
            if (g_tab_stored == 2) {
                float child_w = (content_avail_x() - s_(g_elem.content.padding.x)) / 2;
                float child_h = s_(180);
                
                c_vec2 row1_start = ImGui::GetCursorPos();
                
                if (BeginChildWindow("Utility", c_vec2(child_w, child_h))) {
                    Keybind("Pearl Catch", "Catch pearl on key press", &globals::pearl_catch_keybind, &globals::pearl_catch_mode);
                    if (globals::pearl_catch_keybind != 0) {
                        static std::vector<std::string> pearl_aim_modes = {"Silent (Detected)", "Visible"};
                        Dropdown("Aim Mode", "How rotation is applied", &globals::pearl_catch_aim_mode, pearl_aim_modes);
                    }
                    Checkbox("Sprint", "Always sprint", &globals::sprint_enabled);
                    Checkbox("Flight", "Enable flight", &globals::flight_enabled);
                }
                EndChildWindow();
            }
            
            if (g_tab_stored == 3) {
            }
            
            easing(g_clr.accent, g_accent_color, 24.f);
            
            float view_h = win_size.y;
            float content_h = content_win->ContentSize.y + s_(g_elem.content.padding.y) * 2;
            float max_scroll = ImMax(0.f, content_h - view_h);
            
            c_vec2 mouse = ImGui::GetIO().MousePos;
            c_rect content_rect(content_pos, content_pos + c_vec2(content_width, win_size.y));
            bool hover_content = content_rect.Contains(mouse);
            
            if (hover_content && ImGui::GetIO().MouseWheel != 0.f && !g_child_consumed_scroll) {
                g_content_scroll.scroll -= ImGui::GetIO().MouseWheel * s_(30);
            }
            
            g_content_scroll.scroll = ImClamp(g_content_scroll.scroll, 0.f, max_scroll);
            easing(g_content_scroll.scroll_anim, g_content_scroll.scroll, 14.f);
            content_win->Scroll.y = g_content_scroll.scroll_anim;
            
            if (max_scroll > 0) {
                float track_h = scrollbar_track.GetHeight();
                float grab_h = ImMax(s_(30), track_h * (view_h / content_h));
                float ratio = max_scroll > 0 ? g_content_scroll.scroll_anim / max_scroll : 0.f;
                float grab_y = scrollbar_track.Min.y + (track_h - grab_h) * ratio;
                
                c_rect grab(c_vec2(scrollbar_track.Min.x, grab_y), c_vec2(scrollbar_track.Max.x, grab_y + grab_h));
                
                bool grab_hover = grab.Contains(mouse);
                bool track_hover = scrollbar_track.Contains(mouse) && !grab_hover;
                
                if ((grab_hover || track_hover) && ImGui::IsMouseClicked(0)) {
                    g_content_scroll.drag_scrollbar = true;
                    if (track_hover) {
                        float cr = (mouse.y - scrollbar_track.Min.y - grab_h * 0.5f) / (track_h - grab_h);
                        g_content_scroll.scroll = ImClamp(cr, 0.f, 1.f) * max_scroll;
                    }
                    g_content_scroll.drag_offset = mouse.y - grab_y;
                }
                
                if (g_content_scroll.drag_scrollbar) {
                    if (ImGui::IsMouseDown(0)) {
                        float gy = mouse.y - g_content_scroll.drag_offset;
                        float nr = (gy - scrollbar_track.Min.y) / (track_h - grab_h);
                        g_content_scroll.scroll = ImClamp(nr, 0.f, 1.f) * max_scroll;
                    } else {
                        g_content_scroll.drag_scrollbar = false;
                    }
                }
                
                float alpha_tgt = (grab_hover || g_content_scroll.drag_scrollbar || track_hover) ? 1.f : 0.5f;
                easing(g_content_scroll.scrollbar_alpha, alpha_tgt, 12.f);
                
                float fr = max_scroll > 0 ? g_content_scroll.scroll_anim / max_scroll : 0.f;
                float fgy = scrollbar_track.Min.y + (track_h - grab_h) * fr;
                c_rect fg(c_vec2(scrollbar_track.Min.x, fgy), c_vec2(scrollbar_track.Max.x, fgy + grab_h));
                
                c_vec4 track_c = g_clr.layout;
                track_c.w *= 0.2f * g_content_scroll.scrollbar_alpha;
                rect_filled(dl, scrollbar_track.Min, scrollbar_track.Max, get_clr(track_c), s_(3), 0);
                
                c_vec4 grab_c = g_content_scroll.drag_scrollbar ? g_clr.accent : (grab_hover ? g_clr.text : g_clr.selectable);
                grab_c.w *= g_content_scroll.scrollbar_alpha;
                rect_filled(dl, fg.Min, fg.Max, get_clr(grab_c), s_(3), 0);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        
        ImGui::SetCursorPos(c_vec2(win_size.x - s_(g_elem.settings_bar.size.x), 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, s_(g_elem.settings_bar.padding));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(g_elem.settings_bar.padding));
        ImGui::BeginChild("Settings", s_(g_elem.settings_bar.size), false,
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysUseWindowPadding |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
        {
            if (IconButton("theme", "E")) {
                g_theme = (g_theme + 1) % 3;
            }
            
            static c_vec2 settings_button_pos_stored = ImVec2(0, 0);
            if (IconButton("settings", "A")) {
                open_popup("settings");
            }
            c_vec2 current_pos = ImGui::GetItemRectMin();
            if (current_pos.x >= 0 && current_pos.y >= 0) {
                settings_button_pos_stored = current_pos;
            }
            
            std::string popup_id = "settings";
            int popup_idx = -1;
            for (int i = 0; i < (int)g_popup_storage.size(); ++i) {
                if (g_popup_storage[i].name == popup_id) {
                    popup_idx = i;
                    break;
                }
            }
            
            if (popup_idx == -1) {
                open_popup("settings");
                for (int i = 0; i < (int)g_popup_storage.size(); ++i) {
                    if (g_popup_storage[i].name == popup_id) {
                        popup_idx = i;
                        break;
                    }
                }
            }
            
            if (popup_idx >= 0 && g_popup_storage[popup_idx].alpha > 0.01f) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_popup_storage[popup_idx].alpha);
                ImGui::SetNextWindowPos(settings_button_pos_stored);
                ImGui::SetNextWindowSize(c_vec2(s_(200), 0));
                
                ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | 
                                               ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
                                               ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
                
                if (ImGui::Begin("##settings_popup", nullptr, popup_flags)) {
                    ImGuiWindow* popup_window = ImGui::GetCurrentWindow();
                    c_rect popup_rect = popup_window->Rect();
                    rect_filled(ImGui::GetWindowDrawList(), popup_rect.Min, popup_rect.Max, get_clr(g_clr.layout), s_(5), 0);
                    
                    extern float g_menu_scale;
                    Slider("Menu scale", "Edit menu size", &g_menu_scale, 100.f, 200.f, "%.0f");
                    
                    ImGui::SetWindowFocus();
                    
                    if (!ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()).Contains(ImGui::GetIO().MousePos) && 
                        ImGui::IsMouseClicked(0)) {
                        g_popup_storage[popup_idx].open = false;
                    }
                }
                ImGui::End();
                ImGui::PopStyleVar();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
    
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void RenderGamblingWindow() {
    auto& game = enhance::modules::gambling::g_mines_game;
    if (!game.isWindowOpen()) return;
    
    c_vec2 window_size = s_(500, 600);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, s_(g_elem.content.padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(g_elem.content.padding));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, c_vec4(0, 0, 0, 0));
    
    if (ImGui::Begin("Mines Game", &game.window_open, flags)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        c_vec2 win_pos = ImGui::GetWindowPos();
        c_vec2 win_size = ImGui::GetWindowSize();
        
        rect_filled(dl, win_pos, win_pos + win_size, get_clr(g_clr.layout), s_(g_elem.window.rounding), 0);
        
        if (BeginChildWindow("Mines", s_(0, 0), false)) {
            char balance_text[64];
            snprintf(balance_text, sizeof(balance_text), "Balance: %.2f", game.balance);
            ImGui::Text(balance_text);
            
            if (!game.game_active && !game.game_won && !game.game_lost) {
                Slider("Bet Amount", "Amount to bet", &game.bet_amount, 0.1f, game.balance, "%.2f");
                if (game.bet_amount < 0.1f) game.bet_amount = 0.1f;
                if (game.bet_amount > game.balance) game.bet_amount = game.balance;
                
                float num_mines_f = (float)game.num_mines;
                Slider("Number of Mines", "Mines to place", &num_mines_f, 1.0f, (float)(game.grid_size * game.grid_size - 1), "%.0f");
                game.num_mines = (int)num_mines_f;
                if (game.num_mines < 1) game.num_mines = 1;
                if (game.num_mines > game.grid_size * game.grid_size - 1) game.num_mines = game.grid_size * game.grid_size - 1;
                
                if (Button("Start Game")) {
                    if (game.bet_amount <= game.balance) {
                        game.balance -= game.bet_amount;
                        game.game_active = true;
                        game.generateGrid();
                    }
                }
            } else {
                if (game.game_won) {
                    char win_text[64];
                    snprintf(win_text, sizeof(win_text), "You Won! +%.2f", game.bet_amount * game.current_multiplier);
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), win_text);
                    if (Button("Play Again")) {
                        game.resetGame();
                    }
                } else if (game.game_lost) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "You Hit a Mine!");
                    if (Button("Play Again")) {
                        game.resetGame();
                    }
                } else {
                    char mult_text[64];
                    snprintf(mult_text, sizeof(mult_text), "Multiplier: %.2fx", game.current_multiplier);
                    ImGui::Text(mult_text);
                    char win_text[64];
                    snprintf(win_text, sizeof(win_text), "Potential Win: %.2f", game.bet_amount * game.current_multiplier);
                    ImGui::Text(win_text);
                    
                    if (Button("Cash Out")) {
                        game.balance += game.bet_amount * game.current_multiplier;
                        game.resetGame();
                    }
                    
                    ImGui::Spacing();
                    
                    float tile_size = s_(35);
                    float spacing = s_(4);
                    float total_width = game.grid_size * tile_size + (game.grid_size - 1) * spacing;
                    float start_x = (content_avail_x() - total_width) * 0.5f;
                    
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);
                    
                    for (int i = 0; i < game.grid_size; i++) {
                        for (int j = 0; j < game.grid_size; j++) {
                            if (j > 0) ImGui::SameLine(0, spacing);
                            
                            ImVec4 button_color = g_clr.button;
                            std::string label = "?";
                            
                            if (game.revealed[i][j]) {
                                if (game.grid[i][j] == -1) {
                                    button_color = ImVec4(1, 0, 0, 1);
                                    label = "*";
                                } else {
                                    button_color = g_clr.accent;
                                    label = "";
                                }
                            }
                            
                            ImGui::PushID(i * game.grid_size + j);
                            ImGui::PushStyleColor(ImGuiCol_Button, button_color);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_color.x * 1.2f, button_color.y * 1.2f, button_color.z * 1.2f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_color.x * 0.8f, button_color.y * 0.8f, button_color.z * 0.8f, 1.0f));
                            
                            if (ImGui::Button(label.c_str(), ImVec2(tile_size, tile_size))) {
                                if (game.game_active) {
                                    game.revealTile(i, j);
                                }
                            }
                            
                            ImGui::PopStyleColor(3);
                            ImGui::PopID();
                        }
                    }
                }
            }
            
            ImGui::Spacing();
            if (Button("Close")) {
                game.closeWindow();
            }
        }
        EndChildWindow();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}


bool GUI::get_is_init()
{
	return is_init;
}

bool GUI::get_do_draw()
{
	return do_draw;
}

extern void wolf_menu_reset_popups();

void GUI::set_do_draw(bool new_value)
{
	if (do_draw && !new_value)
	{
		wolf_menu_reset_popups();
	}
	do_draw = new_value;
	globals::show_gui = new_value;
}
