#include "../ServerNP/PipeCommon.h"
#include <aclapi.h>
#include <thread>
#include <vector>
#pragma comment(lib, "Advapi32.lib")

void Check(bool ok, const char* text)
{
    if (!ok) throw std::string(text);
}

void CheckAddress()
{
    wchar_t exe[] = L"client.exe";
    wchar_t host[] = L"192.0.2.10";
    wchar_t* arguments[] = { exe, host };
    Check(GetPipeName(1, arguments) == L"\\\\.\\pipe\\Tube", "local address");
    Check(GetPipeName(2, arguments) == L"\\\\192.0.2.10\\pipe\\Tube", "remote address");
    wchar_t invalid[] = L"\\\\server\\pipe\\Tube";
    arguments[1] = invalid;
    bool rejected = false;
    try { GetPipeName(2, arguments); } catch (const std::string&) { rejected = true; }
    Check(rejected, "invalid server argument accepted");
}

void CheckBoundaryMessages()
{
    HANDLE pipe = OpenPipe(L"\\\\.\\pipe\\Tube");
    SetMessageMode(pipe);
    try
    {
        for (const size_t size : { 1u, 511u, 512u, 513u, 1536u, 65536u })
        {
            const std::string request(size, 'X');
            DWORD written = 0;
            Check(WriteFile(pipe, request.data(), static_cast<DWORD>(request.size()), &written, nullptr)
                && written == request.size(), "boundary write failed");
            std::string reply;
            Check(ReadPipeMessage(pipe, reply) && reply == "Сообщение получено!", "boundary reply failed");
            std::cout << "PASS request bytes=" << size << std::endl;
        }

        PACL acl = nullptr;
        PSECURITY_DESCRIPTOR descriptor = nullptr;
        Check(GetSecurityInfo(pipe, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
            nullptr, nullptr, &acl, nullptr, &descriptor) == ERROR_SUCCESS, "GetSecurityInfo failed");
        BYTE sid[SECURITY_MAX_SID_SIZE];
        DWORD sidSize = sizeof(sid);
        Check(CreateWellKnownSid(WinAuthenticatedUserSid, nullptr, sid, &sidSize) != FALSE, "AU SID failed");
        bool allowed = false;
        for (DWORD i = 0; i < acl->AceCount; ++i)
        {
            void* ace = nullptr;
            Check(GetAce(acl, i, &ace) != FALSE, "GetAce failed");
            const auto header = static_cast<ACE_HEADER*>(ace);
            if (header->AceType == ACCESS_ALLOWED_ACE_TYPE)
            {
                const auto entry = static_cast<ACCESS_ALLOWED_ACE*>(ace);
                if (EqualSid(sid, const_cast<DWORD*>(&entry->SidStart)))
                    allowed = (entry->Mask & (FILE_GENERIC_READ | FILE_GENERIC_WRITE)) ==
                        (FILE_GENERIC_READ | FILE_GENERIC_WRITE);
            }
        }
        LocalFree(descriptor);
        Check(allowed, "AU has no read/write ACE");
        std::cout << "PASS authenticated users DACL" << std::endl;
    }
    catch (...) { CloseHandle(pipe); throw; }
    CloseHandle(pipe);
}

void CheckFragmentedReply()
{
    const std::wstring name = L"\\\\.\\pipe\\Lab06ReadTest-" + std::to_wstring(GetCurrentProcessId());
    HANDLE server = CreateNamedPipeW(name.c_str(), PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 512, 512, 1000, nullptr);
    Check(server != INVALID_HANDLE_VALUE, "test server creation failed");
    bool writerOk = false;
    const std::string expected(1537, 'R');
    std::thread writer([&]() {
        const BOOL connected = ConnectNamedPipe(server, nullptr);
        if (connected || GetLastError() == ERROR_PIPE_CONNECTED)
        {
            DWORD written = 0;
            writerOk = WriteFile(server, expected.data(), static_cast<DWORD>(expected.size()),
                &written, nullptr) && written == expected.size();
            FlushFileBuffers(server);
            DisconnectNamedPipe(server);
        }
        CloseHandle(server);
    });
    HANDLE client = INVALID_HANDLE_VALUE;
    try
    {
        client = OpenPipe(name);
        SetMessageMode(client);
        std::string actual;
        Check(ReadPipeMessage(client, actual) && actual == expected, "fragmented reply mismatch");
        Check(!ReadPipeMessage(client, actual), "disconnect was not detected");
        CloseHandle(client);
        client = INVALID_HANDLE_VALUE;
        writer.join();
        Check(writerOk, "test writer failed");
    }
    catch (...)
    {
        if (client != INVALID_HANDLE_VALUE) CloseHandle(client);
        CancelSynchronousIo(writer.native_handle());
        writer.join();
        throw;
    }
    std::cout << "PASS fragmented response and disconnect" << std::endl;
}

int main()
{
    InitConsole();
    try
    {
        CheckAddress();
        CheckBoundaryMessages();
        CheckFragmentedReply();
        std::cout << "PASS all native tests" << std::endl;
        return 0;
    }
    catch (const std::string& error)
    {
        std::cerr << "FAIL " << error << std::endl;
        return 1;
    }
}
