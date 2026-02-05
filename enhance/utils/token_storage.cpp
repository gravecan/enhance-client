#include "token_storage.h"
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <string>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "crypt32.lib")

namespace token_storage
{
	static const char ENCRYPTION_KEY[] = {
		0x45, 0x6E, 0x68, 0x61, 0x6E, 0x63, 0x65, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74,
		0x32, 0x30, 0x32, 0x34, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x4B, 0x65, 0x79, 0x21, 0x00
	};
	static const size_t ENCRYPTION_KEY_LEN = sizeof(ENCRYPTION_KEY) - 1;

	static const wchar_t* REGISTRY_PATH = L"SOFTWARE\\EnhanceClient";
	static const wchar_t* REGISTRY_VALUE = L"AuthToken";

	std::string EncryptDecrypt(const std::string& data)
	{
		std::string result = data;
		for (size_t i = 0; i < result.length(); ++i)
		{
			result[i] ^= ENCRYPTION_KEY[i % ENCRYPTION_KEY_LEN];
		}
		return result;
	}

	std::string Base64Encode(const std::string& data)
	{
		DWORD encodedLen = 0;
		if (!CryptBinaryToStringA((BYTE*)data.c_str(), (DWORD)data.length(),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &encodedLen))
		{
			return "";
		}

		std::vector<char> buffer(encodedLen);
		if (!CryptBinaryToStringA((BYTE*)data.c_str(), (DWORD)data.length(),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, buffer.data(), &encodedLen))
		{
			return "";
		}

		return std::string(buffer.data(), encodedLen - 1);
	}

	std::string Base64Decode(const std::string& encoded)
	{
		DWORD decodedLen = 0;
		if (!CryptStringToBinaryA(encoded.c_str(), (DWORD)encoded.length(),
			CRYPT_STRING_BASE64, NULL, &decodedLen, NULL, NULL))
		{
			return "";
		}

		std::vector<BYTE> buffer(decodedLen);
		if (!CryptStringToBinaryA(encoded.c_str(), (DWORD)encoded.length(),
			CRYPT_STRING_BASE64, buffer.data(), &decodedLen, NULL, NULL))
		{
			return "";
		}

		return std::string((char*)buffer.data(), decodedLen);
	}

	bool StoreToken(const std::string& token)
	{
		if (token.empty())
		{
			ClearToken();
			return true;
		}

		std::string encrypted = EncryptDecrypt(token);
		std::string encoded = Base64Encode(encrypted);

		if (encoded.empty())
		{
			return false;
		}

		HKEY hKey;
		LONG result = RegCreateKeyExW(
			HKEY_CURRENT_USER,
			REGISTRY_PATH,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&hKey,
			NULL
		);

		if (result != ERROR_SUCCESS)
		{
			return false;
		}

		int size_needed = MultiByteToWideChar(CP_UTF8, 0, encoded.c_str(), (int)encoded.length(), NULL, 0);
		std::vector<wchar_t> wbuffer(size_needed + 1);
		MultiByteToWideChar(CP_UTF8, 0, encoded.c_str(), (int)encoded.length(), wbuffer.data(), size_needed);
		wbuffer[size_needed] = 0;

		result = RegSetValueExW(
			hKey,
			REGISTRY_VALUE,
			0,
			REG_SZ,
			(BYTE*)wbuffer.data(),
			(DWORD)((size_needed + 1) * sizeof(wchar_t))
		);

		RegCloseKey(hKey);

		return (result == ERROR_SUCCESS);
	}

	std::string GetToken()
	{
		HKEY hKey;
		LONG result = RegOpenKeyExW(
			HKEY_CURRENT_USER,
			REGISTRY_PATH,
			0,
			KEY_READ,
			&hKey
		);

		if (result != ERROR_SUCCESS)
		{
			return "";
		}

		DWORD dataSize = 0;
		result = RegQueryValueExW(
			hKey,
			REGISTRY_VALUE,
			NULL,
			NULL,
			NULL,
			&dataSize
		);

		if (result != ERROR_SUCCESS || dataSize == 0)
		{
			RegCloseKey(hKey);
			return "";
		}

		std::vector<wchar_t> buffer(dataSize / sizeof(wchar_t) + 1);
		result = RegQueryValueExW(
			hKey,
			REGISTRY_VALUE,
			NULL,
			NULL,
			(LPBYTE)buffer.data(),
			&dataSize
		);

		RegCloseKey(hKey);

		if (result != ERROR_SUCCESS)
		{
			return "";
		}

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, NULL, 0, NULL, NULL);
		std::vector<char> mbuffer(size_needed);
		WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, mbuffer.data(), size_needed, NULL, NULL);

		std::string encoded(mbuffer.data(), size_needed - 1);

		std::string encrypted = Base64Decode(encoded);
		if (encrypted.empty())
		{
			return "";
		}

		std::string token = EncryptDecrypt(encrypted);
		return token;
	}

	void ClearToken()
	{
		HKEY hKey;
		LONG result = RegOpenKeyExW(
			HKEY_CURRENT_USER,
			REGISTRY_PATH,
			0,
			KEY_WRITE,
			&hKey
		);

		if (result == ERROR_SUCCESS)
		{
			RegDeleteValueW(hKey, REGISTRY_VALUE);
			RegCloseKey(hKey);
		}
	}

	bool HasToken()
	{
		std::string token = GetToken();
		return !token.empty();
	}
}

