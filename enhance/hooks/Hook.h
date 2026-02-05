#ifndef HOOK_H_
#define HOOK_H_

#include <Windows.h>

namespace Hook
{
	bool init();
	void shutdown();

	bool get_is_init();
	HWND get_window();
}

#endif

