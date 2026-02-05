#pragma once

#include <sdk/includes.h>

namespace enhance
{
	namespace modules
	{
		class hitbox_expander
		{
		public:
			static void run();
			static void reset_all();

		private:
			static void expand_entity(jobject entity);
			static void reset_entity(jobject entity);
			static constexpr double DEFAULT_WIDTH = 0.6;
			static constexpr double DEFAULT_HEIGHT = 1.8;
		};
	}
}

