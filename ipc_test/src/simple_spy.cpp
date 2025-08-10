#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <vector>
#include <cstring>
#include <nlohmann/json.hpp>
#include "../shared/shared_memory.hpp"

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

class SimpleMemorySpy {
public:
    SimpleMemorySpy(const std::string& json_file, const std::string& shm_name) 
        : json_file_(json_file), shm_name_(shm_name), shm_(nullptr) {
        load_metadata();
    }

    ~SimpleMemorySpy() {
        cleanup_shm();
    }

    void run() {
        std::cout << "Simple Memory Spy - Monitoring: " << shm_name_ << "\n";
        std::cout << "Press Ctrl+C to exit\n\n";

        while (true) {
            try {
                update();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
                break;
            }
        }
    }

private:
    struct MemberInfo {
        std::string name;
        size_t offset;
        size_t size;
        std::string type;
    };

    void load_metadata() {
        std::ifstream file(json_file_);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open JSON file: " + json_file_);
        }
        
        json metadata = json::parse(file);
        
        if (metadata.contains("system")) {
            const auto& system = metadata["system"];
            std::cout << "System Info:\n";
            std::cout << "  Compiler: " << system["compiler"] << "\n";
            std::cout << "  Architecture: " << system["architecture"] << "\n";
            std::cout << "  Endianness: " << system["endianness"] << "\n\n";
        }
        
        if (metadata.contains("SimpleStruct")) {
            const auto& struct_info = metadata["SimpleStruct"];
            std::cout << "Struct Info:\n";
            std::cout << "  Name: SimpleStruct\n";
            std::cout << "  Size: " << struct_info["size"] << " bytes\n";
            std::cout << "  Align: " << struct_info["align"] << " bytes\n\n";
            
            for (const auto& member : struct_info["members"]) {
                members_.push_back({
                    member["name"],
                    member["offset"],
                    member["size"],
                    member["type"]
                });
            }
        }
    }

    void update() {
        bool shm_exists = SharedMemory::exists(shm_name_);
        
        if (shm_exists && !shm_) {
            try {
                shm_ = std::make_unique<SharedMemory>(shm_name_, 
                    sizeof(SharedMemoryHeader) + sizeof(SimpleStruct), false);
                std::cout << "\n[Connected to shared memory]\n";
            } catch (const std::exception& e) {
                std::cerr << "Failed to connect: " << e.what() << "\n";
                cleanup_shm();
                return;
            }
        } else if (!shm_exists && shm_) {
            std::cout << "\n[Shared memory disconnected]\n";
            cleanup_shm();
            return;
        }

        if (shm_) {
            const SharedMemoryHeader* header = 
                static_cast<const SharedMemoryHeader*>(shm_->data());
            
            if (header->data_valid == 1) {
                const uint8_t* data = 
                    static_cast<const uint8_t*>(shm_->data()) + sizeof(SharedMemoryHeader);
                
                display_data(data, header->struct_name);
            } else {
                std::cout << "\r[Waiting for valid data...] " << std::flush;
            }
        } else {
            std::cout << "\r[Shared memory not available...] " << std::flush;
        }
    }

    void display_data(const uint8_t* data, const std::string& struct_name) {
        if (struct_name != "SimpleStruct") return;

        std::cout << "\r[Data] ";
        for (const auto& member : members_) {
            const void* member_data = data + member.offset;
            
            std::cout << member.name << "=";
            switch (member.size) {
                case 1: {
                    uint8_t value = *static_cast<const uint8_t*>(member_data);
                    if (member.name == "category") {
                        std::cout << "'" << static_cast<char>(value) << "'";
                    } else {
                        std::cout << static_cast<int>(value);
                    }
                    break;
                }
                case 4: {
                    int32_t value = *static_cast<const int32_t*>(member_data);
                    std::cout << value;
                    break;
                }
                case 8: {
                    double value = *static_cast<const double*>(member_data);
                    std::cout << std::fixed << std::setprecision(5) << value;
                    break;
                }
                default:
                    std::cout << "[unknown]";
            }
            std::cout << " ";
        }
        std::cout << std::flush;
    }

    void cleanup_shm() {
        shm_.reset();
    }

    std::string json_file_;
    std::string shm_name_;
    std::unique_ptr<SharedMemory> shm_;
    std::vector<MemberInfo> members_;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <metadata.json> <test_name>\n";
        return 1;
    }

    std::string json_file = argv[1];
    std::string test_name = argv[2];

    try {
        SimpleMemorySpy spy(json_file, test_name);
        spy.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}