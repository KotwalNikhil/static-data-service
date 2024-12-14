cmake_minimum_required( VERSION 3.22)
project(research_project)

# set(CMAKE_CXX_COMPILER /usr/bin/g++)
# set(CMAKE_CC_COMPILER /usr/bin/gcc)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip
)

FetchContent_MakeAvailable(googletest)

# Set the source directory
set(SRC_DIR src)

# Add source files to the target
add_executable(my_executable ${SRC_DIR}/staticDataService.cpp)
add_executable(file_processing_test tests/fileProcessing.ft.cpp)

# Include necessary directories
target_include_directories(my_executable PUBLIC ${SRC_DIR} tests static_data ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(file_processing_test gtest gtest_main)

enable_testing()
add_test(NAME fileProcessingTest COMMAND file_processing_test)

