cmake_minimum_required(VERSION 3.20)

project(embedded_comm_kit C)

# Set C standard
set(CMAKE_C_STANDARD 11)

# Include headers folder
# This tells the compiler where to find .h files
include_directories(${PROJECT_SOURCE_DIR}/include)

# Automatically collect all .c source files in src/
file(GLOB_RECURSE SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/*.c
)

# Add executable from all collected source files
add_executable(${PROJECT_NAME} ${SRC_FILES}
        main.c
)

# explicitly tell CMake where headers are for target
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)