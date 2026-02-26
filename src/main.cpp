#include "headers/config.h"
#include "headers/utils.h"
#include "headers/inject.h"
#include <vector>
#include <string>

int main() {
    SetConsoleTitleW(getExecutableName());

    std::vector<std::wstring> dllList = findDlls(PATTERN_DLL);
    if (dllList.empty())
        return exitWithError(MSG_ERROR_NO_DLL, dllList);

    std::vector<std::wstring> injectDlls;
    if (dllList.size() == 1) {
        injectDlls = dllList;
    }
    else {
        injectDlls = promptDllMultiSelect(dllList);
    }

    int injectCount = (int)injectDlls.size();

    const wchar_t* procPattern = PATTERN_PROC;
    DWORD pid = getProcessId(procPattern);

    if (!pid) {
        g_console.print(MSG_NO_PROCESS);
        launchSteam();
        g_console.print(MSG_WAIT_PROCESS);

        while (!(pid = getProcessId(procPattern)))
            Sleep(PROCESS_POLL_INTERVAL);

        for (int m = DELAY_POST_LAUNCH; m > 0; m -= 1000) {
            g_console.printf(MSG_INJECT_IN, SEC(m));
            Sleep(1000);
        }
        g_console.print(L"\n");
    }

    for (int i = 0; i < injectCount; ++i) {
        const wchar_t* dllName = wcsrchr(injectDlls[i].c_str(), L'\\');
        dllName = dllName ? dllName + 1 : injectDlls[i].c_str();

        g_console.print(MSG_INJECTING_DLL);
        g_console.print(dllName);
        g_console.print(MSG_INJECT_NOTE);

        for (;;) {
            if (injectDll(pid, injectDlls[i].c_str()))
                break;

            DWORD err = GetLastError();
            if (err == ERROR_ELEVATION_REQUIRED || err == ERROR_ACCESS_DENIED)
                return exitWithError(MSG_ERROR_ELEVATED, dllList);

            g_console.print(MSG_ERROR_INJECT);
            Sleep(DELAY_RETRY);

            if (!getProcessId(procPattern))
                return exitWithError(MSG_ERROR_CRASHED, dllList);
        }

        if (i + 1 < injectCount)
            Sleep(INJECT_DELAY);
    }

    // hideConsoleWindow(pid, DELAY_HIDE_CONSOLE_TIMEOUT);
    countdown(DELAY_POST_INJECT);
    return 0;
}