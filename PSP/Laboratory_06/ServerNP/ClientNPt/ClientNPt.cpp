#include "../PipeCommon.h"

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();
    HANDLE client = INVALID_HANDLE_VALUE;
    try
    {
        const std::wstring pipeName = GetPipeName(argc, argv);
        std::cout << "TransactNamedPipe: " << Utf8(pipeName) << std::endl;
        client = OpenPipe(pipeName);
        SetMessageMode(client);
        char request[] = "Привет от ClientNPt (Transact)!";
        char reply[PIPE_BUFFER_SIZE + 1] = {};
        DWORD bytesRead = 0;
        if (!TransactNamedPipe(client, request, sizeof(request),
            reply, PIPE_BUFFER_SIZE, &bytesRead, nullptr))
            throw SetErrorMessageText("Ошибка TransactNamedPipe: ", GetLastError());
        reply[bytesRead] = '\0';
        std::cout << "Получено от сервера: " << reply << std::endl;
        CloseHandle(client);
        client = INVALID_HANDLE_VALUE;
        return 0;
    }
    catch (const std::string& error)
    {
        std::cerr << error << std::endl;
        if (client != INVALID_HANDLE_VALUE) CloseHandle(client);
        return 1;
    }
}
