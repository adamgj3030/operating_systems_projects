#include <iostream>
#include <vector>
#include <pthread.h>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>

// Struct to store frequency and binaryCode for a said character
struct CharCode
{
    char character;
    int freq;
    std::string code;
};

// Struct to store the line, the vector of CharCode, and the encoded line for each thread
struct EncodedMsg
{
    std::string line;
    std::vector<CharCode> charCodeVec;
    std::string encodedLine;
};

// Used for sorting function, Compares frequency first then if equal, compares character 
bool compareFreqChar(CharCode a, CharCode b) 
{
    // first compares frequencies
    if (a.freq != b.freq)
        return a.freq > b.freq;
    
    // if frequencies are even, then compares characters
    return a.character > b.character;
}

// converts the probability to its binary equivalent to a certain precision
std::string decimalToBinary(float decimal, int precision) 
{
    // holds the finished binary code
    std::string binary = "";

    // converts the decimal until it is 0 OR precision reaches 0
    while (decimal > 0 && precision > 0) 
    {
        double temp = decimal * 2;

        // adds a 1 if a 1 is in the one's place
        if (temp >= 1) 
        {
            binary += "1";
            decimal = temp - 1;
        }

        // adds a 0 if a 0 is in the one's place
        else 
        {
            binary += "0";
            decimal = temp;
        }
        --precision;
    }

    // Adds any missing 0's due to previous loop terminating from the decimal reaching 0 first
    while (precision > 0) 
    {
        --precision;
        binary += "0";
    }

    return binary;
}


void *shannonCode(void *void_ptr) 
{
    // change the void_ptr into a string line_ptr
    EncodedMsg *curr_ptr = (EncodedMsg *) void_ptr;
    
    int lineSize = (curr_ptr->line).length();

    // create a mapping to store each unique character and count their appearances 
    std::map<char, int> charCountMap;

    // iterate through each character for the given line
    // NOTE: COULD skip the mapping part and do this in the charCode struct 
    for (int i = 0; i < lineSize; ++i) 
    {
        // Stores unqiue characters and their associated frequencies from the line
        ++charCountMap[curr_ptr->line[i]];
    }

    // vector to sort the frequencies 
    CharCode temp;
    for (const auto& charCount: charCountMap)
    {
        temp.character = charCount.first;
        temp.freq = charCount.second;
        // each entry in the charCodeVec holds its character and associated frequency so far
        curr_ptr->charCodeVec.push_back(temp);
    }

    // sorts the charCodeVec based on frequencies, and if equal then characters
    std::sort(curr_ptr->charCodeVec.begin(), curr_ptr->charCodeVec.end(), compareFreqChar);

    // Keeps track of the Cumulative Probability as we iterate through the charCode vector
    float cumulativeProbability = 0;

    // create a new mapping to hold the characters and their associated binary codes
    std::map<char, std::string> charCodeMap;

    // finds the binary code for each character based on its frequency/probability
    for (auto& charCode : curr_ptr->charCodeVec) 
    {
        // probability = frequency / total freq (total freq is just the length of the line)
        float probability = ((float)charCode.freq/lineSize);

        // precision = ceiling of log base 2 (1/probability)
        int precision = ceil(log2(1/probability));

        // maps the character to its associated code
        std::string ShannonCode = decimalToBinary(cumulativeProbability, precision);
        charCode.code = ShannonCode;
        charCodeMap[charCode.character] = ShannonCode;

        // adds current probability to the total
        cumulativeProbability += probability;
    }

    // holds the finished encoded line
    curr_ptr->encodedLine = "";

    // iterates over the intial line
    for (int i = 0; i < lineSize; ++i) {
        // Adds each encountered characters associated shannon code to the finished encoded line 
        curr_ptr->encodedLine += charCodeMap[curr_ptr->line[i]];
    }

    return nullptr;
}


int main() 
{
    // store each line into a different initialization of a vector of struct EncodedMsg
    EncodedMsg input;
    std::vector<EncodedMsg> threadData;

    // take in the input 
    while(std::getline(std::cin, input.line)) {
        threadData.push_back(input);
    }
    
    int threadSize = threadData.size();

    // store the thread id in tid
    pthread_t *tid = new pthread_t[threadSize];

    // iterate through the vector
    for (int i = 0; i < threadSize; i++)
    {
        // create a thread for each EncodedMsg struct of the vector, and if an error is thrown return 1;
        if(pthread_create(&tid[i], nullptr, shannonCode, &threadData[i]))
        {
            std::cerr << "Error creating thread" << std::endl;
			return 1;
        }
    }

    // join threads back together when they are finished
    for (int i = 0; i < threadSize; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    // prints out the shannon code information for each thread
    for (int i = 0; i < threadSize; i++) {
        EncodedMsg currData = threadData[i];

        std::cout << "Message: " << currData.line << std::endl;
        std::cout << std::endl;

        std::cout << "Alphabet:" << std::endl;

        // prints out the each symbol in the line and their associated frequency and shannon code 
        for (const auto& charCode : currData.charCodeVec) 
        {
            std::cout << "Symbol: "<< charCode.character
            << ", Frequency: " << charCode.freq 
            << ", Shannon code: " << charCode.code << std::endl;
        }

        std::cout << std::endl;

        std::cout << "Encoded message: " << currData.encodedLine << std::endl;

        std::cout << std::endl;

    }

    return 0;
}