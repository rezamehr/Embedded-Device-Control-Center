cmake_minimum_required(VERSION 3.21)

project(EDCC
    VERSION 0.1.0
    DESCRIPTION "Embedded Device Control Center"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Network
    SerialPort
)

add_executable(EDCC
    src/app/main.cpp
)

target_include_directories(EDCC
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(EDCC
    PRIVATE
        Qt6::Widgets
        Qt6::Network
        Qt6::SerialPort
)

# Later we will split into static libraries (core, communication, logging...)
