#include <enhance/enhance.h>
#include "minecraft.h"
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>

std::unique_ptr<sdk::minecraft_client> sdk::instance;

// Helper function to check for exceptions after JNI calls
static void check_jni_exception(JNIEnv* env, const char* operation)
{
	if (env->ExceptionCheck())
	{
		env->ExceptionClear();
	}
}

jclass sdk::minecraft_client::klass()
{
	auto env = enhance::instance->get_env();
	if (!env)
	{
		return nullptr;
	}
	
	return sdk::classloader::find_class(env, sdk::mappings::minecraftclass_sig);
}

jobject sdk::minecraft_client::get_minecraft()
{
	auto env = enhance::instance->get_env();
	if (!env)
	{
		return nullptr;
	}

	jclass minecraft_class = klass();
	if (!minecraft_class)
	{
		return nullptr;
	}

	jfieldID fid = env->GetStaticFieldID(minecraft_class, sdk::mappings::minecraftclient_name, sdk::mappings::minecraftclient_sig);
	check_jni_exception(env, "GetStaticFieldID for minecraft instance");
	if (!fid)
	{
		env->DeleteLocalRef(minecraft_class);
		return nullptr;
	}

	jobject ret = env->GetStaticObjectField(minecraft_class, fid);
	env->DeleteLocalRef(minecraft_class);

	return ret;
}

jobject sdk::minecraft_client::get_player()
{
	auto env = enhance::instance->get_env();
	if (!env)
	{
		return nullptr;
	}

	jclass minecraft_class = klass();
	jobject minecraft = get_minecraft();
	if (!minecraft_class || !minecraft)
	{
		return nullptr;
	}

	jfieldID fid = env->GetFieldID(minecraft_class, sdk::mappings::player_name, sdk::mappings::player_sig);
	check_jni_exception(env, "GetFieldID for player");
	if (!fid)
	{
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);
		return nullptr;
	}

	jobject ret = env->GetObjectField(minecraft, fid);
	env->DeleteLocalRef(minecraft_class);
	env->DeleteLocalRef(minecraft);

	return ret;
}

jobject sdk::minecraft_client::get_world()
{
	auto env = enhance::instance->get_env();
	if (!env) return nullptr;

	jclass minecraft_class = klass();
	jobject minecraft = get_minecraft();
	if (!minecraft_class || !minecraft) return nullptr;

	jfieldID fid = env->GetFieldID(minecraft_class, sdk::mappings::world_name, sdk::mappings::world_sig);
	check_jni_exception(env, "GetFieldID for world");
	if (!fid)
	{
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);
		return nullptr;
	}

	jobject ret = env->GetObjectField(minecraft, fid);

	env->DeleteLocalRef(minecraft_class);
	env->DeleteLocalRef(minecraft);

	return ret;
}

jobject sdk::minecraft_client::get_crosshair_target()
{
	auto env = enhance::instance->get_env();
	if (!env) return nullptr;

	jclass minecraft_class = klass();
	jobject minecraft = get_minecraft();
	if (!minecraft_class || !minecraft) return nullptr;

	jfieldID fid = env->GetFieldID(minecraft_class, sdk::mappings::crosshair_target_name, sdk::mappings::crosshair_target_sig);
	check_jni_exception(env, "GetFieldID for crosshair_target");
	if (!fid)
	{
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);
		return nullptr;
	}

	jobject ret = env->GetObjectField(minecraft, fid);

	env->DeleteLocalRef(minecraft_class);
	env->DeleteLocalRef(minecraft);

	return ret;
}

int sdk::minecraft_client::get_attack_cooldown()
{
	auto env = enhance::instance->get_env();
	if (!env) return 999;

	jclass minecraft_class = klass();
	jobject minecraft = get_minecraft();
	if (!minecraft_class || !minecraft) return 999;

	jfieldID fid = env->GetFieldID(minecraft_class, sdk::mappings::attack_cooldown_name, sdk::mappings::attack_cooldown_sig);
	check_jni_exception(env, "GetFieldID for attack_cooldown");
	if (!fid)
	{
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);
		return 999;
	}

	jint ret = env->GetIntField(minecraft, fid);

	env->DeleteLocalRef(minecraft_class);
	env->DeleteLocalRef(minecraft);

	return ret;
}

bool sdk::minecraft_client::do_attack()
{
	auto env = enhance::instance->get_env();
	if (!env) return false;

	jclass minecraft_class = klass();
	jobject minecraft = get_minecraft();
	if (!minecraft_class || !minecraft) return false;

	jmethodID mid = env->GetMethodID(minecraft_class, sdk::mappings::do_attack_name, sdk::mappings::do_attack_sig);
	check_jni_exception(env, "GetMethodID for do_attack");
	if (!mid)
	{
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);
		return false;
	}

	jboolean ret = env->CallBooleanMethod(minecraft, mid);

	env->DeleteLocalRef(minecraft_class);
	env->DeleteLocalRef(minecraft);

	return ret == JNI_TRUE;
}

	jobject sdk::minecraft_client::get_interaction_manager()
	{
		auto env = enhance::instance->get_env();
		if (!env) return nullptr;

		jclass minecraft_class = klass();
		jobject minecraft = get_minecraft();
		if (!minecraft_class || !minecraft) return nullptr;

		jfieldID fid = env->GetFieldID(minecraft_class, sdk::mappings::interaction_manager_name, sdk::mappings::interaction_manager_sig);
		check_jni_exception(env, "GetFieldID for interaction_manager");
		if (!fid)
		{
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return nullptr;
		}

		jobject ret = env->GetObjectField(minecraft, fid);

		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);

		return ret;
	}

	jobject sdk::minecraft_client::get_network_handler()
	{
		auto env = enhance::instance->get_env();
		if (!env) return nullptr;

		jclass minecraft_class = klass();
		jobject minecraft = get_minecraft();
		if (!minecraft_class || !minecraft) return nullptr;

		// Try getNetworkHandler() method first if mapping exists
		if (sdk::mappings::network_handler_name && sdk::mappings::network_handler_sig)
		{
			jmethodID mid = env->GetMethodID(minecraft_class, sdk::mappings::network_handler_name, sdk::mappings::network_handler_sig);
			if (env->ExceptionCheck()) env->ExceptionClear();
			
			if (mid)
			{
				jobject ret = env->CallObjectMethod(minecraft, mid);
				if (env->ExceptionCheck()) env->ExceptionClear();
				env->DeleteLocalRef(minecraft_class);
				env->DeleteLocalRef(minecraft);
				return ret;
			}
		}
		
		// Try connection field if mapping exists
		jfieldID fid = nullptr;
		if (sdk::mappings::connection_name && sdk::mappings::connection_sig)
		{
			fid = env->GetFieldID(minecraft_class, sdk::mappings::connection_name, sdk::mappings::connection_sig);
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		if (!fid)
		{
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return nullptr;
		}

		jobject ret = env->GetObjectField(minecraft, fid);

		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);

		return ret;
	}

	float sdk::minecraft_client::get_fov()
	{
		auto env = enhance::instance->get_env();
		if (!env) return 70.0f;

		jclass minecraft_class = klass();
		jobject minecraft = get_minecraft();
		if (!minecraft_class || !minecraft) return 70.0f;

		jfieldID gamerenderer_fid = env->GetFieldID(minecraft_class, sdk::mappings::gamerenderer_name, sdk::mappings::gamerenderer_sig);
		check_jni_exception(env, "GetFieldID for gamerenderer (get_fov)");
		if (!gamerenderer_fid)
		{
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return 70.0f;
		}

		jobject game_renderer = env->GetObjectField(minecraft, gamerenderer_fid);
		if (!game_renderer)
		{
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return 70.0f;
		}

		jclass gamerenderer_class = sdk::classloader::find_class(env, sdk::mappings::gamerenderer_class_sig);
		if (!gamerenderer_class)
		{
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return 70.0f;
		}

		jmethodID get_camera_mid = env->GetMethodID(gamerenderer_class, sdk::mappings::get_camera_name, sdk::mappings::get_camera_sig);
		check_jni_exception(env, "GetMethodID for get_camera (get_fov)");
		if (!get_camera_mid)
		{
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return 70.0f;
		}

		jobject camera = env->CallObjectMethod(game_renderer, get_camera_mid);
		if (!camera)
		{
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return 70.0f;
		}

		jmethodID get_fov_mid = env->GetMethodID(gamerenderer_class, sdk::mappings::get_fov_name, sdk::mappings::get_fov_sig);
		check_jni_exception(env, "GetMethodID for get_fov");
		if (!get_fov_mid)
		{
			env->DeleteLocalRef(camera);
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return 70.0f;
		}

		jfloat tick_progress = 0.0f;
		jboolean changing_fov = JNI_TRUE;
		jfloat fov = env->CallFloatMethod(game_renderer, get_fov_mid, camera, tick_progress, changing_fov);

		env->DeleteLocalRef(camera);
		env->DeleteLocalRef(gamerenderer_class);
		env->DeleteLocalRef(game_renderer);
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);

		return fov;
	}

	sdk::camera_data sdk::minecraft_client::get_camera()
	{
		sdk::camera_data result = {0, 0, 0, 0, 0, 70.0f, false};

		auto env = enhance::instance->get_env();
		if (!env) return result;

		jclass minecraft_class = klass();
		jobject minecraft = get_minecraft();
		if (!minecraft_class || !minecraft) return result;

		// Get GameRenderer
		jfieldID gamerenderer_fid = env->GetFieldID(minecraft_class, sdk::mappings::gamerenderer_name, sdk::mappings::gamerenderer_sig);
		check_jni_exception(env, "GetFieldID for gamerenderer (camera)");
		if (!gamerenderer_fid) {
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		jobject game_renderer = env->GetObjectField(minecraft, gamerenderer_fid);
		if (!game_renderer) {
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		jclass gamerenderer_class = sdk::classloader::find_class(env, sdk::mappings::gamerenderer_class_sig);
		if (!gamerenderer_class) {
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		// Get Camera object
		jmethodID get_camera_mid = env->GetMethodID(gamerenderer_class, sdk::mappings::get_camera_name, sdk::mappings::get_camera_sig);
		check_jni_exception(env, "GetMethodID for get_camera (camera)");
		if (!get_camera_mid) {
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		jobject camera = env->CallObjectMethod(game_renderer, get_camera_mid);
		if (!camera) {
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		jclass camera_class = sdk::classloader::find_class(env, sdk::mappings::camera_class_sig);
		if (!camera_class) {
			env->DeleteLocalRef(camera);
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		// Get Camera.getPos() -> Vec3d
		jmethodID camera_get_pos_mid = env->GetMethodID(camera_class, sdk::mappings::camera_get_pos_name, sdk::mappings::camera_get_pos_sig);
		check_jni_exception(env, "GetMethodID for camera_get_pos");
		jmethodID camera_get_yaw_mid = env->GetMethodID(camera_class, sdk::mappings::camera_get_yaw_name, sdk::mappings::camera_get_yaw_sig);
		check_jni_exception(env, "GetMethodID for camera_get_yaw");
		jmethodID camera_get_pitch_mid = env->GetMethodID(camera_class, sdk::mappings::camera_get_pitch_name, sdk::mappings::camera_get_pitch_sig);
		check_jni_exception(env, "GetMethodID for camera_get_pitch");

		if (!camera_get_pos_mid || !camera_get_yaw_mid || !camera_get_pitch_mid) {
			env->DeleteLocalRef(camera_class);
			env->DeleteLocalRef(camera);
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		// Get position Vec3d
		jobject pos_vec3d = env->CallObjectMethod(camera, camera_get_pos_mid);
		if (!pos_vec3d) {
			env->DeleteLocalRef(camera_class);
			env->DeleteLocalRef(camera);
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		jclass vec3d_class = sdk::classloader::find_class(env, sdk::mappings::vec3d_class_sig);
		if (!vec3d_class) {
			env->DeleteLocalRef(pos_vec3d);
			env->DeleteLocalRef(camera_class);
			env->DeleteLocalRef(camera);
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		jfieldID vec3d_x_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_x_name, sdk::mappings::vec3d_x_sig);
		check_jni_exception(env, "GetFieldID for vec3d_x");
		jfieldID vec3d_y_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_y_name, sdk::mappings::vec3d_y_sig);
		check_jni_exception(env, "GetFieldID for vec3d_y");
		jfieldID vec3d_z_fid = env->GetFieldID(vec3d_class, sdk::mappings::vec3d_z_name, sdk::mappings::vec3d_z_sig);
		check_jni_exception(env, "GetFieldID for vec3d_z");

		if (!vec3d_x_fid || !vec3d_y_fid || !vec3d_z_fid) {
			env->DeleteLocalRef(vec3d_class);
			env->DeleteLocalRef(pos_vec3d);
			env->DeleteLocalRef(camera_class);
			env->DeleteLocalRef(camera);
			env->DeleteLocalRef(gamerenderer_class);
			env->DeleteLocalRef(game_renderer);
			env->DeleteLocalRef(minecraft_class);
			env->DeleteLocalRef(minecraft);
			return result;
		}

		// Get camera position
		result.x = env->GetDoubleField(pos_vec3d, vec3d_x_fid);
		result.y = env->GetDoubleField(pos_vec3d, vec3d_y_fid);
		result.z = env->GetDoubleField(pos_vec3d, vec3d_z_fid);

		// Get camera rotation
		result.yaw = env->CallFloatMethod(camera, camera_get_yaw_mid);
		result.pitch = env->CallFloatMethod(camera, camera_get_pitch_mid);

		// Get FOV
		jmethodID get_fov_mid = env->GetMethodID(gamerenderer_class, sdk::mappings::get_fov_name, sdk::mappings::get_fov_sig);
		check_jni_exception(env, "GetMethodID for get_fov (camera_data)");
		if (get_fov_mid) {
			result.fov = env->CallFloatMethod(game_renderer, get_fov_mid, camera, 0.0f, JNI_TRUE);
		}

		result.valid = true;

		// Cleanup
		env->DeleteLocalRef(vec3d_class);
		env->DeleteLocalRef(pos_vec3d);
		env->DeleteLocalRef(camera_class);
		env->DeleteLocalRef(camera);
		env->DeleteLocalRef(gamerenderer_class);
		env->DeleteLocalRef(game_renderer);
		env->DeleteLocalRef(minecraft_class);
		env->DeleteLocalRef(minecraft);

		return result;
	}


