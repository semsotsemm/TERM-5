using System;
using System.Runtime.InteropServices;
using System.Text;

class Program
{
    // Импортируем стандартные функции Linux для работы с динамическими библиотеками (dlfcn.h)
    [DllImport("libdl.so.2", EntryPoint = "dlopen")]
    public static extern IntPtr dlopen(string filename, int flags);

    [DllImport("libdl.so.2", EntryPoint = "dlsym")]
    public static extern IntPtr dlsym(IntPtr handle, string symbol);

    [DllImport("libdl.so.2", EntryPoint = "dlclose")]
    public static extern int dlclose(IntPtr handle);

    [DllImport("libdl.so.2", EntryPoint = "dlerror")]
    public static extern IntPtr dlerror();

    const int RTLD_LAZY = 1;

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    delegate int MathFunc(int x, int y);

    static bool CheckInput(string input, out int result)
    {
        return int.TryParse(input, out result);
    }

    static void Main(string[] args)
    {
        Console.OutputEncoding = Encoding.UTF8;
        Console.WriteLine("--- C# .NET App (SP_08: Динамический вызов .so) ---");

        // Динамически загружаем библиотеку
        IntPtr pLib = dlopen("./libSP_AA.so", RTLD_LAZY);
        if (pLib == IntPtr.Zero)
        {
            IntPtr errPtr = dlerror();
            string errStr = errPtr != IntPtr.Zero ? Marshal.PtrToStringAnsi(errPtr) : "Unknown error";
            Console.WriteLine($"Ошибка: Не удалось загрузить libSP_AA.so: {errStr}");
            return;
        }

        while (true)
        {
            Console.WriteLine("Выберите операцию:");
            Console.WriteLine("1: Сумма\n2: Разность\n3: Произведение\n4: Частное\n5: Выход");
            Console.Write("Ввод: ");
            string userInput = Console.ReadLine();

            if (!CheckInput(userInput, out int userChoice))
            {
                Console.WriteLine("Ошибка ввода.\n");
                continue;
            }

            if (userChoice == 5)
            {
                Console.WriteLine("Выход.");
                dlclose(pLib); 
                break;
            }

            if (userChoice < 1 || userChoice > 4)
            {
                Console.WriteLine("Неверный выбор.\n");
                continue;
            }

            Console.Write("Введите 1-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int firstNumber)) continue;

            Console.Write("Введите 2-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int secondNumber)) continue;

            string funcName = "";
            switch (userChoice)
            {
                case 1: funcName = "sum"; break;
                case 2: funcName = "sub"; break;
                case 3: funcName = "mul"; break;
                case 4: funcName = "my_div"; break;
            }

            // Получаем адрес функции по ее строковому имени (аналог dlsym в C++)
            IntPtr pAddress = dlsym(pLib, funcName);
            if (pAddress == IntPtr.Zero)
            {
                Console.WriteLine($"Ошибка: Функция '{funcName}' не найдена!\n");
                continue;
            }

            MathFunc operation = Marshal.GetDelegateForFunctionPointer<MathFunc>(pAddress);

            if (userChoice == 4 && secondNumber == 0)
            {
                Console.WriteLine("Ошибка: Деление на ноль!");
            }
            else
            {
                int result = operation(firstNumber, secondNumber);
                char op = '+';
                if (userChoice == 2) op = '-';
                else if (userChoice == 3) op = '*';
                else if (userChoice == 4) op = '/';

                Console.WriteLine($"{firstNumber} {op} {secondNumber} = {result}");
            }

            Console.WriteLine("\n=======================================\n");
        }
    }
}