#include "headers/config.h"
#include "headers/utils.h"
#include <tlhelp32.h>
#include <winreg.h>
#include <vector>
#include <wctype.h>
#include <sstream>

Console g_console;

Console::Console() {}

void Console::print(const wchar_t* s) {
    DWORD written;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), s, (DWORD)wcslen(s), &written, nullptr);
}

void Console::printf(const wchar_t* fmt, ...) {
    wchar_t buf[CONSOLE_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vswprintf(buf, CONSOLE_BUFFER_SIZE, fmt, args);
    va_end(args);
    print(buf);
}

void Console::clear() {
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD coord = { 0, 0 };
    DWORD written;
    if (!GetConsoleScreenBufferInfo(hOutput, &info)) return;
    DWORD cells = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacterW(hOutput, L' ', cells, coord, &written);
    FillConsoleOutputAttribute(hOutput, info.wAttributes, cells, coord, &written);
    SetConsoleCursorPosition(hOutput, coord);
}

bool wildcardEquals(const wchar_t* text, const wchar_t* pattern) {
    if (!text || !pattern) return false;
    if (wcscmp(pattern, L"*") == 0) return true;

    const wchar_t* lastWildcard = nullptr;
    const wchar_t* wildMatchStart = nullptr;

    while (*text) {
        if (*pattern == L'*') {
            while (*(pattern + 1) == L'*') pattern++;
            lastWildcard = pattern++;
            wildMatchStart = text;
        }
        else if (towlower(*text) == towlower(*pattern)) {
            text++;
            pattern++;
        }
        else if (lastWildcard) {
            pattern = lastWildcard + 1;
            text = ++wildMatchStart;
        }
        else {
            return false;
        }
    }
    while (*pattern == L'*') pattern++;
    return *pattern == L'\0';
}

void getLongPath(const wchar_t* in, std::wstring& out) {
    if (!in) { out.clear(); return; }
    DWORD len = GetFullPathNameW(in, 0, nullptr, nullptr);
    if (!len || len >= 32760) { out = in; return; }
    std::wstring tmp(len, L'\0');
    GetFullPathNameW(in, len, &tmp[0], nullptr);
    tmp.resize(wcslen(tmp.c_str()));
    if (!wcsncmp(tmp.c_str(), L"\\\\?\\", 4)) out = tmp;
    else if (!wcsncmp(tmp.c_str(), L"\\\\", 2)) { out = L"\\\\?\\UNC\\"; out += tmp.substr(2); }
    else { out = L"\\\\?\\"; out += tmp; }
}

std::vector<std::wstring> findDlls(const wchar_t* pattern) {
    std::vector<std::wstring> list;
    wchar_t dir[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, dir, MAX_PATH)) return list;
    wchar_t* slash = wcsrchr(dir, L'\\');
    if (slash) slash[1] = L'\0';

    wchar_t mask[MAX_PATH];
    swprintf(mask, MAX_PATH, L"%s%s", dir, pattern);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(mask, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return list;

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring full = dir;
            full += findData.cFileName;
            std::wstring longPath;
            getLongPath(full.c_str(), longPath);
            list.push_back(longPath);
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);
    return list;
}

int readLine(wchar_t* buf, DWORD size) {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD read = 0;
    if (!ReadConsoleW(hInput, buf, size - 1, &read, nullptr) || read == 0)
        return 0;
    while (read > 0 && (buf[read - 1] == L'\r' || buf[read - 1] == L'\n'))
        buf[--read] = 0;
    buf[read] = 0;
    return (int)read;
}

int stringToInt(const wchar_t* s) {
    int sign = 1, val = 0;
    while (*s == L' ') s++;
    if (*s == L'-' || *s == L'+') { sign = (*s == L'-') ? -1 : 1; s++; }
    while (*s >= L'0' && *s <= L'9') {
        int digit = *s - L'0';
        if (val > 214748364 || (val == 214748364 && digit > 7))
            return sign == 1 ? 2147483647 : (-2147483647 - 1);
        val = val * 10 + digit;
        s++;
    }
    return sign * val;
}

std::vector<std::wstring> promptDllMultiSelect(const std::vector<std::wstring>& dlls) {
    std::vector<std::wstring> result;
    wchar_t inputBuffer[PROMPT_INPUT_SIZE];

    for (;;) {
        g_console.clear();
        for (size_t i = 0; i < dlls.size(); ++i) {
            const wchar_t* name = wcsrchr(dlls[i].c_str(), L'\\');
            name = name ? name + 1 : dlls[i].c_str();
            g_console.printf(L"%d: %ls\n", (int)i + 1, name);
        }
        g_console.printf(MSG_DLL_PROMPT, (int)dlls.size());

        if (!readLine(inputBuffer, sizeof(inputBuffer) / sizeof(inputBuffer[0]))) continue;

        std::wstring input = inputBuffer;
        std::vector<int> indices;

        if (input == L"*") {
            for (size_t i = 0; i < dlls.size(); ++i)
                indices.push_back((int)i + 1);
        }
        else {
            std::wstringstream ss(input);
            std::wstring token;
            while (ss >> token) {
                int idx = stringToInt(token.c_str());
                if (idx >= 1 && idx <= (int)dlls.size())
                    indices.push_back(idx);
            }
        }

        if (!indices.empty()) {
            for (int idx : indices)
                result.push_back(dlls[idx - 1]);
            return result;
        }
    }
}

DWORD getProcessId(const wchar_t* pattern) {
    PROCESSENTRY32W processEntry = { sizeof(processEntry) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    DWORD pid = 0;
    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            if (wildcardEquals(processEntry.szExeFile, pattern)) {
                pid = processEntry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
    return pid;
}

void countdown(int ms) {
    for (int remaining = ms; remaining > 0; remaining -= 1000) {
        g_console.printf(MSG_EXIT_IN, SEC(remaining));
        Sleep(1000);
    }
    g_console.print(L"\n");
}

bool getSteamExecutable(std::wstring& out) {
    static const HKEY roots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    static const wchar_t* values[] = { L"SteamExe", L"SteamPath" };
    wchar_t buf[MAX_PATH];

    for (int rootIndex = 0; rootIndex < 2; ++rootIndex) {
        for (int valueIndex = 0; valueIndex < 2; ++valueIndex) {
            DWORD len = sizeof(buf);
            DWORD flags = (rootIndex == 1) ? KEY_WOW64_64KEY : 0;
            if (RegGetValueW(roots[rootIndex], L"Software\\Valve\\Steam", values[valueIndex],
                RRF_RT_REG_SZ | flags, nullptr, buf, &len) == ERROR_SUCCESS) {
                if (valueIndex == 1 && !wcsrchr(buf, L'\\'))
                    wcscat(buf, L"\\Steam.exe");
                out = buf;
                return true;
            }
        }
    }
    return false;
}

void launchSteam() {
    std::wstring exePath;
    if (!getSteamExecutable(exePath)) return;

    wchar_t commandLine[MAX_PATH + 64];
    swprintf(commandLine, MAX_PATH + 64, L"\"%ls\" steam://run/%ls", exePath.c_str(), APP_ID);

    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };
    if (CreateProcessW(nullptr, commandLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

const wchar_t* getExecutableName() {
    static wchar_t buf[MAX_PATH];
    wchar_t full[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, full, MAX_PATH)) return L"Injector";

    const wchar_t* slash = wcsrchr(full, L'\\');
    if (!slash) slash = wcsrchr(full, L'/');
    const wchar_t* name = slash ? slash + 1 : full;
    const wchar_t* dot = wcsrchr(name, L'.');
    size_t len = dot ? (size_t)(dot - name) : wcslen(name);
    if (len >= MAX_PATH) len = MAX_PATH - 1;
    wmemcpy(buf, name, len);
    buf[len] = 0;
    return buf;
}

static HWND findConsoleWindow(DWORD pid) {
    struct Param { DWORD pid; HWND hwnd; } param = { pid, nullptr };
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* p = (Param*)lParam;
        DWORD winPid = 0;
        GetWindowThreadProcessId(hwnd, &winPid);
        if (winPid == p->pid) {
            wchar_t className[CLASS_NAME_BUFFER_SIZE];
            GetClassNameW(hwnd, className, CLASS_NAME_BUFFER_SIZE);
            if (wcscmp(className, CONSOLE_WINDOW_CLASS) == 0) {
                p->hwnd = hwnd;
                return FALSE;
            }
        }
        return TRUE;
        }, (LPARAM)&param);
    return param.hwnd;
}

bool hideConsoleWindow(DWORD pid, int timeoutMs) {
    int waited = 0;
    while (waited < timeoutMs) {
        HWND consoleWindow = findConsoleWindow(pid);
        if (consoleWindow) {
            Sleep(POST_HIDE_DELAY);
            ShowWindow(consoleWindow, SW_HIDE);
            return true;
        }
        Sleep(POLL_INTERVAL);
        waited += POLL_INTERVAL;
    }
    return false;
}

int exitWithError(const wchar_t* msg, const std::vector<std::wstring>& dllList) {
    (void)dllList;
    g_console.print(msg);
    countdown(DELAY_EXIT);
    return 1;
}