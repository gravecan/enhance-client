#include <enhance/enhance.h>
#include "entity.h"
#include <sdk/classloader.h>
#include <sdk/mappings/mappings.hpp>

// Helper to check exceptions after JNI calls
static void check_jni_exception(JNIEnv* env, const char* operation)
{
	if (env->ExceptionCheck())
	{
		env->ExceptionClear();
	}
}

sdk::entity_client::entity_client(jobject entity)
{
	this->entity = entity;
}

sdk::entity_client::~entity_client()
{
}

jobject sdk::entity_client::get_entity()
{
	return entity;
}

jobject sdk::entity_client::get_bounding_box()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity)
		return nullptr;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class)
		return nullptr;

	// boundingBox is a FIELD, not a method!
	jfieldID fid = env->GetFieldID(entity_class, sdk::mappings::get_bounding_box_name, sdk::mappings::get_bounding_box_sig);
	check_jni_exception(env, "GetFieldID get_bounding_box");
	if (!fid)
	{
		env->DeleteLocalRef(entity_class);
		return nullptr;
	}

	jobject ret = env->GetObjectField(entity, fid);
	env->DeleteLocalRef(entity_class);

	return ret;
}

void sdk::entity_client::set_bounding_box(jobject box)
{
	auto env = enhance::instance->get_env();
	if (!env || !entity || !box) return;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::set_bounding_box_name, sdk::mappings::set_bounding_box_sig);
	check_jni_exception(env, "GetMethodID set_bounding_box");
	if (mid)
	{
		env->CallBooleanMethod(entity, mid, box);
	}

	env->DeleteLocalRef(entity_class);
}

bool sdk::entity_client::is_same_object(jobject other)
{
	if (!entity || !other) return false;
	return enhance::instance->get_env()->IsSameObject(entity, other);
}

double sdk::entity_client::get_x()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return 0.0;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return 0.0;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_get_x_name, sdk::mappings::entity_get_x_sig);
	check_jni_exception(env, "GetMethodID entity_get_x");
	if (!mid)
	{
		env->DeleteLocalRef(entity_class);
		return 0.0;
	}

	jdouble ret = env->CallDoubleMethod(entity, mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret;
}

double sdk::entity_client::get_y()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return 0.0;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return 0.0;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_get_y_name, sdk::mappings::entity_get_y_sig);
	check_jni_exception(env, "GetMethodID entity_get_y");
	if (!mid)
	{
		env->DeleteLocalRef(entity_class);
		return 0.0;
	}

	jdouble ret = env->CallDoubleMethod(entity, mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret;
}

double sdk::entity_client::get_z()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return 0.0;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return 0.0;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_get_z_name, sdk::mappings::entity_get_z_sig);
	check_jni_exception(env, "GetMethodID entity_get_z");
	if (!mid)
	{
		env->DeleteLocalRef(entity_class);
		return 0.0;
	}

	jdouble ret = env->CallDoubleMethod(entity, mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret;
}

float sdk::entity_client::get_yaw()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return 0.0f;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return 0.0f;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_get_yaw_name, sdk::mappings::entity_get_yaw_sig);
	check_jni_exception(env, "GetMethodID entity_get_yaw");
	if (!mid)
	{
		env->DeleteLocalRef(entity_class);
		return 0.0f;
	}

	jfloat ret = env->CallFloatMethod(entity, mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret;
}

float sdk::entity_client::get_pitch()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return 0.0f;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return 0.0f;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_get_pitch_name, sdk::mappings::entity_get_pitch_sig);
	check_jni_exception(env, "GetMethodID entity_get_pitch");
	if (!mid)
	{
		env->DeleteLocalRef(entity_class);
		return 0.0f;
	}

	jfloat ret = env->CallFloatMethod(entity, mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret;
}

void sdk::entity_client::set_yaw(float yaw)
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_set_yaw_name, sdk::mappings::entity_set_yaw_sig);
	check_jni_exception(env, "GetMethodID entity_set_yaw");
	if (mid)
	{
		env->CallVoidMethod(entity, mid, yaw);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}

	env->DeleteLocalRef(entity_class);
}

void sdk::entity_client::set_pitch(float pitch)
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::entity_set_pitch_name, sdk::mappings::entity_set_pitch_sig);
	check_jni_exception(env, "GetMethodID entity_set_pitch");
	if (mid)
	{
		env->CallVoidMethod(entity, mid, pitch);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}

	env->DeleteLocalRef(entity_class);
}

bool sdk::entity_client::is_on_ground()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return false;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return false;

	jmethodID mid = env->GetMethodID(entity_class, sdk::mappings::is_on_ground_name, sdk::mappings::is_on_ground_sig);
	check_jni_exception(env, "GetMethodID is_on_ground");
	if (!mid)
	{
		env->DeleteLocalRef(entity_class);
		return false;
	}

	jboolean ret = env->CallBooleanMethod(entity, mid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret == JNI_TRUE;
}

double sdk::entity_client::get_fall_distance()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return 0.0;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return 0.0;

	jfieldID fid = env->GetFieldID(entity_class, sdk::mappings::entity_fall_distance_name, sdk::mappings::entity_fall_distance_sig);
	check_jni_exception(env, "GetFieldID entity_fall_distance");
	if (!fid)
	{
		env->DeleteLocalRef(entity_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(entity, fid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return ret;
}

jobject sdk::entity_client::get_velocity()
{
	auto env = enhance::instance->get_env();
	if (!env || !entity) return nullptr;

	// Use classloader to get the entity class (works on both Fabric and vanilla)
	jclass entity_class = sdk::classloader::find_class(env, sdk::mappings::entity_class_sig);
	if (!entity_class) return nullptr;

	jfieldID fid = env->GetFieldID(entity_class, sdk::mappings::entity_velocity_name, sdk::mappings::entity_velocity_sig);
	check_jni_exception(env, "GetFieldID entity_velocity");
	if (!fid)
	{
		env->DeleteLocalRef(entity_class);
		return nullptr;
	}

	jobject velocity = env->GetObjectField(entity, fid);
	if (env->ExceptionCheck()) env->ExceptionClear();
	env->DeleteLocalRef(entity_class);

	return velocity;
}

