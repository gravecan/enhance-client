#ifndef WOLF_MENU_INCLUDED_MODE
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <utils/imgui/imgui.h>
#include <utils/imgui/imgui_internal.h>
#include <utils/imgui/imgui_impl_opengl3.h>
#include "../utils/http_client.h"
#include "../utils/token_storage.h"
#include "../utils/hwid.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>
#include <windows.h>

#include "data/images.h"
#include "data/fonts.h"

using c_vec2 = ImVec2;
using c_vec4 = ImVec4;
using c_col = ImColor;
using c_rect = ImRect;
using c_text = std::string;

static float g_dpi = 1.0f;
static int g_stored_dpi = 100;
static bool g_dpi_changed = true;

#define s_(...) scale_impl(__VA_ARGS__, g_dpi)

inline ImVec2 scale_impl(const ImVec2& vec, float dpi) {
    return ImVec2(roundf(vec.x * dpi), roundf(vec.y * dpi));
}

inline ImVec2 scale_impl(float x, float y, float dpi) {
    return ImVec2(roundf(x * dpi), roundf(y * dpi));
}

inline float scale_impl(float var, float dpi) {
    return roundf(var * dpi);
}

struct MenuColors {
    c_vec4 layout{ c_col(15, 15, 18).Value };
    c_vec4 child{ c_col(17, 17, 21).Value };
    c_vec4 text{ c_col(92, 95, 122).Value };
    c_vec4 white{ c_col(255, 255, 255).Value };
    c_vec4 accent{ c_col(126, 139, 209).Value };
    c_vec4 black{ c_col(0, 0, 0).Value };
    c_vec4 lightchild{ c_col(20, 20, 25).Value };
    c_vec4 selectable{ c_col(22, 22, 28).Value };
    c_vec4 button{ c_col(50, 50, 65).Value };
    
    void reset_to_defaults() {
        layout = c_col(15, 15, 18).Value;
        child = c_col(17, 17, 21).Value;
        text = c_col(92, 95, 122).Value;
        white = c_col(255, 255, 255).Value;
        accent = c_col(126, 139, 209).Value;
        black = c_col(0, 0, 0).Value;
        lightchild = c_col(20, 20, 25).Value;
        selectable = c_col(22, 22, 28).Value;
        button = c_col(50, 50, 65).Value;
    }
    
    bool is_valid() const {
        return white.x > 0.9f && white.y > 0.9f && white.z > 0.9f && accent.w > 0.5f;
    }
};
static MenuColors g_clr;

struct Elements {
    struct { std::string name{"wolf"}; c_vec2 size{590, 350}; float rounding{5}; } window;
    struct { c_vec2 size{70, 350}; } sidebar;
    struct { c_vec2 padding{10, 10}; } content;
    struct { float height{43}; float button_size{14}; float button_rounding{3}; float rounding{5}; c_vec2 padding{10, 10}; } checkbox;
    struct { float height{58}; float button_size{10}; float button_rounding{3}; float rounding{5}; c_vec2 padding{10, 10}; c_vec2 grab_size{10, 10}; float grab_rounding{3}; } slider;
    struct { c_vec2 size{70, 70}; } tab_switcher;
    struct { float height{79}; float rounding{5}; c_vec2 padding{10, 10}; float button_size{31}; float button_rounding{3}; } dropdown;
    struct { float height{68}; float rounding{5}; c_vec2 padding{10, 10}; float button_size{31}; float button_rounding{3}; } button_elem;
    struct { float height{31}; } selectable;
    struct { float circle_size{5}; c_vec2 size{14, 14}; } color_edit_button;
    struct { float popup_rounding{5}, popup_width{180}, sv_rounding{3}, hb_rounding{3}, ab_rounding{3}, circle_radius{3}, circle_thickness{2}, grab_rounding{3}; c_vec2 sv_size{140, 100}, hb_size{10, 100}, ab_size{160, 10}, padding{10, 10}, spacing{10, 10}, hb_grab_size{10, 10}, ab_grab_size{10, 10}; } color_edit;
    struct { c_vec2 size{30, 350}; c_vec2 padding{10, 10}; } settings_bar;
};
static Elements g_elem;

static ImFont* g_font_inter_12 = nullptr;
static ImFont* g_font_inter_11 = nullptr;
static ImFont* g_font_icons_16 = nullptr;
static ImFont* g_font_icons_12 = nullptr;
static ImFont* g_font_icons_8 = nullptr;
static ImFont* g_font_icons_10 = nullptr;

static int g_tab_stored = 0;
static int g_theme = 0;
float g_menu_scale = 138.f;
static c_vec4 g_accent_color = c_col(126, 139, 209).Value;  // Accent color

static bool g_logo_texture_loaded = false;

struct child_state_t {
    c_vec2 size;
    float scroll;
    float scroll_anim;
    float scrollbar_alpha;
    bool drag_scrollbar;
    float drag_offset;
};
static std::unordered_map<ImGuiID, child_state_t> g_child_states;

struct content_scroll_state_t {
    float scroll;
    float scroll_anim;
    float scrollbar_alpha;
    bool drag_scrollbar;
    float drag_offset;
};
static content_scroll_state_t g_content_scroll;

static bool g_child_consumed_scroll = false;

static c_vec2 g_child_start_pos;
static c_vec2 g_child_size_stored;

struct popup_storage_t {
    std::string name;
    bool open;
    float alpha;
    c_vec2 size;
};
static std::vector<popup_storage_t> g_popup_storage;

bool is_popup_open(const std::string& name) {
    for (auto& popup : g_popup_storage) {
        if (popup.name == name) {
            return popup.alpha > 0;
        }
    }
    return false;
}

void open_popup(const std::string& name) {
    for (auto& popup : g_popup_storage) {
        if (popup.name == name) {
            popup.open = true;
            return;
        }
    }
    g_popup_storage.push_back({name, true, 0.f, c_vec2(0, 0)});
}

void close_popup(const std::string& name) {
    for (auto& popup : g_popup_storage) {
        if (popup.name == name) {
            popup.open = false;
            return;
        }
    }
}

void close_all_popups() {
    for (auto& popup : g_popup_storage) {
        popup.open = false;
        popup.alpha = 0.f;
    }
}

static ImGuiID g_keybind_waiting_id = 0;

void wolf_menu_reset_popups() {
    close_all_popups();
    
    g_popup_storage.clear();
    
    g_child_consumed_scroll = false;
    
    g_keybind_waiting_id = 0;
    
    g_content_scroll.scroll = 0.f;
    g_content_scroll.scroll_anim = 0.f;
    g_content_scroll.scrollbar_alpha = 0.5f;
    g_content_scroll.drag_scrollbar = false;
    g_content_scroll.drag_offset = 0.f;
    
    for (auto& [id, state] : g_child_states) {
        state.scroll = 0.f;
        state.scroll_anim = 0.f;
        state.scrollbar_alpha = 0.f;
        state.drag_scrollbar = false;
        state.drag_offset = 0.f;
    }
    
    g_clr.reset_to_defaults();
}

static std::unordered_map<ImGuiID, void*> g_anim_states;

template <typename T>
T* anim_container(ImGuiID id) {
    auto it = g_anim_states.find(id);
    if (it != g_anim_states.end())
        return static_cast<T*>(it->second);
    T* new_state = new T();
    g_anim_states[id] = new_state;
    return new_state;
}

inline float fixed_speed(float speed) { 
    float fps = ImGui::GetIO().Framerate;
    if (fps < 1.0f || fps > 10000.0f || !std::isfinite(fps)) fps = 60.0f;
    float result = speed / fps;
    if (!std::isfinite(result) || result < 0.0f) return 0.0f;
    if (result > 1.0f) return 1.0f;
    return result; 
}

enum easing_type { static_easing, dynamic_easing };

template<typename T>
T& easing(T& value, const T& target, float speed, int type = dynamic_easing) {
    if (type == static_easing) {
        if constexpr (std::is_same<T, ImVec4>::value) {
            return value;
        } else {
            T step = fixed_speed(speed);
            if (value < target) {
                value += step;
                if (value > target) value = target;
            } else if (value > target) {
                value -= step;
                if (value < target) value = target;
            }
        }
    } else {
        if constexpr (std::is_same<T, ImVec4>::value) {
            value = ImLerp(value, target, fixed_speed(speed));
        } else {
            value = ImLerp(value, target, fixed_speed(speed));
        }
    }
    return value;
}

inline ImU32 get_clr(const c_vec4& col, float alpha = 1.f) {
    c_vec4 c = col;
    c.w *= alpha;
    return ImGui::ColorConvertFloat4ToU32(c);
}

void rect_filled(ImDrawList* dl, const c_vec2& p_min, const c_vec2& p_max, ImU32 col, float rounding = 0.f, ImDrawFlags flags = 0) {
    if ((col & IM_COL32_A_MASK) == 0) return;
    dl->AddRectFilled(p_min, p_max, col, rounding, flags);
}

void rect_filled_multicolor(ImDrawList* dl, const c_vec2& p_min, const c_vec2& p_max, 
                            ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left,
                            float rounding = 0.f, ImDrawFlags flags = 0) {
    if (rounding < 0.5f) {
        dl->AddRectFilledMultiColor(p_min, p_max, col_upr_left, col_upr_right, col_bot_right, col_bot_left);
    } else {
        int vtx_start = dl->VtxBuffer.Size;
        dl->AddRectFilled(p_min, p_max, IM_COL32_WHITE, rounding, flags);
        int vtx_end = dl->VtxBuffer.Size;
        
        for (int i = vtx_start; i < vtx_end; i++) {
            ImDrawVert* vert = &dl->VtxBuffer.Data[i];
            float tx = (vert->pos.x - p_min.x) / (p_max.x - p_min.x);
            float ty = (vert->pos.y - p_min.y) / (p_max.y - p_min.y);
            
            c_vec4 top = ImLerp(ImGui::ColorConvertU32ToFloat4(col_upr_left), ImGui::ColorConvertU32ToFloat4(col_upr_right), tx);
            c_vec4 bot = ImLerp(ImGui::ColorConvertU32ToFloat4(col_bot_left), ImGui::ColorConvertU32ToFloat4(col_bot_right), tx);
            c_vec4 final_col = ImLerp(top, bot, ty);
            vert->col = ImGui::ColorConvertFloat4ToU32(final_col);
        }
    }
}

void text_clipped(ImDrawList* dl, ImFont* font, const c_vec2& pos_min, const c_vec2& pos_max, ImU32 color, 
                  const char* text, const c_vec2& align = c_vec2(0.f, 0.f)) {
    if (!font) font = ImGui::GetFont();
    ImGui::PushFont(font);
    c_vec2 text_size = ImGui::CalcTextSize(text);
    c_vec2 pos = pos_min;
    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);
    
    c_vec4 clip(pos_min.x, pos_min.y, pos_max.x, pos_max.y);
    dl->AddText(font, font->FontSize, pos, color, text, nullptr, 0.f, &clip);
    ImGui::PopFont();
}

void InitializeFonts() {
    if (!g_dpi_changed && g_font_inter_12) return;
    
    ImGuiIO& io = ImGui::GetIO();
    if (!io.Fonts) return;
    
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return;
    
    if (ctx->WithinFrameScope) {
        return;
    }
    
    io.Fonts->Clear();
    
    g_font_inter_12 = nullptr;
    g_font_inter_11 = nullptr;
    g_font_icons_16 = nullptr;
    g_font_icons_12 = nullptr;
    g_font_icons_8 = nullptr;
    g_font_icons_10 = nullptr;
    
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    
    if (!inter.empty()) {
        g_font_inter_12 = io.Fonts->AddFontFromMemoryTTF((void*)inter.data(), (int)inter.size(), s_(12), &cfg, io.Fonts->GetGlyphRangesCyrillic());
    } else {
        g_font_inter_12 = io.Fonts->AddFontDefault();
    }
    
    if (!inter.empty()) {
        cfg.FontDataOwnedByAtlas = false;
        g_font_inter_11 = io.Fonts->AddFontFromMemoryTTF((void*)inter.data(), (int)inter.size(), s_(11), &cfg, io.Fonts->GetGlyphRangesCyrillic());
    } else {
        g_font_inter_11 = g_font_inter_12;
    }
    
    if (!icons.empty()) {
        ImFontConfig icon_cfg;
        icon_cfg.FontDataOwnedByAtlas = false;
        g_font_icons_16 = io.Fonts->AddFontFromMemoryTTF((void*)icons.data(), (int)icons.size(), s_(16), &icon_cfg);
    }
    
    if (!icons.empty()) {
        ImFontConfig icon_cfg;
        icon_cfg.FontDataOwnedByAtlas = false;
        g_font_icons_12 = io.Fonts->AddFontFromMemoryTTF((void*)icons.data(), (int)icons.size(), s_(12), &icon_cfg);
    }
    
    if (!icons.empty()) {
        ImFontConfig icon_cfg;
        icon_cfg.FontDataOwnedByAtlas = false;
        g_font_icons_8 = io.Fonts->AddFontFromMemoryTTF((void*)icons.data(), (int)icons.size(), s_(8), &icon_cfg);
    }
    
    if (!icons.empty()) {
        ImFontConfig icon_cfg;
        icon_cfg.FontDataOwnedByAtlas = false;
        g_font_icons_10 = io.Fonts->AddFontFromMemoryTTF((void*)icons.data(), (int)icons.size(), s_(10), &icon_cfg);
    }
    
    io.Fonts->Build();
    
    try {
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();
    } catch (...) {
    }
    
    g_dpi_changed = false;
}

inline float content_avail_x() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window) return 0.f;
    return window->ContentRegionRect.GetWidth();
}

void set_width_fast(float width) {
    ImGui::SetNextItemWidth(width);
}

struct color_edit_state {
    bool init_val;
    float hover;
    float hsv[4];
};

bool sv_edit(const std::string& name, float h, float* s, float* v) {
    struct sv_state { c_vec2 circle_pos; };
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name.c_str());
    sv_state* state = anim_container<sv_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 mouse_pos = ImGui::GetIO().MousePos;
    c_vec2 size = s_(g_elem.color_edit.sv_size);
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    ImGui::ItemAdd(rect, id);
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    if (held) {
        *s = ImSaturate((mouse_pos.x - pos.x) / rect.GetWidth());
        *v = 1.f - ImSaturate((mouse_pos.y - pos.y) / rect.GetHeight());
    }
    
    easing(state->circle_pos.x, rect.GetWidth() * *s, 24.f);
    easing(state->circle_pos.y, rect.GetHeight() * (1.f - *v), 24.f);
    
    c_vec2 circle_pos = pos + state->circle_pos;
    circle_pos.x = ImClamp(circle_pos.x, pos.x + s_(g_elem.color_edit.circle_radius + g_elem.color_edit.circle_thickness / 2), 
                          pos.x + size.x - s_(g_elem.color_edit.circle_radius + g_elem.color_edit.circle_thickness / 2));
    circle_pos.y = ImClamp(circle_pos.y, pos.y + s_(g_elem.color_edit.circle_radius + g_elem.color_edit.circle_thickness / 2), 
                          pos.y + size.y - s_(g_elem.color_edit.circle_radius + g_elem.color_edit.circle_thickness / 2));
    
    ImColor hue_col = ImColor::HSV(h, 1.f, 1.f);
    rect_filled_multicolor(window->DrawList, rect.Min, rect.Max - c_vec2(0, s_(g_elem.color_edit.sv_rounding)), 
                         get_clr(g_clr.white), get_clr(hue_col), get_clr(hue_col), get_clr(g_clr.white), 
                         s_(g_elem.color_edit.sv_rounding));
    rect_filled_multicolor(window->DrawList, rect.Min, rect.Max, (ImU32)0, (ImU32)0, get_clr(g_clr.black), get_clr(g_clr.black), 
                          s_(g_elem.color_edit.sv_rounding));
    
    window->DrawList->AddCircle(circle_pos, s_(g_elem.color_edit.circle_radius), get_clr(g_clr.white), 64, s_(g_elem.color_edit.circle_thickness));
    
    return held;
}

bool hb_edit(const std::string& name, float* h) {
    struct hb_state { c_vec2 grab_pos; };
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name.c_str());
    hb_state* state = anim_container<hb_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 mouse_pos = ImGui::GetIO().MousePos;
    c_vec2 size = s_(g_elem.color_edit.hb_size);
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    ImGui::ItemAdd(rect, id);
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    if (held) {
        *h = ImSaturate((mouse_pos.y - pos.y) / rect.GetHeight());
    }
    
    ImColor col_hues[7] = { ImColor(255, 0, 0), ImColor(255, 255, 0), ImColor(0, 255, 0), 
                           ImColor(0, 255, 255), ImColor(0, 0, 255), ImColor(255, 0, 255), ImColor(255, 0, 0) };
    
    for (int i = 0; i < 6; ++i) {
        float rounding = (i == 0 || i == 5) ? s_(g_elem.color_edit.hb_rounding) : 0;
        ImDrawFlags flags = (i == 0) ? ImDrawFlags_RoundCornersTop : (i == 5) ? ImDrawFlags_RoundCornersBottom : 0;
        c_vec2 seg_min(rect.Min.x, roundf(rect.Min.y + i * (rect.GetHeight() / 6)));
        c_vec2 seg_max(rect.Max.x, roundf(rect.Min.y + (i + 1) * (rect.GetHeight() / 6)));
        rect_filled_multicolor(window->DrawList, seg_min, seg_max, get_clr(col_hues[i]), get_clr(col_hues[i]), 
                              get_clr(col_hues[i + 1]), get_clr(col_hues[i + 1]), rounding, flags);
    }
    
    c_vec2 grab_pos(rect.GetCenter().x - s_(g_elem.color_edit.hb_grab_size.x / 2), pos.y + rect.GetHeight() * *h);
    grab_pos.y = ImClamp(grab_pos.y, pos.y, pos.y + size.y - s_(g_elem.color_edit.hb_grab_size.y));
    
    easing(state->grab_pos.x, grab_pos.x - pos.x, 24.f);
    easing(state->grab_pos.y, grab_pos.y - pos.y, 24.f);
    
    rect_filled(window->DrawList, state->grab_pos + pos, state->grab_pos + pos + s_(g_elem.color_edit.hb_grab_size), get_clr(g_clr.white), s_(g_elem.color_edit.grab_rounding), 0);
    return held;
}

bool ab_edit(const std::string& name, float h, float* a) {
    struct ab_state { c_vec2 grab_pos; };
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name.c_str());
    ab_state* state = anim_container<ab_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 mouse_pos = ImGui::GetIO().MousePos;
    c_vec2 size = s_(g_elem.color_edit.ab_size);
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    ImGui::ItemAdd(rect, id);
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    if (held) {
        *a = ImSaturate((mouse_pos.x - pos.x) / rect.GetWidth());
    }
    
    ImColor hue_col = ImColor::HSV(h, 1.f, 1.f);
    rect_filled_multicolor(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.white), get_clr(hue_col), 
                          get_clr(hue_col), get_clr(g_clr.white), s_(g_elem.color_edit.ab_rounding));
    
    c_vec2 grab_pos(pos.x + rect.GetWidth() * *a, rect.GetCenter().y - s_(g_elem.color_edit.ab_grab_size.y / 2));
    grab_pos.x = ImClamp(grab_pos.x, pos.x, pos.x + size.x - s_(g_elem.color_edit.ab_grab_size.x));
    
    easing(state->grab_pos.x, grab_pos.x - pos.x, 24.f);
    easing(state->grab_pos.y, grab_pos.y - pos.y, 24.f);
    
    rect_filled(window->DrawList, state->grab_pos + pos, state->grab_pos + pos + s_(g_elem.color_edit.ab_grab_size), get_clr(g_clr.white), s_(g_elem.color_edit.grab_rounding), 0);
    return held;
}

bool color_edit_ex(const std::string& name, c_vec4* color, color_edit_state* state, bool alpha) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    c_rect rect = window->Rect();
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.layout), s_(g_elem.color_edit.popup_rounding), 0);
    
    if (!state->init_val) {
        ImGui::ColorConvertRGBtoHSV(color->x, color->y, color->z, state->hsv[0], state->hsv[1], state->hsv[2]);
        state->hsv[3] = color->w;
        state->init_val = true;
    }
    
    sv_edit(name + "##sv_edit", state->hsv[0], &state->hsv[1], &state->hsv[2]);
    ImGui::SameLine();
    hb_edit(name + "##hb_edit", &state->hsv[0]);
    if (alpha) {
        ab_edit(name + "##ab_edit", state->hsv[0], &state->hsv[3]);
    }
    
    ImGui::ColorConvertHSVtoRGB(state->hsv[0], state->hsv[1], state->hsv[2], color->x, color->y, color->z);
    color->w = state->hsv[3];
    
    return true;
}

bool ColorEditButton(const std::string& name, const c_vec2& pos_min, const c_vec2& pos_max, c_vec4* color, bool alpha) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name.c_str());
    color_edit_state* state = anim_container<color_edit_state>(id);
    
    c_rect rect(pos_min, pos_max);
    
    bool hovered = rect.Contains(ImGui::GetIO().MousePos);
    bool pressed = hovered && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(0);
    
    if (pressed) {
        open_popup(std::to_string(id));
    }
    
    easing(state->hover, hovered ? 1.f : 0.f, 7.f, static_easing);
    
    window->DrawList->AddCircleFilled(rect.GetCenter(), s_(g_elem.color_edit_button.circle_size), get_clr(*color), 64);
    
    std::string popup_id = std::to_string(id);
    int popup_idx = -1;
    for (int i = 0; i < (int)g_popup_storage.size(); ++i) {
        if (g_popup_storage[i].name == popup_id) {
            popup_idx = i;
            break;
        }
    }
    
    if (popup_idx == -1) return false;
    
    easing(g_popup_storage[popup_idx].alpha, g_popup_storage[popup_idx].open ? 1.f : 0.f, 9.f, static_easing);
    
    if (g_popup_storage[popup_idx].alpha < 0.01f) return false;
    
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_popup_storage[popup_idx].alpha);
    ImGui::SetNextWindowPos(pos_min);
    ImGui::SetNextWindowSize(c_vec2(s_(g_elem.color_edit.popup_width), 0));
    
    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | 
                                   ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
                                   ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
    
    if (ImGui::Begin(("##color_popup_" + popup_id).c_str(), nullptr, popup_flags)) {
        g_popup_storage[popup_idx].size = ImGui::GetWindowSize();
        
        ImGui::SetWindowFocus();
        
        if (!ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()).Contains(ImGui::GetIO().MousePos) && 
            ImGui::IsMouseClicked(0)) {
            g_popup_storage[popup_idx].open = false;
        }
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, s_(g_elem.color_edit.padding));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(g_elem.color_edit.spacing));
        
        color_edit_ex(name, color, state, alpha);
        
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
    
    ImGui::PopStyleVar();
    
    return pressed;
}


bool Logo() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    // Load logo texture on first call
    // enhance_logo is defined in data/images.h which is included at the top of this file
    if (!g_logo_texture_loaded) {
        load_texture_from_file_memory(enhance_logo, sizeof(enhance_logo), &logo_texture);
        g_logo_texture_loaded = true;
    }
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 size = s_(g_elem.tab_switcher.size);
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, 0)) return false;
    
    c_vec2 center = rect.GetCenter();
    float circle_radius = s_(15);
    
    // Draw circle background matching theme (use accent color)
    window->DrawList->AddCircleFilled(center, circle_radius, get_clr(g_clr.accent), 64);
    
    // Draw logo image from images.h if loaded
    if (logo_texture.loaded && logo_texture.texture_id != 0) {
        // Calculate image size to fit inside circle (with some padding)
        float image_size = circle_radius * 1.6f;  // Slightly larger than circle for better visibility
        c_vec2 image_min = center - c_vec2(image_size / 2, image_size / 2);
        c_vec2 image_max = center + c_vec2(image_size / 2, image_size / 2);
        
        // Use white tint to match theme (image will be white/light colored)
        // The circle background already provides the accent color
        ImU32 tint_color = IM_COL32(255, 255, 255, 255);  // White tint
        
        window->DrawList->AddImage(
            (ImTextureID)(intptr_t)logo_texture.texture_id,
            image_min,
            image_max,
            ImVec2(0, 0),
            ImVec2(1, 1),
            tint_color
        );
    } else {
        // Fallback to icon font if texture not loaded
        if (g_font_icons_16) {
            text_clipped(window->DrawList, g_font_icons_16, rect.Min, rect.Max, get_clr(g_clr.white), "A", c_vec2(0.5f, 0.5f));
        }
    }
    
    return true;
}

bool TabButton(const char* label, const char* icon, bool selected) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiID id = window->GetID(label);
    
    struct tab_state { float alpha; c_vec4 text_color; };
    tab_state* state = anim_container<tab_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 size = s_(g_elem.tab_switcher.size);
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    easing(state->alpha, selected ? 1.f : 0.f, 24.f);
    easing(state->text_color, selected ? g_clr.accent : g_clr.text, 24.f);
    
    float remaining = ImGui::GetContentRegionAvail().y;
    bool is_last = remaining < s_(1);
    float rounding = is_last ? s_(g_elem.window.rounding) : 0.f;
    ImDrawFlags flags = is_last ? ImDrawFlags_RoundCornersBottomLeft : 0;
    
    if (state->alpha > 0.01f) {
        rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.lightchild, state->alpha), rounding, flags);
    }
    
    text_clipped(window->DrawList, g_font_icons_16, rect.Min, rect.Max, get_clr(state->text_color), icon, {0.5f, 0.4f});
    text_clipped(window->DrawList, g_font_inter_11, rect.Min, rect.Max, get_clr(state->text_color), label, {0.5f, 0.7f});
    
    return pressed;
}

bool Checkbox(const char* name, const char* description, bool* value, c_vec4* color = nullptr) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name);
    
    struct checkbox_state { float alpha; c_vec4 text_color; };
    checkbox_state* state = anim_container<checkbox_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    float width = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
                  ? ImGui::GetCurrentContext()->NextItemData.Width : content_avail_x();
    c_vec2 size(width, s_(g_elem.checkbox.height));
    c_rect rect(pos, pos + size);
    c_rect inner(rect.Min + s_(g_elem.checkbox.padding), rect.Max - s_(g_elem.checkbox.padding));
    c_vec2 checkbox_size = s_(g_elem.checkbox.button_size, g_elem.checkbox.button_size);
    c_rect button(c_vec2(inner.Max.x - checkbox_size.x, inner.GetCenter().y - checkbox_size.y / 2), 
                  c_vec2(inner.Max.x, inner.GetCenter().y + checkbox_size.y / 2));
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    if (pressed && value) *value = !*value;
    
    bool is_checked = value ? *value : false;
    easing(state->text_color, is_checked ? g_clr.white : g_clr.text, 24.f);
    easing(state->alpha, is_checked ? 1.f : 0.f, 24.f);
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.child), s_(g_elem.checkbox.rounding), 0);
    
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(state->text_color), name, {0.f, 0.f});
    if (description) {
        text_clipped(window->DrawList, g_font_inter_11, inner.Min, inner.Max, get_clr(g_clr.text), description, {0.f, 1.f});
    }
    
    rect_filled(window->DrawList, button.Min, button.Max, get_clr(g_clr.layout), s_(g_elem.checkbox.button_rounding), 0);
    rect_filled(window->DrawList, button.GetCenter() - (button.GetSize() / 2) * state->alpha, 
                button.GetCenter() + (button.GetSize() / 2) * state->alpha, 
                get_clr(g_clr.accent, state->alpha), s_(g_elem.checkbox.button_rounding), 0);
    
    if (state->alpha > 0.01f && g_font_icons_12) {
        text_clipped(window->DrawList, g_font_icons_12, button.Min, button.Max, get_clr(g_clr.black, state->alpha), "F", {0.5f, 0.5f});
    }
    
    // Color picker button
    if (color) {
        c_vec2 color_pos_min = button.GetTL() - c_vec2(s_(g_elem.checkbox.padding.x + g_elem.color_edit_button.size.x), 0);
        c_vec2 color_pos_max = color_pos_min + s_(g_elem.color_edit_button.size);
        std::string color_id = std::string(name) + "##color_edit";
        ColorEditButton(color_id, color_pos_min, color_pos_max, color, true);
    }
    
    return pressed;
}

bool Slider(const char* name, const char* description, float* value, float vmin, float vmax, const char* format) {
    if (!value) return false;
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name);
    
    struct slider_state { float absolute; };
    slider_state* state = anim_container<slider_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    float width = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
                  ? ImGui::GetCurrentContext()->NextItemData.Width : content_avail_x();
    c_vec2 size(width, s_(g_elem.slider.height));
    c_rect rect(pos, pos + size);
    c_rect inner(rect.Min + s_(g_elem.slider.padding), rect.Max - s_(g_elem.slider.padding));
    c_rect button(inner.GetBL() - c_vec2(0, s_(g_elem.slider.button_size)), inner.GetBR());
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(button, id, &hovered, &held);
    
    if (held) {
        float normalized = ImSaturate((ImGui::GetIO().MousePos.x - button.Min.x) / button.GetWidth());
        *value = vmin + normalized * (vmax - vmin);
    }
    
    float absolute = ImSaturate((*value - vmin) / (vmax - vmin));
    easing(state->absolute, absolute, 24.f);
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.child), s_(g_elem.slider.rounding), 0);
    
    char value_buf[64];
    snprintf(value_buf, sizeof(value_buf), format, *value);
    
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(g_clr.white), name, {0.f, 0.f});
    if (description) {
        text_clipped(window->DrawList, g_font_inter_11, inner.Min, inner.Max, get_clr(g_clr.text), description, {0.f, 0.45f});
    }
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(g_clr.white), value_buf, {1.f, 0.f});
    
    rect_filled(window->DrawList, button.Min, button.Max, get_clr(g_clr.layout), s_(g_elem.slider.button_rounding), 0);
    
    // Fill with gradient
    float fill_width = button.GetWidth() * state->absolute;
    c_vec2 fill_max(button.Min.x + fill_width, button.Max.y);
    
    window->DrawList->PushClipRect(button.Min, fill_max, true);
    rect_filled_multicolor(window->DrawList, button.Min, button.Max, 
                          get_clr(g_clr.accent, 0.7f), get_clr(g_clr.accent), 
                          get_clr(g_clr.accent), get_clr(g_clr.accent, 0.7f),
                          s_(g_elem.slider.button_rounding), 0);
    window->DrawList->PopClipRect();
    
    // Grab
    c_vec2 grab_size = s_(g_elem.slider.grab_size);
    float grab_x = button.GetWidth() * state->absolute;
    grab_x = ImClamp(grab_x, grab_size.x / 2, button.GetWidth() - grab_size.x / 2);
    grab_x += button.Min.x;
    float grab_y = button.GetCenter().y;
    c_vec2 grab_pos(grab_x, grab_y);
    
    rect_filled(window->DrawList, grab_pos - grab_size / 2, grab_pos + grab_size / 2, get_clr(g_clr.white), s_(g_elem.slider.grab_rounding), 0);
    
    return held;
}

bool Selectable(const std::string& name, bool selected) {
    struct selectable_state { float alpha; c_vec4 text_col; };
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name.c_str());
    selectable_state* state = anim_container<selectable_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 size(content_avail_x(), s_(g_elem.selectable.height));
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    float remaining = ImGui::GetContentRegionAvail().y;
    float rounding = (pos.y - window->Pos.y == 0 || remaining < s_(1)) ? s_(g_elem.dropdown.button_rounding) : 0;
    ImDrawFlags flags = (pos.y - window->Pos.y == 0) ? ImDrawFlags_RoundCornersTop : 
                       (remaining < s_(1)) ? ImDrawFlags_RoundCornersBottom : 0;
    
    easing(state->text_col, selected ? g_clr.white : g_clr.text, 24.f);
    easing(state->alpha, selected ? 1.f : 0.f, 24.f);
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.selectable, state->alpha), rounding, flags);
    
    float text_offset = s_(g_elem.dropdown.padding.x) + s_(g_elem.dropdown.padding.x * 2) * state->alpha;
    text_clipped(window->DrawList, g_font_inter_11, 
                rect.Min + c_vec2(text_offset, 0), rect.Max, 
                get_clr(state->text_col), name.c_str(), {0.f, 0.5f});
    text_clipped(window->DrawList, g_font_icons_12, 
                rect.Min + c_vec2(s_(g_elem.dropdown.padding.x) * state->alpha, 0), rect.Max, 
                get_clr(g_clr.accent, state->alpha), "F", {0.f, 0.5f});
    
    return pressed;
}

bool Dropdown(const char* name, const char* description, int* value, const std::vector<std::string>& variants) {
    if (!value || variants.empty()) return false;
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name);
    
    struct dropdown_state { float height; };
    dropdown_state* state = anim_container<dropdown_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    float width = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
                  ? ImGui::GetCurrentContext()->NextItemData.Width : content_avail_x();
    c_vec2 size(width, s_(g_elem.dropdown.height));
    c_rect rect(pos, pos + size);
    c_rect inner(rect.Min + s_(g_elem.dropdown.padding), rect.Max - s_(g_elem.dropdown.padding));
    c_rect button(inner.GetBL() - c_vec2(0, s_(g_elem.dropdown.button_size)), inner.GetBR());
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(button, id, &hovered, &held);
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.child), s_(g_elem.dropdown.rounding), 0);
    
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(g_clr.white), name, {0.f, 0.f});
    if (description) {
        text_clipped(window->DrawList, g_font_inter_11, inner.Min, inner.Max, get_clr(g_clr.text), description, {0.f, 0.25f});
    }
    
    rect_filled(window->DrawList, button.Min, button.Max, get_clr(g_clr.lightchild), s_(g_elem.dropdown.button_rounding), 0);
    
    const char* selected_text = (*value >= 0 && *value < (int)variants.size()) ? variants[*value].c_str() : "-";
    text_clipped(window->DrawList, g_font_inter_11, button.Min + c_vec2(s_(g_elem.dropdown.padding.x), 0), button.Max, 
                get_clr(g_clr.text), selected_text, {0.f, 0.5f});
    text_clipped(window->DrawList, g_font_icons_8, button.Min, button.Max - c_vec2(s_(g_elem.dropdown.padding.x), 0), 
                get_clr(g_clr.text), "C", {1.f, 0.5f});
    
    if (pressed) {
        open_popup(std::to_string(id));
    }
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c_vec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, c_vec2(0, 0));
    
    easing(state->height, is_popup_open(std::to_string(id)) ? (int)variants.size() * s_(g_elem.dropdown.button_size) : s_(1), 16.f);
    
    std::string popup_id = std::to_string(id);
    int popup_idx = -1;
    for (int i = 0; i < (int)g_popup_storage.size(); ++i) {
        if (g_popup_storage[i].name == popup_id) {
            popup_idx = i;
            break;
        }
    }
    
    if (popup_idx >= 0 && g_popup_storage[popup_idx].alpha > 0.01f) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_popup_storage[popup_idx].alpha);
        ImGui::SetNextWindowPos(button.Min);
        ImGui::SetNextWindowSize(c_vec2(button.GetWidth(), state->height));
        
        ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | 
                                       ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
                                       ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
        
        if (ImGui::Begin(("##dropdown_popup_" + popup_id).c_str(), nullptr, popup_flags)) {
            window = ImGui::GetCurrentWindow();
            rect = window->Rect();
            
            rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.lightchild), s_(g_elem.dropdown.button_rounding), 0);
            
            ImGui::SetWindowFocus();
            
            if (!ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()).Contains(ImGui::GetIO().MousePos) && 
                ImGui::IsMouseClicked(0)) {
                g_popup_storage[popup_idx].open = false;
            }
            
            for (int i = 0; i < (int)variants.size(); i++) {
                if (Selectable(variants[i], i == *value)) {
                    *value = i;
                    g_popup_storage[popup_idx].open = false;
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
    
    ImGui::PopStyleVar(2);
    
    return false;
}

bool MultiDropdown(const char* name, const char* description, std::vector<bool>* values, const std::vector<std::string>& variants) {
    if (!values || variants.empty() || values->size() != variants.size()) return false;
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name);
    
    struct dropdown_state { float height; };
    dropdown_state* state = anim_container<dropdown_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    float width = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
                  ? ImGui::GetCurrentContext()->NextItemData.Width : content_avail_x();
    c_vec2 size(width, s_(g_elem.dropdown.height));
    c_rect rect(pos, pos + size);
    c_rect inner(rect.Min + s_(g_elem.dropdown.padding), rect.Max - s_(g_elem.dropdown.padding));
    c_rect button(inner.GetBL() - c_vec2(0, s_(g_elem.dropdown.button_size)), inner.GetBR());
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(button, id, &hovered, &held);
    
    std::string preview = "-";
    for (size_t i = 0; i < variants.size(); ++i) {
        if ((*values)[i]) {
            if (preview == "-") preview = variants[i];
            else preview += ", " + variants[i];
            if (preview.length() > 25) { preview = preview.substr(0, 22) + "..."; break; }
        }
    }
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.child), s_(g_elem.dropdown.rounding), 0);
    
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(g_clr.white), name, {0.f, 0.f});
    if (description) {
        text_clipped(window->DrawList, g_font_inter_11, inner.Min, inner.Max, get_clr(g_clr.text), description, {0.f, 0.25f});
    }
    
    rect_filled(window->DrawList, button.Min, button.Max, get_clr(g_clr.lightchild), s_(g_elem.dropdown.button_rounding), 0);
    text_clipped(window->DrawList, g_font_inter_11, button.Min + c_vec2(s_(g_elem.dropdown.padding.x), 0), button.Max, 
                get_clr(g_clr.text), preview.c_str(), {0.f, 0.5f});
    text_clipped(window->DrawList, g_font_icons_8, button.Min, button.Max - c_vec2(s_(g_elem.dropdown.padding.x), 0), 
                get_clr(g_clr.text), "C", {1.f, 0.5f});
    
    if (pressed) open_popup(std::to_string(id));
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c_vec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, c_vec2(0, 0));
    
    easing(state->height, is_popup_open(std::to_string(id)) ? (int)variants.size() * s_(g_elem.dropdown.button_size) : s_(1), 16.f);
    
    std::string popup_id = std::to_string(id);
    int popup_idx = -1;
    for (int i = 0; i < (int)g_popup_storage.size(); ++i) {
        if (g_popup_storage[i].name == popup_id) {
            popup_idx = i;
            break;
        }
    }
    
    if (popup_idx >= 0 && g_popup_storage[popup_idx].alpha > 0.01f) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_popup_storage[popup_idx].alpha);
        ImGui::SetNextWindowPos(button.Min);
        ImGui::SetNextWindowSize(c_vec2(button.GetWidth(), state->height));
        
        ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | 
                                       ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
                                       ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
        
        if (ImGui::Begin(("##multidropdown_popup_" + popup_id).c_str(), nullptr, popup_flags)) {
            window = ImGui::GetCurrentWindow();
            rect = window->Rect();
            
            rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.lightchild), s_(g_elem.dropdown.button_rounding), 0);
            
            ImGui::SetWindowFocus();
            
            if (!ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()).Contains(ImGui::GetIO().MousePos) && 
                ImGui::IsMouseClicked(0)) {
                g_popup_storage[popup_idx].open = false;
            }
            
            for (int i = 0; i < (int)variants.size(); i++) {
                if (Selectable(variants[i], (*values)[i])) {
                    (*values)[i] = !(*values)[i];
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
    
    ImGui::PopStyleVar(2);
    
    return false;
}

bool Button(const char* name) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name);
    
    struct button_state { bool active; float timer; c_vec4 layout_col; };
    button_state* state = anim_container<button_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    float width = content_avail_x();
    c_vec2 size(width, s_(g_elem.button_elem.height));
    c_rect rect(pos, pos + size);
    c_rect inner(rect.Min + s_(g_elem.button_elem.padding), rect.Max - s_(g_elem.button_elem.padding));
    c_rect button(inner.GetBL() - c_vec2(0, s_(g_elem.button_elem.button_size)), inner.GetBR());
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(button, id, &hovered, &held);
    
    if (pressed) {
        state->active = true;
        state->timer = 0.f;
        state->layout_col = g_clr.lightchild;
    }
    
    if (state->active) {
        state->timer += fixed_speed(9.f);
        if (state->timer > 1.f) state->active = false;
    }
    
    easing(state->layout_col, state->active ? g_clr.button : g_clr.lightchild, 10.f);
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.child), s_(g_elem.button_elem.rounding), 0);
    
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(g_clr.white), name, c_vec2(0.f, 0.f));
    
    rect_filled(window->DrawList, button.Min, button.Max, get_clr(state->layout_col), s_(g_elem.button_elem.button_rounding), 0);
    text_clipped(window->DrawList, g_font_inter_11, button.Min, button.Max, get_clr(g_clr.white), "Press me!", {0.5f, 0.5f});
    
    return pressed;
}

// Simple styled button (for login screen)
bool SimpleButton(const char* label, float width = 0.f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(label);
    
    struct simple_button_state { float hover; c_vec4 bg_col; };
    simple_button_state* state = anim_container<simple_button_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    // Check for SetNextItemWidth
    float w = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
              ? ImGui::GetCurrentContext()->NextItemData.Width 
              : (width > 0 ? width : content_avail_x());
    c_vec2 size(w, s_(40));
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    easing(state->hover, hovered ? 1.f : 0.f, 12.f);
    c_vec4 target_col = held ? c_vec4(g_clr.accent.x * 0.7f, g_clr.accent.y * 0.7f, g_clr.accent.z * 0.7f, 1.f) :
                        hovered ? c_vec4(g_clr.accent.x * 1.1f, g_clr.accent.y * 1.1f, g_clr.accent.z * 1.1f, 1.f) : 
                        g_clr.accent;
    easing(state->bg_col, target_col, 12.f);
    
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(state->bg_col), s_(8), 0);
    text_clipped(window->DrawList, g_font_inter_12, rect.Min, rect.Max, get_clr(g_clr.white), label, {0.5f, 0.5f});
    
    return pressed;
}

// Text input field for login
bool InputText(const char* label, char* buf, size_t buf_size, bool password = false) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(label);
    
    struct input_state { float focus_anim; bool focused; };
    input_state* state = anim_container<input_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    // Check for SetNextItemWidth
    float width = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
                  ? ImGui::GetCurrentContext()->NextItemData.Width 
                  : content_avail_x();
    float label_height = s_(18);
    float input_height = s_(40);
    c_vec2 size(width, label_height + input_height + s_(6));
    
    c_rect label_rect(pos, c_vec2(pos.x + width, pos.y + label_height));
    c_rect input_rect(c_vec2(pos.x, pos.y + label_height + s_(6)), c_vec2(pos.x + width, pos.y + size.y));
    c_rect full_rect(pos, pos + size);
    
    ImGui::ItemSize(full_rect);
    if (!ImGui::ItemAdd(full_rect, id)) return false;
    
    // Label
    text_clipped(window->DrawList, g_font_inter_11, label_rect.Min, label_rect.Max, get_clr(g_clr.text), label, {0.f, 0.5f});
    
    // Input background with focus animation
    easing(state->focus_anim, state->focused ? 1.f : 0.f, 12.f);
    c_vec4 bg_col = ImLerp(g_clr.child, g_clr.lightchild, state->focus_anim * 0.3f);
    c_vec4 border_col = ImLerp(g_clr.selectable, g_clr.accent, state->focus_anim);
    
    rect_filled(window->DrawList, input_rect.Min, input_rect.Max, get_clr(bg_col), s_(8), 0);
    
    // Border
    window->DrawList->AddRect(input_rect.Min, input_rect.Max, get_clr(border_col, 0.6f + state->focus_anim * 0.4f), s_(8), 0, s_(1));
    
    // Handle click to focus
    bool hovered = input_rect.Contains(ImGui::GetIO().MousePos);
    if (hovered && ImGui::IsMouseClicked(0)) {
        state->focused = true;
    }
    // Unfocus when clicking elsewhere
    if (!hovered && ImGui::IsMouseClicked(0)) {
        state->focused = false;
    }
    
    // Handle text input when focused
    bool changed = false;
    if (state->focused) {
        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
            unsigned int c = io.InputQueueCharacters[i];
            if (c == '\b') {
                size_t len = strlen(buf);
                if (len > 0) {
                    buf[len - 1] = '\0';
                    changed = true;
                }
            } else if (c >= 32 && c < 127) {
                size_t len = strlen(buf);
                if (len + 1 < buf_size) {
                    buf[len] = (char)c;
                    buf[len + 1] = '\0';
                    changed = true;
                }
            }
        }
        // Handle backspace key
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            size_t len = strlen(buf);
            if (len > 0) {
                buf[len - 1] = '\0';
                changed = true;
            }
        }
    }
    
    // Display text (masked if password)
    std::string display_text;
    if (password) {
        display_text = std::string(strlen(buf), '*');
    } else {
        display_text = buf;
    }
    
    // Show cursor when focused
    if (state->focused) {
        float time = (float)ImGui::GetTime();
        if (fmodf(time, 1.0f) < 0.5f) {
            display_text += "|";
        }
    }
    
    c_vec2 text_pos = input_rect.Min + s_(12, 0);
    text_clipped(window->DrawList, g_font_inter_12, text_pos, input_rect.Max - s_(12, 0), 
                 get_clr(strlen(buf) > 0 ? g_clr.white : g_clr.text), 
                 display_text.empty() ? "" : display_text.c_str(), {0.f, 0.5f});
    
    return changed;
}

// Clickable text link
bool TextLink(const char* text, const c_vec4& color = c_vec4(0, 0, 0, 0)) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(text);
    
    struct link_state { float hover; };
    link_state* state = anim_container<link_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    ImGui::PushFont(g_font_inter_11);
    c_vec2 text_size = ImGui::CalcTextSize(text);
    ImGui::PopFont();
    c_rect rect(pos, pos + text_size);
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered = rect.Contains(ImGui::GetIO().MousePos);
    bool pressed = hovered && ImGui::IsMouseClicked(0);
    
    easing(state->hover, hovered ? 1.f : 0.f, 12.f);
    
    c_vec4 use_color = (color.w > 0) ? color : g_clr.accent;
    c_vec4 final_color = ImLerp(use_color, g_clr.white, state->hover * 0.3f);
    
    text_clipped(window->DrawList, g_font_inter_11, rect.Min, rect.Max, get_clr(final_color), text, {0.f, 0.5f});
    
    // Underline on hover
    if (state->hover > 0.01f) {
        window->DrawList->AddLine(
            c_vec2(rect.Min.x, rect.Max.y), 
            c_vec2(rect.Max.x, rect.Max.y), 
            get_clr(final_color, state->hover), s_(1));
    }
    
    return pressed;
}

bool IconButton(const std::string& name, const std::string& icon) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name.c_str());
    
    struct icon_state { c_vec4 icon_col; };
    icon_state* state = anim_container<icon_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    c_vec2 size = s_(10, 10);
    c_rect rect(pos, pos + size);
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    
    easing(state->icon_col, hovered ? g_clr.white : g_clr.text, 24.f);
    
    text_clipped(window->DrawList, g_font_icons_10, rect.Min, rect.Max, get_clr(state->icon_col), icon.c_str(), {0.5f, 0.5f});
    
    return pressed;
}

// Helper function to get key name from virtual key code
static const char* GetKeyName(int vk) {
    static char buf[32];
    if (vk == 0) return "None";
    if (vk >= 'A' && vk <= 'Z') { buf[0] = (char)vk; buf[1] = 0; return buf; }
    if (vk >= '0' && vk <= '9') { buf[0] = (char)vk; buf[1] = 0; return buf; }
    if (vk >= VK_F1 && vk <= VK_F12) { sprintf(buf, "F%d", vk - VK_F1 + 1); return buf; }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) { sprintf(buf, "Num%d", vk - VK_NUMPAD0); return buf; }
    switch (vk) {
        case VK_LBUTTON: return "LMB";
        case VK_RBUTTON: return "RMB";
        case VK_MBUTTON: return "MMB";
        case VK_XBUTTON1: return "Mouse4";
        case VK_XBUTTON2: return "Mouse5";
        case VK_SPACE: return "Space";
        case VK_TAB: return "Tab";
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: return "Shift";
        case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return "Ctrl";
        case VK_MENU: case VK_LMENU: case VK_RMENU: return "Alt";
        case VK_ESCAPE: return "Esc";
        case VK_CAPITAL: return "CapsLock";
        case VK_RETURN: return "Enter";
        case VK_BACK: return "Backspace";
        case VK_DELETE: return "Delete";
        case VK_INSERT: return "Insert";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_PRIOR: return "PgUp";
        case VK_NEXT: return "PgDn";
        case VK_UP: return "Up";
        case VK_DOWN: return "Down";
        case VK_LEFT: return "Left";
        case VK_RIGHT: return "Right";
        default: sprintf(buf, "[%d]", vk); return buf;
    }
}

static const char* GetModeName(int mode) {
    switch (mode) {
        case 0: return "Hold";
        case 1: return "Toggle";
        case 2: return "Always";
        default: return "Toggle";
    }
}

// Keybind widget - matches checkbox style with right-click mode selection
bool Keybind(const char* name, const char* description, int* keybind, int* mode = nullptr) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    
    ImGuiID id = window->GetID(name);
    std::string popup_id = std::string(name) + "_mode_popup";
    
    struct keybind_state { 
        bool waiting; 
        c_vec4 text_col;
        float alpha;
        float mode_popup_height;
    };
    keybind_state* state = anim_container<keybind_state>(id);
    
    c_vec2 pos = window->DC.CursorPos;
    float width = (ImGui::GetCurrentContext()->NextItemData.Flags & ImGuiNextItemDataFlags_HasWidth) 
                  ? ImGui::GetCurrentContext()->NextItemData.Width : content_avail_x();
    c_vec2 size(width, s_(g_elem.checkbox.height));
    c_rect rect(pos, pos + size);
    c_rect inner(rect.Min + s_(g_elem.checkbox.padding), rect.Max - s_(g_elem.checkbox.padding));
    
    // Keybind button on the right (like checkbox toggle)
    c_vec2 button_size = s_(g_elem.checkbox.button_size * 2.5f, g_elem.checkbox.button_size);
    c_rect button(c_vec2(inner.Max.x - button_size.x, inner.GetCenter().y - button_size.y / 2), 
                  c_vec2(inner.Max.x, inner.GetCenter().y + button_size.y / 2));
    
    // Mode indicator (small text next to button)
    float mode_width = mode ? s_(40) : 0;
    c_rect mode_rect(c_vec2(button.Min.x - mode_width - s_(4), button.Min.y), 
                     c_vec2(button.Min.x - s_(4), button.Max.y));
    
    ImGui::ItemSize(rect);
    if (!ImGui::ItemAdd(rect, id)) return false;
    
    bool hovered = rect.Contains(ImGui::GetIO().MousePos);
    bool button_hovered = button.Contains(ImGui::GetIO().MousePos);
    bool pressed = button_hovered && ImGui::IsMouseClicked(0);
    bool right_clicked = hovered && ImGui::IsMouseClicked(1) && mode;
    
    // Sync waiting state with global ID
    state->waiting = (g_keybind_waiting_id == id);
    
    // Handle key detection when waiting
    if (state->waiting) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            g_keybind_waiting_id = 0;
            state->waiting = false;
        }
        else {
            // Check mouse buttons (except left/right when clicking UI)
            if (!ImGui::IsMouseDown(0) && !ImGui::IsMouseDown(1)) {
                for (int vk : {VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2}) {
                    if (GetAsyncKeyState(vk) & 0x8000) {
                        *keybind = vk;
                        g_keybind_waiting_id = 0;
                        state->waiting = false;
                        break;
                    }
                }
            }
            // Check keyboard keys
            if (state->waiting) {
                for (int vk = 0x08; vk <= 0xFE; vk++) {
                    if (vk == VK_ESCAPE || vk == VK_LBUTTON || vk == VK_RBUTTON) continue;
                    if (GetAsyncKeyState(vk) & 0x8000) {
                        *keybind = vk;
                        g_keybind_waiting_id = 0;
                        state->waiting = false;
                        break;
                    }
                }
            }
        }
    }
    
    if (pressed && !state->waiting) {
        g_keybind_waiting_id = id;
        state->waiting = true;
    }
    
    // Right-click opens mode popup
    if (right_clicked) {
        open_popup(popup_id);
    }
    
    // Animations
    bool has_key = keybind && *keybind != 0;
    easing(state->text_col, has_key ? g_clr.white : g_clr.text, 24.f);
    easing(state->alpha, has_key ? 1.f : 0.f, 24.f);
    
    // Background
    rect_filled(window->DrawList, rect.Min, rect.Max, get_clr(g_clr.child), s_(g_elem.checkbox.rounding), 0);
    
    // Labels (like checkbox)
    text_clipped(window->DrawList, g_font_inter_12, inner.Min, inner.Max, get_clr(state->text_col), name, {0.f, 0.f});
    if (description) {
        text_clipped(window->DrawList, g_font_inter_11, inner.Min, inner.Max, get_clr(g_clr.text), description, {0.f, 1.f});
    }
    
    // Mode text (if mode provided)
    if (mode) {
        text_clipped(window->DrawList, g_font_inter_11, mode_rect.Min, mode_rect.Max, 
                     get_clr(g_clr.text), GetModeName(*mode), {1.f, 0.5f});
    }
    
    // Keybind button (like checkbox toggle but wider)
    rect_filled(window->DrawList, button.Min, button.Max, get_clr(g_clr.layout), s_(g_elem.checkbox.button_rounding), 0);
    rect_filled(window->DrawList, button.GetCenter() - (button.GetSize() / 2) * state->alpha, 
                button.GetCenter() + (button.GetSize() / 2) * state->alpha, 
                get_clr(state->waiting ? g_clr.accent : g_clr.accent, state->alpha), 
                s_(g_elem.checkbox.button_rounding), 0);
    
    // Key text
    const char* key_text = state->waiting ? "..." : GetKeyName(keybind ? *keybind : 0);
    text_clipped(window->DrawList, g_font_inter_11, button.Min, button.Max, 
                 get_clr(has_key || state->waiting ? g_clr.white : g_clr.text), key_text, {0.5f, 0.5f});
    
    // Mode selection popup
    if (mode) {
        int popup_idx = -1;
        for (int i = 0; i < (int)g_popup_storage.size(); ++i) {
            if (g_popup_storage[i].name == popup_id) {
                popup_idx = i;
                break;
            }
        }
        
        if (popup_idx >= 0 && g_popup_storage[popup_idx].alpha > 0.01f) {
            float item_h = s_(28);
            float target_h = item_h * 3 + s_(8);
            easing(state->mode_popup_height, g_popup_storage[popup_idx].open ? target_h : 0.f, 14.f);
            
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_popup_storage[popup_idx].alpha);
            ImGui::SetNextWindowPos(c_vec2(rect.Min.x, rect.Max.y + s_(4)));
            ImGui::SetNextWindowSize(c_vec2(s_(120), state->mode_popup_height));
            
            ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | 
                                           ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
                                           ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
            
            if (ImGui::Begin(("##" + popup_id).c_str(), nullptr, popup_flags)) {
                ImGuiWindow* popup_win = ImGui::GetCurrentWindow();
                c_rect popup_rect = popup_win->Rect();
                rect_filled(popup_win->DrawList, popup_rect.Min, popup_rect.Max, get_clr(g_clr.child), s_(5), 0);
                
                ImGui::SetWindowFocus();
                
                // Close if clicked outside
                if (!popup_rect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseClicked(0)) {
                    g_popup_storage[popup_idx].open = false;
                }
                
                const char* modes[] = {"Hold", "Toggle", "Always"};
                for (int i = 0; i < 3; i++) {
                    if (Selectable(modes[i], *mode == i)) {
                        *mode = i;
                        g_popup_storage[popup_idx].open = false;
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }
    
    return pressed;
}

bool BeginChildWindow(const char* title, const c_vec2& default_size = c_vec2(0, 0), bool border = true) {
    ImGuiWindow* parent = ImGui::GetCurrentWindow();
    if (parent->SkipItems) return false;
    
    ImGuiID id = parent->GetID(title);
    child_state_t& state = g_child_states[id];
    
    c_vec2 size;
    size.x = default_size.x > 0 ? default_size.x : content_avail_x();
    size.y = default_size.y > 0 ? default_size.y : s_(250);
    
    float header_h = s_(26);
    float scrollbar_w = s_(5);
    float pad = s_(8);
    float round = s_(4);
    
    c_vec2 screen_pos = parent->DC.CursorPos;
    g_child_start_pos = ImGui::GetCursorPos();
    g_child_size_stored = size;
    
    c_rect box(screen_pos, screen_pos + size);
    c_rect header(screen_pos, c_vec2(screen_pos.x + size.x, screen_pos.y + header_h));
    c_rect content_box(c_vec2(screen_pos.x, screen_pos.y + header_h), 
                       c_vec2(screen_pos.x + size.x - scrollbar_w, screen_pos.y + size.y));
    c_rect scrollbar_area(c_vec2(screen_pos.x + size.x - scrollbar_w, screen_pos.y + header_h + s_(3)), 
                          c_vec2(screen_pos.x + size.x - s_(2), screen_pos.y + size.y - s_(3)));
    
    ImDrawList* dl = parent->DrawList;
    
    rect_filled(dl, box.Min, box.Max, get_clr(g_clr.child), round, 0);
    rect_filled(dl, header.Min, header.Max, get_clr(g_clr.lightchild), round, ImDrawFlags_RoundCornersTop);
    
    if (title && g_font_inter_12) {
        ImGui::PushFont(g_font_inter_12);
        c_vec2 txt_size = ImGui::CalcTextSize(title);
        c_vec2 txt_pos(header.Min.x + pad, header.GetCenter().y - txt_size.y * 0.5f);
        dl->AddText(txt_pos, get_clr(g_clr.white), title);
        ImGui::PopFont();
    }
    
    ImGui::SetCursorPos(g_child_start_pos + c_vec2(0, header_h));
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c_vec2(pad, pad));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, c_vec2(pad, pad));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    
    c_vec2 inner_size(content_box.GetWidth(), content_box.GetHeight());
    std::string cid = std::string("##cw_") + title;
    
    bool ret = ImGui::BeginChild(cid.c_str(), inner_size, false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);
    
    if (ret) {
        ImGuiWindow* cw = ImGui::GetCurrentWindow();
        
        float content_h = cw->ContentSize.y + pad * 2;
        float view_h = inner_size.y;
        float max_scroll = ImMax(0.f, content_h - view_h);
        
        c_vec2 m = ImGui::GetIO().MousePos;
        bool hover_content = content_box.Contains(m);
        
        if (hover_content && ImGui::GetIO().MouseWheel != 0.f && max_scroll > 0) {
            state.scroll -= ImGui::GetIO().MouseWheel * s_(40);
            state.scroll = ImClamp(state.scroll, 0.f, max_scroll);
            g_child_consumed_scroll = true;
        }
        
        // If content fits, reset scroll to 0
        if (max_scroll <= 0) {
            state.scroll = 0.f;
            state.scroll_anim = 0.f;
        }
        
        state.scroll = ImClamp(state.scroll, 0.f, max_scroll);
        easing(state.scroll_anim, state.scroll, 14.f);
        cw->Scroll.y = state.scroll_anim;
        
        if (max_scroll > 0) {
            float track_h = scrollbar_area.GetHeight();
            float grab_h = ImMax(s_(20), track_h * (view_h / content_h));
            float ratio = max_scroll > 0 ? state.scroll_anim / max_scroll : 0.f;
            float grab_y = scrollbar_area.Min.y + (track_h - grab_h) * ratio;
            
            c_rect grab(c_vec2(scrollbar_area.Min.x, grab_y), c_vec2(scrollbar_area.Max.x, grab_y + grab_h));
            
            bool grab_hover = grab.Contains(m);
            bool track_hover = scrollbar_area.Contains(m) && !grab_hover;
            
            if ((grab_hover || track_hover) && ImGui::IsMouseClicked(0)) {
                state.drag_scrollbar = true;
                if (track_hover) {
                    float cr = (m.y - scrollbar_area.Min.y - grab_h * 0.5f) / (track_h - grab_h);
                    state.scroll = ImClamp(cr, 0.f, 1.f) * max_scroll;
                }
                state.drag_offset = m.y - grab_y;
            }
            
            if (state.drag_scrollbar) {
                if (ImGui::IsMouseDown(0)) {
                    float gy = m.y - state.drag_offset;
                    float nr = (gy - scrollbar_area.Min.y) / (track_h - grab_h);
                    state.scroll = ImClamp(nr, 0.f, 1.f) * max_scroll;
                } else {
                    state.drag_scrollbar = false;
                }
            }
            
            float alpha_target = (grab_hover || state.drag_scrollbar || track_hover) ? 1.f : 0.4f;
            easing(state.scrollbar_alpha, alpha_target, 12.f);
            
            float fr = max_scroll > 0 ? state.scroll_anim / max_scroll : 0.f;
            float fgy = scrollbar_area.Min.y + (track_h - grab_h) * fr;
            c_rect fg(c_vec2(scrollbar_area.Min.x, fgy), c_vec2(scrollbar_area.Max.x, fgy + grab_h));
            
            c_vec4 track_c = g_clr.layout;
            track_c.w *= 0.25f * state.scrollbar_alpha;
            rect_filled(dl, scrollbar_area.Min, scrollbar_area.Max, get_clr(track_c), s_(2), 0);
            
            c_vec4 grab_c = state.drag_scrollbar ? g_clr.accent : (grab_hover ? g_clr.text : g_clr.selectable);
            grab_c.w *= state.scrollbar_alpha;
            rect_filled(dl, fg.Min, fg.Max, get_clr(grab_c), s_(2), 0);
        }
    }
    
    return ret;
}

void EndChildWindow() {
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
    
    ImGui::SetCursorPos(g_child_start_pos + c_vec2(0, g_child_size_stored.y + s_(g_elem.content.padding.y)));
}

// ============================================
// Apply theme
// ============================================
void ApplyTheme(int theme) {
    switch (theme) {
        case 0:
            easing(g_clr.layout, c_col(15, 15, 18).Value, 24.f);
            easing(g_clr.child, c_col(17, 17, 21).Value, 24.f);
            easing(g_clr.text, c_col(92, 95, 122).Value, 24.f);
            easing(g_clr.lightchild, c_col(20, 20, 25).Value, 24.f);
            easing(g_clr.selectable, c_col(22, 22, 28).Value, 24.f);
            easing(g_clr.button, c_col(50, 50, 65).Value, 24.f);
            break;
        case 1:
            easing(g_clr.layout, c_col(26, 26, 30).Value, 24.f);
            easing(g_clr.child, c_col(31, 31, 36).Value, 24.f);
            easing(g_clr.text, c_col(128, 128, 143).Value, 24.f);
            easing(g_clr.lightchild, c_col(36, 36, 42).Value, 24.f);
            easing(g_clr.selectable, c_col(40, 40, 46).Value, 24.f);
            easing(g_clr.button, c_col(70, 70, 90).Value, 24.f);
            break;
        case 2:
            easing(g_clr.layout, c_col(8, 8, 8).Value, 24.f);
            easing(g_clr.child, c_col(10, 10, 10).Value, 24.f);
            easing(g_clr.text, c_col(150, 150, 150).Value, 24.f);
            easing(g_clr.lightchild, c_col(15, 15, 15).Value, 24.f);
            easing(g_clr.selectable, c_col(22, 22, 22).Value, 24.f);
            easing(g_clr.button, c_col(50, 50, 50).Value, 24.f);
            break;
    }
}

// ============================================
// Update DPI and rebuild fonts (called from GUI.cpp)
// ============================================
void UpdateWolfMenuDPI() {
    // Update DPI from menu scale
    int new_dpi = (int)g_menu_scale;
    // Clamp to valid range to prevent crashes
    if (new_dpi < 50) new_dpi = 50;
    if (new_dpi > 300) new_dpi = 300;
    
    if (g_stored_dpi != new_dpi) {
        g_stored_dpi = new_dpi;
        g_dpi_changed = true;
    }
    g_dpi = g_stored_dpi / 100.f;
    
    // Only rebuild fonts if DPI changed
    // This is called before ImGui::NewFrame(), so it's safe to rebuild fonts here
    if (g_dpi_changed) {
        InitializeFonts();
    }
}

static bool g_logged_in = true;
static bool g_login_loading = false;
static bool g_token_validated = true;
static char g_login_email[128] = "";
static char g_login_password[128] = "";
static std::string g_login_error = "";
static float g_login_error_alpha = 0.f;

void ValidateStoredToken();

bool IsLoggedIn() {
    if (!g_token_validated) {
        ValidateStoredToken();
    }
    return g_logged_in;
}

void ResetLogin() {
    g_logged_in = false;
    g_token_validated = false;
    memset(g_login_email, 0, sizeof(g_login_email));
    memset(g_login_password, 0, sizeof(g_login_password));
    g_login_error = "";
    g_login_error_alpha = 0.f;
    token_storage::ClearToken();
}

void ValidateStoredToken() {
    if (g_token_validated) return;
    g_token_validated = true;
    
    std::string token = token_storage::GetToken();
    if (token.empty()) {
        return;
    }
    
    // Add HWID to validation request
    std::string hwid_str = hwid::GetHWID();
    std::string api_url = http_client::GetApiUrl() + "/auth/me?hwid=" + hwid_str;
    http_client::Response response = http_client::Get(api_url, token);
    
    if (response.success && response.status_code == 200) {
        g_logged_in = true;
    } else {
        // Handle HWID mismatch
        if (response.status_code == 401 && response.body.find("HWID") != std::string::npos) {
            g_login_error = "HWID mismatch. This account is locked to another PC.";
        }
        token_storage::ClearToken();
    }
}

void RenderLoginScreen() {
    if (!g_clr.is_valid()) {
        g_clr.reset_to_defaults();
    }
    
    ApplyTheme(g_theme);
    
    c_vec2 login_size = s_(300, 380);
    ImVec2 display_size = ImGui::GetIO().DisplaySize;
    ImVec2 login_pos((display_size.x - login_size.x) / 2, (display_size.y - login_size.y) / 2);
    
    ImGui::SetNextWindowPos(login_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(login_size, ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | 
                             ImGuiWindowFlags_NoMove;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c_vec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(0, 8));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, c_vec4(0, 0, 0, 0));
    
    if (ImGui::Begin("##login_window", nullptr, flags)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        c_vec2 win_pos = ImGui::GetWindowPos();
        c_vec2 win_size = ImGui::GetWindowSize();
        
        float rounding = s_(12);
        float padding = s_(24);
        
        rect_filled(dl, win_pos, win_pos + win_size, get_clr(g_clr.layout), rounding, 0);
        
        dl->AddRect(win_pos, win_pos + win_size, get_clr(g_clr.child, 0.5f), rounding, 0, s_(1));
        
        float header_height = s_(50);
        
        c_vec2 title_min(win_pos.x, win_pos.y + s_(10));
        c_vec2 title_max(win_pos.x + win_size.x, win_pos.y + header_height);
        text_clipped(dl, g_font_inter_12, title_min, title_max, 
                    get_clr(g_clr.white), "Enhance Client", {0.5f, 0.5f});
        
        ImGui::SetCursorPos(c_vec2(padding, header_height + s_(10)));
        
        float content_width = win_size.x - padding * 2;
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, s_(0, 6));
        
        ImGui::SetNextItemWidth(content_width);
        InputText("Email", g_login_email, sizeof(g_login_email), false);
        
        ImGui::SetCursorPosX(padding);
        ImGui::Dummy(s_(0, 2));
        
        ImGui::SetCursorPosX(padding);
        ImGui::SetNextItemWidth(content_width);
        InputText("Password", g_login_password, sizeof(g_login_password), true);
        
        ImGui::PopStyleVar();
        
        if (!g_login_error.empty()) {
            easing(g_login_error_alpha, 1.f, 8.f);
            ImGui::SetCursorPosX(padding);
            ImGui::Dummy(s_(0, 8));
            c_vec2 error_pos = ImGui::GetCursorScreenPos();
            text_clipped(dl, g_font_inter_11, error_pos, 
                        c_vec2(error_pos.x + content_width, error_pos.y + s_(14)),
                        get_clr(c_vec4(1.f, 0.4f, 0.4f, g_login_error_alpha)), g_login_error.c_str(), {0.5f, 0.5f});
            ImGui::Dummy(s_(0, 14));
        } else {
            g_login_error_alpha = 0.f;
            ImGui::Dummy(s_(0, 12));
        }
        
        ImGui::SetCursorPosX(padding);
        ImGui::SetNextItemWidth(content_width);
        
        if (g_login_loading) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
        
        if (SimpleButton(g_login_loading ? "Logging in..." : "Login")) {
            if (!g_login_loading) {
                // Validate input
                if (strlen(g_login_email) == 0 || strlen(g_login_password) == 0) {
                    g_login_error = "Please enter email and password";
                    g_login_error_alpha = 0.f;
                } else {
                    g_login_loading = true;
                    g_login_error = "";
                    
                    std::string email_copy(g_login_email);
                    std::string password_copy(g_login_password);
                    
                    SecureZeroMemory(g_login_password, sizeof(g_login_password));
                    
                    std::thread([email_copy, password_copy]() mutable {
                        if (!http_client::Initialize()) {
                            g_login_error = "Failed to initialize network";
                            g_login_error_alpha = 0.f;
                            g_login_loading = false;
                            return;
                        }
                        
                        auto escape_json = [](const std::string& str) -> std::string {
                            std::string result;
                            result.reserve(str.length() * 2);
                            for (char c : str) {
                                if (c == '"') result += "\\\"";
                                else if (c == '\\') result += "\\\\";
                                else if (c == '\n') result += "\\n";
                                else if (c == '\r') result += "\\r";
                                else if (c == '\t') result += "\\t";
                                else result += c;
                            }
                            return result;
                        };
                        
                        // Get HWID
                        std::string hwid_str = hwid::GetHWID();
                        
                        std::ostringstream json;
                        json << "{\"email\":\"" << escape_json(email_copy) 
                             << "\",\"password\":\"" << escape_json(password_copy) 
                             << "\",\"captchaToken\":\"\""
                             << ",\"hwid\":\"" << hwid_str << "\"}";
                        
                        std::string json_str = json.str();
                        
                        if (!password_copy.empty()) {
                            char* pwd_ptr = const_cast<char*>(password_copy.data());
                            for (size_t i = 0; i < password_copy.length(); ++i) {
                                pwd_ptr[i] = 0;
                            }
                            password_copy.clear();
                            password_copy.shrink_to_fit();
                        }
                        
                        std::string api_url = http_client::GetApiUrl() + "/auth/login";
                        http_client::Response response = http_client::Post(api_url, json_str);
                        
                        if (!json_str.empty()) {
                            char* json_ptr = const_cast<char*>(json_str.data());
                            for (size_t i = 0; i < json_str.length(); ++i) {
                                json_ptr[i] = 0;
                            }
                            json_str.clear();
                            json_str.shrink_to_fit();
                        }
                        
                        if (response.success && response.status_code == 200) {
                            std::string token;
                            size_t token_pos = response.body.find("\"token\":\"");
                            if (token_pos != std::string::npos) {
                                token_pos += 9;
                                size_t token_end = response.body.find("\"", token_pos);
                                if (token_end != std::string::npos) {
                                    token = response.body.substr(token_pos, token_end - token_pos);
                                }
                            }
                            
                            if (token.empty()) {
                                token_pos = response.body.find("\"token\"");
                                if (token_pos != std::string::npos) {
                                    token_pos = response.body.find(":", token_pos);
                                    if (token_pos != std::string::npos) {
                                        token_pos = response.body.find("\"", token_pos);
                                        if (token_pos != std::string::npos) {
                                            token_pos++;
                                            size_t token_end = response.body.find("\"", token_pos);
                                            if (token_end != std::string::npos) {
                                                token = response.body.substr(token_pos, token_end - token_pos);
                                            }
                                        }
                                    }
                                }
                            }
                            
                            if (!token.empty()) {
                                if (token_storage::StoreToken(token)) {
                                    char* token_ptr = const_cast<char*>(token.data());
                                    for (size_t i = 0; i < token.length(); ++i) {
                                        token_ptr[i] = 0;
                                    }
                                    token.clear();
                                    token.shrink_to_fit();
                                    
                                    g_logged_in = true;
                                    g_login_error = "";
                                } else {
                                    char* token_ptr = const_cast<char*>(token.data());
                                    for (size_t i = 0; i < token.length(); ++i) {
                                        token_ptr[i] = 0;
                                    }
                                    token.clear();
                                    token.shrink_to_fit();
                                    
                                    g_login_error = "Failed to save authentication";
                                    g_login_error_alpha = 0.f;
                                }
                            } else {
                                g_login_error = "Invalid server response";
                                g_login_error_alpha = 0.f;
                            }
                        } else {
                            if (response.status_code == 400 || response.status_code == 401) {
                                size_t msg_pos = response.body.find("\"message\":\"");
                                if (msg_pos != std::string::npos) {
                                    msg_pos += 10;
                                    size_t msg_end = response.body.find("\"", msg_pos);
                                    if (msg_end != std::string::npos) {
                                        g_login_error = response.body.substr(msg_pos, msg_end - msg_pos);
                                    } else {
                                        g_login_error = "Invalid email or password";
                                    }
                                } else {
                                    // Check for specific error messages
                                    if (response.body.find("HWID") != std::string::npos) {
                                        if (response.body.find("mismatch") != std::string::npos) {
                                            g_login_error = "HWID mismatch. This account is locked to another PC. Please contact support.";
                                        } else {
                                            g_login_error = "HWID verification failed. Please contact support.";
                                        }
                                    } else if (response.body.find("Invalid") != std::string::npos) {
                                        g_login_error = "Invalid email or password";
                                    } else if (response.body.find("Too many") != std::string::npos) {
                                        g_login_error = "Too many login attempts. Please try again later.";
                                    } else {
                                        g_login_error = "Invalid email or password";
                                    }
                                }
                            } else if (response.status_code == 0) {
                                g_login_error = "Network error: Unable to connect to server";
                            } else if (response.status_code == 429) {
                                g_login_error = "Too many attempts. Please wait.";
                            } else {
                                g_login_error = "Server error: " + std::to_string(response.status_code);
                            }
                            g_login_error_alpha = 0.f;
                        }
                        
                        g_login_loading = false;
                    }).detach();
                }
            }
        }
        
        if (g_login_loading) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }
        
        // Register link at bottom
        ImGui::Dummy(s_(0, 16));
        
        // Calculate centered position for the register text
        if (g_font_inter_11) {
            ImGui::PushFont(g_font_inter_11);
            const char* prefix = "Don't have an account? ";
            const char* link_text = "Register";
            c_vec2 prefix_size = ImGui::CalcTextSize(prefix);
            c_vec2 link_size = ImGui::CalcTextSize(link_text);
            ImGui::PopFont();
            
            float total_width = prefix_size.x + link_size.x;
            float start_x = win_pos.x + (win_size.x - total_width) / 2;
            float text_y = ImGui::GetCursorScreenPos().y;
            
            // Draw prefix text
            dl->AddText(g_font_inter_11, g_font_inter_11->FontSize, 
                       c_vec2(start_x, text_y), 
                       get_clr(g_clr.text), prefix);
            
            // Register link (clickable)
            c_vec2 link_min(start_x + prefix_size.x, text_y);
            c_vec2 link_max(link_min.x + link_size.x, text_y + link_size.y);
            c_rect link_rect(link_min, link_max);
            
            bool link_hovered = link_rect.Contains(ImGui::GetIO().MousePos);
            bool link_clicked = link_hovered && ImGui::IsMouseClicked(0);
            
            static float register_hover_anim = 0.f;
            easing(register_hover_anim, link_hovered ? 1.f : 0.f, 12.f);
            
            c_vec4 link_color = ImLerp(g_clr.accent, g_clr.white, register_hover_anim * 0.3f);
            
            dl->AddText(g_font_inter_11, g_font_inter_11->FontSize,
                       link_min, get_clr(link_color), link_text);
            
            // Underline
            float underline_alpha = 0.3f + register_hover_anim * 0.7f;
            dl->AddLine(
                c_vec2(link_min.x, link_max.y + s_(1)),
                c_vec2(link_max.x, link_max.y + s_(1)),
                get_clr(link_color, underline_alpha), s_(1));
            
            // Open URL on click
            if (link_clicked) {
                ShellExecuteA(NULL, "open", "https://enhanceclient.store", NULL, NULL, SW_SHOWNORMAL);
            }
            
            // Reserve space for the link text
            ImGui::Dummy(s_(0, link_size.y + 8));
        }
    }
    ImGui::End();
    
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// ============================================
// Main Render Function
// ============================================
// RenderWolfMenu() implementation is in GUI.cpp
// Forward declaration here for other files that might need it
void RenderWolfMenu();
