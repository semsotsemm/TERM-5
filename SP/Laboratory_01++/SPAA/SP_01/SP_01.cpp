#include <iostream>
#include <string>

#include "../SPAA/SPAA.h"

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


int main()
{
    setlocale(LC_CTYPE, "Russian");

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


        switch (user_choice)
        {
            case 1:
            {
                cout << first_number << " + " << second_number << " = " << sum(first_number, second_number) << "\n";
                break;
            }
            case 2:
            {
                cout << first_number << " - " << second_number << " = " << sub(first_number, second_number) << "\n";
                break;
            }
            case 3:
            {
                cout << first_number << " * " << second_number << " = " << mul(first_number, second_number) << "\n";
                break;
            }
            case 4:
            {
                cout << first_number << " / " << second_number << " = " << my_div(first_number, second_number) << "\n";
                break;
            }
            default: {
                cout << "Ошибка ввода, попробуйте еще раз.\n";
                break;
            }
        }

        cout << "\n=======================================\n\n";

    }
    
    return 0;
}