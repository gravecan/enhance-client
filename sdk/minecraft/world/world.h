#pragma once

#include <sdk/includes.h>
#include <vector>

namespace sdk
{
	class world_client
	{
	private:
		jobject world;

	public:
		world_client(jobject world);
		~world_client();

		std::vector<jobject> get_players();
	};
}

