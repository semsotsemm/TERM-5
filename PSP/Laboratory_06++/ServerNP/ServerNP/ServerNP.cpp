#include "../PipeCommon.h"
#include <sddl.h>
#pragma comment(lib, "Advapi32.lib")

using namespace std;

int main()
{
    InitConsole();
    HANDLE server = INVALID_HANDLE_VALUE;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    try
    {
        // Список управления доступом, Allow, Generic All, System, Build Administator, Generic Read Generic Write, Authenticate Users.
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:(A;;GA;;;SY)(A;;GA;;;BA)(A;;GRGW;;;AU)", SDDL_REVISION_1, &descriptor, nullptr))
        {
            throw SetErrorMessageText("Ошибка создания DACL: ", GetLastError());
        }
        SECURITY_ATTRIBUTES security = { sizeof(SECURITY_ATTRIBUTES), descriptor, FALSE };


        DWORD openMode = PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE;
        DWORD pipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS;
        server = CreateNamedPipeW(L"\\\\.\\pipe\\Tube", openMode, pipeMode, 1, PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE, PIPE_WAIT_MS, &security);
        const DWORD createError = server == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;


        LocalFree(descriptor);
        descriptor = nullptr;
        if (server == INVALID_HANDLE_VALUE)
        {
            throw SetErrorMessageText("Ошибка CreateNamedPipe: ", createError);
        }


        char computer[MAX_COMPUTERNAME_LENGTH + 1] = {};
        DWORD computerSize = sizeof(computer);
        GetComputerNameA(computer, &computerSize);
        cout << "Tube создан. Сервер: " << computer << "\nКлиент:" << computer << "\n";
        
        
        while (true)
        {
            cout << "Ожидание клиента..." << endl;
            const BOOL connected = ConnectNamedPipe(server, nullptr);
            if (!connected && GetLastError() != ERROR_PIPE_CONNECTED)
            {
                throw SetErrorMessageText("Ошибка ConnectNamedPipe: ", GetLastError());
            }
            cout << "Клиент подключился." << endl;
            
            
            try
            {
                string request;
                while (ReadPipeMessage(server, request))
                {
                    cout << "Получено от клиента: " << request << endl;
                    const char reply[] = "Сообщение получено!";
                    DWORD written = 0;
                    if (!WriteFile(server, reply, sizeof(reply), &written, nullptr))
                    {
                        throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
                    }
                    if (written != sizeof(reply))
                    {
                        throw string("Ответ записан не полностью.\n");
                    }
                }
            }
            catch (const string& error)
            {
                cerr << error << endl;
            }
            if (!DisconnectNamedPipe(server))
            {
                throw SetErrorMessageText("Ошибка DisconnectNamedPipe: ", GetLastError());
            }
            cout << "Клиент отключился." << endl;
        }
    }
    catch (const string& error)
    {
        cerr << error << endl;
        if (descriptor)
        {
            LocalFree(descriptor);
        }
        if (server != INVALID_HANDLE_VALUE)
        {
            CloseHandle(server);
        }
        return 1;
    }
}
