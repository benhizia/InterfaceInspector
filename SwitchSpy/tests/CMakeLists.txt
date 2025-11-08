# SwitchSpy Tests
# TODO: Add actual tests

add_executable(test_networking test_networking.cpp)
target_link_libraries(test_networking PRIVATE switchspy_lib)

add_test(NAME NetworkingTest COMMAND test_networking)
