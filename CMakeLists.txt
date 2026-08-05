cmake_minimum_required(VERSION 3.21)

project(EDCC
    VERSION 0.1.2
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
    src/app/MainWindow.h
    src/app/MainWindow.cpp

    src/core/Types.h
    src/core/IDevice.h
    src/core/ICommunication.h
    src/core/Device.h
    src/core/Device.cpp
    src/core/DeviceManager.h
    src/core/DeviceManager.cpp

    src/communication/SerialCommunication.h
    src/communication/SerialCommunication.cpp
    src/communication/TcpCommunication.h
    src/communication/TcpCommunication.cpp



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
