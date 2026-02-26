#include "headers/config.h"
#include "headers/inject.h"
#include "headers/utils.h"
#include <tlhelp32.h>
#include <psapi.h>

class HandleGuard {
    HANDLE handle_;
public:
    explicit HandleGuard(HANDLE handle = nullptr) : handle_(handle) {}
    ~HandleGuard() { if (handle_ && handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_); }
    operator HANDLE() const { return handle_; }
    HANDLE get() const { return handle_; }
};

FARPROC getRemoteLoadLibraryAddress(HANDLE process) {
    if (!process) return nullptr;

    HMODULE hLocalKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hLocalKernel32) return nullptr;

    FARPROC localLoadLibraryW = GetProcAddress(hLocalKernel32, "LoadLibraryW");
    if (!localLoadLibraryW) return nullptr;

    uintptr_t offset = (uintptr_t)localLoadLibraryW - (uintptr_t)hLocalKernel32;

    HMODULE hMods[1024];
    DWORD cbNeeded = 0;
    if (!EnumProcessModulesEx(process, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
        return nullptr;
    }

    size_t moduleCount = cbNeeded / sizeof(HMODULE);
    HMODULE hRemoteKernel32 = nullptr;
    wchar_t moduleName[MAX_PATH] = { 0 };

    for (size_t i = 0; i < moduleCount; ++i) {
        if (GetModuleBaseNameW(process, hMods[i], moduleName, MAX_PATH)) {
            if (_wcsicmp(moduleName, L"kernel32.dll") == 0) {
                hRemoteKernel32 = hMods[i];
                break;
            }
        }
    }

    if (!hRemoteKernel32) return nullptr;

    return (FARPROC)((uintptr_t)hRemoteKernel32 + offset);
}

int queueUserApc(DWORD pid, FARPROC loaderAddress, LPVOID remoteMemory) {
    HandleGuard threadSnap(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
    if (threadSnap.get() == INVALID_HANDLE_VALUE) return 0;

    THREADENTRY32 threadEntry = { sizeof(threadEntry) };
    if (Thread32First(threadSnap, &threadEntry)) {
        do {
            if (threadEntry.th32OwnerProcessID == pid) {
                HandleGuard hThread(OpenThread(THREAD_SET_CONTEXT, FALSE, threadEntry.th32ThreadID));
                if (hThread.get())
                    return QueueUserAPC((PAPCFUNC)loaderAddress, hThread, (ULONG_PTR)remoteMemory) ? 1 : 0;
                break;
            }
        } while (Thread32Next(threadSnap, &threadEntry));
    }
    return 0;
}

int injectDll(DWORD pid, const wchar_t* dllPath) {
    if (!dllPath || !*dllPath) return 0;

    SIZE_T sizeBytes = (wcslen(dllPath) + 1) * sizeof(wchar_t);

    HandleGuard process(OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, pid));
    if (!process.get()) return 0;

    LPVOID remoteMemory = VirtualAllocEx(process, nullptr, sizeBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMemory) return 0;

    SIZE_T bytesWritten = 0;
    if (!WriteProcessMemory(process, remoteMemory, dllPath, sizeBytes, &bytesWritten)) {
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        return 0;
    }

    FARPROC loaderAddress = getRemoteLoadLibraryAddress(process);
    if (!loaderAddress) {
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        return 0;
    }

    int result = queueUserApc(pid, loaderAddress, remoteMemory);
    if (!result) {
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        return 0;
    }
    return 1;
}