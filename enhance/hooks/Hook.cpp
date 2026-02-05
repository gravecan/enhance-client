#include <sdk/includes.h>

#include "Hook.h"
#include "../gui/GUI.h"

typedef BOOL(__stdcall* TWglSwapBuffers) (HDC hDc);

static bool is_init{};
static HWND wnd_handle{};
static WNDPROC origin_wndproc{};
void* p_swap_buffers{};
TWglSwapBuffers origin_wglSwapBuffers{};

// Static variables for GUI context - reset on each injection
static HGLRC new_context{};
static bool gui_was_init{};

static LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool __stdcall wglSwapBuffers(HDC hDc);

bool Hook::init()
{
	if (is_init)
	{
		return false;
	}

	gui_was_init = false;
	new_context = nullptr;

	MH_STATUS mh_status = MH_Initialize();
	if (mh_status == MH_ERROR_ALREADY_INITIALIZED)
	{
		MH_Uninitialize();
		Sleep(50);
		mh_status = MH_Initialize();
	}
	
	if (mh_status != MH_OK)
	{
		return true;
	}

	{
		wnd_handle = FindWindowA("LWJGL", nullptr);

		if (!wnd_handle)
		{
			wnd_handle = FindWindowA("GLFW30", nullptr);

			if (!wnd_handle)
			{
				return true;
			}
		}

		WNDPROC current_proc = (WNDPROC)GetWindowLongPtrW(wnd_handle, GWLP_WNDPROC);
		
		if (current_proc == WndProc)
		{
			origin_wndproc = nullptr;
		}
		else
		{
			origin_wndproc = current_proc;
		}

		SetWindowLongPtrW(wnd_handle, GWLP_WNDPROC, (LONG_PTR)WndProc);
	}

	{
		p_swap_buffers = (void*)GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");

		if (p_swap_buffers == nullptr)
		{
			return true;
		}

		MH_RemoveHook(p_swap_buffers);
		
		MH_STATUS create_status = MH_CreateHook(p_swap_buffers, &wglSwapBuffers, (LPVOID*)&origin_wglSwapBuffers);
		if (create_status != MH_OK && create_status != MH_ERROR_ALREADY_CREATED)
		{
			return true;
		}
	}

	MH_EnableHook(MH_ALL_HOOKS);

	is_init = true;

	return false;
}

void Hook::shutdown()
{
	if (!is_init)
	{
		return;
	}

	is_init = false;

	if (gui_was_init)
	{
		try
		{
			GUI::shutdown();
		}
		catch (...)
		{
		}
		gui_was_init = false;
	}

	MH_DisableHook(MH_ALL_HOOKS);
	
	Sleep(300);
	
	if (wnd_handle && IsWindow(wnd_handle))
	{
		WNDPROC current_proc = (WNDPROC)GetWindowLongPtrW(wnd_handle, GWLP_WNDPROC);
		
		if (current_proc == WndProc && origin_wndproc && origin_wndproc != WndProc)
		{
			SetWindowLongPtrW(wnd_handle, GWLP_WNDPROC, (LONG_PTR)origin_wndproc);
		}
	}
	
	Sleep(200);
	
	MH_RemoveHook(MH_ALL_HOOKS);

	new_context = nullptr;
	
	MH_Uninitialize();

	wnd_handle = nullptr;
	origin_wndproc = nullptr;
	p_swap_buffers = nullptr;
	origin_wglSwapBuffers = nullptr;
}

bool Hook::get_is_init()
{
	return is_init;
}

HWND Hook::get_window()
{
	return wnd_handle;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (GUI::get_is_init())
	{
		if (msg == WM_KEYDOWN && wParam == VK_INSERT)
			GUI::set_do_draw(!GUI::get_do_draw());

		if (GUI::get_do_draw() && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;
	}

	return CallWindowProcA(origin_wndproc, hWnd, msg, wParam, lParam);
}

bool __stdcall wglSwapBuffers(HDC hDc)
{
	if (!is_init || !origin_wglSwapBuffers)
	{
		if (!origin_wglSwapBuffers)
		{
			return TRUE;
		}
		return origin_wglSwapBuffers(hDc);
	}

	HGLRC origin_context{ wglGetCurrentContext() };

	if (!gui_was_init)
	{
		new_context = wglCreateContext(hDc);
		if (!new_context)
		{
			return origin_wglSwapBuffers(hDc);
		}

		wglMakeCurrent(hDc, new_context);

		typedef BOOL(__stdcall* wglSwapIntervalEXT)(int);
		wglSwapIntervalEXT wglSwapInterval = (wglSwapIntervalEXT)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapInterval)
		{
			wglSwapInterval(0);
		}

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glViewport(0, 0, viewport[2], viewport[3]);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);

		GUI::init(wnd_handle);

		gui_was_init = true;
	}
	else if (new_context)
	{
		wglMakeCurrent(hDc, new_context);
		GUI::draw();
	}

	wglMakeCurrent(hDc, origin_context);

	return origin_wglSwapBuffers(hDc);
}
