/**
 * @file multiThreading.cpp
 * @brief Implementation of Shannon encoding using multithreading
 * 
 * This file demonstrates parallel processing concepts through a practical implementation
 * of Shannon encoding. Each input string is processed in a separate thread, showcasing:
 * - Thread creation and management
 * - Data structure synchronization
 * - Parallel algorithm execution
 * - Resource management
 */

#include <iostream>
#include <vector>
#include <pthread.h>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <memory>

namespace {
    // Log levels for better debugging and monitoring
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    // Thread-safe logging function
    void log(LogLevel level, const std::string& message) {
        static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&logMutex);
        
        switch(level) {
            case LogLevel::INFO:
                std::cout << "[INFO] ";
                break;
            case LogLevel::WARNING:
                std::cerr << "[WARNING] ";
                break;
            case LogLevel::ERROR:
                std::cerr << "[ERROR] ";
                break;
        }
        
        std::cout << message << std::endl;
        pthread_mutex_unlock(&logMutex);
    }
}

/**
 * @struct CharCode
 * @brief Stores character encoding information
 * 
 * This structure maintains the relationship between characters and their
 * Shannon encoding properties including frequency and binary code.
 */
struct CharCode {
    char character;          ///< The character being encoded
    int freq;               ///< Frequency of occurrence
    std::string code;       ///< Generated Shannon code
};

/**
 * @struct EncodedMsg
 * @brief Container for message encoding data
 * 
 * Holds all necessary data for encoding a message, including the original
 * message, character frequencies, and the final encoded result.
 */
struct EncodedMsg {
    std::string line;                   ///< Original input message
    std::vector<CharCode> charCodeVec;  ///< Vector of character encodings
    std::string encodedLine;            ///< Final encoded message
};

/**
 * @brief Comparison function for sorting CharCode objects
 * @param a First CharCode object
 * @param b Second CharCode object
 * @return true if a should come before b in sorted order
 */
bool compareFreqChar(const CharCode& a, const CharCode& b) {
    return (a.freq != b.freq) ? (a.freq > b.freq) : (a.character > b.character);
}

/**
 * @brief Converts a decimal probability to binary representation
 * @param decimal The decimal number to convert
 * @param precision The number of binary digits to generate
 * @return Binary string representation
 */
std::string decimalToBinary(float decimal, int precision) {
    if (precision < 0) {
        throw std::invalid_argument("Precision cannot be negative");
    }
    
    std::string binary;
    binary.reserve(precision); // Optimize string operations

    while (decimal > 0 && precision > 0) {
        double temp = decimal * 2;
        binary += (temp >= 1) ? '1' : '0';
        decimal = (temp >= 1) ? (temp - 1) : temp;
        --precision;
    }

    // Pad remaining positions with zeros
    binary.append(precision, '0');
    return binary;
}

/**
 * @brief Thread function for Shannon encoding
 * @param void_ptr Pointer to EncodedMsg structure
 * @return nullptr
 */
void* shannonCode(void* void_ptr) {
    try {
        EncodedMsg* curr_ptr = static_cast<EncodedMsg*>(void_ptr);
        if (!curr_ptr) {
            throw std::runtime_error("Invalid thread argument");
        }

        log(LogLevel::INFO, "Starting Shannon encoding for thread");
        
        const int lineSize = curr_ptr->line.length();
        std::map<char, int> charCountMap;

        // Count character frequencies
        for (char c : curr_ptr->line) {
            ++charCountMap[c];
        }

        // Prepare character codes
        curr_ptr->charCodeVec.reserve(charCountMap.size());
        for (const auto& [character, freq] : charCountMap) {
            curr_ptr->charCodeVec.push_back({character, freq, ""});
        }

        // Sort by frequency and character
        std::sort(curr_ptr->charCodeVec.begin(), curr_ptr->charCodeVec.end(), compareFreqChar);

        // Calculate Shannon codes
        float cumulativeProbability = 0;
        std::map<char, std::string> charCodeMap;

        for (auto& charCode : curr_ptr->charCodeVec) {
            float probability = static_cast<float>(charCode.freq) / lineSize;
            int precision = std::ceil(std::log2(1.0f/probability));
            
            charCode.code = decimalToBinary(cumulativeProbability, precision);
            charCodeMap[charCode.character] = charCode.code;
            cumulativeProbability += probability;
        }

        // Generate encoded message
        curr_ptr->encodedLine.reserve(curr_ptr->line.length() * 8); // Estimate capacity
        for (char c : curr_ptr->line) {
            curr_ptr->encodedLine += charCodeMap[c];
        }

        log(LogLevel::INFO, "Completed Shannon encoding for thread");
        return nullptr;

    } catch (const std::exception& e) {
        log(LogLevel::ERROR, std::string("Thread error: ") + e.what());
        return nullptr;
    }
}

int main() {
    try {
        log(LogLevel::INFO, "Starting Shannon encoding program");

        std::vector<EncodedMsg> threadData;
        EncodedMsg input;
        
        // Read input
        while (std::getline(std::cin, input.line)) {
            if (!input.line.empty()) {
                threadData.push_back(input);
            }
        }

        if (threadData.empty()) {
            log(LogLevel::WARNING, "No valid input provided");
            return 0;
        }

        const int threadSize = threadData.size();
        log(LogLevel::INFO, "Processing " + std::to_string(threadSize) + " messages");

        // Create and manage threads
        std::vector<pthread_t> threads(threadSize);

        for (int i = 0; i < threadSize; i++) {
            if (pthread_create(&threads[i], nullptr, shannonCode, &threadData[i])) {
                throw std::runtime_error("Failed to create thread " + std::to_string(i));
            }
            log(LogLevel::INFO, "Created thread " + std::to_string(i));
        }

        // Join threads
        for (int i = 0; i < threadSize; i++) {
            if (pthread_join(threads[i], nullptr)) {
                throw std::runtime_error("Failed to join thread " + std::to_string(i));
            }
            log(LogLevel::INFO, "Joined thread " + std::to_string(i));
        }

        // Output results
        for (const auto& data : threadData) {
            std::cout << "\nMessage: " << data.line << "\n\nAlphabet:\n";
            
            for (const auto& charCode : data.charCodeVec) {
                std::cout << "Symbol: " << charCode.character
                         << ", Frequency: " << charCode.freq
                         << ", Shannon code: " << charCode.code << '\n';
            }
            
            std::cout << "\nEncoded message: " << data.encodedLine << "\n\n";
        }

        log(LogLevel::INFO, "Program completed successfully");
        return 0;

    } catch (const std::exception& e) {
        log(LogLevel::ERROR, std::string("Program error: ") + e.what());
        return 1;
    }
}