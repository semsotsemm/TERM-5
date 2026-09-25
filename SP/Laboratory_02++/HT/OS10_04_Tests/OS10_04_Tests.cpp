#include <iostream>
#include "HT.h"
#include <cassert> 

using namespace std;

void Test_CreateAndClose() {
    cout << "Test_CreateAndClose... ";
    HT::HTHANDLE* ht = HT::Create(10, 2, 10, 20, "TestHT1.ht");
    assert(ht != nullptr);

    BOOL closed = HT::Close(ht);
    assert(closed == TRUE);
    cout << "PASSED\n";
}

void Test_InsertAndGet() {
    cout << "Test_InsertAndGet... ";
    HT::HTHANDLE* ht = HT::Create(10, 2, 10, 20, "TestHT2.ht");

    HT::Element* el = new HT::Element("testKey", 7, "testData", 8);
    BOOL inserted = HT::Insert(ht, el);
    assert(inserted == TRUE);

    HT::Element* getEl = new HT::Element("testKey", 7);
    HT::Element* result = HT::Get(ht, getEl);

    assert(result != nullptr);

    HT::Close(ht);
    cout << "PASSED\n";
}

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "=== ЗАПУСК ТЕСТОВ HT API ===\n";

    Test_CreateAndClose();
    Test_InsertAndGet();

    cout << "=== ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО ===\n";
    return 0;
}