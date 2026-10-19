#include "../PipeCommon.h"

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();
    HANDLE client = INVALID_HANDLE_VALUE;
    try
    {
        const std::wstring pipeName = GetPipeName(argc, argv);
        std::cout << "Подключение к " << Utf8(pipeName) << std::endl;
        client = OpenPipe(pipeName);
        SetMessageMode(client);
        std::cout << "Успешное подключение к серверу." << std::endl;
        const char request[] = "Привет от ClientNP!";
        for (int i = 0; i < 3; ++i)
        {
            DWORD written = 0;
            if (!WriteFile(client, request, sizeof(request), &written, nullptr))
                throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
            if (written != sizeof(request))
                throw std::string("Запрос записан не полностью.\n");
            std::string reply;
            if (!ReadPipeMessage(client, reply))
                throw SetErrorMessageText("Сервер отключился: ", ERROR_BROKEN_PIPE);
            std::cout << "Получено от сервера: " << reply << std::endl;
            if (i != 2) Sleep(1000);
        }
        CloseHandle(client);
        client = INVALID_HANDLE_VALUE;
        std::cout << "Клиент завершает работу." << std::endl;
        return 0;
    }
    catch (const std::string& error)
    {
        std::cerr << error << std::endl;
        if (client != INVALID_HANDLE_VALUE) CloseHandle(client);
        return 1;
    }
}
