#pragma once

// Include necessary headers
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <filesystem>

// Include custom headers
#include "colors.h"

// Function to trim leading and trailing spaces from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');  // Find the first non-space character
    size_t last = str.find_last_not_of(' ');    // Find the last non-space character
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

// Function to parse an INI file and return its data as a nested map
std::map<std::string, std::map<std::string, std::string>> parseINI(const std::string& filename) {
    std::ifstream file(filename);  // Open the input file stream
    std::string line;              // String to store each line read from the file
    std::string section;           // Current section in the INI file
    std::map<std::string, std::map<std::string, std::string>> iniData;  // Map to store parsed INI data

    // Read each line from the file
    while (std::getline(file, line)) {
        line = trim(line);  // Trim leading and trailing spaces from the line

        // Ignore empty lines and comments (lines starting with ';' or '#')
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        // Check if the line defines a section ([SectionName])
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);  // Extract the section name
        }
        else {
            // Find the position of the '=' character to split key and value
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                // Extract key and value, trimming any spaces around them
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));

                // Store key-value pair in the appropriate section of the map
                iniData[section][key] = value;
            }
        }
    }

    file.close();  // Close the file stream
    return iniData;  // Return the parsed INI data
}

// Function to write INI data to a file
void writeINI(const std::string& filename, const std::map<std::string, std::map<std::string, std::string>>& iniData) {
    std::ofstream file(filename);  // Open the output file stream

    // Iterate through sections in iniData
    for (const auto& section : iniData) {
        file << "[" << section.first << "]\n";  // Write section header
        // Iterate through key-value pairs in each section
        for (const auto& pair : section.second) {
            file << pair.first << " = " << pair.second << "\n";  // Write key-value pair
        }
        file << "\n";  // Blank line between sections
    }

    file.close();  // Close the file stream
}

// Function to create a default INI file if it does not exist
void createDefaultINI(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {  // Check if the file does not exist
        // Initialize default INI data
        std::map<std::string, std::map<std::string, std::string>> iniData;

        iniData["Settings"]["ProcessName"] = "app.exe";
        iniData["Settings"]["BaseModule"] = "module.dll";

        iniData["WebApi"]["IP"] = "127.0.0.1";
        iniData["WebApi"]["PORT"] = "8080";

        iniData["Signatures"]["Sample_1"] = "F8,01,74,04,83,65";
        iniData["Signatures"]["Sample_2"] = "5E,5F,5D,C3,A1,00,00,00,00,89,00,04";

        // Write default INI data to the file
        writeINI(filename, iniData);

        // Print a message indicating that the default configuration file was created
        std::cout << type::inf << "Default configuration file created: " << filename << std::endl;
        std::cout << type::inf << "Please configurate the config.ini and restart the Script." << std::endl;
        std::cout << type::nor << "Press ENTER to exit!" << std::endl;
        std::cin.get();
        exit(1);
    }
}

// Function to initialize configuration by creating default INI file if necessary and parsing it
auto init_config() {
    std::string filename = "config.ini";  // Default INI file name

    // Create default config.ini if it does not exist
    createDefaultINI(filename);

    // Read config.ini file and return its parsed data
    auto iniData = parseINI(filename);

    return iniData;  // Return parsed INI data
}
