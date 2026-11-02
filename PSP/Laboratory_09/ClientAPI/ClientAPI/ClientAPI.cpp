#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include "ClientAPI.h" 
#include <string>
#include <iostream>


using namespace std;


ConcurrentServer::ConcurrentServer() : tcp_socket(INVALID_SOCKET), tcp_port(0)
{
    InitWSA();
}


ConcurrentServer::~ConcurrentServer()
{
    Disconnect();
    WSACleanup();
}


bool ConcurrentServer::InitWSA()
{
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}


void ConcurrentServer::Disconnect()
{
    if (tcp_socket != INVALID_SOCKET)
    {
        closesocket(tcp_socket);
        tcp_socket = INVALID_SOCKET;
    }
}


bool ConcurrentServer::GetServer(int udp_port, const string& callstring)
{
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == INVALID_SOCKET)
    {
        return false;
    }

    BOOL bOptVal = TRUE;
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, sizeof(BOOL));

    sockaddr_in broadcast_addr = {};
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(udp_port);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    if (sendto(udp_socket, callstring.c_str(), callstring.length(), 0, (sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) == SOCKET_ERROR)
    {
        closesocket(udp_socket);
        return false;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(udp_socket, &read_fds);
    timeval timeout = { 2, 0 }; 
    
    // Массив сокетов, для проверки на запись, исключения
    if (select(0, &read_fds, NULL, NULL, &timeout) > 0)
    {
        char buffer[256] = {};
        sockaddr_in server_addr = {};
        int addr_len = sizeof(server_addr);

        int bytes = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&server_addr, &addr_len);
        if (bytes > 0)
        {
            string response(buffer);
            size_t pos = response.find("TCP=");
            if (pos != string::npos)
            {
                this->server_ip = inet_ntoa(server_addr.sin_addr);
                this->tcp_port = stoi(response.substr(pos + 4));
                closesocket(udp_socket);
                return true;
            }
        }
    }

    closesocket(udp_socket);
    return false;
}


bool ConcurrentServer::Connect(const string& service_name)
{
    tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_socket == INVALID_SOCKET)
    {
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(tcp_port);
    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if (connect(tcp_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        Disconnect();
        return false;
    }

    string req = service_name + "\n";
    send(tcp_socket, req.c_str(), req.length(), 0);

    char buffer[50] = {};
    int bytes = recv(tcp_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
    {
        string response(buffer);
        if (response.find("OK\n") != string::npos || response.find("OK") != string::npos)
        {
            return true;
        }
    }

    Disconnect();
    return false;
}


bool ConcurrentServer::RequestEcho(const char* data, int size, char* buffer)
{
    int sent = 0;
    while (sent < size)
    {
        int s = send(tcp_socket, data + sent, size - sent, 0);
        if (s <= 0)
        {
            return false;
        }
        sent += s;
    }

    int received = 0;
    while (received < size)
    {
        int r = recv(tcp_socket, buffer + received, size - received, 0);
        if (r <= 0)
        {
            return false;
        }
        received += r;
    }
    return true;
}


string ConcurrentServer::RequestTime()
{
    string req = "time\n";
    if (send(tcp_socket, req.c_str(), req.length(), 0) <= 0)
    {
        return "";
    }

    string result = "";
    char symbol;
    while (recv(tcp_socket, &symbol, 1, 0) > 0)
    {
        if (symbol == '\n') break;
        result += symbol;
    }
    return result;
}


int32_t ConcurrentServer::RequestRand()
{
    string req = "rand\n";
    if (send(tcp_socket, req.c_str(), req.length(), 0) <= 0) return 0;

    int32_t number = 0;
    int received = 0;
    char* ptr = (char*)&number;

    while (received < sizeof(int32_t))
    {
        int r = recv(tcp_socket, ptr + received, sizeof(int32_t) - received, 0);
        if (r <= 0)
        {
            return 0;
        }
        received += r;
    }
    return number;
}