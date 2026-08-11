cmake_minimum_required(VERSION 3.21)

project(EDCC
    VERSION 1.6.0
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
    Test
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
    src/core/IPacketParser.h

    src/communication/SerialCommunication.h
    src/communication/SerialCommunication.cpp
    src/communication/TcpCommunication.h
    src/communication/TcpCommunication.cpp
    src/communication/CommunicationWorker.h
    src/communication/CommunicationWorker.cpp
    src/communication/SimplePacketParser.h
    src/communication/SimplePacketParser.cpp

    src/logging/Logger.h
    src/logging/Logger.cpp

    src/utils/JsonConfig.h
    src/utils/JsonConfig.cpp
    src/core/IPacketParser.cpp
    src/firmware/IMemoryTarget.h
    src/firmware/FirmwareUpdater.h
    src/firmware/FirmwareUpdater.cpp
    src/firmware/DummyMemoryTarget.cpp
    src/firmware/DummyMemoryTarget.h





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

option(EDCC_BUILD_TESTS "Build unit tests" ON)

if(EDCC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
