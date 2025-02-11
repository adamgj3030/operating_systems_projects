# Operating Systems Concepts Implementation

This repository demonstrates key operating systems concepts through practical implementations in C++. It serves as both an educational resource and a demonstration of systems programming capabilities.

## 🔍 Core Concepts Demonstrated

### 1. Multithreading
- Parallel processing implementation
- Thread creation and management
- Thread synchronization
- Performance optimization through parallel execution
- Located in `src/threading/`

### 2. Client-Server Communication
- Socket programming
- Network communication protocols
- Data serialization and transmission
- Server-side concurrency handling
- Located in `src/network/`

### 3. Mutual Exclusion (Mutex)
- Thread synchronization mechanisms
- Critical section handling
- Deadlock prevention
- Race condition mitigation
- Located in `src/sync/`

## 🛠 Technical Implementation

### Multithreading (multiThreading.cpp)
- Implementation of Shannon coding algorithm with parallel processing
- Dynamic thread creation based on input size
- Thread-safe data structures
- Efficient workload distribution

### Client-Server Architecture (client.cpp, server.cpp)
- TCP/IP socket implementation
- Concurrent client handling through forking
- Robust error handling
- Clean connection termination

### Mutex Implementation (mutex.cpp)
- Multiple mutex coordination
- Conditional variables usage
- Critical section management
- Deadlock prevention mechanisms

## 🔧 Building and Running

### Prerequisites
- C++ compiler (g++ recommended)
- POSIX-compliant system
- pthread library
- Network connectivity (for client-server components)

### Build Instructions
```bash
# Compile the server
g++ server.cpp -o server

# Compile the client with pthread support
g++ client.cpp -o client -lpthread

# Compile the multithreading example
g++ multiThreading.cpp -o multiThreading -lpthread

# Compile the mutex example
g++ mutex.cpp -o mutex -lpthread
```

### Running the Programs

#### Client-Server Example
```bash
# Start the server (in one terminal)
./server 1234

# Run the client (in another terminal)
./client localhost 1234
```

#### Multithreading Example
```bash
./multiThreading
# Enter strings as input, press Ctrl+D when done
```

#### Mutex Example
```bash
./mutex
# Enter strings as input, press Ctrl+D when done
```

## 🎓 High Level Concepts

### Key Concepts Explained

1. **Multithreading**
   - Parallel execution of tasks
   - Thread lifecycle management
   - Synchronization mechanisms
   - Performance considerations

2. **Client-Server Architecture**
   - Socket programming basics
   - Network communication patterns
   - Concurrent server design
   - Error handling strategies

3. **Mutex and Synchronization**
   - Critical section protection
   - Race condition prevention
   - Deadlock avoidance
   - Thread coordination

### Implementation Details

Each component demonstrates:
- Modern C++ practices
- POSIX compliance
- Error handling
- Resource management
- Performance optimization