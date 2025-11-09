cmake_minimum_required(VERSION 3.15)
project(SwitchSpyTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific libraries
if(WIN32)
    set(NETWORK_LIBS ws2_32)
else()
    set(NETWORK_LIBS)
endif()

# Test 01: TCP Basic
add_executable(test01_sender 01_tcp_basic/sender.cpp)
add_executable(test01_receiver 01_tcp_basic/receiver.cpp)
target_link_libraries(test01_sender ${NETWORK_LIBS})
target_link_libraries(test01_receiver ${NETWORK_LIBS})

# Test 02: UDP Relay
add_executable(test02_sender 02_udp_relay/sender.cpp)
add_executable(test02_receiver 02_udp_relay/receiver.cpp)
target_link_libraries(test02_sender ${NETWORK_LIBS})
target_link_libraries(test02_receiver ${NETWORK_LIBS})

# Test 03: Multicast Broadcast
add_executable(test03_sender 03_multicast/sender.cpp)
add_executable(test03_receiver 03_multicast/receiver.cpp)
target_link_libraries(test03_sender ${NETWORK_LIBS})
target_link_libraries(test03_receiver ${NETWORK_LIBS})

# Test 04: High Throughput
add_executable(test04_sender 04_high_throughput/sender.cpp)
add_executable(test04_receiver 04_high_throughput/receiver.cpp)
target_link_libraries(test04_sender ${NETWORK_LIBS})
target_link_libraries(test04_receiver ${NETWORK_LIBS})

# Test 05: Rate Mismatch
add_executable(test05_sender 05_rate_mismatch/sender.cpp)
add_executable(test05_receiver 05_rate_mismatch/receiver.cpp)
target_link_libraries(test05_sender ${NETWORK_LIBS})
target_link_libraries(test05_receiver ${NETWORK_LIBS})

# Test 06: Complex Structs
add_executable(test06_sender 06_complex_structs/sender.cpp)
add_executable(test06_receiver 06_complex_structs/receiver.cpp)
target_link_libraries(test06_sender ${NETWORK_LIBS})
target_link_libraries(test06_receiver ${NETWORK_LIBS})

message(STATUS "SwitchSpy Tests configured")
