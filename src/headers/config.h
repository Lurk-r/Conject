#pragma once
#define _CRT_SECURE_NO_WARNINGS

constexpr wchar_t PATTERN_PROC[] = L"pixel*gun*.exe";
constexpr wchar_t PATTERN_DLL[] = L"*.dll";
constexpr wchar_t APP_ID[] = L"2524890";

constexpr int DELAY_POST_LAUNCH = 10000;
constexpr int DELAY_POST_INJECT = 5000;
constexpr int DELAY_EXIT = 5000;
constexpr int DELAY_RETRY = 3000;
constexpr int INJECT_DELAY = 500;
constexpr int DELAY_HIDE_CONSOLE_TIMEOUT = 60000;
constexpr int POLL_INTERVAL = 100;
constexpr int POST_HIDE_DELAY = 500;
constexpr int PROCESS_POLL_INTERVAL = 2000;

constexpr size_t CONSOLE_BUFFER_SIZE = 256;
constexpr size_t PROMPT_INPUT_SIZE = 64;
constexpr size_t CLASS_NAME_BUFFER_SIZE = 64;

constexpr wchar_t CONSOLE_WINDOW_CLASS[] = L"ConsoleWindowClass";

#define SEC(ms) (((ms)+999)/1000)

#define MSG_ERROR_NO_DLL     L"ERROR: No DLL found!\n"
#define MSG_ERROR_ELEVATED   L"ERROR: Process may be elevated! Rerun injector as Admin.\n"
#define MSG_ERROR_INJECT     L"ERROR: Inject failed (you may need to rerun injector as Admin)! Retrying\n"
#define MSG_ERROR_CRASHED    L"ERROR: Process crashed!\n"
#define MSG_NO_PROCESS       L"No matching process found. Launching\n"
#define MSG_WAIT_PROCESS     L"Waiting for process\n"
#define MSG_INJECTING_DLL    L"- Injecting "
#define MSG_INJECT_NOTE      L"\n(if injector doesn't close after ~10s then feel free to ignore, go back to process & press keybind)\n\n"
#define MSG_INJECT_IN        L"\rInject in %2d"
#define MSG_EXIT_IN          L"\rExit in %2d"
#define MSG_DLL_PROMPT       L"\nPick [1-%d, space > multi, * > all]: "