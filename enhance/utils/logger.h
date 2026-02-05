#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <Windows.h>

namespace logger
{
	void init(HMODULE h_module = nullptr);
	void shutdown();
	void log(const std::string& message);
	void log_error(const std::string& message);
	void log_debug(const std::string& message);
}

