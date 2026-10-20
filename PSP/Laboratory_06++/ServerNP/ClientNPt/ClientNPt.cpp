#include "../PipeCommon.h"

using namespace std;

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();
    HANDLE client = INVALID_HANDLE_VALUE;


    try
    {
        const wstring pipeName = GetPipeName(argc, argv);
        cout << "TransactNamedPipe: " << Utf8(pipeName) << endl;
        client = OpenPipe(pipeName);
        SetMessageMode(client);


        char request[] = "Привет от ClientNPt (Transact)!";
        char reply[PIPE_BUFFER_SIZE + 1] = {};
        DWORD bytesRead = 0;
        if (!TransactNamedPipe(client, request, sizeof(request), reply, PIPE_BUFFER_SIZE, &bytesRead, nullptr))
        {
            throw SetErrorMessageText("Ошибка TransactNamedPipe: ", GetLastError());
        }
        reply[bytesRead] = '\0';
        cout << "Получено от сервера: " << reply << endl;


        CloseHandle(client);
        client = INVALID_HANDLE_VALUE;
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
