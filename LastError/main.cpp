#include "LastError.h"

LPSTR FormatLastError(DWORD dwErrorID)
{
    LPSTR lpszMessage = NULL;
    FormatMessage
    (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErrorID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_RUSSIAN_RUSSIA),
        (LPWSTR)&lpszMessage,
        NULL,
        NULL
    );
    return lpszMessage;
}

void PrintLastError(DWORD dwErrorID)
{
    LPSTR lpszMessage = FormatLastError(dwErrorID);
    std::cout << lpszMessage << std::endl;
    LocalFree(lpszMessage);
}