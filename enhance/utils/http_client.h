#pragma once
#include <string>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace http_client
{
	struct Response
	{
		int status_code;
		std::string body;
		bool success;
		std::string error;
	};

	bool Initialize();
	void Cleanup();
	Response Post(const std::string& url, const std::string& json_data, const std::string& auth_token = "");
	Response Get(const std::string& url, const std::string& auth_token = "");
	std::string GetApiUrl();
}

