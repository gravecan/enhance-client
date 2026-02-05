#include <enhance/enhance.h>
#include "../minecraft.h"
#include "player.h"
#include <sdk/classloader.h>

// Helper to check exceptions after JNI calls
static void check_jni_exception(JNIEnv* env, const char* operation)
{
	if (env->ExceptionCheck())
	{
		env->ExceptionClear();
	}
}

player_client::player_client()
{
	player = nullptr;
}

player_client::player_client(jobject player)
{
	this->player = player;
}

player_client::~player_client()
{
}

jobject player_client::get_player()
{
	return sdk::instance->get_player();
}

jobject player_client::get_capabilities()
{
	jobject current_player = get_player();
	if (!current_player) return nullptr;
	
	auto env = enhance::instance->get_env();
	if (!env)
	{
		return nullptr;
	}

	jclass player_class = env->GetObjectClass(current_player);
	if (!player_class)
	{
		env->DeleteLocalRef(current_player);
		return nullptr;
	}

	jfieldID fid = env->GetFieldID(player_class, sdk::mappings::abilities_name, sdk::mappings::abilities_sig);
	check_jni_exception(env, "GetFieldID abilities");
	if (!fid)
	{
		env->DeleteLocalRef(player_class);
		env->DeleteLocalRef(current_player);
		return nullptr;
	}

	jobject ret = env->GetObjectField(current_player, fid);

	env->DeleteLocalRef(player_class);
	env->DeleteLocalRef(current_player);

	return ret;
}

void player_client::set_flying(bool state)
{
	jobject capabilities = get_capabilities();
	if (!capabilities)
	{ 
		return;
	}
		
	auto env = enhance::instance->get_env();
	if (!env) return;

	jclass capabilities_class = env->GetObjectClass(capabilities);
	if (!capabilities_class)
	{
		env->DeleteLocalRef(capabilities);
		return;
	}

	jfieldID fid = env->GetFieldID(capabilities_class, sdk::mappings::fly_name, sdk::mappings::fly_sig);
	check_jni_exception(env, "GetFieldID fly");
	if (fid)
	{
		env->SetBooleanField(capabilities, fid, state);
	}
	
	env->DeleteLocalRef(capabilities_class);
	env->DeleteLocalRef(capabilities);
}

void player_client::set_sprinting(bool state)
{
	jobject current_player = get_player();
	if (!current_player) return;
	
	auto env = enhance::instance->get_env();
	if (!env) return;

	jclass player_class = env->GetObjectClass(current_player);
	if (!player_class)
	{
		env->DeleteLocalRef(current_player);
		return;
	}

	jmethodID mid = env->GetMethodID(player_class, sdk::mappings::set_sprinting_name, sdk::mappings::set_sprinting_sig);
	check_jni_exception(env, "GetMethodID set_sprinting");
	if (mid)
	{
		env->CallVoidMethod(current_player, mid, state);
	}

	env->DeleteLocalRef(player_class);
	env->DeleteLocalRef(current_player);
}

float player_client::get_attack_cooldown_progress(float base_time)
{
	jobject current_player = get_player();
	if (!current_player) return 0.0f;
	
	auto env = enhance::instance->get_env();
	if (!env) return 0.0f;

	jclass player_class = sdk::classloader::find_class(env, sdk::mappings::player_entity_class_sig);
	if (!player_class)
	{
		env->DeleteLocalRef(current_player);
		return 0.0f;
	}

	jmethodID mid = env->GetMethodID(player_class, sdk::mappings::get_attack_cooldown_progress_name, sdk::mappings::get_attack_cooldown_progress_sig);
	check_jni_exception(env, "GetMethodID get_attack_cooldown_progress");
	if (!mid)
	{
		env->DeleteLocalRef(player_class);
		env->DeleteLocalRef(current_player);
		return 0.0f;
	}

	jfloat progress = env->CallFloatMethod(current_player, mid, base_time);

	env->DeleteLocalRef(player_class);
	env->DeleteLocalRef(current_player);

	return progress;
}
