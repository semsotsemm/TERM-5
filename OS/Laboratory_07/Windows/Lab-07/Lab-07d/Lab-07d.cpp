#include <iostream>
#include <windows.h>
#include <string>

using namespace std;


int main()
{
	setlocale(LC_CTYPE, "Russian");


	STARTUPINFOA first_startup_info, second_startup_info;
	ZeroMemory(&first_startup_info, sizeof(first_startup_info));
	first_startup_info.cb = sizeof(first_startup_info);
	ZeroMemory(&second_startup_info, sizeof(second_startup_info));
	second_startup_info.cb = sizeof(second_startup_info);


	PROCESS_INFORMATION first_process_info, second_process_info;


	char start_command[] = "Lab-07x.exe";
	char first_thread_work_time [] = "60";
	char second_thread_work_time [] = "120";

	string cmd = string(start_command) + " " + first_thread_work_time;
	CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &first_startup_info, &first_process_info);
	
	cmd = string(start_command) + " " + second_thread_work_time;
	CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &second_startup_info, &second_process_info);

	
	HANDLE process[2] = { first_process_info.hProcess, second_process_info.hProcess};
	WaitForMultipleObjects(2, process, TRUE, INFINITE);


	CloseHandle(first_process_info.hProcess);
	CloseHandle(second_process_info.hProcess);
	CloseHandle(first_process_info.hThread);
	CloseHandle(second_process_info.hThread);
	return 0;
}