#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Common.h>

BUFFER SendRequest(WCHAR *url, WCHAR *method, PPACKAGE Package);
BUFFER FileFetch(PCHAR FileId, PCHAR TaskId);
void Response(PCHAR Id, PVOID Data, BOOL Error, LPWSTR Endpoint);

#endif
