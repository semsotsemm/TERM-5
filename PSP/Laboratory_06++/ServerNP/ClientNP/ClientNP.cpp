#include "../PipeCommon.h"

using namespace std;

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();
    HANDLE client = INVALID_HANDLE_VALUE;
    try
    {
        const wstring pipeName = GetPipeName(argc, argv);
        cout << "Подключение к " << Utf8(pipeName) << endl;
        client = OpenPipe(pipeName);
        SetMessageMode(client);
        cout << "Успешное подключение к серверу." << endl;


        const char request[] = "Привет от ClientNP!";
        for (int i = 0; i < 3; ++i)
        {
            DWORD written = 0;
            if (!WriteFile(client, request, sizeof(request), &written, nullptr))
            {
                throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
            }
            if (written != sizeof(request))
            {
                throw string("Запрос записан не полностью.\n");
            }


            string reply;
            if (!ReadPipeMessage(client, reply))
            {
                throw SetErrorMessageText("Сервер отключился: ", ERROR_BROKEN_PIPE);
            }
            cout << "Получено от сервера: " << reply << endl;
            if (i != 2)
            {
                Sleep(1000);
            }
        }


        CloseHandle(client);
        client = INVALID_HANDLE_VALUE;
        cout << "Клиент завершает работу." << endl;
        return 0;
    }
    catch (const string& error)
    {
        cerr << error << endl;
        if (client != INVALID_HANDLE_VALUE)
        {
            CloseHandle(client);
        }
        return 1;
    }
}
