#include "logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <mutex>

static std::ofstream log_file;
static std::mutex log_mutex;
static bool is_initialized = false;

void logger::init(HMODULE h_module)
{
	std::lock_guard<std::mutex> lock(log_mutex);
	
	if (is_initialized)
		return;

	std::string path = "enhance_log.txt";
	
	if (h_module)
	{
		char dll_path[MAX_PATH];
		if (GetModuleFileNameA(h_module, dll_path, MAX_PATH))
		{
			std::string full_path(dll_path);
			size_t last_slash = full_path.find_last_of("\\/");
			if (last_slash != std::string::npos)
			{
				path = full_path.substr(0, last_slash + 1) + "enhance_log.txt";
			}
		}
	}
	
	log_file.open(path, std::ios::out | std::ios::app);
	if (log_file.is_open())
	{
		log_file << "\n=== Log session started ===\n";
		log_file.flush();
	}

	is_initialized = true;
}

void logger::shutdown()
{
	std::lock_guard<std::mutex> lock(log_mutex);
	
	if (log_file.is_open())
	{
		log("Logger shutting down");
		log_file.close();
	}
	
	is_initialized = false;
}

void logger::log(const std::string& message)
{
	std::lock_guard<std::mutex> lock(log_mutex);
	
	if (!is_initialized)
		return;

	auto now = std::chrono::system_clock::now();
	auto time_t = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::stringstream ss;
	ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
	ss << "." << std::setfill('0') << std::setw(3) << ms.count();
	std::string timestamp = ss.str();

	std::string log_message = "[" + timestamp + "] " + message;

	if (log_file.is_open())
	{
		log_file << log_message << std::endl;
		log_file.flush();
	}
}

void logger::log_error(const std::string& message)
{
	log("[ERROR] " + message);
}

void logger::log_debug(const std::string& message)
{
	log("[DEBUG] " + message);
}

