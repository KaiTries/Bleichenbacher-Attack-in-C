cmake_minimum_required(VERSION 3.30)
project(BleichenbacherAttack C)

# Specify the target architecture
set(CMAKE_GENERATOR_PLATFORM x64)

# Set the compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -IC:/Users/Kai/Documents/GitHub/vcpkg/packages/gmp_x64-windows-static/include")

# Find GMP
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED gmp)

# Include directories
include_directories(${GMP_INCLUDE_DIRS})

# Linker flags
link_directories(${GMP_LIBRARY_DIRS})
set(GMP_LIBS ${GMP_LIBRARIES})

# Source files for the main program
set(SRCS main.c interval.c bleichenbacher.c rsa.c)

# Source files for the test program
set(TEST_SRCS interval.c bleichenbacher.c rsa.c tests/test.c)

# Add executable for the main program
add_executable(main ${SRCS})
target_link_libraries(main ${GMP_LIBS})

# Add executable for the test program
add_executable(test_program ${TEST_SRCS})
target_link_libraries(test_program ${GMP_LIBS})

# Custom target for running tests
add_custom_target(tests
    COMMAND test_program
    DEPENDS test_program
)

# Clean rule to remove executables and object files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "main;test_program")
