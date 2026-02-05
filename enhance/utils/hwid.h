#pragma once
#include <string>

namespace hwid
{
	std::string GetHWID();
	std::string GetCPUID();
	std::string GetMotherboardSerial();
	std::string GetHDDSerial();
	std::string GetMachineGUID();
	std::string SHA256(const std::string& data);
}
