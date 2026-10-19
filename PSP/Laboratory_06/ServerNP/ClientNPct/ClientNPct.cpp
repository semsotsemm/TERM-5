#include "../PipeCommon.h"

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();
    try
    {
        const std::wstring pipeName = GetPipeName(argc, argv);
        std::cout << "CallNamedPipe: " << Utf8(pipeName) << std::endl;
        char request[] = "Привет от ClientNPct (Call)!";
        char reply[PIPE_BUFFER_SIZE + 1] = {};
        DWORD bytesRead = 0;
        // Конечное ожидание занятого экземпляра вместо NMPWAIT_WAIT_FOREVER.
        if (!CallNamedPipeW(pipeName.c_str(), request, sizeof(request),
            reply, PIPE_BUFFER_SIZE, &bytesRead, PIPE_WAIT_MS))
            throw SetErrorMessageText("Ошибка CallNamedPipe: ", GetLastError());
        reply[bytesRead] = '\0';
        std::cout << "Успешно! Получено от сервера: " << reply << std::endl;
        return 0;
    }
    catch (const std::string& error)
    {
        std::cerr << error << std::endl;
        return 1;
    }
}
