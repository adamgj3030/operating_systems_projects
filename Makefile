# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

# Directories
SRC_DIR = src
BUILD_DIR = build
SYNC_DIR = $(SRC_DIR)/sync
THREAD_DIR = $(SRC_DIR)/threading
NETWORK_DIR = $(SRC_DIR)/network

# Source files
MUTEX_SRC = $(SYNC_DIR)/mutex.cpp
THREADING_SRC = $(THREAD_DIR)/multiThreading.cpp
CLIENT_SRC = $(NETWORK_DIR)/client.cpp
SERVER_SRC = $(NETWORK_DIR)/server.cpp

# Output executables
MUTEX = $(BUILD_DIR)/mutex
THREADING = $(BUILD_DIR)/multiThreading
CLIENT = $(BUILD_DIR)/client
SERVER = $(BUILD_DIR)/server

# Targets
.PHONY: all clean directories

all: directories $(MUTEX) $(THREADING) $(CLIENT) $(SERVER)

directories:
	@mkdir -p $(BUILD_DIR)

# Build rules
$(MUTEX): $(MUTEX_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

$(THREADING): $(THREADING_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

$(CLIENT): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

$(SERVER): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build all executables (default)"
	@echo "  clean      - Remove build directory and executables"
	@echo "  help       - Display this help message"
	@echo ""
	@echo "Individual targets:"
	@echo "  mutex      - Build mutex synchronization example"
	@echo "  threading  - Build multithreading example"
	@echo "  client     - Build network client"
	@echo "  server     - Build network server"