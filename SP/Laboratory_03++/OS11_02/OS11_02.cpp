#include "../Common/Worker.h"

using namespace std;

BOOL InsertNumber(HT::HTHANDLE* ht, int32_t key)
{
    int32_t value = 0;
    HT::Element element(&key, sizeof(key), &value, sizeof(value));
    if (!HT::Insert(ht, &element))
    {
        return FALSE;
    }

    cout << "Добавлено значение " << value << '.' << endl;
    return TRUE;
}

int wmain(int argc, wchar_t* argv[])
{
    return RunWorker(argc, argv, "OS11_02", InsertNumber);
}
