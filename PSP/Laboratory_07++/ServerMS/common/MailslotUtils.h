#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const char CLIENT_MESSAGE[] = "Hello from Maislot-client";
const DWORD DEFAULT_MESSAGE_SIZE = 300;
const DWORD BENCH_MESSAGES_COUNT = 1000;


inline string SetErrorMessageText(string message_text, DWORD error_code)
{
    string error_message = message_text + to_string(error_code) + ": ";
    switch (error_code)
    {
    case ERROR_FILE_NOT_FOUND:
    {
        error_message += "Почтовый ящик не найден. Сначала запустите сервер.";
        break;
    }
    case ERROR_ACCESS_DENIED:
    {
        error_message += "Доступ запрещен. Проверьте права и параметры безопасности.";
        break;
    }
    case ERROR_ALREADY_EXISTS:
    case ERROR_SHARING_VIOLATION:
    {
        error_message += "Имя уже занято другим экземпляром сервера.";
        break;
    }
    case ERROR_INVALID_NAME:
    {
        error_message += "Неверный формат имени Mailslot.";
        break;
    }
    case ERROR_INVALID_PARAMETER:
    {
        error_message += "Неверный параметр или сообщение превышает лимит сервера.";
        break;
    }
    case ERROR_INSUFFICIENT_BUFFER:
    case ERROR_MORE_DATA:
    {
        error_message += "Недостаточный размер буфера для сообщения.";
        break;
    }
    case ERROR_SEM_TIMEOUT:
    {
        error_message += "Истек интервал ожидания сообщения.";
        break;
    }
    case ERROR_BAD_NETPATH:
    case ERROR_BAD_NET_NAME:
    {
        error_message += "Сетевой путь недоступен. Проверьте имя ПК, поддержку удаленных Mailslots и размер сообщения.";
        break;
    }
    case ERROR_NETWORK_UNREACHABLE:
    case ERROR_HOST_UNREACHABLE:
    {
        error_message += "Удаленный компьютер недоступен.";
        break;
    }
    case ERROR_NOT_SUPPORTED:
    {
        error_message += "Операция не поддерживается данной конфигурацией Windows.";
        break;
    }
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
    {
        error_message += "Недостаточно оперативной памяти.";
        break;
    }
    case ERROR_INVALID_HANDLE:
    {
        error_message += "Недопустимый дескриптор почтового ящика.";
        break;
    }
    default:
    {
        wchar_t system_message[1024]{};
        DWORD length = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_code, 0, system_message, 1024, NULL);
        if (length > 0)
        {
            int bytes_count = WideCharToMultiByte(CP_UTF8, 0, system_message,
                static_cast<int>(length), NULL, 0, NULL, NULL);
            string description(bytes_count, '\0');
            WideCharToMultiByte(CP_UTF8, 0, system_message, static_cast<int>(length),
                description.data(), bytes_count, NULL, NULL);
            error_message += description;
        }
        else
        {
            error_message += "Неизвестная ошибка Windows.";
        }
        break;
    }
    }
    return error_message;
}


inline void CloseMailslot(HANDLE& mailslot_handle)
{
    HANDLE saved_handle = mailslot_handle;
    mailslot_handle = INVALID_HANDLE_VALUE;
    if (saved_handle != INVALID_HANDLE_VALUE && !CloseHandle(saved_handle))
    {
        DWORD error_code = GetLastError();
        throw SetErrorMessageText("Ошибка CloseHandle: ", error_code);
    }
}


inline DWORD ReadNumber(string text, DWORD minimum, DWORD maximum)
{
    if (text.empty() || text.find_first_not_of("0123456789") != string::npos)
    {
        throw string("Ожидалось целое неотрицательное число: ") + text;
    }
    try
    {
        unsigned long long value = stoull(text);
        if (value < minimum || value > maximum)
        {
            throw string("Число вне допустимого диапазона: ") + text;
        }
        return static_cast<DWORD>(value);
    }
    catch (const exception&)
    {
        throw string("Слишком большое число: ") + text;
    }
}


inline string MakeLocalName(string slot_name)
{
    if (slot_name.empty() || slot_name.find_first_of("\\/,:*") != string::npos)
    {
        throw string("Укажите короткое имя ящика без пути, например Box или Box1.");
    }
    return "\\\\.\\mailslot\\" + slot_name;
}


inline string MakeClientName(string target, string slot_name)
{
    if (target.rfind("\\\\", 0) == 0)
    {
        size_t host_end = target.find('\\', 2);
        if (host_end == string::npos || host_end == 2 ||
            target.compare(host_end, 10, "\\mailslot\\") != 0 || target.size() <= host_end + 10)
        {
            throw string("Неверный UNC-путь Mailslot: ") + target;
        }
        return target;
    }
    if (target.empty() || target.find_first_of("\\/ ,:") != string::npos)
    {
        throw string("Неверное имя компьютера: ") + target;
    }
    MakeLocalName(slot_name);
    return "\\\\" + target + "\\mailslot\\" + slot_name;
}


inline vector<string> GetTargets(string target_list, string slot_name)
{
    vector<string> targets;
    size_t start = 0;
    while (true)
    {
        size_t separator = target_list.find(',', start);
        string target = target_list.substr(start, separator == string::npos ? string::npos : separator - start);
        string mailslot_name = MakeClientName(target, slot_name);
        for (const string& previous : targets)
        {
            if (_stricmp(previous.c_str(), mailslot_name.c_str()) == 0)
            {
                throw string("Повтор адреса сервера: ") + mailslot_name;
            }
        }
        targets.push_back(mailslot_name);
        if (separator == string::npos)
        {
            break;
        }
        start = separator + 1;
    }
    if (targets.size() > 32)
    {
        throw string("За один запуск допускается не более 32 адресов.");
    }
    return targets;
}


inline string MakeBenchMessage(string run_id, DWORD message_number, DWORD messages_count, DWORD message_size)
{
    string message = "MSBENCH|" + run_id + "|" + to_string(message_number) +
        "|" + to_string(messages_count) + "|";
    if (message.size() > message_size)
    {
        throw string("Размер сообщения недостаточен для заголовка.");
    }
    message.resize(message_size, 'x');
    return message;
}