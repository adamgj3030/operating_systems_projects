/**
 * @file mutex.cpp
 * @brief Implementation of thread synchronization using mutexes
 * 
 * This file demonstrates:
 * - Mutex-based thread synchronization
 * - Critical section management
 * - Condition variables
 * - Thread-safe data structures
 * - Shannon encoding algorithm with synchronized output
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

// Configuration namespace
namespace Config {
    constexpr int MAX_THREADS = 1000;
    constexpr bool ENABLE_LOGGING = true;
}

// Thread-safe logging
namespace Logger {
    static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;
    
    void log(const std::string& message) {
        if (Config::ENABLE_LOGGING) {
            pthread_mutex_lock(&logMutex);
            std::cout << "[LOG] " << message << std::endl;
            pthread_mutex_unlock(&logMutex);
        }
    }
}

/**
 * @struct CharCode
 * @brief Character encoding information
 */
struct CharCode {
    char character;
    int freq;
    std::string code;
};

/**
 * @struct SharedData
 * @brief Thread-shared data with synchronization primitives
 */
struct SharedData {
    std::string line;            // Input line to process
    int id;                      // Thread ID
    int* counter;                // Shared counter for synchronization
    pthread_mutex_t* mutex1;     // First mutex for critical section
    pthread_mutex_t* mutex2;     // Second mutex for output synchronization
    pthread_cond_t* condition;   // Condition variable for thread coordination
    
    SharedData() : id(0), counter(nullptr), mutex1(nullptr), mutex2(nullptr), condition(nullptr) {}
};

/**
 * @brief Comparison function for CharCode sorting
 */
bool compareFreqChar(const CharCode& a, const CharCode& b) {
    return (a.freq != b.freq) ? (a.freq > b.freq) : (a.character > b.character);
}

/**
 * @brief Convert decimal to binary with specified precision
 */
std::string decimalToBinary(float decimal, int precision) {
    if (precision < 0) {
        throw std::invalid_argument("Precision cannot be negative");
    }
    
    std::string binary;
    binary.reserve(precision);
    
    while (decimal > 0 && precision > 0) {
        double temp = decimal * 2;
        binary += (temp >= 1) ? '1' : '0';
        decimal = (temp >= 1) ? (temp - 1) : temp;
        --precision;
    }
    
    binary.append(precision, '0');
    return binary;
}

/**
 * @class ShannonEncoder
 * @brief Handles Shannon encoding with thread synchronization
 */
class ShannonEncoder {
private:
    std::vector<CharCode> charCodeVec;
    std::map<char, std::string> charCodeMap;
    std::string encodedLine;
    const std::string& input;
    
    void calculateFrequencies() {
        std::map<char, int> charCountMap;
        for (char c : input) {
            ++charCountMap[c];
        }
        
        charCodeVec.reserve(charCountMap.size());
        for (const auto& [character, freq] : charCountMap) {
            charCodeVec.push_back({character, freq, ""});
        }
        
        std::sort(charCodeVec.begin(), charCodeVec.end(), compareFreqChar);
    }
    
    void generateCodes() {
        float cumulativeProbability = 0;
        const int lineSize = input.length();
        
        for (auto& charCode : charCodeVec) {
            float probability = static_cast<float>(charCode.freq) / lineSize;
            int precision = std::ceil(std::log2(1.0f/probability));
            
            charCode.code = decimalToBinary(cumulativeProbability, precision);
            charCodeMap[charCode.character] = charCode.code;
            cumulativeProbability += probability;
        }
    }
    
    void encodeMessage() {
        encodedLine.reserve(input.length() * 8);
        for (char c : input) {
            encodedLine += charCodeMap[c];
        }
    }
    
public:
    explicit ShannonEncoder(const std::string& inputLine) : input(inputLine) {}
    
    void encode() {
        calculateFrequencies();
        generateCodes();
        encodeMessage();
    }
    
    void displayResults() const {
        std::cout << "Message: " << input << std::endl << std::endl;
        std::cout << "Alphabet:" << std::endl;
        
        for (const auto& charCode : charCodeVec) {
            std::cout << "Symbol: " << charCode.character
                     << ", Frequency: " << charCode.freq
                     << ", Shannon code: " << charCode.code << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "Encoded message: " << encodedLine << std::endl << std::endl;
    }
};

/**
 * @brief Thread function for Shannon encoding
 */
void* shannonCode(void* void_ptr) {
    try {
        SharedData* data = static_cast<SharedData*>(void_ptr);
        if (!data) throw std::runtime_error("Invalid thread data");
        
        // Store local copies of shared data
        std::string localLine = data->line;
        int localId = data->id;
        
        // Unlock first mutex to allow other threads to start
        Logger::log("Thread " + std::to_string(localId) + " starting processing");
        pthread_mutex_unlock(data->mutex1);
        
        // Process the line
        ShannonEncoder encoder(localLine);
        encoder.encode();
        
        // Synchronize output using second mutex and condition variable
        pthread_mutex_lock(data->mutex2);
        while (localId != *(data->counter)) {
            pthread_cond_wait(data->condition, data->mutex2);
        }
        pthread_mutex_unlock(data->mutex2);
        
        // Display results in synchronized manner
        encoder.displayResults();
        
        // Update counter and signal next thread
        pthread_mutex_lock(data->mutex2);
        (*(data->counter))++;
        pthread_cond_broadcast(data->condition);
        pthread_mutex_unlock(data->mutex2);
        
        Logger::log("Thread " + std::to_string(localId) + " completed");
        return nullptr;
        
    } catch (const std::exception& e) {
        Logger::log("Thread error: " + std::string(e.what()));
        return nullptr;
    }
}

int main() {
    try {
        // Initialize synchronization primitives
        pthread_mutex_t mutex1;
        pthread_mutex_t mutex2;
        pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
        pthread_mutex_init(&mutex1, nullptr);
        pthread_mutex_init(&mutex2, nullptr);
        
        int counter = 0;
        std::vector<std::string> inputLines;
        std::string line;
        
        // Read input
        while (std::getline(std::cin, line)) {
            if (!line.empty()) {
                inputLines.push_back(line);
            }
        }
        
        if (inputLines.empty()) {
            std::cout << "No input provided." << std::endl;
            return 0;
        }
        
        // Initialize shared data
        SharedData sharedData;
        sharedData.counter = &counter;
        sharedData.mutex1 = &mutex1;
        sharedData.mutex2 = &mutex2;
        sharedData.condition = &condition;
        
        // Create threads
        std::vector<pthread_t> threads;
        threads.reserve(inputLines.size());
        
        for (size_t i = 0; i < inputLines.size(); ++i) {
            pthread_t tid;
            pthread_mutex_lock(&mutex1);
            
            sharedData.line = inputLines[i];
            sharedData.id = i;
            
            if (pthread_create(&tid, nullptr, shannonCode, &sharedData)) {
                throw std::runtime_error("Failed to create thread " + std::to_string(i));
            }
            
            threads.push_back(tid);
        }
        
        // Join threads
        for (pthread_t tid : threads) {
            pthread_join(tid, nullptr);
        }
        
        // Cleanup
        pthread_mutex_destroy(&mutex1);
        pthread_mutex_destroy(&mutex2);
        pthread_cond_destroy(&condition);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}