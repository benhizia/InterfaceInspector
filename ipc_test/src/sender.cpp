#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "../shared/shared_memory.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Define SimpleStruct here since we can't include the original due to path issues
struct SimpleStruct {
    int value;
    double price;
    bool is_active;
    char category;
};

struct SharedMemoryHeader {
    char struct_name[64];
    uint32_t struct_size;
    uint32_t data_valid;
    uint32_t version;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <metadata.json> <test_name> [delay_seconds]\n";
        return 1;
    }

    std::string metadata_file = argv[1];
    std::string test_name = argv[2];
    int delay = (argc > 3) ? std::stoi(argv[3]) : 0;

    try {
        // Load metadata
        std::ifstream file(metadata_file);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open metadata file: " + metadata_file);
        }
        json metadata = json::parse(file);

        // Prepare test data
        SimpleStruct test_data;
        test_data.value = 42;
        test_data.price = 3.14159;
        test_data.is_active = true;
        test_data.category = 'X';

        // Calculate total size needed
        size_t total_size = sizeof(SharedMemoryHeader) + sizeof(SimpleStruct);
        
        // Create shared memory
        SharedMemory shm(test_name, total_size, true);
        
        // Write header
        SharedMemoryHeader* header = static_cast<SharedMemoryHeader*>(shm.data());
        strncpy(header->struct_name, "SimpleStruct", sizeof(header->struct_name) - 1);
        header->struct_name[sizeof(header->struct_name) - 1] = '\0';
        header->struct_size = sizeof(SimpleStruct);
        header->data_valid = 0;
        header->version = 1;

        // Write data
        SimpleStruct* data = reinterpret_cast<SimpleStruct*>(
            static_cast<char*>(shm.data()) + sizeof(SharedMemoryHeader)
        );
        
        if (delay > 0) {
            std::cout << "Waiting " << delay << " seconds before writing data...\n";
            std::this_thread::sleep_for(std::chrono::seconds(delay));
        }

        *data = test_data;
        
        // Mark data as valid
        header->data_valid = 1;

        std::cout << "Sender: Written test data to shared memory '" << test_name << "'\n";
        std::cout << "  Value: " << test_data.value << "\n";
        std::cout << "  Price: " << test_data.price << "\n";
        std::cout << "  Is Active: " << test_data.is_active << "\n";
        std::cout << "  Category: " << test_data.category << "\n";
        
        // Keep the process running
        std::cout << "Press Enter to exit and cleanup...\n";
        std::cin.get();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}