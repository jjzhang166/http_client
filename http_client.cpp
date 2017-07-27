#include "http_client.h"

#include <windows.h>

// #include <WinInet.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

bool CHttpClient::Get(const wchar_t* url, Writeable* writeable)
{
	URL_COMPONENTS urlComp;
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);
	

	wchar_t lpszScheme[MAX_PATH] = { 0 };
	wchar_t lpszHostName[MAX_PATH] = { 0 };
	wchar_t lpszPath[MAX_PATH] = { 0 };
	wchar_t lpszExtra[MAX_PATH] = { 0 };

	urlComp.lpszScheme = lpszScheme;
	urlComp.dwSchemeLength = MAX_PATH;

	urlComp.lpszHostName = lpszHostName;
	urlComp.dwHostNameLength = MAX_PATH;
	urlComp.lpszUrlPath = lpszPath;
	urlComp.dwUrlPathLength = MAX_PATH;
	urlComp.lpszExtraInfo = lpszExtra;
	urlComp.dwExtraInfoLength = MAX_PATH;

	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	bool bSuccessed = FALSE;

	if (!WinHttpCrackUrl(url, wcslen(url), 0, &urlComp)) {
		goto _END;
	}

	hSession = WinHttpOpen(L"WinHTTP 1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession) {
		goto _END;
	}

	hConnect = WinHttpConnect(
		hSession,
		urlComp.lpszHostName,
		urlComp.nPort, 0);

	if (!hConnect) {
		goto _END;
	}

	hRequest = WinHttpOpenRequest(
		hConnect,
		L"GET",
		urlComp.lpszUrlPath,
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_REFRESH);

	if (!hRequest) {
		goto _END;
	}

	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		goto _END;
	}

	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		goto _END;
	}

	DWORD dwStatusCode = 0;
	DWORD dwSize = sizeof(dwStatusCode);
	if (!WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&dwStatusCode, 
		&dwSize, 
		WINHTTP_NO_HEADER_INDEX)) {
		goto _END;
	}

	if (dwStatusCode != HTTP_STATUS_OK) {
		goto _END;
	}

	DWORD dwNumberOfBytesToRead;
	DWORD dwNumberOfBytesRead;
	bSuccessed = TRUE;
	do
	{
		dwNumberOfBytesToRead = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwNumberOfBytesToRead)) {
			bSuccessed = FALSE;
			break;
		}

		LPBYTE pszOutBuffer = new BYTE[dwNumberOfBytesToRead];
		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwNumberOfBytesToRead, &dwNumberOfBytesRead)) {
			delete[]pszOutBuffer;
			bSuccessed = FALSE;
			break;
		}

		writeable->write(pszOutBuffer, dwNumberOfBytesRead);

		delete[]pszOutBuffer;
	} while (dwNumberOfBytesToRead > 0);

_END:
	if (hSession) {
		WinHttpCloseHandle(hSession);
	}
	if (hConnect) {
		WinHttpCloseHandle(hConnect);
	}
	if (hRequest) {
		WinHttpCloseHandle(hRequest);
	}

	return bSuccessed;
}
