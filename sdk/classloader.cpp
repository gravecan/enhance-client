#include "classloader.h"
#include <enhance/enhance.h>
#include <cstring>
#include <string>
#include <algorithm>

namespace sdk
{
	namespace classloader
	{
		static jobject classloader_obj = nullptr;
		static jmethodID findclass_md = nullptr;
		static bool fabric_detected = false;
		static bool initialized = false;

		// Find class loader by iterating through all threads (exact copy from reference)
		static bool findClsLoaderByThreads(JNIEnv* env, const char* cls_loader_name)
		{
			jclass threadCls = env->FindClass("java/lang/Thread");
			jmethodID allStackTracesMd = env->GetStaticMethodID(threadCls, "getAllStackTraces", "()Ljava/util/Map;");
			if (!allStackTracesMd)
			{
				return false;
			}
			jobject threadMap = env->CallStaticObjectMethod(threadCls, allStackTracesMd);
			if (!threadMap)
			{
				return false;
			}

			jclass mapCls = env->FindClass("java/util/Map");
			jmethodID entrySetMd = env->GetMethodID(mapCls, "entrySet", "()Ljava/util/Set;");
			jobject entrySet = env->CallObjectMethod(threadMap, entrySetMd);
			jclass setCls = env->FindClass("java/util/Set");
			jmethodID iteratorMd = env->GetMethodID(setCls, "iterator", "()Ljava/util/Iterator;");
			jobject it = env->CallObjectMethod(entrySet, iteratorMd);
			jclass itCls = env->FindClass("java/util/Iterator");
			jmethodID hasNextMd = env->GetMethodID(itCls, "hasNext", "()Z");
			jmethodID nextMd = env->GetMethodID(itCls, "next", "()Ljava/lang/Object;");

			jclass entryCls = env->FindClass("java/util/Map$Entry");
			jmethodID getKeyMd = env->GetMethodID(entryCls, "getKey", "()Ljava/lang/Object;");
			jmethodID getContextMd = env->GetMethodID(threadCls, "getContextClassLoader", "()Ljava/lang/ClassLoader;");

			jclass classCls = env->FindClass("java/lang/Class");
			jmethodID getNameMd = env->GetMethodID(classCls, "getName", "()Ljava/lang/String;");

			while (env->CallBooleanMethod(it, hasNextMd))
			{
				jobject entry = env->CallObjectMethod(it, nextMd);
				jobject threadObj = env->CallObjectMethod(entry, getKeyMd);
				jobject loader = env->CallObjectMethod(threadObj, getContextMd);
				if (!loader)
				{
					env->DeleteLocalRef(entry);
					env->DeleteLocalRef(threadObj);
					continue;
				}

				jclass loaderCls = env->GetObjectClass(loader);
				jstring nameStr = (jstring)env->CallObjectMethod(loaderCls, getNameMd);
				const char* nameC = env->GetStringUTFChars(nameStr, nullptr);

				if (nameC && strcmp(nameC, cls_loader_name) == 0)
				{
					classloader_obj = env->NewGlobalRef(loader);
					env->ReleaseStringUTFChars(nameStr, nameC);
					env->DeleteLocalRef(nameStr);
					env->DeleteLocalRef(loaderCls);
					env->DeleteLocalRef(loader);
					env->DeleteLocalRef(entry);
					env->DeleteLocalRef(threadObj);
					return true;
				}

				env->ReleaseStringUTFChars(nameStr, nameC);
				env->DeleteLocalRef(nameStr);
				env->DeleteLocalRef(loaderCls);
				env->DeleteLocalRef(loader);
				env->DeleteLocalRef(entry);
				env->DeleteLocalRef(threadObj);
			}

			return false;
		}

		bool init(JNIEnv* env)
		{
			if (!env)
			{
				return false;
			}
			
			if (initialized)
			{
				return true;
			}

			// Try to find class loaders in order: Fabric, Forge, LaunchWrapper
			if (findClsLoaderByThreads(env, "net.fabricmc.loader.impl.launch.knot.KnotClassLoader"))
			{
				fabric_detected = true;
			}
			else if (findClsLoaderByThreads(env, "cpw.mods.modlauncher.TransformingClassLoader"))
			{
				fabric_detected = false;
			}
			else if (findClsLoaderByThreads(env, "net.minecraft.launchwrapper.LaunchClassLoader"))
			{
				fabric_detected = false;
			}
			else
			{
				fabric_detected = false;
			}

			// Get the loadClass method if we found a class loader
			// Match reference exactly: get method with (String, boolean) signature
			if (classloader_obj)
			{
				jclass knotCls = env->GetObjectClass(classloader_obj);
				findclass_md = env->GetMethodID(knotCls, "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
				env->DeleteLocalRef(knotCls);
			}

			initialized = true;
			return true;
		}

		bool is_fabric()
		{
			return fabric_detected;
		}

		jclass find_class(JNIEnv* env, const char* class_name)
		{
			if (!env || !class_name)
			{
				return nullptr;
			}

			// Ensure we're initialized
			if (!initialized)
			{
				init(env);
			}

			// If we have a class loader, use it; otherwise use FindClass
			if (classloader_obj && findclass_md)
			{
				// Convert class name from slashes to dots (e.g., "net/minecraft/class_310" -> "net.minecraft.class_310")
				std::string class_name_format(class_name);
				std::replace(class_name_format.begin(), class_name_format.end(), '/', '.');

				jstring jname = env->NewStringUTF(class_name_format.c_str());
				if (!jname)
				{
					return nullptr;
				}

				// Call loadClass - match reference exactly: call with only string parameter
				// Even though signature is (Ljava/lang/String;Z)Ljava/lang/Class;
				jclass cls = reinterpret_cast<jclass>(env->CallObjectMethod(classloader_obj, findclass_md, jname));

				env->DeleteLocalRef(jname);

				// Check for exceptions
				if (env->ExceptionCheck())
				{
					env->ExceptionClear();
					return nullptr;
				}

				return cls;
			}
			
			// Fallback to FindClass for vanilla
			return env->FindClass(class_name);
		}

		// Cleanup function (can be called during shutdown)
		void cleanup(JNIEnv* env)
		{
			if (env && classloader_obj)
			{
				env->DeleteGlobalRef(classloader_obj);
				classloader_obj = nullptr;
			}
			findclass_md = nullptr;
			fabric_detected = false;
			initialized = false;
		}
	}
}
