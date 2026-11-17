#include "../Common/Worker.h"

using namespace std;

BOOL IncreaseNumber(HT::HTHANDLE* ht, int32_t key)
{
    HT::Element element(&key, sizeof(key));
    int32_t value;
    if (!HT::Increment(ht, &element, &value))
    {
        return FALSE;
    }

    cout << "Значение увеличено: " << value - 1 << " -> " << value << '.' << endl;
    return TRUE;
}

int wmain(int argc, wchar_t* argv[])
{
    return RunWorker(argc, argv, "OS11_04", IncreaseNumber);
}
