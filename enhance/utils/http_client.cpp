#include "http_client.h"
#include <sstream>
#include <vector>

namespace http_client
{
	static HINTERNET g_hSession = nullptr;
	static std::string g_api_url = "https://enhanceclient.store/api";

	bool Initialize()
	{
		if (g_hSession)
			return true;

		g_hSession = WinHttpOpen(
			L"Enhance Client/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0
		);

		if (!g_hSession)
		{
			return false;
		}

		WinHttpSetTimeouts(g_hSession, 10000, 10000, 10000, 10000);

		return true;
	}

	void Cleanup()
	{
		if (g_hSession)
		{
			WinHttpCloseHandle(g_hSession);
			g_hSession = nullptr;
		}
	}

	std::string GetApiUrl()
	{
		return g_api_url;
	}

	std::wstring StringToWString(const std::string& str)
	{
		if (str.empty())
			return std::wstring();

		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}

	std::string WStringToString(const std::wstring& wstr)
	{
		if (wstr.empty())
			return std::string();

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}

	Response Post(const std::string& url, const std::string& json_data, const std::string& auth_token)
	{
		Response response;
		response.success = false;
		response.status_code = 0;

		if (!g_hSession)
		{
			if (!Initialize())
			{
				response.error = "Failed to initialize WinHTTP";
				return response;
			}
		}

		std::wstring wurl = StringToWString(url);
		
		URL_COMPONENTSW urlComp;
		ZeroMemory(&urlComp, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.dwSchemeLength = (DWORD)-1;
		urlComp.dwHostNameLength = (DWORD)-1;
		urlComp.dwUrlPathLength = (DWORD)-1;

		if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp))
		{
			response.error = "Invalid URL";
			return response;
		}

		std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
		std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

		HINTERNET hConnect = WinHttpConnect(g_hSession, hostName.c_str(), urlComp.nPort, 0);
		if (!hConnect)
		{
			response.error = "Failed to connect to server";
			return response;
		}

		DWORD flags = 0;
		if (urlComp.nScheme == INTERNET_SCHEME_HTTPS)
		{
			flags = WINHTTP_FLAG_SECURE;
		}

		HINTERNET hRequest = WinHttpOpenRequest(
			hConnect,
			L"POST",
			urlPath.c_str(),
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			flags
		);

		if (!hRequest)
		{
			WinHttpCloseHandle(hConnect);
			response.error = "Failed to open request";
			return response;
		}

		std::wstring headers = L"Content-Type: application/json\r\n";
		if (!auth_token.empty())
		{
			std::wstring auth_header = L"Authorization: Bearer " + StringToWString(auth_token) + L"\r\n";
			headers += auth_header;
		}

		if (!WinHttpSendRequest(
			hRequest,
			headers.c_str(),
			(DWORD)headers.length(),
			(LPVOID)json_data.c_str(),
			(DWORD)json_data.length(),
			(DWORD)json_data.length(),
			0))
		{
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			response.error = "Failed to send request";
			return response;
		}

		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			response.error = "Failed to receive response";
			return response;
		}

		DWORD statusCode = 0;
		DWORD statusCodeSize = sizeof(statusCode);
		WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
		response.status_code = statusCode;

		std::vector<char> buffer;
		DWORD bytesRead = 0;
		do
		{
			char tempBuffer[4096];
			if (!WinHttpReadData(hRequest, tempBuffer, sizeof(tempBuffer), &bytesRead))
			{
				break;
			}
			if (bytesRead > 0)
			{
				buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesRead);
			}
		} while (bytesRead > 0);

		if (!buffer.empty())
		{
			response.body = std::string(buffer.begin(), buffer.end());
		}

		response.success = (statusCode >= 200 && statusCode < 300);

		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);

		return response;
	}

	Response Get(const std::string& url, const std::string& auth_token)
	{
		Response response;
		response.success = false;
		response.status_code = 0;

		if (!g_hSession)
		{
			if (!Initialize())
			{
				response.error = "Failed to initialize WinHTTP";
				return response;
			}
		}

		std::wstring wurl = StringToWString(url);
		
		URL_COMPONENTSW urlComp;
		ZeroMemory(&urlComp, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.dwSchemeLength = (DWORD)-1;
		urlComp.dwHostNameLength = (DWORD)-1;
		urlComp.dwUrlPathLength = (DWORD)-1;

		if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp))
		{
			response.error = "Invalid URL";
			return response;
		}

		std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
		std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

		HINTERNET hConnect = WinHttpConnect(g_hSession, hostName.c_str(), urlComp.nPort, 0);
		if (!hConnect)
		{
			response.error = "Failed to connect to server";
			return response;
		}

		DWORD flags = 0;
		if (urlComp.nScheme == INTERNET_SCHEME_HTTPS)
		{
			flags = WINHTTP_FLAG_SECURE;
		}

		HINTERNET hRequest = WinHttpOpenRequest(
			hConnect,
			L"GET",
			urlPath.c_str(),
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			flags
		);

		if (!hRequest)
		{
			WinHttpCloseHandle(hConnect);
			response.error = "Failed to open request";
			return response;
		}

		std::wstring headers;
		if (!auth_token.empty())
		{
			headers = L"Authorization: Bearer " + StringToWString(auth_token) + L"\r\n";
		}

		if (!WinHttpSendRequest(
			hRequest,
			headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
			headers.empty() ? 0 : (DWORD)headers.length(),
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0))
		{
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			response.error = "Failed to send request";
			return response;
		}

		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			response.error = "Failed to receive response";
			return response;
		}

		DWORD statusCode = 0;
		DWORD statusCodeSize = sizeof(statusCode);
		WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
		response.status_code = statusCode;

		std::vector<char> buffer;
		DWORD bytesRead = 0;
		do
		{
			char tempBuffer[4096];
			if (!WinHttpReadData(hRequest, tempBuffer, sizeof(tempBuffer), &bytesRead))
			{
				break;
			}
			if (bytesRead > 0)
			{
				buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesRead);
			}
		} while (bytesRead > 0);

		if (!buffer.empty())
		{
			response.body = std::string(buffer.begin(), buffer.end());
		}

		response.success = (statusCode >= 200 && statusCode < 300);

		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);

		return response;
	}
}

