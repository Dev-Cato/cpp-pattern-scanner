#pragma once

// Windows headers for process and module handling
#include <windows.h>
#include <tlhelp32.h>

// Include necessary headers
#include <iostream>
#include <vector>

// Function to get the process ID by its name
DWORD GetProcessIdByName(const std::wstring& processName) {
    PROCESSENTRY32 pe32;  // Process entry structure
    pe32.dwSize = sizeof(PROCESSENTRY32);  // Set the size of the structure
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // Take a snapshot of all processes

    // Iterate through the processes in the snapshot
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Compare process name with the name of the current process in the iteration
            if (processName == pe32.szExeFile) {
                CloseHandle(hSnapshot);  // Close the snapshot handle
                return pe32.th32ProcessID;  // Return the process ID
            }
        } while (Process32Next(hSnapshot, &pe32));  // Move to the next process in the snapshot
    }

    CloseHandle(hSnapshot);  // Close the snapshot handle
    return 0;  // Return 0 if process ID is not found
}

// Function to get the base address of the main module of a process
DWORD_PTR GetModuleBaseAddress(DWORD processId, const std::wstring& moduleName) {
    MODULEENTRY32 me32;  // Module entry structure
    me32.dwSize = sizeof(MODULEENTRY32);  // Set the size of the structure
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);  // Take a snapshot of modules in a process

    // Iterate through the modules in the snapshot
    if (Module32First(hSnapshot, &me32)) {
        do {
            // Compare module name with the name of the current module in the iteration
            if (moduleName == me32.szModule) {
                CloseHandle(hSnapshot);  // Close the snapshot handle
                return reinterpret_cast<DWORD_PTR>(me32.modBaseAddr);  // Return the base address of the module
            }
        } while (Module32Next(hSnapshot, &me32));  // Move to the next module in the snapshot
    }

    CloseHandle(hSnapshot);  // Close the snapshot handle
    return 0;  // Return 0 if module base address is not found
}
