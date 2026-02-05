#ifndef GUI_H_
#define GUI_H_

#include <Windows.h>
#include <utils/imgui/imgui.h>
#include <utils/imgui/imgui_impl_win32.h>
#include <utils/imgui/imgui_impl_opengl3.h>

namespace GUI
{
	bool init(HWND wnd_handle);
	void shutdown();

	void draw();

	bool get_is_init();
	bool get_do_draw();

	void set_do_draw(bool new_value);
}

#endif

