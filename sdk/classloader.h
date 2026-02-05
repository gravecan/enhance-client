#pragma once

#include <sdk/includes.h>

namespace sdk
{
	namespace classloader
	{
		// Initialize the class loader system (should be called after JVM attach)
		bool init(JNIEnv* env);

		// Get a class by name, using Fabric's class loader if available, otherwise fallback to FindClass
		jclass find_class(JNIEnv* env, const char* class_name);

		// Check if Fabric is present
		bool is_fabric();

		// Cleanup (call during shutdown)
		void cleanup(JNIEnv* env);
	}
}
