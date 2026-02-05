#include "reach_hook.h"
#include "../../enhance.h"
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <cstdio>

static jmethodID ORIG_getEntityInteractionRange = nullptr;
static jclass g_player_entity_class = nullptr;
static double g_reach_override = -1.0;

jdouble hkGetEntityInteractionRange(JNIEnv *env, jobject thiz)
{
	if (g_reach_override > 0.0)
	{
		return static_cast<jdouble>(g_reach_override);
	}
	
	if (ORIG_getEntityInteractionRange && thiz && g_player_entity_class)
	{
		return env->CallNonvirtualDoubleMethod(thiz, g_player_entity_class, ORIG_getEntityInteractionRange);
	}
	
	return 3.0;
}

static bool jnihook_initialized = false;

bool enhance::modules::reach_hook::init()
{
	if (g_player_entity_class != nullptr)
	{
		return true;
	}

	auto env = enhance::instance->get_env();
	auto jvm = enhance::instance->get_java_vm();
	if (!env || !jvm) 
	{
		return false;
	}

	g_reach_override = -1.0;
	ORIG_getEntityInteractionRange = nullptr;

	if (!jnihook_initialized)
	{
		jnihook_result_t result = JNIHook_Init(jvm);
		if (result != JNIHOOK_OK)
		{
			if (result == 3)
			{
				JNIHook_Shutdown();
				Sleep(100);
				result = JNIHook_Init(jvm);
			}
			
			if (result != JNIHOOK_OK)
			{
				return false;
			}
		}
		jnihook_initialized = true;
	}

	jclass player_entity_class = sdk::classloader::find_class(env, sdk::mappings::player_entity_class_sig);
	if (!player_entity_class)
	{

		return false;
	}

	jmethodID method_id = env->GetMethodID(player_entity_class, 
		sdk::mappings::get_entity_interaction_range_name, 
		sdk::mappings::get_entity_interaction_range_sig);
	
	if (!method_id)
	{
		env->DeleteLocalRef(player_entity_class);
		return false;
	}

	jnihook_result_t result = JNIHook_Attach(method_id, reinterpret_cast<void*>(hkGetEntityInteractionRange), &ORIG_getEntityInteractionRange);
	if (result != JNIHOOK_OK)
	{
		env->DeleteLocalRef(player_entity_class);
		return false;
	}

	g_player_entity_class = reinterpret_cast<jclass>(env->NewGlobalRef(player_entity_class));
	env->DeleteLocalRef(player_entity_class);
	
	if (!g_player_entity_class)
	{
		return false;
	}

	return true;
}

void enhance::modules::reach_hook::shutdown()
{
	g_reach_override = -1.0;
	ORIG_getEntityInteractionRange = nullptr;
	
	if (enhance::instance)
	{
		try
		{
			auto env = enhance::instance->get_env();
			if (g_player_entity_class && env)
			{
				env->DeleteGlobalRef(g_player_entity_class);
				g_player_entity_class = nullptr;
			}
		}
		catch (...)
		{
			g_player_entity_class = nullptr;
		}
	}
	else
	{
		g_player_entity_class = nullptr;
	}
	
	if (jnihook_initialized)
	{
		try
		{
			JNIHook_Shutdown();
			jnihook_initialized = false;
		}
		catch (...)
		{
		}
	}
}

void enhance::modules::reach_hook::set_reach(double distance)
{
	g_reach_override = distance;
}
