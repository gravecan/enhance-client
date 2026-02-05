#pragma once

namespace enhance
{
	namespace modules
	{
		namespace server_rotation
		{
			// Main loop function
			void run();

			// Call before an action to set server-side rotation
			void set_server_rotation(float yaw, float pitch);

			// Call after action to restore real rotation
			void restore_rotation();
		}
	}
}
