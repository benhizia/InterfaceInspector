cmake_minimum_required(VERSION 3.10)
project(IPC_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Use submodules directly
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/json/include
)

# Common sources
set(COMMON_SOURCES
    shared/shared_memory.hpp
)

# Sender executable
add_executable(sender
    src/sender.cpp
    ${COMMON_SOURCES}
)

# Receiver executable
add_executable(receiver
    src/receiver.cpp
    ${COMMON_SOURCES}
)

# Simple spy executable (console-based - always works)
add_executable(simple_spy
    src/simple_spy.cpp
    ${COMMON_SOURCES}
)

# Spy executable (console - fallback)
add_executable(spy
    src/spy.cpp
    ${COMMON_SOURCES}
)

# Optional ImGui build (only if GLFW available)
find_package(OpenGL QUIET)
find_package(glfw3 QUIET)

if(OpenGL_FOUND AND glfw3_FOUND)
    set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    
    add_executable(imgui_spy
        src/spy.cpp
        ${IMGUI_SOURCES}
        ${COMMON_SOURCES}
    )
    target_include_directories(imgui_spy PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_link_libraries(imgui_spy PRIVATE 
        ${OPENGL_LIBRARIES}
        glfw
    )
    
    if(APPLE)
        find_library(COCOA_LIBRARY Cocoa)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(COREVIDEO_LIBRARY CoreVideo)
        target_link_libraries(imgui_spy PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
    elseif(WIN32)
        target_link_libraries(imgui_spy PRIVATE opengl32)
    endif()
endif()

# Copy test headers
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../test/01_simple_struct.hpp
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy metadata
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../MetaDataGen/01_simple_struct.meta.json
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Install targets
install(TARGETS sender receiver simple_spy spy DESTINATION bin)
if(TARGET imgui_spy)
    install(TARGETS imgui_spy DESTINATION bin)
endif()
