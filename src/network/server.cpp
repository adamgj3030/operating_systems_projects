/**
 * @file server.cpp
 * @brief Server implementation for Shannon encoding service
 * 
 * This server demonstrates:
 * - Socket programming
 * - Concurrent client handling through forking
 * - Shannon encoding algorithm
 * - Resource management and cleanup
 */

#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <signal.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include <sys/wait.h>
#include <stdexcept>

// Constants for server configuration
namespace ServerConfig {
    constexpr int MAX_CONNECTIONS = 5;
    constexpr int BUFFER_SIZE = 32;
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
 * @struct EncodedMsg
 * @brief Message encoding container
 */
struct EncodedMsg {
    std::string line;
    std::vector<CharCode> charCodeVec;
    std::string encodedLine;
};

/**
 * @brief Compare function for sorting CharCodes
 */
bool compareFreqChar(const CharCode& a, const CharCode& b) {
    if (a.freq != b.freq)
        return a.freq > b.freq;
    return a.character > b.character;
}

/**
 * @brief Convert decimal to binary with specified precision
 */
std::string decimalToBinary(float decimal, int precision) {
    if (precision < 0) throw std::invalid_argument("Negative precision");
    
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
 * @brief Shannon encoding implementation
 */
void shannonCode(EncodedMsg& msg) {
    try {
        int lineSize = msg.line.length();
        std::map<char, int> charCountMap;
        
        // Count frequencies
        for (char c : msg.line) {
            ++charCountMap[c];
        }
        
        // Prepare character codes
        msg.charCodeVec.reserve(charCountMap.size());
        for (const auto& [character, freq] : charCountMap) {
            msg.charCodeVec.push_back({character, freq, ""});
        }
        
        std::sort(msg.charCodeVec.begin(), msg.charCodeVec.end(), compareFreqChar);
        
        // Calculate Shannon codes
        float cumulativeProbability = 0;
        std::map<char, std::string> charCodeMap;
        
        for (auto& charCode : msg.charCodeVec) {
            float probability = static_cast<float>(charCode.freq) / lineSize;
            int precision = std::ceil(std::log2(1.0f/probability));
            
            charCode.code = decimalToBinary(cumulativeProbability, precision);
            charCodeMap[charCode.character] = charCode.code;
            cumulativeProbability += probability;
        }
        
        // Generate encoded message
        msg.encodedLine.reserve(lineSize * 8);
        for (char c : msg.line) {
            msg.encodedLine += charCodeMap[c];
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error in Shannon encoding: " << e.what() << std::endl;
        throw;
    }
}

/**
 * @brief Zombie process cleanup handler
 */
void fireman(int) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * @class Server
 * @brief Encapsulates server functionality
 */
class Server {
private:
    int sockfd;
    int portno;
    sockaddr_in serv_addr;
    
    void setupSocket() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Error opening socket");
        }
        
        bzero((char*)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        
        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            throw std::runtime_error("Error on binding");
        }
    }
    
    void handleClient(int newsockfd) {
        char message[ServerConfig::BUFFER_SIZE];
        int n = read(newsockfd, message, sizeof(message));
        if (n < 0) throw std::runtime_error("Error reading from socket");
        
        EncodedMsg msg;
        msg.line = message;
        shannonCode(msg);
        
        // Send character vector size
        int charCodeVecSize = msg.charCodeVec.size();
        n = write(newsockfd, &charCodeVecSize, sizeof(int));
        if (n < 0) throw std::runtime_error("Error writing to socket");
        
        // Send character data
        for (const auto& charCode : msg.charCodeVec) {
            write(newsockfd, &charCode.character, sizeof(char));
            write(newsockfd, &charCode.freq, sizeof(int));
            
            int codeLength = charCode.code.size();
            write(newsockfd, &codeLength, sizeof(int));
            write(newsockfd, charCode.code.c_str(), codeLength);
        }
        
        // Send encoded message
        int encodedSize = msg.encodedLine.size();
        write(newsockfd, &encodedSize, sizeof(int));
        write(newsockfd, msg.encodedLine.c_str(), encodedSize);
    }
    
public:
    Server(int port) : portno(port) {
        setupSocket();
    }
    
    ~Server() {
        close(sockfd);
    }
    
    void run() {
        listen(sockfd, ServerConfig::MAX_CONNECTIONS);
        signal(SIGCHLD, fireman);
        
        std::cout << "Server running on port " << portno << std::endl;
        
        while (true) {
            sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);
            
            int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
            if (newsockfd < 0) {
                std::cerr << "Error on accept" << std::endl;
                continue;
            }
            
            if (fork() == 0) {
                try {
                    close(sockfd);  // Child doesn't need the listening socket
                    handleClient(newsockfd);
                    close(newsockfd);
                    _exit(0);
                } catch (const std::exception& e) {
                    std::cerr << "Client handling error: " << e.what() << std::endl;
                    close(newsockfd);
                    _exit(1);
                }
            }
            
            close(newsockfd);  // Parent doesn't need the connected socket
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            throw std::runtime_error("Usage: " + std::string(argv[0]) + " <port>");
        }
        
        Server server(std::stoi(argv[1]));
        server.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}