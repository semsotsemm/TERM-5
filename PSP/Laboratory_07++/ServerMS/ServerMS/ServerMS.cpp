#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include "../common/MailslotUtils.h"

using namespace std;


// Читаем именно байты сообщения. Завершающий '\0' от клиента не требуется.
bool ReadMessage(HANDLE mailslot_handle, vector<char>& receive_buffer, string& message)
{
    DWORD bytes_read = 0;
    if (!ReadFile(mailslot_handle, receive_buffer.data(),
        static_cast<DWORD>(receive_buffer.size()), &bytes_read, NULL))
    {
        DWORD error_code = GetLastError();
        if (error_code == ERROR_SEM_TIMEOUT)
        {
            return false;
        }
        throw SetErrorMessageText("Ошибка ReadFile: ", error_code);
    }
    message.assign(receive_buffer.data(), bytes_read);
    return true;
}


// Отделяем нумерацию от полезной нагрузки, чтобы учитывать потери и повторы.
bool GetBenchHeader(const string& message, string& run_id, DWORD& message_number, DWORD& messages_count)
{
    if (message.rfind("MSBENCH|", 0) != 0)
    {
        return false;
    }
    istringstream stream(message);
    string field, number_text, count_text;
    getline(stream, field, '|');
    if (!getline(stream, run_id, '|') || run_id.empty() ||
        !getline(stream, number_text, '|') || !getline(stream, count_text, '|'))
    {
        return false;
    }
    try
    {
        messages_count = ReadNumber(count_text, 1, 1000000);
        message_number = ReadNumber(number_text, 1, messages_count);
    }
    catch (const string&)
    {
        return false;
    }
    return true;
}


// Это интервал приема от первого до последнего сообщения, а не RTT.
bool ReceiveBench(HANDLE mailslot_handle, DWORD maximum_size, DWORD read_timeout)
{
    vector<char> receive_buffer(maximum_size);
    vector<bool> received(BENCH_MESSAGES_COUNT + 1, false);
    string active_run;
    DWORD received_count = 0;
    DWORD duplicate_count = 0;
    DWORD foreign_count = 0;
    DWORD out_of_order_count = 0;
    DWORD last_number = 0;
    unsigned long long received_bytes = 0;
    LARGE_INTEGER frequency{}, first_time{}, last_time{};
    if (!QueryPerformanceFrequency(&frequency))
    {
        throw SetErrorMessageText("Ошибка QueryPerformanceFrequency: ", GetLastError());
    }

    cout << "Ожидание серии из " << BENCH_MESSAGES_COUNT << " сообщений.\n" << flush;
    ULONGLONG deadline = GetTickCount64() + read_timeout;
    while (received_count < BENCH_MESSAGES_COUNT)
    {
        // Ограничиваем общее ожидание: посторонние сообщения не продлевают его.
        ULONGLONG now = GetTickCount64();
        if (now >= deadline)
        {
            break;
        }
        DWORD remaining_timeout = static_cast<DWORD>(deadline - now);
        if (!SetMailslotInfo(mailslot_handle, remaining_timeout))
        {
            throw SetErrorMessageText("Ошибка SetMailslotInfo: ", GetLastError());
        }
        string message;
        if (!ReadMessage(mailslot_handle, receive_buffer, message))
        {
            break;
        }
        string run_id;
        DWORD message_number = 0, messages_count = 0;
        if (!GetBenchHeader(message, run_id, message_number, messages_count) ||
            messages_count != BENCH_MESSAGES_COUNT || (!active_run.empty() && active_run != run_id))
        {
            foreign_count++;
            continue;
        }
        if (active_run.empty())
        {
            active_run = run_id;
            QueryPerformanceCounter(&first_time);
            deadline = GetTickCount64() + read_timeout;
        }
        if (received[message_number])
        {
            duplicate_count++;
            continue;
        }
        received[message_number] = true;
        received_count++;
        received_bytes += message.size();
        if (message_number < last_number)
        {
            out_of_order_count++;
        }
        last_number = message_number;
        QueryPerformanceCounter(&last_time);
    }

    double total_time = received_count == 0 ? 0.0 :
        static_cast<double>(last_time.QuadPart - first_time.QuadPart) / frequency.QuadPart;
    bool complete = received_count == BENCH_MESSAGES_COUNT;
    cout << fixed << setprecision(9);
    cout << "RUN=" << active_run << "\nBENCH_RECEIVED=" << received_count
        << " EXPECTED=" << BENCH_MESSAGES_COUNT << " COMPLETE=" << (complete ? 1 : 0) << endl;
    cout << "BYTES_RECEIVED=" << received_bytes << " DUPLICATES=" << duplicate_count
        << " OUT_OF_ORDER=" << out_of_order_count << " FOREIGN=" << foreign_count << endl;
    cout << "RECEIVE_SPAN_SECONDS=" << total_time << endl;
    if (total_time > 0.0 && received_count > 1)
    {
        cout << "RECEIVE_INTERVALS_PER_SECOND=" << (received_count - 1) / total_time << endl;
    }
    if (!complete)
    {
        cout << "Получены не все сообщения. Нельзя выдавать эту серию за передачу 1000 сообщений.\n";
    }
    return complete;
}


int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE mailslot_handle = INVALID_HANDLE_VALUE;

    try
    {
        if (argc > 5)
        {
            throw string("Использование: ServerMS [create|receive|once|timeout|bench] [300|500] [-1|timeout_ms] [Box]");
        }
        string mode = argc > 1 ? argv[1] : "receive";
        if (mode != "create" && mode != "receive" && mode != "once" && mode != "timeout" && mode != "bench")
        {
            throw string("Неизвестный режим сервера: ") + mode;
        }
        DWORD maximum_size = argc > 2 ? ReadNumber(argv[2], 1, 1048576) : DEFAULT_MESSAGE_SIZE;
        DWORD read_timeout = mode == "timeout" ? 180000 :
            mode == "bench" ? 10000 : MAILSLOT_WAIT_FOREVER;
        if (argc > 3)
        {
            read_timeout = string(argv[3]) == "-1" ? MAILSLOT_WAIT_FOREVER : ReadNumber(argv[3], 1, 3600000);
        }
        if (mode == "bench" && read_timeout == MAILSLOT_WAIT_FOREVER)
        {
            throw string("Для bench задайте конечное ожидание, например 10000 мс.");
        }
        string slot_name = argc > 4 ? argv[4] : "Box";
        string mailslot_name = MakeLocalName(slot_name);

        // Блок 1. Сервер всегда создает локальный ящик, даже для сетевых клиентов.
        mailslot_handle = CreateMailslotA(mailslot_name.c_str(), maximum_size, read_timeout, NULL);
        if (mailslot_handle == INVALID_HANDLE_VALUE)
        {
            throw SetErrorMessageText("Ошибка CreateMailslot: ", GetLastError());
        }
        cout << "ServerMS: ящик создан: " << mailslot_name << endl;
        cout << "MAX_MESSAGE_SIZE=" << maximum_size << " TIMEOUT_MS=";
        if (read_timeout == MAILSLOT_WAIT_FOREVER)
        {
            cout << "INFINITE\n";
        }
        else
        {
            cout << read_timeout << "\n";
        }
        cout << flush;

        if (mode == "create")
        {
            // Задание 1: без ReadFile создание и закрытие не блокируют поток.
            CloseMailslot(mailslot_handle);
            cout << "Ящик закрыт. Программа завершилась без ожидания клиента.\n";
            return 0;
        }
        if (mode == "bench")
        {
            bool complete = ReceiveBench(mailslot_handle, maximum_size, read_timeout);
            CloseMailslot(mailslot_handle);
            return complete ? 0 : 2;
        }

        vector<char> receive_buffer(maximum_size);
        DWORD messages_count = 0;
        while (true)
        {
            // Блок 2. При пустом ящике блокируется ReadFile, а не CreateMailslot.
            cout << "Ожидание сообщения (ReadFile)...\n" << flush;
            ULONGLONG start_time = GetTickCount64();
            string message;
            if (!ReadMessage(mailslot_handle, receive_buffer, message))
            {
                double elapsed = (GetTickCount64() - start_time) / 1000.0;
                cout << "Таймаут: сообщение за заданный интервал не поступило.\n";
                cout << "TIMEOUT_ELAPSED_SECONDS=" << elapsed << endl;
                break;
            }
            messages_count++;
            cout << "Получено сообщение #" << messages_count << ": " << message << endl;
            cout << "BYTES_RECEIVED=" << message.size() << endl;
            if (mode == "once" || mode == "timeout")
            {
                break;
            }
        }

        // Блок 3. После закрытия серверного дескриптора ящик перестает существовать.
        CloseMailslot(mailslot_handle);
        cout << "Почтовый ящик закрыт.\n";
    }
    catch (const string& error_message)
    {
        cerr << error_message << endl;
        if (mailslot_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(mailslot_handle);
        }
        return 1;
    }
    catch (const exception& error)
    {
        cerr << "Ошибка программы: " << error.what() << endl;
        if (mailslot_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(mailslot_handle);
        }
        return 1;
    }
    return 0;
}
