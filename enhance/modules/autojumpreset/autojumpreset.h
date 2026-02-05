#pragma once

#include <sdk/includes.h>
#include <Windows.h>

namespace enhance
{
	namespace modules
	{
		class autojumpreset
		{
		public:
			static void run();
			static void on_hit(); // Called when player takes damage
		};
	}
}

