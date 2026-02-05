#pragma once

#include <jni.h>

namespace enhance
{
	namespace modules
	{
		namespace wtap
		{
			void run();
			void on_hit(); // Called when a hit is registered
		}
	}
}
