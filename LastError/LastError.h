#pragma once
#ifndef LASTERROR_H
#define LASTERROR_H

#include <Windows.h>
#include <iostream>

// Function declarations
LPSTR FormatLastError(DWORD dwErrorID);
void PrintLastError(DWORD dwErrorID);

#endif