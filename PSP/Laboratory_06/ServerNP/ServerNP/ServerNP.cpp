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
    HANDLE server_handle = INVALID_HANDLE_VALUE;
    
    try 
    {
        setlocale(LC_CTYPE, "Russian");

        LPCWSTR pipe_name = L"\\\\.\\pipe\\Tube";
        int exemplar_count = 1;
        int input_buffer_size = 512, output_buffer_size = 512;
        int waiting_time = 0;
        
        cout << "Создание канала...\n";
        server_handle = CreateNamedPipe(
            pipe_name,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            exemplar_count,
            output_buffer_size,
            input_buffer_size,
            waiting_time,
            NULL
        );

        if (server_handle == INVALID_HANDLE_VALUE) 
        {
            throw(SetErrorMessageText("Ошибка CreateNamedPipe: ", GetLastError()));
        }
        cout << "Именнованый калан успешно создан.\nОжидание клиента...";

        bool is_connected = ConnectNamedPipe(server_handle, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!is_connected) 
        {
            throw(SetErrorMessageText("Ошибка CreateNamedPipe: ", GetLastError()));
        }



        cout << "Клиент успешно подключился.\n";
        char readBuf[512];
        char writeBuf[] = "Сообщение получено!";
        DWORD bytesRead, bytesWritten;

        while (true)
        {
            if (!ReadFile(server_handle, readBuf, sizeof(readBuf), &bytesRead, NULL))
            {
                // Клиент отключился самостоятельно
                if (GetLastError() == ERROR_BROKEN_PIPE)
                {
                    break;
                }
                throw SetErrorMessageText("Ошибка ReadFile: ", GetLastError());
            }
            readBuf[bytesRead] = '\0';
            cout << "Получено от клиента: " << readBuf << endl;

            if (!WriteFile(server_handle, writeBuf, sizeof(writeBuf), &bytesWritten, NULL)) 
            {
                throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
            }
        }



        if (!DisconnectNamedPipe(server_handle))
        {
            throw SetErrorMessageText("Ошибка DisconnectNamedPipe: ", GetLastError());
        }
        if (!CloseHandle(server_handle))
        {
            throw SetErrorMessageText("Ошибка CloseHandle: ", GetLastError());
        }
        cout << "Соединение разорвано, дескриптор канала закрыт.\n";

    }
    catch (string error_message) 
    {
        cerr << error_message << "\n";

        if (server_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(server_handle);
        }
        return 1;
    }   

    system("pause");
    return 0;
}