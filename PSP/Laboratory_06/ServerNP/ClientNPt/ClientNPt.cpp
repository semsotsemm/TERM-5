#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

string SetErrorMessageText(string message_text, int error_code)
{
    string error_message = message_text;
    error_message += to_string(error_code) + ": ";

    switch (error_code)
    {
    case ERROR_FILE_NOT_FOUND:
        error_message += "Файл (или канал) не найден.\n";
        break;
    case ERROR_PATH_NOT_FOUND:
        error_message += "Путь не найден.\n";
        break;
    case ERROR_ACCESS_DENIED:
        error_message += "Доступ запрещен. Недостаточно прав.\n";
        break;
    case ERROR_INVALID_HANDLE:
        error_message += "Неверный дескриптор канала.\n";
        break;
    case ERROR_OUTOFMEMORY:
        error_message += "Недостаточно памяти для выполнения операции.\n";
        break;
    case ERROR_INVALID_PARAMETER:
        error_message += "Передан неверный параметр в функцию API.\n";
        break;
    case ERROR_BROKEN_PIPE:
        error_message += "Канал был разорван (клиент отключился).\n";
        break;
    case ERROR_PIPE_BUSY:
        error_message += "Все экземпляры канала заняты.\n";
        break;
    case ERROR_NO_DATA:
        error_message += "Канал закрывается, нет данных.\n";
        break;
    case ERROR_PIPE_NOT_CONNECTED:
        error_message += "Нет процесса на другом конце канала.\n";
        break;
    case ERROR_PIPE_CONNECTED:
        error_message += "Процесс (клиент) уже подключен к каналу.\n";
        break;
    case ERROR_PIPE_LISTENING:
        error_message += "Ожидание подключения к каналу.\n";
        break;
    case ERROR_OPERATION_ABORTED:
        error_message += "Операция отменена.\n";
        break;
    default:
        error_message += "Неизвестная системная ошибка.\n";
        break;
    }

    return error_message;
}

int main()
{
    setlocale(LC_CTYPE, "Russian");
    HANDLE client_handle = INVALID_HANDLE_VALUE;

    try
    {
        DWORD share_mode = 0;
        LPSECURITY_ATTRIBUTES security_attributes = NULL;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = NULL;
        LPCWSTR pipe_name = L"\\\\.\\pipe\\Tube";
        
        client_handle = CreateFile(
            pipe_name,
            GENERIC_READ | GENERIC_WRITE,
            share_mode,
            security_attributes,
            OPEN_EXISTING,
            flags_and_attributes,
            template_file
        );
        if (client_handle == INVALID_HANDLE_VALUE)
        {
            throw SetErrorMessageText("Ошибка CreateFile: ", GetLastError());
        }

        DWORD read_mode = PIPE_READMODE_MESSAGE;
        LPDWORD max_collection_count = NULL;
        LPDWORD collect_data_timeout = NULL;
        
        if (!SetNamedPipeHandleState(client_handle, &read_mode, max_collection_count, collect_data_timeout))
        {
            throw SetErrorMessageText("Ошибка SetNamedPipeHandleState: ", GetLastError());
        }

        char writeBuf[] = "Привет от ClientNPt (Transact)!";
        char readBuf[512];
        DWORD bytesRead;

        if (!TransactNamedPipe(client_handle, writeBuf, sizeof(writeBuf), readBuf, sizeof(readBuf), &bytesRead, NULL))
        {
            throw SetErrorMessageText("Ошибка TransactNamedPipe: ", GetLastError());
        }

        readBuf[bytesRead] = '\0';
        cout << "Получено от сервера: " << readBuf << endl;

        CloseHandle(client_handle);
    }
    catch (string error_message)
    {
        cerr << error_message << endl;
        if (client_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(client_handle);
        }
    }
    return 0;
}