#pragma once
#include <windows.h>

int injectDll(DWORD pid, const wchar_t* dllPath);
int queueUserApc(DWORD pid, FARPROC loaderAddress, LPVOID remoteMemory);
FARPROC getRemoteLoadLibraryAddress(HANDLE process);