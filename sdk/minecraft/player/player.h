#pragma once

#include <sdk/includes.h>

class player_client
{
private:
	jobject player;
public:
	player_client();
	player_client(jobject player);
	~player_client();

	jobject get_capabilities();
	void set_flying(bool state);
	void set_sprinting(bool state);
	float get_attack_cooldown_progress(float base_time = 0.5f);

	jobject get_player();
};
