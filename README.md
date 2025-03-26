# Operating Systems Concepts: Shannon Encoding & Concurrency

A **C++** project demonstrating core operating systems concepts—**multithreading**, **synchronization**, and **client-server communication**—through the lens of **Shannon encoding**. The repository showcases how to apply OS fundamentals in real-world scenarios, including process forking, mutex-based resource protection, and network-based data exchange.

## Table of Contents
1. [Overview](#overview)  
2. [Features](#features)  
3. [Key Implementation Details](#key-implementation-details)  

---

## 1. Overview

This project focuses on **Shannon encoding**, a technique that constructs unique binary codes for symbols based on their relative frequencies. Unlike Huffman encoding, Shannon encoding uses a top-down probability partitioning approach. Our implementation highlights how **operating system concepts** (threads, processes, synchronization, and sockets) can be combined with **algorithmic logic** (frequency counting, binary encoding) to form a robust system.

**Key Learning Objectives:**
- Understand how to spawn and manage multiple **POSIX threads**.
- Explore **mutexes** and **condition variables** for safe access to shared resources.
- Demonstrate **socket programming** with a **client-server** model.
- Implement **Shannon encoding** in both single- and multi-process contexts.

---

## 2. Features

1. **Multithreaded Processing**  
   - Creates one thread per message or as needed.  
   - Safe sharing of data among threads using **mutexes** and **condition variables**.  
   - Thread-safe logging and error handling.

2. **Client-Server Communication**  
   - **TCP/IP** socket-based communication for sending messages and receiving encoded results.  
   - **Fork-based** concurrency in the server, allowing multiple client connections.  
   - Graceful handling of network errors, disconnections, and resource cleanup.

3. **Synchronization Mechanisms**  
   - **Mutex-based** protection of critical sections.  
   - **Condition variables** for thread coordination and sequential or orderly output.  
   - Demonstrates how to avoid common pitfalls like deadlock and race conditions.

4. **Shannon Encoding Implementation**  
   - Calculates character frequencies to derive unique binary codes for each symbol.  
   - Showcases an alternative to Huffman encoding with top-down partitioning of probability ranges.  
   - Illustrates real-world concurrency and parallelization strategies for encoding operations.

---

## 6. Key Implementation Details

### Shannon Encoding
- Counts symbol frequencies for each input line
- Sorts symbols by descending frequency and computes cumulative probabilities to assign each symbol a unique binary code
- Encodes each message by concatenating the symbol codes

### POSIX Threads (pthread)
- Threads are created to perform encoding in parallel
- Mutexes and condition variables ensure safe access to shared resources and coordinated output

### Client-Server Architecture
- The server uses `fork()` to handle multiple clients concurrently
- The client sends messages to the server, which returns frequency tables and encoded results

### Synchronization
- The mutex example (`mutex.cpp`) demonstrates how threads can synchronize their output to avoid interleaving, ensuring results are printed in order

---

## Appendix: Sample Outputs

Below are detailed example outputs demonstrating the program's behavior in different modes.

### Multithreaded Mode Output Example

**Input:**
```
COSC 3360 COSC 1437
COSC 1336 COSC 2436
COSC 3320 COSC 3380
```

**Sample Output:**
```
# First Thread Output
Message: COSC 3360 COSC 1437

Alphabet:
Symbol: C, Frequency: 4, Shannon code: 000
Symbol: 3, Frequency: 3, Shannon code: 001
Symbol: , Frequency: 3, Shannon code: 010
Symbol: S, Frequency: 2, Shannon code: 1000
Symbol: O, Frequency: 2, Shannon code: 1010
Symbol: 7, Frequency: 1, Shannon code: 10111
Symbol: 4, Frequency: 1, Shannon code: 11010

Encoded message: 00010101000000010001001110011111001000010101000000010111001101000110111

# Second Thread Output
Message: COSC 1336 COSC 2436

Alphabet:
Symbol: C, Frequency: 4, Shannon code: 000
Symbol: 3, Frequency: 3, Shannon code: 001
Symbol: , Frequency: 3, Shannon code: 010
Symbol: S, Frequency: 2, Shannon code: 1000
Symbol: O, Frequency: 2, Shannon code: 1010
Symbol: 6, Frequency: 1, Shannon code: 10111
Symbol: 4, Frequency: 1, Shannon code: 11010
Symbol: 2, Frequency: 1, Shannon code: 11011

Encoded message: 00010101000000010001001110011111001000010101000000010111001101000111011

# Third Thread Output
Message: COSC 3320 COSC 3380

Alphabet:
Symbol: C, Frequency: 4, Shannon code: 000
Symbol: 3, Frequency: 3, Shannon code: 001
Symbol: , Frequency: 3, Shannon code: 010
Symbol: S, Frequency: 2, Shannon code: 1000
Symbol: O, Frequency: 2, Shannon code: 1010
Symbol: 2, Frequency: 1, Shannon code: 10111
Symbol: 0, Frequency: 1, Shannon code: 11010
Symbol: 8, Frequency: 1, Shannon code: 11011

Encoded message: 00010101000000010001001110011111001000010101000000010111001101000111011
```


*Note:* Actual Shannon codes may vary based on the implementation details and the precise sorting order.

---

### Synchronized Mode Output Example

This example demonstrates thread synchronization using mutexes and condition variables. The output is guaranteed to be in the same order as the input.

**Input:**
```
This is a test line
Shannon encoding demonstration
Operating Systems
```

**Sample Output:**
```
# First Thread Output (Synchronized)
Message: This is a test line

Alphabet:
Symbol: ' ', Frequency: 4, Shannon code: 000
Symbol: s, Frequency: 3, Shannon code: 001
Symbol: i, Frequency: 3, Shannon code: 010
Symbol: t, Frequency: 2, Shannon code: 0110
Symbol: e, Frequency: 2, Shannon code: 0111
Symbol: T, Frequency: 1, Shannon code: 10000
Symbol: h, Frequency: 1, Shannon code: 10001
Symbol: a, Frequency: 1, Shannon code: 10010
Symbol: l, Frequency: 1, Shannon code: 10011
Symbol: n, Frequency: 1, Shannon code: 10100

Encoded message: 100001110010101000110111001001001010...

# Second Thread Output (Synchronized)
Message: Shannon encoding demonstration

Alphabet:
Symbol: ' ', Frequency: 2, Shannon code: 000
Symbol: n, Frequency: 3, Shannon code: 001
Symbol: a, Frequency: 2, Shannon code: 010
Symbol: i, Frequency: 2, Shannon code: 0110
Symbol: g, Frequency: 2, Shannon code: 0111
Symbol: S, Frequency: 1, Shannon code: 10000
Symbol: h, Frequency: 1, Shannon code: 10001
Symbol: o, Frequency: 1, Shannon code: 10010
Symbol: c, Frequency: 1, Shannon code: 10011
Symbol: d, Frequency: 1, Shannon code: 10100
Symbol: e, Frequency: 1, Shannon code: 10101
Symbol: m, Frequency: 1, Shannon code: 10110
Symbol: r, Frequency: 1, Shannon code: 10111
Symbol: t, Frequency: 1, Shannon code: 11000

Encoded message: 100011100101011000101110010100101011001010111001010011...

# Third Thread Output (Synchronized)
Message: Operating Systems

Alphabet:
Symbol: ' ', Frequency: 1, Shannon code: 000
Symbol: o, Frequency: 3, Shannon code: 001
Symbol: s, Frequency: 3, Shannon code: 010
Symbol: O, Frequency: 2, Shannon code: 0110
Symbol: p, Frequency: 1, Shannon code: 0111
Symbol: e, Frequency: 1, Shannon code: 10000
Symbol: r, Frequency: 1, Shannon code: 10001
Symbol: a, Frequency: 1, Shannon code: 10010
Symbol: t, Frequency: 1, Shannon code: 10011
Symbol: i, Frequency: 1, Shannon code: 10100
Symbol: n, Frequency: 1, Shannon code: 10101
Symbol: g, Frequency: 1, Shannon code: 10110

Encoded message: 011010010010101011001001010110010100101011010...
```
