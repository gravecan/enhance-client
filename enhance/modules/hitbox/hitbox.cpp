#include "hitbox.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/minecraft/util/box.h>

void enhance::modules::hitbox_expander::run()
{
	if (!globals::hitbox_enabled)
	{
		reset_all();
		return;
	}

	jobject world = sdk::instance->get_world();
	if (!world)
		return;

	jobject local_player = sdk::instance->get_player();
	if (!local_player)
	{
		auto env = enhance::instance->get_env();
		if (env && world)
			env->DeleteLocalRef(world);
		return;
	}

	sdk::world_client world_client(world);
	std::vector<jobject> players = world_client.get_players();

	for (size_t i = 0; i < players.size(); i++)
	{
		jobject player = players[i];
		if (!player)
			continue;

		sdk::entity_client entity_client(player);
		if (entity_client.is_same_object(local_player))
			continue;

		expand_entity(player);
	}

	for (jobject player : players)
	{
		if (player)
		{
			auto env = enhance::instance->get_env();
			if (env)
				env->DeleteLocalRef(player);
		}
	}

	auto env = enhance::instance->get_env();
	if (env && world)
		env->DeleteLocalRef(world);
	if (env && local_player)
		env->DeleteLocalRef(local_player);
}

void enhance::modules::hitbox_expander::reset_all()
{
	jobject world = sdk::instance->get_world();
	if (!world)
		return;

	jobject local_player = sdk::instance->get_player();
	if (!local_player)
	{
		auto env = enhance::instance->get_env();
		if (env && world)
			env->DeleteLocalRef(world);
		return;
	}

	sdk::world_client world_client(world);
	std::vector<jobject> players = world_client.get_players();

	for (jobject player : players)
	{
		if (!player) continue;

		sdk::entity_client entity_client(player);
		if (entity_client.is_same_object(local_player))
			continue;

		reset_entity(player);
	}

	for (jobject player : players)
	{
		if (player)
		{
			auto env = enhance::instance->get_env();
			if (env)
				env->DeleteLocalRef(player);
		}
	}

	auto env = enhance::instance->get_env();
	if (env && world)
		env->DeleteLocalRef(world);
	if (env && local_player)
		env->DeleteLocalRef(local_player);
}

void enhance::modules::hitbox_expander::expand_entity(jobject entity)
{
	sdk::entity_client entity_client_obj(entity);
	jobject bounding_box_obj = entity_client_obj.get_bounding_box();
	if (!bounding_box_obj)
		return;

	sdk::box_client box(bounding_box_obj);

	double min_x = box.get_min_x();
	double max_x = box.get_max_x();
	double min_y = box.get_min_y();
	double max_y = box.get_max_y();
	double min_z = box.get_min_z();
	double max_z = box.get_max_z();

	double width = max_x - min_x;
	double height = max_y - min_y;

	if (width > 0.601 || height > 1.801)
	{
		auto env = enhance::instance->get_env();
		if (env)
			env->DeleteLocalRef(bounding_box_obj);
		return;
	}

	box.set_min_x(min_x - globals::hitbox_expand_width);
	box.set_max_x(max_x + globals::hitbox_expand_width);
	box.set_min_y(min_y - globals::hitbox_expand_height);
	box.set_max_y(max_y + globals::hitbox_expand_height);
	box.set_min_z(min_z - globals::hitbox_expand_width);
	box.set_max_z(max_z + globals::hitbox_expand_width);

	entity_client_obj.set_bounding_box(bounding_box_obj);

	auto env = enhance::instance->get_env();
	if (env)
		env->DeleteLocalRef(bounding_box_obj);
}

void enhance::modules::hitbox_expander::reset_entity(jobject entity)
{
	sdk::entity_client entity_client_obj(entity);
	jobject bounding_box_obj = entity_client_obj.get_bounding_box();
	if (!bounding_box_obj) return;

	sdk::box_client box(bounding_box_obj);

	double min_x = box.get_min_x();
	double max_x = box.get_max_x();
	double min_y = box.get_min_y();
	double max_y = box.get_max_y();
	double min_z = box.get_min_z();
	double max_z = box.get_max_z();

	double width = max_x - min_x;
	double height = max_y - min_y;

	if (std::abs(DEFAULT_WIDTH - width) < 0.001 && std::abs(DEFAULT_HEIGHT - height) < 0.001)
	{
		auto env = enhance::instance->get_env();
		if (env)
			env->DeleteLocalRef(bounding_box_obj);
		return;
	}

	double center_x = min_x + width / 2.0;
	double center_y = min_y + height / 2.0;
	double center_z = min_z + width / 2.0;

	box.set_min_x(center_x - DEFAULT_WIDTH / 2.0);
	box.set_max_x(center_x + DEFAULT_WIDTH / 2.0);
	box.set_min_y(center_y - DEFAULT_HEIGHT / 2.0);
	box.set_max_y(center_y + DEFAULT_HEIGHT / 2.0);
	box.set_min_z(center_z - DEFAULT_WIDTH / 2.0);
	box.set_max_z(center_z + DEFAULT_WIDTH / 2.0);

	entity_client_obj.set_bounding_box(bounding_box_obj);

	auto env = enhance::instance->get_env();
	if (env)
		env->DeleteLocalRef(bounding_box_obj);
}

