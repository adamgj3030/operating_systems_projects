/**
 * @file client.cpp
 * @brief Client implementation for Shannon encoding service
 * 
 * This client demonstrates:
 * - Socket programming
 * - Multithreaded network communication
 * - Error handling and resource management
 * - Thread synchronization
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <pthread.h>
#include <vector>
#include <stdexcept>
#include <memory>

// Configuration constants
namespace ClientConfig {
    constexpr int BUFFER_SIZE = 32;
    constexpr int MAX_RETRIES = 3;
}

/**
 * @struct CharCode
 * @brief Stores character encoding information
 */
struct CharCode {
    char character;
    int freq;
    std::string code;
};

/**
 * @struct ThreadData
 * @brief Thread communication data structure
 */
struct ThreadData {
    int portno;
    std::string hostname;
    std::string line;
    std::string encodedLine;
    std::vector<CharCode> charCodeVec;
    
    ThreadData(const std::string& host, int port) 
        : portno(port), hostname(host) {}
};

/**
 * @class NetworkClient
 * @brief Handles network communication for a single thread
 */
class NetworkClient {
private:
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    
    void connectToServer(const std::string& hostname, int portno) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Error opening socket");
        }
        
        server = gethostbyname(hostname.c_str());
        if (!server) {
            throw std::runtime_error("Error: no such host");
        }
        
        bzero((char*)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno);
        
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            throw std::runtime_error("Error connecting to server");
        }
    }
    
public:
    NetworkClient(const std::string& hostname, int portno) {
        connectToServer(hostname, portno);
    }
    
    ~NetworkClient() {
        if (sockfd >= 0) {
            close(sockfd);
        }
    }
    
    void sendMessage(const std::string& message) {
        char buffer[ClientConfig::BUFFER_SIZE];
        strncpy(buffer, message.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        
        if (write(sockfd, buffer, sizeof(buffer)) < 0) {
            throw std::runtime_error("Error writing to socket");
        }
    }
    
    void receiveResponse(ThreadData& data) {
        // Receive character vector size
        int charCodeVecSize;
        if (read(sockfd, &charCodeVecSize, sizeof(int)) < 0) {
            throw std::runtime_error("Error reading from socket");
        }
        
        // Receive character data
        data.charCodeVec.clear();
        data.charCodeVec.reserve(charCodeVecSize);
        
        for (int i = 0; i < charCodeVecSize; ++i) {
            CharCode charCode;
            
            // Read character
            if (read(sockfd, &charCode.character, sizeof(char)) < 0) {
                throw std::runtime_error("Error reading character from socket");
            }
            
            // Read frequency
            if (read(sockfd, &charCode.freq, sizeof(int)) < 0) {
                throw std::runtime_error("Error reading frequency from socket");
            }
            
            // Read code length
            int codeLength;
            if (read(sockfd, &codeLength, sizeof(int)) < 0) {
                throw std::runtime_error("Error reading code length from socket");
            }
            
            // Read code
            std::vector<char> codeBuffer(codeLength + 1);
            if (read(sockfd, codeBuffer.data(), codeLength) < 0) {
                throw std::runtime_error("Error reading code from socket");
            }
            codeBuffer[codeLength] = '\0';
            charCode.code = std::string(codeBuffer.data());
            
            data.charCodeVec.push_back(charCode);
        }
        
        // Receive encoded message size
        int encodedSize;
        if (read(sockfd, &encodedSize, sizeof(int)) < 0) {
            throw std::runtime_error("Error reading encoded size from socket");
        }
        
        // Receive encoded message
        std::vector<char> encodedBuffer(encodedSize + 1);
        if (read(sockfd, encodedBuffer.data(), encodedSize) < 0) {
            throw std::runtime_error("Error reading encoded message from socket");
        }
        encodedBuffer[encodedSize] = '\0';
        data.encodedLine = std::string(encodedBuffer.data());
    }
};

/**
 * @brief Thread function for communicating with server
 */
void* communicateWithServer(void* void_ptr) {
    ThreadData* data = static_cast<ThreadData*>(void_ptr);
    
    try {
        NetworkClient client(data->hostname, data->portno);
        client.sendMessage(data->line);
        client.receiveResponse(*data);
    } catch (const std::exception& e) {
        std::cerr << "Thread error: " << e.what() << std::endl;
    }
    
    return nullptr;
}

/**
 * @brief Display encoding results
 */
void displayResults(const std::vector<ThreadData>& threadData) {
    for (const auto& data : threadData) {
        std::cout << "\nMessage: " << data.line << "\n\nAlphabet:\n";
        
        for (const auto& charCode : data.charCodeVec) {
            std::cout << "Symbol: " << charCode.character
                     << ", Frequency: " << charCode.freq
                     << ", Shannon code: " << charCode.code << '\n';
        }
        
        std::cout << "\nEncoded message: " << data.encodedLine << "\n\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            throw std::runtime_error("Usage: " + std::string(argv[0]) + " hostname port");
        }
        
        const std::string hostname = argv[1];
        const int portno = std::stoi(argv[2]);
        
        // Read input lines
        std::vector<ThreadData> threadData;
        std::string line;
        
        while (std::getline(std::cin, line)) {
            if (!line.empty()) {
                threadData.emplace_back(hostname, portno);
                threadData.back().line = line;
            }
        }
        
        if (threadData.empty()) {
            std::cout << "No input provided." << std::endl;
            return 0;
        }
        
        // Create threads
        std::vector<pthread_t> threads(threadData.size());
        
        for (size_t i = 0; i < threadData.size(); ++i) {
            if (pthread_create(&threads[i], nullptr, communicateWithServer, &threadData[i])) {
                throw std::runtime_error("Error creating thread " + std::to_string(i));
            }
        }
        
        // Join threads
        for (size_t i = 0; i < threads.size(); ++i) {
            if (pthread_join(threads[i], nullptr)) {
                throw std::runtime_error("Error joining thread " + std::to_string(i));
            }
        }
        
        // Display results
        displayResults(threadData);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}