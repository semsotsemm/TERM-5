using System;
using System.Runtime.InteropServices;
using System.Text;

class Program
{
    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr LoadLibrary(string lpFileName);

    [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
    static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    static extern bool FreeLibrary(IntPtr hModule);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    delegate int MathFunc(int x, int y);

    static bool CheckInput(string input, out int result)
    {
        return int.TryParse(input, out result);
    }

    static void Main(string[] args)
    {
        Console.OutputEncoding = Encoding.UTF8;

        IntPtr pDll = LoadLibrary("SPAA.dll");

        while (true)
        {
            Console.WriteLine("Выберите операцию:");
            Console.WriteLine("1: Сумма\n2: Разность\n3: Произведение\n4: Частное\n5: Выход");
            string userInput = Console.ReadLine();

            if (!CheckInput(userInput, out int userChoice))
            {
                Console.WriteLine("Ошибка ввода, попробуйте еще раз.\n");
                continue;
            }

            if (userChoice == 5)
            {
                Console.WriteLine("Успешный выход из программы.");
                FreeLibrary(pDll);
                break;
            }

            if (userChoice < 1 || userChoice > 4)
            {
                Console.WriteLine("Неверный выбор операции, попробуйте еще раз.\n");
                continue;
            }

            Console.Write("Введите 1-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int firstNumber))
            {
                Console.WriteLine("Ошибка ввода числа.\n");
                continue;
            }

            Console.Write("Введите 2-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int secondNumber))
            {
                Console.WriteLine("Ошибка ввода числа.\n");
                continue;
            }

            Console.WriteLine("\n=======================================\n");

            string funcName = "";
            switch (userChoice)
            {
                case 1: funcName = "sum"; break;
                case 2: funcName = "sub"; break;
                case 3: funcName = "mul"; break;
                case 4: funcName = "my_div"; break;
            }

            IntPtr pAddress = GetProcAddress(pDll, funcName);

            MathFunc operation = Marshal.GetDelegateForFunctionPointer<MathFunc>(pAddress);

            int result = operation(firstNumber, secondNumber);
            char op = '+';
            if (userChoice == 2) op = '-';
            else if (userChoice == 3) op = '*';
            else if (userChoice == 4) op = '/';
            Console.WriteLine($"{firstNumber} {op} {secondNumber} = {result}");

            Console.WriteLine("\n=======================================\n");
        }
    }
}