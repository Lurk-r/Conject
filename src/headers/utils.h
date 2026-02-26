#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include "config.h"

class Console {
public:
    Console();
    void print(const wchar_t* s);
    void printf(const wchar_t* fmt, ...);
    void clear();
};

extern Console g_console;

bool wildcardEquals(const wchar_t* text, const wchar_t* pattern);
void getLongPath(const wchar_t* in, std::wstring& out);
std::vector<std::wstring> findDlls(const wchar_t* pattern);
int readLine(wchar_t* buf, DWORD size);
int stringToInt(const wchar_t* s);
std::vector<std::wstring> promptDllMultiSelect(const std::vector<std::wstring>& dlls);
DWORD getProcessId(const wchar_t* pattern);
void countdown(int ms);
bool getSteamExecutable(std::wstring& out);
void launchSteam();
const wchar_t* getExecutableName();
bool hideConsoleWindow(DWORD pid, int timeoutMs);
int exitWithError(const wchar_t* msg, const std::vector<std::wstring>& dllList);