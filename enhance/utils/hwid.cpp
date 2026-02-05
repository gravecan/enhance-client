#include "hwid.h"
#include <windows.h>
#include <wincrypt.h>
#include <intrin.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#pragma comment(lib, "advapi32.lib")

namespace hwid
{
	std::string GetCPUID()
	{
		int cpuInfo[4] = { 0 };
		__cpuid(cpuInfo, 1);
		
		std::ostringstream oss;
		oss << std::hex << std::setfill('0')
			<< std::setw(8) << cpuInfo[0]
			<< std::setw(8) << cpuInfo[1]
			<< std::setw(8) << cpuInfo[2]
			<< std::setw(8) << cpuInfo[3];
		return oss.str();
	}

	std::string GetMotherboardSerial()
	{
		std::string result;
		HKEY hKey;
		
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			"HARDWARE\\DESCRIPTION\\System\\BIOS",
			0,
			KEY_READ,
			&hKey) == ERROR_SUCCESS)
		{
			char buffer[256] = { 0 };
			DWORD bufferSize = sizeof(buffer);
			
			if (RegQueryValueExA(hKey, "BaseBoardProduct", NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
			{
				result += buffer;
			}
			
			bufferSize = sizeof(buffer);
			if (RegQueryValueExA(hKey, "SystemManufacturer", NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
			{
				result += buffer;
			}
			
			RegCloseKey(hKey);
		}
		
		return result.empty() ? "UNKNOWN_MB" : result;
	}

	std::string GetHDDSerial()
	{
		HANDLE hDevice = CreateFileA("\\\\.\\PhysicalDrive0",
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return "UNKNOWN_HDD";
		}

		STORAGE_PROPERTY_QUERY query;
		ZeroMemory(&query, sizeof(query));
		query.PropertyId = StorageDeviceProperty;
		query.QueryType = PropertyStandardQuery;

		DWORD bytesReturned = 0;
		BYTE buffer[10000];
		
		if (DeviceIoControl(hDevice,
			IOCTL_STORAGE_QUERY_PROPERTY,
			&query,
			sizeof(query),
			&buffer,
			sizeof(buffer),
			&bytesReturned,
			NULL))
		{
			STORAGE_DEVICE_DESCRIPTOR* desc = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
			if (desc->SerialNumberOffset > 0)
			{
				std::string serial((char*)buffer + desc->SerialNumberOffset);
				CloseHandle(hDevice);
				return serial;
			}
		}

		CloseHandle(hDevice);
		return "UNKNOWN_HDD";
	}

	std::string GetMachineGUID()
	{
		// Windows Machine GUID - very stable identifier that doesn't change
		// Stored in: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Cryptography\MachineGuid
		std::string result;
		HKEY hKey;
		
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Cryptography",
			0,
			KEY_READ | KEY_WOW64_64KEY,
			&hKey) == ERROR_SUCCESS)
		{
			char buffer[256] = { 0 };
			DWORD bufferSize = sizeof(buffer);
			
			if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
			{
				result = buffer;
			}
			
			RegCloseKey(hKey);
		}
		
		if (result.empty())
		{
			DWORD volumeSerial = 0;
			if (GetVolumeInformationA("C:\\", NULL, 0, &volumeSerial, NULL, NULL, NULL, 0))
			{
				std::ostringstream oss;
				oss << std::hex << std::setfill('0') << std::setw(8) << volumeSerial;
				result = oss.str();
			}
		}
		
		return result.empty() ? "UNKNOWN_GUID" : result;
	}

	std::string SHA256(const std::string& data)
	{
		HCRYPTPROV hProv = 0;
		HCRYPTHASH hHash = 0;
		BYTE hash[32];
		DWORD hashLen = 32;
		
		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
		{
			return "";
		}
		
		if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
		{
			CryptReleaseContext(hProv, 0);
			return "";
		}
		
		if (!CryptHashData(hHash, (BYTE*)data.c_str(), (DWORD)data.length(), 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hProv, 0);
			return "";
		}
		
		if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hProv, 0);
			return "";
		}
		
		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		for (DWORD i = 0; i < hashLen; i++)
		{
			oss << std::setw(2) << (int)hash[i];
		}
		
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		
		return oss.str();
	}

	std::string GetHWID()
	{
		std::string cpuid = GetCPUID();
		std::string motherboard = GetMotherboardSerial();
		std::string hdd = GetHDDSerial();
		std::string machineGuid = GetMachineGUID();
		
		std::string combined = cpuid + motherboard + hdd + machineGuid;
		std::string hash = SHA256(combined);
		
		// Debug output
		char debug[512];
		sprintf_s(debug, "[HWID] Generated: %s (len: %d)", hash.c_str(), (int)hash.length());
		OutputDebugStringA(debug);
		
		return hash;
	}
}
