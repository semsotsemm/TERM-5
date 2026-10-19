#include "../PipeCommon.h"
#include <sddl.h>
#pragma comment(lib, "Advapi32.lib")

int main()
{
    InitConsole();
    HANDLE server = INVALID_HANDLE_VALUE;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    try
    {
        // NULL давал другим пользователям лишь чтение. AU = пользователи,
        // прошедшие вход Windows: обмен разрешен, анонимный/гостевой вход — нет.
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            L"D:(A;;GA;;;SY)(A;;GA;;;BA)(A;;GRGW;;;AU)", SDDL_REVISION_1,
            &descriptor, nullptr))
            throw SetErrorMessageText("Ошибка создания DACL: ", GetLastError());
        SECURITY_ATTRIBUTES security = { sizeof(SECURITY_ATTRIBUTES), descriptor, FALSE };
        server = CreateNamedPipeW(L"\\\\.\\pipe\\Tube",
            PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS,
            1, PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE, PIPE_WAIT_MS, &security);
        const DWORD createError = server == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;
        LocalFree(descriptor);
        descriptor = nullptr;
        if (server == INVALID_HANDLE_VALUE)
            throw SetErrorMessageText("Ошибка CreateNamedPipe: ", createError);

        char computer[MAX_COMPUTERNAME_LENGTH + 1] = {};
        DWORD computerSize = sizeof(computer);
        GetComputerNameA(computer, &computerSize);
        std::cout << "Канал Tube создан. Сервер: " << computer
            << "\nКлиент на другом ПК: ClientNP.exe " << computer << "\n";
        // Один экземпляр обслуживает клиентов по очереди, без перезапуска сервера.
        while (true)
        {
            std::cout << "Ожидание клиента... (Ctrl+C — завершить)" << std::endl;
            const BOOL connected = ConnectNamedPipe(server, nullptr);
            if (!connected && GetLastError() != ERROR_PIPE_CONNECTED)
                throw SetErrorMessageText("Ошибка ConnectNamedPipe: ", GetLastError());
            std::cout << "Клиент подключился." << std::endl;
            try
            {
                std::string request;
                while (ReadPipeMessage(server, request))
                {
                    std::cout << "Получено от клиента: " << request << std::endl;
                    const char reply[] = "Сообщение получено!";
                    DWORD written = 0;
                    if (!WriteFile(server, reply, sizeof(reply), &written, nullptr))
                        throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
                    if (written != sizeof(reply))
                        throw std::string("Ответ записан не полностью.\n");
                }
            }
            catch (const std::string& error)
            {
                std::cerr << error << std::endl;
            }
            // Ответы уже прочитаны: клиенты закрывают канал после получения ответа.
            if (!DisconnectNamedPipe(server))
                throw SetErrorMessageText("Ошибка DisconnectNamedPipe: ", GetLastError());
            std::cout << "Клиент отключился." << std::endl;
        }
    }
    catch (const std::string& error)
    {
        std::cerr << error << std::endl;
        if (descriptor) LocalFree(descriptor);
        if (server != INVALID_HANDLE_VALUE) CloseHandle(server);
        return 1;
    }
}
