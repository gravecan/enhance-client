#include "backtrack_hook.h"
#include "../../enhance.h"
#include "../../utils/logger.h"
#include <sdk/mappings/mappings.hpp>
#include <sdk/classloader.h>
#include <utils/jnihook-master/include/jnihook.h>
#include <cstdio>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>

struct DelayedPacket
{
	jobject packet;
	jobject context;
	jobject handler;
	ULONGLONG release_time_ms;
};

static std::queue<DelayedPacket> g_packet_queue;
static std::mutex g_packet_queue_mutex;
static bool g_delay_enabled = false;
static int g_delay_ms = 200;
static jmethodID ORIG_channelRead0 = nullptr;
static jclass g_channel_handler_class = nullptr;
static bool jnihook_initialized = false;
static std::thread* g_packet_processor_thread = nullptr;
static bool g_should_stop_processor = false;
static bool g_hook_ready = false;

// Cached packet classes for fast instanceof checks
static jclass g_entity_position_packet_class = nullptr;
static jclass g_entity_move_packet_class = nullptr;
static jclass g_entity_teleport_packet_class = nullptr;
static jclass g_entity_s2c_packet_class = nullptr;

// Check if packet is an entity position update packet (fast version using cached classes)
static bool is_entity_position_packet(JNIEnv *env, jobject packet)
{
	if (!env || !packet || !g_hook_ready) return false;
	
	// Fast instanceof checks using cached classes
	// Try EntityPositionS2CPacket (most common)
	if (g_entity_position_packet_class)
	{
		if (env->IsInstanceOf(packet, g_entity_position_packet_class))
		{
			return true;
		}
	}
	
	// Try EntityMoveS2CPacket (if cached)
	if (g_entity_move_packet_class)
	{
		if (env->IsInstanceOf(packet, g_entity_move_packet_class))
		{
			return true;
		}
	}
	
	// Try EntityTeleportS2CPacket (if cached)
	if (g_entity_teleport_packet_class)
	{
		if (env->IsInstanceOf(packet, g_entity_teleport_packet_class))
		{
			return true;
		}
	}
	
	// Fallback: Check if it's an EntityS2CPacket (parent class)
	// Only use this if we have at least one specific mapping
	if (g_entity_s2c_packet_class && g_entity_position_packet_class)
	{
		if (env->IsInstanceOf(packet, g_entity_s2c_packet_class))
		{
			return true;
		}
	}
	
	return false;
}

// Hook function for channelRead0
void hkChannelRead0(JNIEnv *env, jobject thiz, jobject ctx, jobject msg)
{
	// CRITICAL: This hook is called for EVERY packet, so it must be FAST and SAFE
	// Wrap everything in try-catch to prevent crashes
	
	// Safety checks - if anything is invalid, just pass through immediately
	if (!env || !thiz || !msg || !ORIG_channelRead0 || !g_channel_handler_class || !g_hook_ready)
	{
		// If we have the original method, try to call it anyway
		if (ORIG_channelRead0 && thiz && g_channel_handler_class && env)
		{
			try
			{
				env->CallNonvirtualVoidMethod(thiz, g_channel_handler_class, ORIG_channelRead0, ctx, msg);
				if (env->ExceptionCheck())
				{
					env->ExceptionClear();
				}
			}
			catch (...)
			{
				// Silent catch - don't log in hook (too frequent)
			}
		}
		return;
	}

	// Fast path: if backtrack is disabled, pass through immediately
	if (!g_delay_enabled || g_delay_ms <= 0 || g_delay_ms > 10000)
	{
		try
		{
			env->CallNonvirtualVoidMethod(thiz, g_channel_handler_class, ORIG_channelRead0, ctx, msg);
			if (env->ExceptionCheck())
			{
				env->ExceptionClear();
			}
		}
		catch (...)
		{
			// Silent catch - don't crash
		}
		return;
	}

	// Check if this is an entity position packet
	bool should_delay = false;
	try
	{
		should_delay = is_entity_position_packet(env, msg);
		if (env->ExceptionCheck())
		{
			env->ExceptionClear();
			should_delay = false; // On error, don't delay
		}
	}
	catch (...)
	{
		should_delay = false; // On exception, don't delay
		// Silent catch - don't log in hook
	}
	
	if (should_delay)
	{
		try
		{
			// Store packet for delayed processing
			ULONGLONG current_time = GetTickCount64();
			ULONGLONG release_time = current_time + g_delay_ms;
			
			// Create global references
			jobject global_packet = nullptr;
			jobject global_ctx = nullptr;
			jobject global_handler = nullptr;
			
			global_packet = env->NewGlobalRef(msg);
			if (env->ExceptionCheck())
			{
				env->ExceptionClear();
				global_packet = nullptr;
			}
			
			if (global_packet)
			{
				if (ctx)
				{
					global_ctx = env->NewGlobalRef(ctx);
					if (env->ExceptionCheck())
					{
						env->ExceptionClear();
						// Continue without ctx
					}
				}
				
				if (thiz)
				{
					global_handler = env->NewGlobalRef(thiz);
					if (env->ExceptionCheck())
					{
						env->ExceptionClear();
						// Clean up and fail
						if (global_packet)
						{
							env->DeleteGlobalRef(global_packet);
							global_packet = nullptr;
						}
						if (global_ctx)
						{
							env->DeleteGlobalRef(global_ctx);
							global_ctx = nullptr;
						}
					}
				}
			}
			
			if (global_packet && global_handler)
			{
				// Limit queue size to prevent memory issues
				{
					std::lock_guard<std::mutex> lock(g_packet_queue_mutex);
					if (g_packet_queue.size() > 1000) // Max 1000 queued packets
					{
						// Queue too large, don't delay this packet
						// Clean up global refs
						if (global_packet) env->DeleteGlobalRef(global_packet);
						if (global_ctx) env->DeleteGlobalRef(global_ctx);
						if (global_handler) env->DeleteGlobalRef(global_handler);
						
						// Pass through immediately (fall through to end of function)
						global_packet = nullptr; // Mark as not queued
					}
					
					if (global_packet)
					{
						DelayedPacket delayed;
						delayed.packet = global_packet;
						delayed.context = global_ctx;
						delayed.handler = global_handler;
						delayed.release_time_ms = release_time;
						
						g_packet_queue.push(delayed);
						
						// Don't call original immediately - packet will be released later
						return;
					}
				}
			}
			else
			{
				// Failed to create global refs, clean up what we have
				if (global_packet) env->DeleteGlobalRef(global_packet);
				if (global_ctx) env->DeleteGlobalRef(global_ctx);
				if (global_handler) env->DeleteGlobalRef(global_handler);
			}
		}
		catch (...)
		{
			// Silent catch - if anything fails, just pass through
		}
	}
	
	// Not an entity position packet or failed to create global ref - pass through immediately
	try
	{
		env->CallNonvirtualVoidMethod(thiz, g_channel_handler_class, ORIG_channelRead0, ctx, msg);
		if (env->ExceptionCheck())
		{
			env->ExceptionClear();
		}
	}
	catch (...)
	{
		// Silent catch - don't crash
	}
}

// Packet processor thread function
static void packet_processor_thread()
{
	logger::log_debug("[BacktrackHook] Packet processor thread starting");
	
	JavaVM* jvm = enhance::instance ? enhance::instance->get_java_vm() : nullptr;
	if (!jvm)
	{
		logger::log_error("[BacktrackHook] Packet processor: No JVM");
		return;
	}
	
	JNIEnv* thread_env = nullptr;
	jint attach_result = jvm->AttachCurrentThread(reinterpret_cast<void**>(&thread_env), nullptr);
	if (attach_result != JNI_OK || !thread_env)
	{
		std::stringstream ss;
		ss << "[BacktrackHook] Packet processor: Failed to attach thread, result=" << attach_result;
		logger::log_error(ss.str());
		return;
	}
	
	logger::log_debug("[BacktrackHook] Packet processor thread attached successfully");
	
	static int loop_count = 0;
	while (!g_should_stop_processor)
	{
		loop_count++;
		
		if (!g_delay_enabled)
		{
			Sleep(10);
			continue;
		}
		
		try
		{
			ULONGLONG current_time = GetTickCount64();
			std::vector<DelayedPacket> packets_to_release;
			
			{
				std::lock_guard<std::mutex> lock(g_packet_queue_mutex);
				while (!g_packet_queue.empty())
				{
					DelayedPacket& packet = g_packet_queue.front();
					if (current_time >= packet.release_time_ms)
					{
						packets_to_release.push_back(packet);
						g_packet_queue.pop();
					}
					else
					{
						break; // Packets are in order, so we can stop here
					}
				}
			}
			
			if (loop_count % 1000 == 0 && !packets_to_release.empty())
			{
				std::stringstream ss;
				ss << "[BacktrackHook] Processor: Releasing " << packets_to_release.size() << " packets";
				logger::log_debug(ss.str());
			}
		
			// Release packets by calling the original method
			for (auto& packet : packets_to_release)
			{
				try
				{
					if (packet.packet && packet.handler && packet.context && ORIG_channelRead0 && g_channel_handler_class && thread_env)
					{
						// Call original method with delayed packet
						thread_env->CallNonvirtualVoidMethod(packet.handler, g_channel_handler_class, ORIG_channelRead0, packet.context, packet.packet);
						if (thread_env->ExceptionCheck())
						{
							thread_env->ExceptionClear();
							if (loop_count % 100 == 0)
							{
								logger::log_debug("[BacktrackHook] Processor: Exception when releasing packet, cleared");
							}
						}
					}
				}
				catch (...)
				{
					if (loop_count % 100 == 0)
					{
						logger::log_error("[BacktrackHook] Processor: Exception caught when releasing packet");
					}
				}
				
				// Clean up global references
				if (thread_env)
				{
					try
					{
						if (packet.packet)
						{
							thread_env->DeleteGlobalRef(packet.packet);
						}
						if (packet.context)
						{
							thread_env->DeleteGlobalRef(packet.context);
						}
						if (packet.handler)
						{
							thread_env->DeleteGlobalRef(packet.handler);
						}
					}
					catch (...)
					{
						if (loop_count % 100 == 0)
						{
							logger::log_error("[BacktrackHook] Processor: Exception during cleanup");
						}
					}
				}
			}
		}
		catch (...)
		{
			if (loop_count % 100 == 0)
			{
				logger::log_error("[BacktrackHook] Processor: Exception in main loop");
			}
		}
		
		Sleep(5); // Small sleep to avoid busy-waiting
	}
	
	logger::log_debug("[BacktrackHook] Packet processor thread stopping");
	
	// Detach thread from JVM
	try
	{
		jvm->DetachCurrentThread();
		logger::log_debug("[BacktrackHook] Packet processor thread detached");
	}
	catch (...)
	{
		logger::log_error("[BacktrackHook] Exception detaching processor thread");
	}
}

bool enhance::modules::backtrack_hook::init()
{
	logger::log_debug("[BacktrackHook] init() called");
	
	if (g_channel_handler_class != nullptr)
	{
		logger::log_debug("[BacktrackHook] Already initialized");
		return true;
	}

	auto env = enhance::instance->get_env();
	auto jvm = enhance::instance->get_java_vm();
	if (!env || !jvm) 
	{
		logger::log_error("[BacktrackHook] init() failed: No env or jvm");
		return false;
	}
	
	logger::log_debug("[BacktrackHook] Got env and jvm");
	
	// Ensure mappings are loaded
	if (!sdk::mappings::channel_inbound_handler_adapter_class_sig ||
	    !sdk::mappings::channel_read0_name ||
	    !sdk::mappings::channel_read0_sig)
	{
		logger::log_error("[BacktrackHook] init() failed: Mappings not loaded");
		return false;
	}
	
	logger::log_debug("[BacktrackHook] Mappings loaded");

	g_delay_enabled = false;
	g_delay_ms = 200;
	ORIG_channelRead0 = nullptr;

	if (!jnihook_initialized)
	{
		logger::log_debug("[BacktrackHook] Initializing JNIHook");
		jnihook_result_t result = JNIHook_Init(jvm);
		
		// Result 3 (JNIHOOK_ERR_SETUP_CLASS_FILE_LOAD_HOOK) can mean:
		// 1. JVMTI is already enabled/initialized by another module (like reach_hook)
		// 2. The class file load hook setup failed
		// In case 1, we can still use JNIHook_Attach, so we continue
		// In case 2, we'll find out when trying to attach the hook
		if (result == 3)
		{
			logger::log_debug("[BacktrackHook] JNIHook_Init returned 3 (JNIHOOK_ERR_SETUP_CLASS_FILE_LOAD_HOOK)");
			logger::log_debug("[BacktrackHook] This might mean JVMTI is already initialized by another module");
			logger::log_debug("[BacktrackHook] Continuing anyway - will attempt hook attachment");
			// Mark as initialized so we can try to attach the hook
			// If hook attachment fails, we'll return false then
			jnihook_initialized = true;
		}
		else if (result != JNIHOOK_OK)
		{
			std::stringstream ss;
			ss << "[BacktrackHook] JNIHook_Init failed with result=" << result << " (not OK, not 3)";
			logger::log_error(ss.str());
			return false;
		}
		else
		{
			jnihook_initialized = true;
			logger::log_debug("[BacktrackHook] JNIHook initialized successfully (result=OK)");
		}
	}

	// Find ChannelInboundHandlerAdapter class
	// Note: This is a Netty class (not obfuscated), so we can use it directly
	logger::log_debug("[BacktrackHook] Finding ChannelInboundHandlerAdapter class");
	jclass channel_handler_class = sdk::classloader::find_class(env, sdk::mappings::channel_inbound_handler_adapter_class_sig);
	if (!channel_handler_class)
	{
		// Try alternative: use JNI FindClass directly since Netty classes are in system classloader
		logger::log_debug("[BacktrackHook] ClassLoader failed, trying FindClass directly");
		channel_handler_class = env->FindClass("io/netty/channel/ChannelInboundHandlerAdapter");
		if (env->ExceptionCheck())
		{
			env->ExceptionClear();
			channel_handler_class = nullptr;
		}
	}
	
	if (!channel_handler_class)
	{
		logger::log_error("[BacktrackHook] Failed to find ChannelInboundHandlerAdapter class (both methods failed)");
		return false;
	}
	logger::log_debug("[BacktrackHook] Found ChannelInboundHandlerAdapter class");

	// Find channelRead method (public method that calls channelRead0)
	// channelRead0 is abstract/protected and not accessible, but channelRead is public
	logger::log_debug("[BacktrackHook] Finding channelRead method (public wrapper)");
	
	// channelRead signature: (Lio/netty/channel/ChannelHandlerContext;Ljava/lang/Object;)V
	const char* channel_read_name = "channelRead";
	const char* channel_read_sig = "(Lio/netty/channel/ChannelHandlerContext;Ljava/lang/Object;)V";
	
	jmethodID method_id = env->GetMethodID(channel_handler_class, channel_read_name, channel_read_sig);
	
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
		logger::log_error("[BacktrackHook] Exception getting channelRead method");
	}
	
	if (!method_id)
	{
		logger::log_error("[BacktrackHook] Failed to find channelRead method - GetMethodID returned NULL");
		env->DeleteLocalRef(channel_handler_class);
		return false;
	}
	logger::log_debug("[BacktrackHook] Found channelRead method");

	logger::log_debug("[BacktrackHook] Attaching hook to channelRead");
	jnihook_result_t result = JNIHook_Attach(method_id, reinterpret_cast<void*>(hkChannelRead0), &ORIG_channelRead0);
	if (result != JNIHOOK_OK)
	{
		std::stringstream ss;
		ss << "[BacktrackHook] JNIHook_Attach failed with result=" << result;
		logger::log_error(ss.str());
		env->DeleteLocalRef(channel_handler_class);
		return false;
	}
	logger::log_debug("[BacktrackHook] Hook attached successfully");

	g_channel_handler_class = reinterpret_cast<jclass>(env->NewGlobalRef(channel_handler_class));
	env->DeleteLocalRef(channel_handler_class);
	
	if (!g_channel_handler_class)
	{
		return false;
	}
	
	// Cache packet classes for fast instanceof checks
	logger::log_debug("[BacktrackHook] Caching packet classes");
	
	// EntityPositionS2CPacket (most common)
	if (sdk::mappings::entity_position_s2c_packet_class_sig)
	{
		jclass cls = sdk::classloader::find_class(env, sdk::mappings::entity_position_s2c_packet_class_sig);
		if (cls)
		{
			g_entity_position_packet_class = reinterpret_cast<jclass>(env->NewGlobalRef(cls));
			env->DeleteLocalRef(cls);
			logger::log_debug("[BacktrackHook] Cached EntityPositionS2CPacket");
		}
		else
		{
			logger::log_error("[BacktrackHook] Failed to find EntityPositionS2CPacket");
		}
	}
	
	// EntityMoveS2CPacket
	if (sdk::mappings::entity_move_s2c_packet_class_sig)
	{
		jclass cls = sdk::classloader::find_class(env, sdk::mappings::entity_move_s2c_packet_class_sig);
		if (cls)
		{
			g_entity_move_packet_class = reinterpret_cast<jclass>(env->NewGlobalRef(cls));
			env->DeleteLocalRef(cls);
			logger::log_debug("[BacktrackHook] Cached EntityMoveS2CPacket");
		}
	}
	
	// EntityTeleportS2CPacket
	if (sdk::mappings::entity_teleport_s2c_packet_class_sig)
	{
		jclass cls = sdk::classloader::find_class(env, sdk::mappings::entity_teleport_s2c_packet_class_sig);
		if (cls)
		{
			g_entity_teleport_packet_class = reinterpret_cast<jclass>(env->NewGlobalRef(cls));
			env->DeleteLocalRef(cls);
			logger::log_debug("[BacktrackHook] Cached EntityTeleportS2CPacket");
		}
	}
	
	// EntityS2CPacket (parent class)
	if (sdk::mappings::entity_s2c_packet_class_sig)
	{
		jclass cls = sdk::classloader::find_class(env, sdk::mappings::entity_s2c_packet_class_sig);
		if (cls)
		{
			g_entity_s2c_packet_class = reinterpret_cast<jclass>(env->NewGlobalRef(cls));
			env->DeleteLocalRef(cls);
			logger::log_debug("[BacktrackHook] Cached EntityS2CPacket");
		}
	}
	
	// Mark hook as ready only if we have at least one packet class
	if (g_entity_position_packet_class || g_entity_move_packet_class || g_entity_teleport_packet_class)
	{
		g_hook_ready = true;
		logger::log_debug("[BacktrackHook] Hook marked as ready");
	}
	else
	{
		logger::log_error("[BacktrackHook] No packet classes cached, hook not ready");
	}
	
	// Start packet processor thread
	if (!g_packet_processor_thread)
	{
		logger::log_debug("[BacktrackHook] Starting packet processor thread");
		g_should_stop_processor = false;
		g_packet_processor_thread = new std::thread(packet_processor_thread);
		logger::log_debug("[BacktrackHook] Packet processor thread started");
	}
	
	logger::log_debug("[BacktrackHook] init() completed successfully");
	return true;
}

void enhance::modules::backtrack_hook::shutdown()
{
	logger::log_debug("[BacktrackHook] shutdown() called");
	
	g_hook_ready = false;
	g_delay_enabled = false;
	g_delay_ms = 200;
	
	// Stop packet processor thread
	if (g_packet_processor_thread)
	{
		g_should_stop_processor = true;
		if (g_packet_processor_thread->joinable())
		{
			g_packet_processor_thread->join();
		}
		delete g_packet_processor_thread;
		g_packet_processor_thread = nullptr;
	}
	
	// Clean up packet queue
	{
		std::lock_guard<std::mutex> lock(g_packet_queue_mutex);
		auto env = enhance::instance ? enhance::instance->get_env() : nullptr;
		if (env)
		{
			while (!g_packet_queue.empty())
			{
				DelayedPacket packet = g_packet_queue.front();
				if (packet.packet)
				{
					env->DeleteGlobalRef(packet.packet);
				}
				if (packet.context)
				{
					env->DeleteGlobalRef(packet.context);
				}
				if (packet.handler)
				{
					env->DeleteGlobalRef(packet.handler);
				}
				g_packet_queue.pop();
			}
		}
	}
	
	ORIG_channelRead0 = nullptr;
	
	if (enhance::instance)
	{
		try
		{
			auto env = enhance::instance->get_env();
			if (env)
			{
				// Clean up cached packet classes
				if (g_entity_position_packet_class)
				{
					env->DeleteGlobalRef(g_entity_position_packet_class);
					g_entity_position_packet_class = nullptr;
				}
				if (g_entity_move_packet_class)
				{
					env->DeleteGlobalRef(g_entity_move_packet_class);
					g_entity_move_packet_class = nullptr;
				}
				if (g_entity_teleport_packet_class)
				{
					env->DeleteGlobalRef(g_entity_teleport_packet_class);
					g_entity_teleport_packet_class = nullptr;
				}
				if (g_entity_s2c_packet_class)
				{
					env->DeleteGlobalRef(g_entity_s2c_packet_class);
					g_entity_s2c_packet_class = nullptr;
				}
				if (g_channel_handler_class)
				{
					env->DeleteGlobalRef(g_channel_handler_class);
					g_channel_handler_class = nullptr;
				}
			}
		}
		catch (...)
		{
			g_channel_handler_class = nullptr;
			g_entity_position_packet_class = nullptr;
			g_entity_move_packet_class = nullptr;
			g_entity_teleport_packet_class = nullptr;
			g_entity_s2c_packet_class = nullptr;
		}
	}
	else
	{
		g_channel_handler_class = nullptr;
		g_entity_position_packet_class = nullptr;
		g_entity_move_packet_class = nullptr;
		g_entity_teleport_packet_class = nullptr;
		g_entity_s2c_packet_class = nullptr;
	}
	
	// Don't shutdown JNIHook here - it might be used by other modules (like reach_hook)
	// Only mark as not initialized locally
	if (jnihook_initialized)
	{
		logger::log_debug("[BacktrackHook] Marking JNIHook as not initialized locally (keeping global state)");
		jnihook_initialized = false;
	}
}

void enhance::modules::backtrack_hook::set_delay_enabled(bool enabled)
{
	g_delay_enabled = enabled;
}

void enhance::modules::backtrack_hook::set_delay_ms(int delay_ms)
{
	g_delay_ms = delay_ms;
}
