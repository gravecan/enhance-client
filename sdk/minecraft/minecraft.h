#pragma once

#include <sdk/includes.h>

namespace sdk
{
	struct camera_data
	{
		double x, y, z;
		float yaw, pitch;
		float fov;
		bool valid;
	};

	class minecraft_client
	{
	private:
	public:
		jclass klass();
		jobject get_minecraft();
		jobject get_player();
		jobject get_world();
		jobject get_crosshair_target();
		int get_attack_cooldown();
		bool do_attack();
		jobject get_interaction_manager();
		jobject get_network_handler();
		float get_fov();
		camera_data get_camera();
	};

	extern std::unique_ptr<minecraft_client> instance;
}
