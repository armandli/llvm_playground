cmake_minimum_required(VERSION 3.16)

project(llvm_playground)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure to use angle brackets for includes
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Find required packages
find_package(LLVM REQUIRED CONFIG)
# Make Boost optional - only needed for certain components
find_package(Boost COMPONENTS program_options)
find_package(GTest)

# Report found versions
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
if(Boost_FOUND)
    message(STATUS "Found Boost ${Boost_VERSION}")
else()
    message(STATUS "Boost not found - some components may not be available")
endif()
if(GTest_FOUND)
    message(STATUS "Found GTest ${GTest_VERSION}")
else()
    message(STATUS "GTest not found - tests may not be available")
endif()

# Add LLVM definitions and include directories
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

# LLVM libraries
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Add project include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add subdirectories
add_subdirectory(src/util)
add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/codegen)
add_subdirectory(src/app)
if(GTest_FOUND)
    add_subdirectory(test)
endif()

# Global compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Create directories for build artifacts
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)