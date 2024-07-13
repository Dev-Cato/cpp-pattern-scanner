#pragma once

// Include necessary headers
#include <tuple>
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
#include "signature.h"

// Typedefs for better code readability
typedef unsigned char BYTE;
typedef uintptr_t DWORD_PTR;

// Structure to store a signature and its corresponding address
struct Signature {
    std::vector<BYTE> sig;  // Vector to store the signature bytes
    DWORD_PTR address;      // Address of the signature
};

// Function to parse signatures from a given map of strings
// The input map contains signature names and their respective data as strings
std::map<std::string, Signature> parseSignatures(const std::map<std::string, std::string>& signatureData) {
    std::map<std::string, Signature> signatures;  // Map to store parsed signatures

    // Iterate through the input map
    for (const auto& [name, value] : signatureData) {
        // Find the position of the delimiter separating the signature and the address
        size_t delimiterPos = value.find(';');

        // Extract the signature string
        std::string sigStr = value.substr(0, delimiterPos);

        // Extract the address and convert it to a DWORD_PTR
        DWORD_PTR address = std::stoul(value.substr(delimiterPos + 1), nullptr, 16);

        // Vector to store the signature bytes
        std::vector<BYTE> sig;
        std::istringstream iss(sigStr);  // Input string stream for the signature string
        std::string byteStr;  // Temporary string to store each byte

        // Split the signature string by commas and convert each part to a byte
        while (std::getline(iss, byteStr, ',')) {
            sig.push_back(static_cast<BYTE>(std::stoul(byteStr, nullptr, 16)));
        }

        // Store the parsed signature and address in the map
        signatures[name] = { sig, address };
    }

    return signatures;  // Return the map of parsed signatures
}

// Function to convert a vector of bytes to a hex string representation
std::string byteVectorToHexString(const std::vector<BYTE>& bytes) {
    std::ostringstream oss;  // Output string stream
    oss << "[";  // Start the JSON array

    // Iterate through the byte vector
    for (size_t i = 0; i < bytes.size(); ++i) {
        // Append each byte in hex format
        oss << "\"0x" << std::hex << (int)bytes[i] << "\"";

        // Add a comma between bytes, except for the last byte
        if (i != bytes.size() - 1) {
            oss << ",";
        }
    }

    oss << "]";  // Close the JSON array
    return oss.str();  // Return the hex string
}

// Function to resolve patterns by finding signatures in a process
std::tuple<bool, std::string> resolve_patterns(HANDLE hProcess, const std::map<std::string, std::string>& signatureData) {
    // Parse the signature data
    std::map<std::string, Signature> signatures = parseSignatures(signatureData);
    std::ostringstream resultJson;  // Output string stream for JSON results
    resultJson << "{";  // Start the JSON object

    bool found_all = true;  // Flag to track if all signatures are found

    // Iterate through the parsed signatures
    for (auto it = signatures.begin(); it != signatures.end(); ++it) {
        std::string name = it->first;  // Get the signature name
        auto& signature = it->second;  // Get the signature structure

        // Convert the signature vector to a byte array for the FindSignature function
        BYTE* sigBytes = signature.sig.data();
        size_t sigSize = signature.sig.size();

        // Call the FindSignature function to get the address of the signature in the process
        signature.address = FindSignature(hProcess, sigBytes, sigSize);

        resultJson << "\"" << name << "\": {";  // Start a JSON object for the current signature

        // Check if the signature was found
        if (signature.address == 0 || signature.address == 0xffffffffffffffff) {
            signature.address = 0x0; // Using 0x0 instead of 0xffffffffffffffff
            found_all = false;  // Set the flag to false as not all signatures are found

            std::cout << type::err << "Signature \"" << name << "\" could not be found!" << std::endl;
            resultJson << "\"status\": \"not_found\",";  // Update JSON status
        }
        else {
            std::cout << type::suc << "Signature \"" << name << "\" found at address 0x" << std::hex << signature.address << std::endl;
            resultJson << "\"status\": \"found\",";  // Update JSON status
        }

        // Append the signature and address to the JSON object
        resultJson << "\"signature\": " << byteVectorToHexString(signature.sig) << ",";
        resultJson << "\"address\": " << "\"0x" << std::hex << signature.address << "\"";
        resultJson << "}";  // Close the JSON object for the current signature

        // Add a comma between signatures, except for the last signature
        if (std::next(it) != signatures.end()) {
            resultJson << ",";
        }
    }

    resultJson << "}";  // Close the JSON object
    return std::make_tuple(found_all, resultJson.str());  // Return the result as a tuple
}
