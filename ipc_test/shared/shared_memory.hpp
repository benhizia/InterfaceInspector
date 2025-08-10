#pragma once
#include <string>
#include <memory>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>
#endif

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size, bool create = true)
        : name_(name), size_(size), create_(create), data_(nullptr) {
        
        if (name_.empty()) {
            throw std::runtime_error("Shared memory name cannot be empty");
        }
        
        if (size_ == 0) {
            throw std::runtime_error("Shared memory size must be > 0");
        }

#ifdef _WIN32
        handle_ = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            nullptr,
            create_ ? PAGE_READWRITE : FILE_MAP_READ,
            0,
            static_cast<DWORD>(size_),
            name_.c_str()
        );
        
        if (!handle_) {
            throw std::runtime_error("Failed to create/open shared memory: " + std::to_string(GetLastError()));
        }
        
        data_ = MapViewOfFile(
            handle_,
            create_ ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
            0, 0, size_
        );
        
        if (!data_) {
            CloseHandle(handle_);
            throw std::runtime_error("Failed to map shared memory: " + std::to_string(GetLastError()));
        }
#else
        int flags = create_ ? (O_CREAT | O_RDWR) : O_RDONLY;
        int mode = create_ ? 0666 : 0;
        
        fd_ = shm_open(("/" + name_).c_str(), flags, mode);
        if (fd_ == -1) {
            throw std::runtime_error("Failed to create/open shared memory");
        }
        
        if (create_) {
            if (ftruncate(fd_, size_) == -1) {
                close(fd_);
                throw std::runtime_error("Failed to set shared memory size");
            }
        }
        
        int prot = create_ ? (PROT_READ | PROT_WRITE) : PROT_READ;
        data_ = mmap(nullptr, size_, prot, MAP_SHARED, fd_, 0);
        
        if (data_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("Failed to map shared memory");
        }
#endif
    }

    ~SharedMemory() {
#ifdef _WIN32
        if (data_) {
            UnmapViewOfFile(data_);
        }
        if (handle_) {
            CloseHandle(handle_);
        }
#else
        if (data_) {
            munmap(data_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
#endif
    }

    void* data() const { return data_; }
    size_t size() const { return size_; }
    const std::string& name() const { return name_; }

    static bool exists(const std::string& name) {
#ifdef _WIN32
        HANDLE h = OpenFileMappingA(FILE_MAP_READ, FALSE, name.c_str());
        if (h) {
            CloseHandle(h);
            return true;
        }
        return false;
#else
        int fd = shm_open(("/" + name).c_str(), O_RDONLY, 0);
        if (fd != -1) {
            close(fd);
            return true;
        }
        return false;
#endif
    }

    static void remove(const std::string& name) {
#ifdef _WIN32
        // Windows doesn't need explicit removal
#else
        shm_unlink(("/" + name).c_str());
#endif
    }

private:
    std::string name_;
    size_t size_;
    bool create_;
    void* data_;
    
#ifdef _WIN32
    HANDLE handle_{nullptr};
#else
    int fd_{-1};
#endif
};