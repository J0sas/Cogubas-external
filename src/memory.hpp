#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string_view>
#include "offsets.hpp"

// my own header file

class memory
{
private:
    DWORD process_id;
    HANDLE process_handle;

public:

    // make a function to get the process id 
    memory(std::wstring_view proc_name) noexcept
    {

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
        {
            CloseHandle(snapshot);
            exit;
        }

        PROCESSENTRY32 entry = {};
        entry.dwSize = sizeof(PROCESSENTRY32);

        do {
            if (proc_name == entry.szExeFile)
            {
                process_id = entry.th32ProcessID;
            }

        } while (Process32Next(snapshot, &entry));

        process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);

        if (snapshot)
        {
            CloseHandle(snapshot);
        }
    }
    // make a function to get the base module address

    std::uintptr_t get_module_address(std::wstring_view client_name) const
    {
        /* std::wstring client_name = L"client.dll";*/
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

        std::uintptr_t client_base_address{};

        if (snapshot == INVALID_HANDLE_VALUE)
        {
            CloseHandle(snapshot);
        }
        MODULEENTRY32 entry = {};
        entry.dwSize = sizeof(MODULEENTRY32);

        if (!Module32First(snapshot, &entry))
        {
            std::cout << "Could not get the first module!";
        }

        do {
            if (client_name == entry.szModule)
            {
                client_base_address = (std::uintptr_t)entry.modBaseAddr;
            }
        } while (Module32Next(snapshot, &entry));

        if (snapshot)
        {
            CloseHandle(snapshot);
        }

        return client_base_address;
    }
    // make a template to read and write process memory  there you go, now you have a csgo cheat :D 



    template <typename T>
    const T read(const std::uintptr_t& dw_address) const noexcept
    {
        T value = { };
        ReadProcessMemory(process_handle, (LPCVOID)dw_address, &value, sizeof(T), NULL);
        return value;
    }

    template <typename T>
    const void write(const std::uintptr_t& dw_address, const T& value) const noexcept
    {
        WriteProcessMemory(process_handle, (LPVOID)(dw_address), (LPCVOID)&value, sizeof(T), NULL);
    }
    /*template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept
    {
        ::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
    }*/
};