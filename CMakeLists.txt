cmake_minimum_required(VERSION 3.10)
project(calc)

set(CMAKE_C_STANDARD 11)

AUX_SOURCE_DIRECTORY(src/ SOURCE_FILES)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} m)
