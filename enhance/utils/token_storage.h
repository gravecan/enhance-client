#pragma once
#include <string>

namespace token_storage
{
	bool StoreToken(const std::string& token);
	std::string GetToken();
	void ClearToken();
	bool HasToken();
}

