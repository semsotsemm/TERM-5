#pragma once
#include <winsock2.h>

using namespace std;

class ConcurrentServer
{
	private:
		SOCKET tcp_socket;
		string server_ip;
		int tcp_port;

		bool InitWSA();

	public:
		ConcurrentServer();
		~ConcurrentServer();

		bool GetServer(int udp_port, const string& callstring);
		bool Connect(const string& service_name);	
		bool RequestEcho(const char* data, int size, char* buffer);
		string RequestTime();
		int32_t RequestRand();
		void Disconnect();
};