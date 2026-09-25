#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

bool CheckInput(string input) {
    try {
        int target_number = stoi(input);

        return true;
    }
    catch (exception e) {
        return false;
    }
}


// ОБъявляем указатель на функцию
typedef int(*AddFunc)(int, int);


int main()
{
    setlocale(LC_CTYPE, "Russian");
    AddFunc function_name;
    HMODULE my_lib;

    while (true)
    {
        string user_input;
        int user_choice, first_number, second_number = 0;

        cout << "Выберите операцию:\n";
        cout << "1: Сумма\n2: Разность\n3: Произведение\n4: Частное\n";
        cin >> user_input;

        if (CheckInput(user_input))
        {
            user_choice = stoi(user_input);
        }
        else {
            cout << "Ошибка ввода, попробуйте еще раз.\n";
            continue;
        }

        cout << "Введите 1-ое число: ";
        cin >> user_input;
        if (CheckInput(user_input))
        {
            first_number = stoi(user_input);
        }
        else {
            cout << "Ошибка ввода, попробуйте еще раз.\n";
            continue;
        }

        cout << "Введите 2-ое число: ";
        cin >> user_input;
        if (CheckInput(user_input))
        {
            second_number = stoi(user_input);
        }
        else {
            cout << "Ошибка ввода, попробуйте еще раз.\n";
            continue;
        }

        cout << "\n=======================================\n\n";

        // Динамическая загрузка библиотеки
        my_lib = LoadLibrary(TEXT("SPAA.dll"));

        switch (user_choice)
        {
            case 1:
            {
                // Получаем имя функции

                function_name = (AddFunc)GetProcAddress(my_lib, "sum");
                cout << first_number << " + " << second_number << " = " << function_name(first_number, second_number) << "\n";
                break;
            }
            case 2:
            {
                function_name = (AddFunc)GetProcAddress(my_lib, "sub");
                cout << first_number << " - " << second_number << " = " << function_name(first_number, second_number) << "\n";
                break;
            }
            case 3:
            {
                function_name = (AddFunc)GetProcAddress(my_lib, "mul");
                cout << first_number << " * " << second_number << " = " << function_name(first_number, second_number) << "\n";
                break;
            }
            case 4:
            {
                function_name = (AddFunc)GetProcAddress(my_lib, "my_div");
                cout << first_number << " / " << second_number << " = " << function_name(first_number, second_number) << "\n";
                break;
            }
            default: {
                cout << "Ошибка ввода, попробуйте еще раз.\n";
                break;
            }
        }
        FreeLibrary(my_lib);

        cout << "\n=======================================\n\n";

    }

    return 0;
}