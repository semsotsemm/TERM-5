#include <iostream>
#include "HT.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");

    HT::HTHANDLE* ht = nullptr;
    try
    {
        ht = HT::Create(1000, 3, 10, 256, "D:\\HTspace.ht");
        if (ht) cout << "-- Create: успешно\n";
        else throw "-- Create: ошибка";

        if (HT::Insert(ht, new HT::Element("key222", 7, "payload", 8))) cout << "-- Insert: успешно\n";
        else throw "-- Insert: ошибка";

        HT::Element* hte = HT::Get(ht, new HT::Element("key222", 7));
        if (hte) cout << "-- Get: успешно\n";
        else throw "-- Get: ошибка";

        HT::print(hte);

        if (HT::Snap(ht)) cout << "-- Snap: успешно\n";
        else throw "-- Snap: ошибка";

        if (HT::Update(ht, hte, "newpayload", 11)) cout << "-- Update: успешно\n";
        else throw "-- Update: ошибка";

        HT::Element* hte1 = HT::Get(ht, new HT::Element("key222", 7));
        if (hte1) cout << "-- Get: успешно\n";
        else throw "-- Get: ошибка";

        HT::print(hte1);

        if (HT::Delete(ht, hte1)) cout << "-- Delete: успешно\n";
        else throw "-- Delete: ошибка";

        if (HT::Close(ht)) cout << "-- Close: успешно\n";
        else throw "-- Close: ошибка";
    }
    catch (const char* msg)
    {
        cout << msg << "\n";
        if (ht != nullptr) cout << HT::GetLastError(ht) << "\n";
    }

    HT::HTHANDLE* ht_open = nullptr;
    try
    {
        ht_open = HT::Open("D:\\HTspace.ht");
        if (ht_open) cout << "-- Open: успешно\n";
        else throw "-- Open: ошибка";

        if (HT::Insert(ht_open, new HT::Element("key333", 7, "payload", 8))) cout << "-- Insert: успешно\n";
        else throw "-- Insert: ошибка";

        HT::Element* hte2 = HT::Get(ht_open, new HT::Element("key333", 7));
        if (hte2) cout << "-- Get: успешно\n";
        else throw "-- Get: ошибка";

        if (HT::Close(ht_open)) cout << "-- Close: успешно\n";
        else throw "-- Close: ошибка";
    }
    catch (const char* msg)
    {
        cout << msg << "\n";
        if (ht_open != nullptr) cout << HT::GetLastError(ht_open) << "\n";
    }

    return 0;
}