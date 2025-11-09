cmake_minimum_required(VERSION 3.15)
project(SwitchSpy VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Options
option(SWITCHSPY_BUILD_TESTS "Build tests" ON)
option(SWITCHSPY_USE_GLFW "Use GLFW backend for ImGui" ON)

# Dependencies directory
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Find or download dependencies
find_package(Threads REQUIRED)

# ImGui
if(NOT EXISTS ${VENDOR_DIR}/imgui)
    message(STATUS "ImGui not found. Please run: git submodule update --init")
endif()

# nlohmann/json
if(NOT EXISTS ${VENDOR_DIR}/json)
    message(STATUS "nlohmann/json not found. Please run: git submodule update --init")
endif()

# inih (INI parser)
if(NOT EXISTS ${VENDOR_DIR}/inih)
    message(STATUS "inih not found. Please run: git submodule update --init")
endif()

# GLFW (optional)
if(SWITCHSPY_USE_GLFW)
    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        message(STATUS "GLFW3 not found, will try pkg-config")
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            pkg_check_modules(GLFW3 glfw3)
        endif()
    endif()
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VENDOR_DIR}/json/include
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/imgui/backends
    ${VENDOR_DIR}/inih
)

# ImGui library
add_library(imgui STATIC
    ${VENDOR_DIR}/imgui/imgui.cpp
    ${VENDOR_DIR}/imgui/imgui_draw.cpp
    ${VENDOR_DIR}/imgui/imgui_tables.cpp
    ${VENDOR_DIR}/imgui/imgui_widgets.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/imgui/backends
)
target_link_libraries(imgui PUBLIC OpenGL::GL)
if(TARGET glfw)
    target_link_libraries(imgui PUBLIC glfw)
elseif(GLFW3_FOUND)
    target_include_directories(imgui PUBLIC ${GLFW3_INCLUDE_DIRS})
    target_link_libraries(imgui PUBLIC ${GLFW3_LIBRARIES})
endif()

# inih library
add_library(inih STATIC
    ${VENDOR_DIR}/inih/ini.c
    ${VENDOR_DIR}/inih/cpp/INIReader.cpp
)
target_include_directories(inih PUBLIC ${VENDOR_DIR}/inih ${VENDOR_DIR}/inih/cpp)

# SwitchSpy Core Library
set(SWITCHSPY_SOURCES
    # Common
    src/common/logger.cpp
    src/common/benchmark.cpp

    # Core
    src/core/network_connection.cpp
    src/core/tcp_connection.cpp
    src/core/udp_connection.cpp
    src/core/multicast_connection.cpp
    src/core/network_manager.cpp

    # Config
    src/config/ini_parser.cpp
    src/config/config_manager.cpp
    src/config/port_mapping.cpp

    # Model
    src/model/connection_state.cpp
    src/model/data_buffer.cpp
    src/model/session_data.cpp
    src/model/replay_data.cpp

    # Serialization
    src/serialization/binary_serializer.cpp
    src/serialization/binary_deserializer.cpp
    src/serialization/packet_formatter.cpp

    # Controller
    src/controller/connection_manager.cpp
    src/controller/relay_controller.cpp
    src/controller/recording_controller.cpp
    src/controller/replay_controller.cpp
    src/controller/flow_control.cpp

    # View
    src/view/main_window.cpp
    src/view/connection_tab.cpp
    src/view/tree_viewer.cpp
    src/view/struct_renderer.cpp
    src/view/stats_panel.cpp
    src/view/metadata_parser.cpp
    src/view/log_panel.cpp
    src/view/performance_panel.cpp
)

set(SWITCHSPY_HEADERS
    # Common
    include/switchspy/common/types.hpp
    include/switchspy/common/platform.hpp
    include/switchspy/common/logger.hpp
    include/switchspy/common/benchmark.hpp

    # Core
    include/switchspy/core/network_connection.hpp
    include/switchspy/core/tcp_connection.hpp
    include/switchspy/core/udp_connection.hpp
    include/switchspy/core/multicast_connection.hpp
    include/switchspy/core/network_manager.hpp

    # Config
    include/switchspy/config/ini_parser.hpp
    include/switchspy/config/config_manager.hpp
    include/switchspy/config/port_mapping.hpp

    # Model
    include/switchspy/model/connection_state.hpp
    include/switchspy/model/data_buffer.hpp
    include/switchspy/model/session_data.hpp
    include/switchspy/model/replay_data.hpp

    # Serialization
    include/switchspy/serialization/binary_serializer.hpp
    include/switchspy/serialization/binary_deserializer.hpp
    include/switchspy/serialization/packet_formatter.hpp

    # Controller
    include/switchspy/controller/connection_manager.hpp
    include/switchspy/controller/relay_controller.hpp
    include/switchspy/controller/recording_controller.hpp
    include/switchspy/controller/replay_controller.hpp
    include/switchspy/controller/flow_control.hpp

    # View
    include/switchspy/view/main_window.hpp
    include/switchspy/view/connection_tab.hpp
    include/switchspy/view/tree_viewer.hpp
    include/switchspy/view/struct_renderer.hpp
    include/switchspy/view/stats_panel.hpp
    include/switchspy/view/metadata_parser.hpp
    include/switchspy/view/log_panel.hpp
    include/switchspy/view/performance_panel.hpp
)

add_library(switchspy_lib STATIC ${SWITCHSPY_SOURCES} ${SWITCHSPY_HEADERS})
target_include_directories(switchspy_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(switchspy_lib PUBLIC
    Threads::Threads
    imgui
    inih
)

# Platform-specific networking
if(WIN32)
    target_link_libraries(switchspy_lib PUBLIC ws2_32)
elseif(LINUX)
    target_link_libraries(switchspy_lib PUBLIC rt)
endif()

# Main executable
add_executable(switchspy src/main.cpp)
target_link_libraries(switchspy PRIVATE switchspy_lib)

# Install
install(TARGETS switchspy DESTINATION bin)
install(DIRECTORY config/ DESTINATION share/switchspy/config)

# Tests
if(SWITCHSPY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print configuration
message(STATUS "SwitchSpy Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
if(SWITCHSPY_USE_GLFW)
    message(STATUS "  ImGui Backend: GLFW + OpenGL3")
endif()
