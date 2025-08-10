#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
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

struct MemberInfo {
    std::string name;
    size_t offset;
    size_t size;
    size_t align;
};

struct SharedMemoryHeader {
    char struct_name[64];
    uint32_t struct_size;
    uint32_t data_valid;
    uint32_t version;
};

class JsonDeserializer {
public:
    JsonDeserializer(const std::string& json_file) {
        std::ifstream file(json_file);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open JSON file: " + json_file);
        }
        
        metadata = json::parse(file);
        
        // Extract SimpleStruct metadata
        if (metadata.contains("SimpleStruct")) {
            const auto& struct_info = metadata["SimpleStruct"];
            struct_size = struct_info["size"];
            struct_align = struct_info["align"];
            
            for (const auto& member : struct_info["members"]) {
                members.push_back({
                    member["name"],
                    member["offset"],
                    member["size"],
                    member["align"]
                });
            }
        }
    }

    void deserialize(const void* data, const std::string& struct_name) {
        if (struct_name != "SimpleStruct") {
            throw std::runtime_error("Unsupported struct type: " + struct_name);
        }
        
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        
        for (const auto& member : members) {
            std::cout << "  " << member.name << ": ";
            
            switch (member.size) {
                case 1: {
                    uint8_t value = *reinterpret_cast<const uint8_t*>(bytes + member.offset);
                    if (member.name == "category") {
                        std::cout << "'" << static_cast<char>(value) << "' (0x" << std::hex << static_cast<int>(value) << std::dec << ")";
                    } else {
                        std::cout << static_cast<int>(value);
                    }
                    break;
                }
                case 4: {
                    int32_t value = *reinterpret_cast<const int32_t*>(bytes + member.offset);
                    std::cout << value;
                    break;
                }
                case 8: {
                    double value = *reinterpret_cast<const double*>(bytes + member.offset);
                    std::cout << value;
                    break;
                }
                default:
                    std::cout << "[unknown type, size=" << member.size << "]";
            }
            std::cout << " (offset: " << member.offset << ", size: " << member.size << ")\n";
        }
    }

    bool validate(const void* data, const std::string& struct_name) {
        if (struct_name != "SimpleStruct") {
            return false;
        }
        
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        
        // Expected values for validation
        const int32_t expected_value = 42;
        const double expected_price = 3.14159;
        const uint8_t expected_is_active = 1;
        const char expected_category = 'X';
        
        bool valid = true;
        
        for (const auto& member : members) {
            if (member.name == "value") {
                int32_t actual = *reinterpret_cast<const int32_t*>(bytes + member.offset);
                if (actual != expected_value) {
                    std::cout << "Validation failed for value: expected " << expected_value 
                              << ", got " << actual << "\n";
                    valid = false;
                }
            } else if (member.name == "price") {
                double actual = *reinterpret_cast<const double*>(bytes + member.offset);
                if (std::abs(actual - expected_price) > 1e-6) {
                    std::cout << "Validation failed for price: expected " << expected_price 
                              << ", got " << actual << "\n";
                    valid = false;
                }
            } else if (member.name == "is_active") {
                uint8_t actual = *reinterpret_cast<const uint8_t*>(bytes + member.offset);
                if (actual != expected_is_active) {
                    std::cout << "Validation failed for is_active: expected " << static_cast<int>(expected_is_active) 
                              << ", got " << static_cast<int>(actual) << "\n";
                    valid = false;
                }
            } else if (member.name == "category") {
                char actual = *reinterpret_cast<const char*>(bytes + member.offset);
                if (actual != expected_category) {
                    std::cout << "Validation failed for category: expected '" << expected_category 
                              << "', got '" << actual << "'\n";
                    valid = false;
                }
            }
        }
        
        return valid;
    }

    void print_metadata() {
        std::cout << "Metadata:\n";
        std::cout << "  System: " << metadata["system"]["compiler"] << "\n";
        std::cout << "  Architecture: " << metadata["system"]["architecture"] << "\n";
        std::cout << "  Endianness: " << metadata["system"]["endianness"] << "\n";
        
        std::cout << "\nStruct info:\n";
        std::cout << "  Size: " << struct_size << "\n";
        std::cout << "  Align: " << struct_align << "\n";
        std::cout << "  Members:\n";
        
        for (const auto& member : members) {
            std::cout << "    " << member.name << ": offset=" << member.offset 
                      << ", size=" << member.size << ", align=" << member.align << "\n";
        }
    }

private:
    json metadata;
    size_t struct_size = 0;
    size_t struct_align = 0;
    std::vector<MemberInfo> members;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <metadata.json> <test_name> [validate]\n";
        return 1;
    }

    std::string metadata_file = argv[1];
    std::string test_name = argv[2];
    bool validate = (argc > 3) && std::string(argv[3]) == "validate";

    try {
        JsonDeserializer deserializer(metadata_file);
        
        if (validate) {
            deserializer.print_metadata();
            std::cout << "\n";
        }

        // Open shared memory
        SharedMemory shm(test_name, sizeof(SharedMemoryHeader) + sizeof(SimpleStruct), false);
        
        // Read header
        const SharedMemoryHeader* header = static_cast<const SharedMemoryHeader*>(shm.data());
        
        if (header->data_valid != 1) {
            std::cerr << "Error: Shared memory data not valid\n";
            return 1;
        }
        
        // Read data
        const void* data = static_cast<const char*>(shm.data()) + sizeof(SharedMemoryHeader);
        
        std::cout << "Receiver: Reading data from shared memory '" << test_name << "'\n";
        std::cout << "Struct: " << header->struct_name << "\n";
        
        deserializer.deserialize(data, header->struct_name);
        
        if (validate) {
            bool valid = deserializer.validate(data, header->struct_name);
            std::cout << "\nValidation: " << (valid ? "PASSED" : "FAILED") << "\n";
            return valid ? 0 : 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}