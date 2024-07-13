#pragma once

// Windows headers for process and memory management
#include <windows.h>
#include <tlhelp32.h>

// Include necessary headers
#include <iostream>
#include <vector>

// Function to find a signature in a process memory
DWORD_PTR FindSignature(HANDLE hProcess, BYTE* signature, size_t sigSize) {
    SYSTEM_INFO si;  // System information structure
    GetSystemInfo(&si);  // Retrieve system information
    MEMORY_BASIC_INFORMATION mbi;  // Memory basic information structure
    DWORD_PTR address = (DWORD_PTR)si.lpMinimumApplicationAddress;  // Start address for scanning
    DWORD_PTR end = (DWORD_PTR)si.lpMaximumApplicationAddress;  // End address for scanning

    // Iterate through memory regions of the process
    while (address < end) {
        // Retrieve information about the current memory region
        if (VirtualQueryEx(hProcess, (LPCVOID)address, &mbi, sizeof(mbi))) {
            // Check if the memory region is committed and accessible
            if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_GUARD) == 0 && (mbi.Protect & PAGE_NOACCESS) == 0) {
                // Allocate a buffer to read the memory region
                std::vector<BYTE> buffer(mbi.RegionSize);
                SIZE_T bytesRead;

                // Read the memory region into the buffer
                if (ReadProcessMemory(hProcess, (LPCVOID)address, buffer.data(), mbi.RegionSize, &bytesRead)) {
                    // Search for the signature in the buffer
                    for (size_t i = 0; i < bytesRead - sigSize; ++i) {
                        bool found = true;
                        // Compare each byte of the signature with the corresponding bytes in the buffer
                        for (size_t j = 0; j < sigSize; ++j) {
                            if (signature[j] != 0x00 && signature[j] != buffer[i + j]) {
                                found = false;
                                break;
                            }
                        }
                        // If the signature is found, return the address where it starts
                        if (found) {
                            return address + i;
                        }
                    }
                }
            }
            // Move to the next memory region
            address += mbi.RegionSize;
        }
        else {
            // If VirtualQueryEx fails, move to the next page
            address += si.dwPageSize;
        }
    }

    // Return -1 if the signature is not found
    return -1;
}
