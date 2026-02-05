#include <enhance/enhance.h>
#include "world.h"
#include <sdk/mappings/mappings.hpp>

sdk::world_client::world_client(jobject world)
{
	this->world = world;
}

sdk::world_client::~world_client()
{
}

std::vector<jobject> sdk::world_client::get_players()
{
	std::vector<jobject> players;
	auto env = enhance::instance->get_env();
	if (!env || !world) return players;

	jclass world_class = env->GetObjectClass(world);
	if (!world_class) return players;

	jfieldID fid = env->GetFieldID(world_class, sdk::mappings::players_field_name, sdk::mappings::players_field_sig);
	if (!fid)
	{
		env->DeleteLocalRef(world_class);
		return players;
	}

	jobject players_list = env->GetObjectField(world, fid);
	if (!players_list)
	{
		env->DeleteLocalRef(world_class);
		return players;
	}

	jclass list_class = env->GetObjectClass(players_list);
	if (!list_class)
	{
		env->DeleteLocalRef(world_class);
		env->DeleteLocalRef(players_list);
		return players;
	}

	jmethodID size_method = env->GetMethodID(list_class, "size", "()I");
	jmethodID get_method = env->GetMethodID(list_class, "get", "(I)Ljava/lang/Object;");

	if (size_method && get_method)
	{
		jint list_size = env->CallIntMethod(players_list, size_method);
		for (jint i = 0; i < list_size; i++)
		{
			jobject player = env->CallObjectMethod(players_list, get_method, i);
			if (player)
			{
				players.push_back(player);
			}
		}
	}

	env->DeleteLocalRef(list_class);
	env->DeleteLocalRef(players_list);
	env->DeleteLocalRef(world_class);

	return players;
}

