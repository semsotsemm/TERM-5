#include "../Common/Worker.h"

using namespace std;

BOOL DeleteNumber(HT::HTHANDLE* ht, int32_t key)
{
    HT::Element element(&key, sizeof(key));
    if (!HT::Delete(ht, &element))
    {
        return FALSE;
    }

    cout << "Запись удалена." << endl;
    return TRUE;
}

int wmain(int argc, wchar_t* argv[])
{
    return RunWorker(argc, argv, "OS11_03", DeleteNumber);
}
