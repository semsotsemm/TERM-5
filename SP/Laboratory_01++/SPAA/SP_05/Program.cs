using System;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("SPAA.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int sum(int x, int y);

    [DllImport("SPAA.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int sub(int x, int y);

    [DllImport("SPAA.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int mul(int x, int y);

    [DllImport("SPAA.dll", CallingConvention = CallingConvention.Cdecl)]
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
                Console.WriteLine("Ошибка ввода, попробуйте еще раз.\n");
                continue;
            }

            if (userChoice == 5)
            {
                Console.WriteLine("Успешный выход из программы.");
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
                Console.WriteLine("Ошибка ввода числа, попробуйте еще раз.\n");
                continue;
            }

            Console.Write("Введите 2-ое число: ");
            if (!CheckInput(Console.ReadLine(), out int secondNumber))
            {
                Console.WriteLine("Ошибка ввода числа, попробуйте еще раз.\n");
                continue;
            }

            Console.WriteLine("\n=======================================\n");

            try
            {
                switch (userChoice)
                {
                    case 1:
                        Console.WriteLine($"{firstNumber} + {secondNumber} = {sum(firstNumber, secondNumber)}");
                        break;
                    case 2:
                        Console.WriteLine($"{firstNumber} - {secondNumber} = {sub(firstNumber, secondNumber)}");
                        break;
                    case 3:
                        Console.WriteLine($"{firstNumber} * {secondNumber} = {mul(firstNumber, secondNumber)}");
                        break;
                    case 4:
                        Console.WriteLine($"{firstNumber} / {secondNumber} = {my_div(firstNumber, secondNumber)}");
                        break;
                }
            }
            catch (DllNotFoundException)
            {
                Console.WriteLine("ОШИБКА: Не удалось найти файл SPAA.dll рядом с исполняемым файлом программы!");
            }

            Console.WriteLine("\n=======================================\n");
        }
    }
}