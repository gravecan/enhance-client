#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		namespace reach_hook
		{
			bool init();
			void shutdown();
			void set_reach(double distance);
		}
	}
}
