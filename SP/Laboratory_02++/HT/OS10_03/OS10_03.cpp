#include <iostream>
#include "HT.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    HT::HTHANDLE* ht1 = nullptr;
    HT::HTHANDLE* ht2 = nullptr;

    try {
        ht1 = HT::Create(1000, 3, 10, 256, "D:\\HTspace1.ht");
        if (ht1) cout << "HT 1 создан успешно.\n";

        ht2 = HT::Create(500, 2, 10, 128, "D:\\HTspace2.ht");
        if (ht2) cout << "HT 2 создан успешно.\n";

        if (HT::Insert(ht1, new HT::Element("key1", 4, "data1", 5)))
            cout << "Данные в HT 1 добавлены.\n";

        if (HT::Insert(ht2, new HT::Element("key2", 4, "data2", 5)))
            cout << "Данные в HT 2 добавлены.\n";

        HT::Close(ht1);
        HT::Close(ht2);
        cout << "Оба хранилища успешно закрыты.\n";
    }
    catch (...) {
        cout << "Произошла ошибка!" << endl;
    }

    return 0;
}