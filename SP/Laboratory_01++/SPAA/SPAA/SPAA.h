#pragma once

#ifdef SPAA_EXPORTS
#define SPAA_API __declspec(dllexport)
#else
#define SPAA_API __declspec(dllimport)
#endif

extern "C"
{
	__declspec(dllexport) int sum(int x, int y);
	__declspec(dllexport) int sub(int x, int y);
	__declspec(dllexport) int mul(int x, int y);
	__declspec(dllexport) int my_div(int x, int y);
}