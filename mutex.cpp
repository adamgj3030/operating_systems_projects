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

// struct to store the shared data of the threads, including the line, id, counter, mutex1, mutex2, and condition
struct SharedData {
    std::string line;
    int id;
    int* counter;
    pthread_mutex_t* mutex1;
    pthread_mutex_t* mutex2;
    pthread_cond_t* condition;
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
    SharedData *data = (SharedData *) void_ptr;

    // copy the data into local variables
    std::string line = data->line;
    int localID = data->id;

    // unlock mutex1 to allow other threads to set their data
    pthread_mutex_unlock(data->mutex1);

    int lineSize = line.length();

    // create a mapping to store each unique character and count their appearances 
    std::map<char, int> charCountMap;

    // iterate through each character for the given line
    for (int i = 0; i < lineSize; ++i)
    {
        // Stores unqiue characters and their associated frequencies from the line
        ++charCountMap[line[i]];
    }

    // vector to store CharCode
    std::vector<CharCode> charCodeVec;

    CharCode temp;
    for (const auto& charCount: charCountMap)
    {
        temp.character = charCount.first;
        temp.freq = charCount.second;
        // each entry in the charCodeVec holds its character and associated frequency so far
        charCodeVec.push_back(temp);
    }

    // sorts the charCodeVec based on frequencies, and if equal then characters
    std::sort(charCodeVec.begin(), charCodeVec.end(), compareFreqChar);

    // Keeps track of the Cumulative Probability as we iterate through the charCode vector
    float cumulativeProbability = 0;

    // create a new mapping to hold the characters and their associated binary codes
    std::map<char, std::string> charCodeMap;

    // finds the binary code for each character based on its frequency/probability
    for (auto& charCode : charCodeVec) 
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
    std::string encodedLine = "";

    // iterates over the intial line
    for (int i = 0; i < lineSize; ++i) {
        // Adds each encountered characters associated shannon code to the finished encoded line 
        encodedLine += charCodeMap[line[i]];
    }

    pthread_mutex_lock(data->mutex2);

    // Before printing, synchronize to ensure threads print in order
    while (localID != *(data->counter)) {
        pthread_cond_wait(data->condition, data->mutex2);
    }

    pthread_mutex_unlock(data->mutex2);

    // prints out the shannon code information for each thread
    std::cout << "Message: " << line << std::endl;
    std::cout << std::endl;

    std::cout << "Alphabet:" << std::endl;

    // prints out the each symbol in the line and their associated frequency and shannon code 
    for (const auto& charCode : charCodeVec) {
        std::cout << "Symbol: " << charCode.character
        << ", Frequency: " << charCode.freq
        << ", Shannon code: " << charCode.code << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Encoded message: " << encodedLine << std::endl;

    std::cout << std::endl;

    // Lock mutex2 before modifying sharedData
    pthread_mutex_lock(data->mutex2);

    // increment the counter
    (*(data->counter))++;

    // signal the condition variable
    pthread_cond_broadcast(data->condition);

    pthread_mutex_unlock(data->mutex2);

    return nullptr;
}


int main() 
{
    // store each line into a different initialization of a vector
    std::vector<std::string> inputLines;
    std::string line;

    // take in the input 
    while (std::getline(std::cin, line)) {
        inputLines.push_back(line);
    }

    int threadSize = inputLines.size();

    // initialize synchronization variables
    pthread_mutex_t mutex1;
    pthread_mutex_init(&mutex1, nullptr);

    pthread_mutex_t mutex2;
    pthread_mutex_init(&mutex2, nullptr);

    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

    int counter = 0;

    // initialize shared data
    SharedData sharedData;
    sharedData.counter = &counter;
    sharedData.mutex1 = &mutex1;
    sharedData.mutex2 = &mutex2;
    sharedData.condition = &condition;

    // vector to store thread IDs to join later
    std::vector<pthread_t> threadVector;

    for (int i = 0; i < threadSize; i++) {
        pthread_t tid;

        // lock mutex1 before modifying sharedData
        pthread_mutex_lock(&mutex1);

        // set the data (line and id) for the thread
        sharedData.line = inputLines[i];
        sharedData.id = i;

        // create a thread for each EncodedMsg struct of the vector, and if an error is thrown return 1;
        if(pthread_create(&tid, nullptr, shannonCode, &sharedData)) 
        {
            std::cerr << "Error creating thread" << std::endl;
            return 1;
        }

        // store thread IDs in vector
        threadVector.push_back(tid);
    }

    // join threads back together when they are finished
    for (int i = 0; i < threadSize; i++) 
    {
        pthread_join(threadVector[i], nullptr);
    }

    return 0;
}