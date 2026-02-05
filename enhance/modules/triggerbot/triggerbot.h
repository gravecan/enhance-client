#pragma once

#include <sdk/includes.h>
#include <Windows.h>

namespace enhance
{
	namespace modules
	{
		class triggerbot
		{
		public:
			static void run();
			static void cleanup();

		private:
			static bool is_entity_hit_result(jobject hit_result);
			static jobject get_entity_from_hit_result(jobject hit_result);
			static void cleanup_refs(JNIEnv* env, jobject minecraft, jobject crosshair, jobject entity);
			
			static jobject last_targeted_entity;
			static ULONGLONG last_attack_time;
			static int current_delay;
		};
	}
}
