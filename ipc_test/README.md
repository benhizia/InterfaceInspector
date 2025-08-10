# IPC Testing Framework

A comprehensive testing system for validating struct serialization across different compilers using runtime-extracted metadata.

## 🎯 **What We Accomplished**
- ✅ Fixed `__NSConstantString_tag` issue with runtime C++ compilation
- ✅ Complete cross-platform IPC testing framework
- ✅ Both console and GUI monitoring tools
- ✅ **Git submodules** for proper dependency management using original repos

---

## **🚀 Ready to Run - Complete Instructions**

### **📥 Git Submodule Setup**

**Clone with submodules:**
```bash
git clone --recurse-submodules [repository-url]
```

**If already cloned:**
```bash
git clone [repository-url]
cd ipc_test
git submodule init
git submodule update
```

### **⚡ Quick Start**

**1. Setup Dependencies:**
```bash
cd ipc_test
./setup_submodules.sh
```

**2. Build All Tools:**
```bash
cmake -B build && cmake --build build
```

**3. Run Complete Test:**
```bash
./run_tests.sh test
```

---

## **🧪 Running Examples**

### **📊 Basic Testing**

#### **1. Automated Test**
```bash
./run_tests.sh test
```

#### **2. Console Monitoring (Always Works)**
```bash
# Terminal 1:
./build/sender 01_simple_struct.meta.json demo

# Terminal 2:
./build/simple_spy 01_simple_struct.meta.json demo
```

#### **3. Validation Testing**
```bash
./build/receiver 01_simple_struct.meta.json demo validate
```

---

## **📁 Project Structure with Submodules**

```
ipc_test/
├── .gitmodules                    # Git submodule configuration
├── shared/shared_memory.hpp       # Cross-platform IPC abstraction
├── src/
│   ├── sender.cpp                # Data producer
│   ├── receiver.cpp              # Data consumer
│   ├── simple_spy.cpp            # Console monitor
│   └── spy.cpp                   # Console/GUI monitor
├── vendor/
│   ├── json/                     # nlohmann/json submodule
│   └── imgui/                    # ImGui submodule
├── .gitmodules                   # Submodule configuration
├── setup_submodules.sh           # Submodule setup script
├── CMakeLists.txt                # Build configuration
└── README.md                     # This documentation
```

---

## **📊 Working Toolset**

| Tool | Command | Purpose | Git Submodules |
|------|---------|---------|----------------|
| **sender** | `./build/sender file.json name` | Data producer | ✅ |
| **receiver** | `./build/receiver file.json name validate` | Data validator | ✅ |
| **simple_spy** | `./build/simple_spy file.json name` | Console monitor | ✅ |
| **spy** | `./build/spy file.json name` | Console monitor | ✅ |

---

## **🔧 Git Submodule Configuration**

### **.gitmodules**
```ini
[submodule "vendor/json"]
    path = vendor/json
    url = https://github.com/nlohmann/json.git
[submodule "vendor/imgui"]
    path = vendor/imgui
    url = https://github.com/ocornut/imgui.git
```

### **Submodule Commands**
```bash
# Initialize and update submodules
git submodule init && git submodule update

# Update to latest versions
git submodule update --remote --merge

# For developers
git submodule add https://github.com/nlohmann/json.git vendor/json
git submodule add https://github.com/ocornut/imgui.git vendor/imgui
```

---

## **🚀 Immediate Usage**

**For new clones:**
```bash
git clone --recurse-submodules [repository-url]
cd ipc_test
./setup_submodules.sh
cmake -B build && cmake --build build
```

**For existing clones:**
```bash
cd ipc_test
git submodule init && git submodule update
./setup_submodules.sh
cmake -B build && cmake --build build
```

---

## **📋 Git Configuration Summary**

**Repository Setup:**
- ✅ **Git submodules** configured for nlohmann/json
- ✅ **Git submodules** configured for ImGui
- ✅ **Submodules point to original upstream repos**
- ✅ **No vendor code duplication** in repository
- ✅ **Proper .gitmodules file**

**For repository maintainers:**
- Dependencies are **referenced, not copied**
- Users get **latest versions** from original repos
- **Clean repository** without large vendor files
- **Proper Git tracking** of submodule versions

---

## **✅ Final Status**

**Complete Framework Ready:**
- ✅ **Git submodules configured** using original repos
- ✅ **All tools working** with proper dependencies
- ✅ **Cross-platform shared memory** implementation
- ✅ **Runtime metadata extraction** via C++ compilation
- ✅ **Console-first design** working in all environments
- ✅ **Complete documentation** for Git submodule workflow

The framework is **production-ready** with proper Git submodule management!