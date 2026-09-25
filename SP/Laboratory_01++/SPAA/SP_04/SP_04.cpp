#include <iostream>
#include <string>
#include <dlfcn.h>


using namespace std;
typedef int (*MathFunc)(int, int);


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

    // Ищем адреса функции в момент вызова
    void* my_lib = dlopen("./libSP_AA.so", RTLD_LAZY);

    while (true)
    {
        string user_input;
        int user_choice, first_number, second_number = 0;

        cout << "Выберите операцию:\n";
        cout << "1: Сумма\n2: Разность\n3: Произведение\n4: Частное\n5: Выход\n";
        cin >> user_input;

        if (CheckInput(user_input))
        {
            user_choice = stoi(user_input);
            if (user_choice == 5) {
                cout << "Успешный выход из программы.\n";
                dlclose(my_lib);
                return 0;
            }
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

        // получаем адрес функции по ее имени 
        MathFunc my_function = nullptr;

        switch (user_choice)
        {
        case 1:
        {
            // Загружаем имя функции 
            my_function = (MathFunc)dlsym(my_lib, "sum");
            cout << first_number << " + " << second_number << " = " << my_function(first_number, second_number) << "\n";
            break;
        }
        case 2:
        {
            my_function = (MathFunc)dlsym(my_lib, "sub");   
            cout << first_number << " - " << second_number << " = " << my_function(first_number, second_number) << "\n";
            break;
        }
        case 3:
        {
            my_function = (MathFunc)dlsym(my_lib, "mul");
            cout << first_number << " * " << second_number << " = " << my_function(first_number, second_number) << "\n";
            break;
        }
        case 4:
        {
            my_function = (MathFunc)dlsym(my_lib, "my_div");
            cout << first_number << " / " << second_number << " = " << my_function(first_number, second_number) << "\n";
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