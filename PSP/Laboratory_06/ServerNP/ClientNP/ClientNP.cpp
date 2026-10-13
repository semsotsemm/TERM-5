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
        cout << "Подключение к каналу...\n";
        LPCWSTR pipe_name = L"\\\\.\\pipe\\Tube";

        client_handle = CreateFile(
            pipe_name,                     
            GENERIC_READ | GENERIC_WRITE,  
            0,                             
            NULL,                          
            OPEN_EXISTING,                 
            0,                             
            NULL                           
        );

        if (client_handle == INVALID_HANDLE_VALUE)
        {
            throw SetErrorMessageText("Ошибка CreateFile: ", GetLastError());
        }
        cout << "Успешное подключение к серверу по каналу Tube.\n";

        char writeBuf[] = "Привет от ClientNP!";
        char readBuf[512];
        DWORD bytesWritten, bytesRead;

        for (int i = 0; i < 3; i++) 
        {
            if (!WriteFile(client_handle, writeBuf, sizeof(writeBuf), &bytesWritten, NULL))
            {
                throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
            }

            if (!ReadFile(client_handle, readBuf, sizeof(readBuf), &bytesRead, NULL)) 
            {
                throw SetErrorMessageText("Ошибка ReadFile: ", GetLastError());
            }

            readBuf[bytesRead] = '\0';
            cout << "Получено от сервера: " << readBuf << endl;
            Sleep(1000);
        }

        cout << "Закрытие канала...\n";
        if (!CloseHandle(client_handle))
        {
            throw SetErrorMessageText("Ошибка CloseHandle: ", GetLastError());
        }
        cout << "Клиент завершает работу.\n";
    }
    catch (string error_message)
    {
        cerr << error_message << "\n";

        if (client_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(client_handle);
        }
    }

    system("pause");
    return 0;
}