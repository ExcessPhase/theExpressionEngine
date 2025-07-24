cmake_minimum_required(VERSION 3.15)
project(MyCompilerTools)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #set(SANITIZER_FLAGS "-fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -g")
    set(SANITIZER_FLAGS "-fsanitize=thread -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
endif()


# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Find Boost (headers only or specific components)
find_package(Boost REQUIRED)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Add LLVM include and definitions
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Enable all LLVM components statically linked (optional: customize)
llvm_map_components_to_libnames(LLVM_LIBS support core irreader analysis orcjit native mcjit)


# Define sources for each executable
set(SOURCE_TOOL_A test.cpp)
set(SOURCE_TOOL_B theExpressionEngine.cpp)

# Add executables
add_executable(test.exe ${SOURCE_TOOL_A})
add_executable(theExpressionEngine.exe ${SOURCE_TOOL_B})

# Link Boost and LLVM to both
target_include_directories(test.exe PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(theExpressionEngine.exe PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(test.exe PRIVATE ${LLVM_LIBS})
target_link_libraries(theExpressionEngine.exe PRIVATE ${LLVM_LIBS})

# Enable ThreadSanitizer
#target_compile_options(theExpressionEngine.exe PRIVATE -fsanitize=thread -g -O0 -fno-omit-frame-pointer -fno-inline)
#target_link_options(theExpressionEngine.exe PRIVATE -fsanitize=thread)
enable_testing()

add_test(NAME BoostTest COMMAND ${CMAKE_BINARY_DIR}/test.exe)
