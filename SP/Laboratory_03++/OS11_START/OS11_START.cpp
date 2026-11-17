#include "../Common/Console.h"

using namespace std;

bool StopFromInput()
{
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(input, &mode))
    {
        DWORD available = 0;
        if (!GetNumberOfConsoleInputEvents(input, &available))
        {
            return true;
        }
        while (available-- > 0)
        {
            INPUT_RECORD event;
            DWORD read;
            if (!ReadConsoleInputW(input, &event, 1, &read))
            {
                return true;
            }
            if (read == 1 && event.EventType == KEY_EVENT && event.Event.KeyEvent.bKeyDown &&
                event.Event.KeyEvent.uChar.UnicodeChar != L'\0')
            {
                return true;
            }
        }
        return false;
    }

    if (GetFileType(input) == FILE_TYPE_PIPE)
    {
        DWORD available = 0;
        if (!PeekNamedPipe(input, nullptr, 0, nullptr, &available, nullptr)) return true;
        if (available == 0) return false;
    }

    char symbol;
    DWORD read;
    ReadFile(input, &symbol, 1, &read, nullptr);
    return true;
}

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();

    char file_name[512];
    if (argc != 2 || !FileNameToUtf8(argv[1], file_name))
    {
        cerr << "Использование: OS11_START.exe <имя файла>\n";
        return 1;
    }

    HT::HTHANDLE* ht = HT::Start(file_name);
    if (ht == nullptr)
    {
        PrintStorageError();
        return 1;
    }

    if (!SetUpStopEvent())
    {
        cerr << "Не удалось установить обработчик Ctrl+C.\n";
        HT::Close(ht);
        return 1;
    }

    PrintStorage("Start", ht);
    cout << "Хранилище доступно. Для завершения нажмите любой символ." << endl;

    const ULONGLONG snapshot_interval = ht->SecSnapshotInterval * 1000ULL;
    ULONGLONG next_snapshot = GetTickCount64() + snapshot_interval;
    int exit_code = 0;
    while (WaitForSingleObject(stop_event, 50) == WAIT_TIMEOUT && !StopFromInput())
    {
        if (GetTickCount64() >= next_snapshot)
        {
            if (!HT::Snap(ht))
            {
                PrintStorageError(ht);
                exit_code = 1;
                break;
            }
            cout << "Snapshot: образ сохранён." << endl;
            next_snapshot = GetTickCount64() + snapshot_interval;
        }
    }

    if (!HT::Close(ht))
    {
        PrintStorageError();
        exit_code = 1;
    }
    else
    {
        cout << "Финальный snapshot: образ сохранён. Хранилище закрыто." << endl;
    }
    CloseStopEvent();
    return exit_code;
}
