#include <iostream>
#include <thread>
#include <string>

#include <Windows.h>
#include <Psapi.h>

#include "proc_thread_list.h"

using namespace std;

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string get_last_error_as_string()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

void caret_pos_watcher() {
    ::GUITHREADINFO gui_thread_info;
    gui_thread_info.cbSize = sizeof(::GUITHREADINFO);
    string last_fg_window_title;
    string current_fg_window_title;

    while (true) {
        ::HWND foreground_wnd_handle = ::GetForegroundWindow();
        DWORD fg_window_thread_id = NULL;
        ::DWORD fg_wnd_pid = 0;
        if (foreground_wnd_handle != NULL) {
            fg_window_thread_id = ::GetWindowThreadProcessId(foreground_wnd_handle, &fg_wnd_pid);
            HANDLE fg_process_handle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                                     FALSE,
                                                     fg_wnd_pid);
            CHAR base_name_chars[255];
            size_t proc_name_length = ::GetModuleBaseName(fg_process_handle, NULL, base_name_chars, 255);

            if (proc_name_length != 0) {
                current_fg_window_title = string(base_name_chars, proc_name_length);
            }
        } else {
            std::cout << "Ooops, loose foreground window handle... :(" << std::endl;
        }

        if (current_fg_window_title != last_fg_window_title) {
            std::cout << "WOW! Received new foreground window title: " << current_fg_window_title << std::endl;
        }

        last_fg_window_title = current_fg_window_title;

        auto proc_threads = ListProcessThreads(fg_wnd_pid);

//        for (auto proc_thread : proc_threads) {
            ::BOOL get_gui_thread_info_succeeded = ::GetGUIThreadInfo(fg_window_thread_id, &gui_thread_info);
            if (!get_gui_thread_info_succeeded) {
                std::cerr << "Failed to get GUI thread info: " << get_last_error_as_string() << std::endl;
            } else {
                std::cout << "Caret rect: " << gui_thread_info.rcCaret.left << std::endl;
                AttachThreadInput(GetCurrentThreadId(), fg_window_thread_id, TRUE);
                POINT caret_pos;
                memset(&caret_pos, 0, sizeof(POINT));
                GetCaretPos(&caret_pos);
                std::cout << "Caret pos: " << caret_pos.x << std::endl;
                AttachThreadInput(GetCurrentThreadId(), fg_window_thread_id, FALSE);
            }
//        }


        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

int main(int argc, char *argv[])
{
    cout << "Starting caret position watcher..." << endl;

    std::thread cr_pos_watcher_thread(&caret_pos_watcher);
    cr_pos_watcher_thread.join();

    return 0;
}
