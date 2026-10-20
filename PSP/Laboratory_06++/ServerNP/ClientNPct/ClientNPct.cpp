#include "../PipeCommon.h"

using namespace std;

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();
    try
    {
        const wstring pipeName = GetPipeName(argc, argv);
        cout << "CallNamedPipe: " << Utf8(pipeName) << endl;


        char request[] = "Привет от ClientNPct (Call)!";
        char reply[PIPE_BUFFER_SIZE + 1] = {};
        DWORD bytesRead = 0;
        if (!CallNamedPipeW(pipeName.c_str(), request, sizeof(request), reply, PIPE_BUFFER_SIZE, &bytesRead, PIPE_WAIT_MS))
        {
            throw SetErrorMessageText("Ошибка CallNamedPipe: ", GetLastError());
        }
        reply[bytesRead] = '\0';


        cout << "Успешно! Получено от сервера: " << reply << endl;
        return 0;
    }
    catch (const string& error)
    {
        cerr << error << endl;
        return 1;
    }
}
