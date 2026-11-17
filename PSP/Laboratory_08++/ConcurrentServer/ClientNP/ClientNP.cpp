#include "PipeCommon.h"


void GetInteractionInstruction() 
{
    cout << "Консоль подключена.\n";
    cout << "start      — разрешить подключение новых клиентов\n";
    cout << "stop       — приостановить подключение новых клиентов\n";
    cout << "wait       — дождаться текущих клиентов, затем возобновить приём\n";
    cout << "statistics — статистика подключений и ошибок\n";
    cout << "shutdown   — обслужить текущих клиентов и завершить сервер\n";
    cout << "exit       — завершить сервер, прервав активные подключения\n";
    cout << "help       — повторить список команд\n";
    cout << "quit       — закрыть только эту консоль\n";
}


int wmain(int argc, wchar_t* argv[])
{
    setlocale(LC_ALL, ".UTF8");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE client = INVALID_HANDLE_VALUE;

    try
    {
        if (argc == 2 && wstring(argv[1]) == L"--help")
        {
            cout << "ClientNP.exe [IP или имя сервера] [имя канала]\nПо умолчанию: . ServerPipe\n";
            return 0;
        }

        const wstring pipe_name = GetPipeName(argc, argv);
        cout << "========== ClientNP: управление сервером ==========\n" << "Подключение к " << Utf8(pipe_name) << "...\n";
        client = OpenPipe(pipe_name);
        SetMessageMode(client);

        GetInteractionInstruction();
        while (true)
        {
            cout << "\nУправление> " << flush;
            string command;
            if (!getline(cin, command) || command == "quit")
            {
                break;
            }
            if (command.empty())
            {
                continue;
            }
            if (command == "help")
            {
                GetInteractionInstruction();
                continue;
            }
            if (command.size() > 100)
            {
                cout << "Слишком длинная команда. Для подсказки введите help.\n";
                continue;
            }

            DWORD written = 0;
            if (!WriteFile(client, command.c_str(), static_cast<DWORD>(command.size()), &written, nullptr) || written != command.size())
            {
                throw SetErrorMessageText("Ошибка WriteFile: ", GetLastError());
            }
            if (command == "wait")
            {
                cout << "Ожидаем завершения активных клиентов...\n" << flush;
            }

            string reply;
            if (!ReadPipeMessage(client, reply))
            {
                if (command == "exit" || command == "shutdown")
                {
                    cout << "Сервер завершает работу.\n";
                    break;
                }
                throw string("Сервер закрыл канал управления.\n");
            }

            if (reply.rfind("statistics\n", 0) == 0)
            {
                cout << "\n" << reply.substr(11) << '\n';
            }
            else if (reply == "start")
            {
                cout << "Приём новых клиентов разрешён.\n";
            }
            else if (reply == "stop")
            {
                cout << "Приём новых клиентов приостановлен. Активные клиенты продолжают работу.\n";
            }
            else if (reply == "wait")
            {
                cout << "Все текущие клиенты обслужены. Приём возобновлён.\n";
            }
            else if (reply == "shutdown")
            {
                cout << "Сервер обслужит текущих клиентов и завершит работу.\n";
            }
            else if (reply == "exit")
            {
                cout << "Сервер завершает работу. Активные подключения прерываются.\n";
            }
            else if (reply == "nocmd")
            {
                cout << "Неизвестная команда. Введите help для подсказки.\n";
            }
            else
            {
                cout << "Ответ сервера: " << reply << '\n';
            }

            if (command == "exit" || command == "shutdown")
            {
                break;
            }
        }

        CloseHandle(client);
        cout << "Консоль управления отключена.\n";
        return 0;
    }
    catch (const string& error)
    {
        cerr << "[ClientNP] " << error << '\n';
        if (client != INVALID_HANDLE_VALUE)
        {
            CloseHandle(client);
        }
        return 1;
    }
}