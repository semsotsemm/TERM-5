#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include "../common/MailslotUtils.h"

using namespace std;


int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    vector<HANDLE> mailslot_handles;

    try
    {
        if (argc > 6)
        {
            throw string("Использование: ClientMS [.|имя_ПК] [count] [bytes] [Box] [pause_ms]");
        }
        string target = argc > 1 ? argv[1] : ".";
        DWORD messages_count = argc > 2 ? ReadNumber(argv[2], 1, 1000000) : 1;
        DWORD message_size = argc > 3 ? ReadNumber(argv[3], 0, 1048576) : 0;
        string slot_name = argc > 4 ? argv[4] : "Box";
        DWORD pause_ms = argc > 5 ? ReadNumber(argv[5], 0, 60000) : 0;
        vector<string> mailslot_names = GetTargets(target, slot_name);
        string run_id = to_string(GetCurrentProcessId()) + "-" + to_string(GetTickCount64());

        if (message_size == 0)
        {
            message_size = messages_count == 1 ? static_cast<DWORD>(string(CLIENT_MESSAGE).size()) : 64;
        }
        if (messages_count > 1)
        {
            MakeBenchMessage(run_id, messages_count, messages_count, message_size);
        }
        mailslot_handles.reserve(mailslot_names.size());
        for (const string& mailslot_name : mailslot_names)
        {
            cout << "Открываем ящик: " << mailslot_name << endl;
            bool local_target = mailslot_name.rfind("\\\\.\\mailslot\\", 0) == 0;
            if (!local_target && message_size > 424)
            {
                cout << "Предупреждение: удаленные Mailslots ограничены 424 байтами. "
                    "Отправка большого сообщения может завершиться ошибкой.\n";
            }

            HANDLE mailslot_handle = CreateFileA(mailslot_name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (mailslot_handle == INVALID_HANDLE_VALUE)
            {
                DWORD error_code = GetLastError();
                throw SetErrorMessageText("Ошибка CreateFile (" + mailslot_name + "): ", error_code);
            }
            mailslot_handles.push_back(mailslot_handle);
        }

        cout << "MESSAGES_PER_TARGET=" << messages_count << " MESSAGE_SIZE=" << message_size << " TARGETS=" << mailslot_handles.size() << endl;
        if (messages_count > 1)
        {
            cout << "RUN=" << run_id << endl;
        }
        LARGE_INTEGER frequency{}, start_time{}, end_time{};
        if (!QueryPerformanceFrequency(&frequency))
        {
            throw SetErrorMessageText("Ошибка QueryPerformanceFrequency: ", GetLastError());
        }
        unsigned long long writes_count = 0;
        unsigned long long sent_bytes = 0;
        clock_t start_clock = clock();
        QueryPerformanceCounter(&start_time);

        for (DWORD i = 1; i <= messages_count; i++)
        {
            string message = messages_count == 1 ? CLIENT_MESSAGE :
                MakeBenchMessage(run_id, i, messages_count, message_size);
            message.resize(message_size, 'x');
            for (size_t j = 0; j < mailslot_handles.size(); j++)
            {
                DWORD bytes_written = 0;
                if (!WriteFile(mailslot_handles[j], message.data(), static_cast<DWORD>(message.size()),
                    &bytes_written, NULL))
                {
                    DWORD error_code = GetLastError();
                    cerr << "До ошибки выполнено WriteFile: " << writes_count << endl;
                    throw SetErrorMessageText("Ошибка WriteFile (" + mailslot_names[j] + "): ", error_code);
                }
                if (bytes_written != message.size())
                {
                    throw string("WriteFile записала не все байты сообщения.");
                }
                writes_count++;
                sent_bytes += bytes_written;
            }
            if (pause_ms > 0 && i < messages_count)
            {
                Sleep(pause_ms);
            }
        }
        QueryPerformanceCounter(&end_time);
        clock_t end_clock = clock();
        double total_time = static_cast<double>(end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
        double clock_time = static_cast<double>(end_clock - start_clock) / CLOCKS_PER_SEC;

        cout << fixed << setprecision(9);
        cout << "WRITE_COUNT=" << writes_count << " BYTES_WRITTEN=" << sent_bytes << endl;
        cout << "SEND_SECONDS=" << total_time << " CLOCK_SECONDS=" << clock_time << endl;
        if (total_time > 0.0)
        {
            cout << "WRITES_PER_SECOND=" << writes_count / total_time << endl;
            cout << "BYTES_PER_SECOND=" << sent_bytes / total_time << endl;
            cout << "AVERAGE_WRITE_SECONDS=" << total_time / writes_count << endl;
        }

        for (HANDLE& mailslot_handle : mailslot_handles)
        {
            CloseMailslot(mailslot_handle);
        }
    }
    catch (const string& error_message)
    {
        cerr << error_message << endl;
        for (HANDLE mailslot_handle : mailslot_handles)
        {
            if (mailslot_handle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(mailslot_handle);
            }
        }
        return 1;
    }
    catch (const exception& error)
    {
        cerr << "Ошибка программы: " << error.what() << endl;
        for (HANDLE mailslot_handle : mailslot_handles)
        {
            if (mailslot_handle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(mailslot_handle);
            }
        }
        return 1;
    }
    return 0;
}
