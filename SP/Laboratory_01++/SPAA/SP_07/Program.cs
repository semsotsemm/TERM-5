using System;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("libSP_AA.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int sum(int x, int y);

    [DllImport("libSP_AA.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int sub(int x, int y);

    [DllImport("libSP_AA.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int mul(int x, int y);

    [DllImport("libSP_AA.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int my_div(int x, int y);

    static bool CheckInput(string input, out int result)
    {
        return int.TryParse(input, out result);
    }

    static void Main(string[] args)
    {
        Console.OutputEncoding = System.Text.Encoding.UTF8;
        while (true)
        {
            Console.WriteLine("Выберите операцию:");
            Console.WriteLine("1: Сумма\n2: Разность\n3: Произведение\n4: Частное\n5: Выход");
            string userInput = Console.ReadLine();

            if (!CheckInput(userInput, out int userChoice))
            {
                Console.WriteLine("Ошибка ввода.\n");
                continue;
            }

            if (userChoice == 5)
            {
                Console.WriteLine("Выход.");
                break;
            }

            Console.Write("Введите 1-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int firstNumber)) continue;

            Console.Write("Введите 2-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int secondNumber)) continue;

            try
            {
                switch (userChoice)
                {
                    case 1:
                        Console.WriteLine($"Результат: {sum(firstNumber, secondNumber)}");
                        break;
                    case 2:
                        Console.WriteLine($"Результат: {sub(firstNumber, secondNumber)}");
                        break;
                    case 3:
                        Console.WriteLine($"Результат: {mul(firstNumber, secondNumber)}");
                        break;
                    case 4:
                        if (secondNumber == 0) Console.WriteLine("Ошибка: Деление на ноль!");
                        else Console.WriteLine($"Результат: {my_div(firstNumber, secondNumber)}");
                        break;
                }
            }
            catch (DllNotFoundException ex)
            {
                Console.WriteLine($"Ошибка загрузки .so: {ex.Message}");
            }
        }
    }
}