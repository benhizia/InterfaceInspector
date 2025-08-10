#pragma once

#include <string>
#include <vector>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size)
        : name_(name), size_(size), fd_(-1), ptr_(nullptr) {}

    bool create() {
        fd_ = shm_open(name_.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ == -1) {
            return false;
        }
        if (ftruncate(fd_, size_) == -1) {
            return false;
        }
        ptr_ = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        return ptr_ != MAP_FAILED;
    }

    bool open() {
        fd_ = shm_open(name_.c_str(), O_RDWR, 0666);
        if (fd_ == -1) {
            return false;
        }
        ptr_ = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        return ptr_ != MAP_FAILED;
    }

    void close() {
        if (ptr_ != MAP_FAILED) {
            munmap(ptr_, size_);
        }
        if (fd_ != -1) {
            ::close(fd_);
        }
    }

    void remove() {
        shm_unlink(name_.c_str());
    }

    void* get_ptr() {
        return ptr_;
    }

private:
    std::string name_;
    size_t size_;
    int fd_;
    void* ptr_;
};
