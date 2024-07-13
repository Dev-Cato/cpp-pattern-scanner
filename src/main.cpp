#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>
#include <tuple>

// External library includes
#include "include/httplib.h"

// Custom utility headers
#include "utils/colors.h"
#include "utils/addresses.h"
#include "utils/process.h"
#include "utils/signature.h"
#include "utils/config.h"

// Function to convert any type to string
template<typename T>
std::string to_str(T in) {
    return std::to_string(in);
}

// Function to convert a standard string to a wide string
std::wstring to_wstr(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

// Function to convert a value to hexadecimal string
template<typename T>
std::string to_hex(T in) {
    std::stringstream stream;
    stream << "0x" << std::hex << in;
    return stream.str();
}

// Main function
int main() {
    // Initialize configuration from config.ini
    auto config = init_config();

    // Retrieve process name from configuration and convert to wide string
    std::wstring processName = to_wstr(config["Settings"]["ProcessName"]);

    // Output waiting message for process
    std::wcerr << type::nor << L"Waiting for " << processName << std::endl;
    Sleep(200);

    // Retrieve process ID by name
    DWORD pid = GetProcessIdByName(processName);
    while (pid == 0) {
        pid = GetProcessIdByName(processName);
        Sleep(1000);
    }
    // Process found message
    std::wcerr << type::suc << processName << L" found." << std::endl;



    // Retrieve base module name from configuration and convert to wide string
    std::wstring baseName = to_wstr(config["Settings"]["BaseModule"]);

    // Output message for getting module base address
    std::wcerr << type::nor << L"Getting Module Base Address..." << std::endl;
    Sleep(200);

    // Retrieve module base address by process ID and module name
    DWORD_PTR baseAddress = GetModuleBaseAddress(pid, baseName);
    while (baseAddress == 0) {
        std::wcerr << type::nor << L"Trying to find Base-Module Address!" << std::endl;
        baseAddress = GetModuleBaseAddress(pid, baseName);
        Sleep(1000);
    }
    // Base module address found message
    std::wcerr << type::suc << L"Found Base-Module Address." << std::endl;



    // Output message for opening process
    std::wcerr << type::nor << L"Opening Process..." << std::endl;
    Sleep(200);

    // Open process handle with read and query access
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) {
        // Error message if failed to open process
        std::wcerr << type::err << L"Failed to open Process!" << std::endl;
        std::cin.get();
        return 1;
    }
    // Process opened successfully message
    std::wcerr << type::suc << L"Process opened successfully." << std::endl;



    // Output message for resolving patterns
    std::wcerr << type::nor << L"Resolving Patterns..." << std::endl;
    Sleep(200);

    // Resolve patterns from process memory using signatures from configuration
    auto [all_resolved, jsonString] = resolve_patterns(hProcess, config["Signatures"]);
    if (all_resolved) {
        // Success message if all patterns resolved
        std::wcerr << "\n" << type::suc << L"All Signatures resolved." << std::endl;
    }
    else {
        // Warning message if not all patterns resolved
        std::wcerr << "\n" << type::war << L"Could not resolve all Patterns! Using 0x0 for not found" << std::endl;
    }



    // Output message for starting WebApi
    std::wcerr << type::nor << L"Starting WebApi..." << std::endl;
    Sleep(200);

    // Initialize HTTP server using httplib
    httplib::Server web_server;

    // Set handler for root endpoint to return JSON string of resolved patterns
    web_server.Get("/", [jsonString](const httplib::Request& req, httplib::Response& res) {
        res.set_content(jsonString, "application/json");
        });

    // Print message indicating WebApi is running
    std::cout << type::suc << "WebApi running on http://" << config["WebApi"]["IP"] << ":" << config["WebApi"]["PORT"] << std::endl;

    // Listen on configured IP address and port
    web_server.listen(config["WebApi"]["IP"], std::stoi(config["WebApi"]["PORT"]));



    // Close process handle
    CloseHandle(hProcess);

    return EXIT_SUCCESS;  // Return success
}
