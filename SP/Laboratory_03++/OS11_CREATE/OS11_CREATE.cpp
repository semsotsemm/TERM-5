#include "../Common/Console.h"
#include <cerrno>
#include <cwchar>

using namespace std;

bool ReadPositiveNumber(const wchar_t* text, int& number)
{
    wchar_t* end;
    errno = 0;
    long value = wcstol(text, &end, 10);
    if (text == end || *end != L'\0' || errno == ERANGE || value <= 0 || value > INT_MAX)
    {
        return false;
    }
    number = static_cast<int>(value);
    return true;
}

int wmain(int argc, wchar_t* argv[])
{
    InitConsole();

    char file_name[512];
    int snapshot_interval, capacity, max_key_length, max_data_length;
    if (argc != 6 || !FileNameToUtf8(argv[1], file_name) ||
        !ReadPositiveNumber(argv[2], snapshot_interval) || !ReadPositiveNumber(argv[3], capacity) ||
        !ReadPositiveNumber(argv[4], max_key_length) || !ReadPositiveNumber(argv[5], max_data_length))
    {
        cerr << "Использование: OS11_CREATE.exe <файл> <интервал, сек> <ёмкость> <размер ключа> <размер данных>\n";
        return 1;
    }

    HT::HTHANDLE* ht = HT::Create(capacity, snapshot_interval, max_key_length, max_data_length, file_name);
    if (ht == nullptr)
    {
        PrintStorageError();
        return 1;
    }

    HT::HTHANDLE info = *ht;
    if (!HT::Close(ht))
    {
        PrintStorageError();
        return 1;
    }
    PrintStorage("Created", &info);
    return 0;
}
