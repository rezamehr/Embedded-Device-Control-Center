find_package(Qt6 REQUIRED COMPONENTS Test)


# ---------------- Protocol / Packet Parser ----------------
add_executable(test_protocol
    test_protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/communication/SimplePacketParser.cpp
    ${CMAKE_SOURCE_DIR}/src/communication/SimplePacketParser.h
    ${CMAKE_SOURCE_DIR}/src/core/IPacketParser.h
    ${CMAKE_SOURCE_DIR}/src/core/IPacketParser.cpp
)

target_include_directories(test_protocol
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_protocol
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(NAME test_protocol COMMAND test_protocol)

# ---------------- JSON Config ----------------
add_executable(test_jsonconfig
    test_jsonconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/JsonConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/JsonConfig.h
    ${CMAKE_SOURCE_DIR}/src/core/Types.h

)

target_include_directories(test_jsonconfig
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_jsonconfig
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(NAME test_jsonconfig COMMAND test_jsonconfig)

# ---------------- Firmware Updater ----------------
add_executable(test_firmware
    test_firmware.cpp

    ${CMAKE_SOURCE_DIR}/src/firmware/FirmwareUpdater.cpp
    ${CMAKE_SOURCE_DIR}/src/firmware/FirmwareUpdater.h
    ${CMAKE_SOURCE_DIR}/src/firmware/FirmwareUpdateWorker.cpp
    ${CMAKE_SOURCE_DIR}/src/firmware/FirmwareUpdateWorker.h
    ${CMAKE_SOURCE_DIR}/src/firmware/DummyMemoryTarget.cpp
    ${CMAKE_SOURCE_DIR}/src/firmware/DummyMemoryTarget.h
    ${CMAKE_SOURCE_DIR}/src/firmware/IMemoryTarget.h

    ${CMAKE_SOURCE_DIR}/src/logging/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/logging/Logger.h


)

target_include_directories(test_firmware
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_firmware
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(NAME test_firmware COMMAND test_firmware)


# ---------------- Serial Bootloader Target ----------------
add_executable(test_serialbootloader
    test_serialbootloader.cpp
    MockCommunication.h

    # Firmware
    ${CMAKE_SOURCE_DIR}/src/firmware/SerialBootloaderTarget.cpp
    ${CMAKE_SOURCE_DIR}/src/firmware/SerialBootloaderTarget.h
    ${CMAKE_SOURCE_DIR}/src/firmware/IMemoryTarget.h
    ${CMAKE_SOURCE_DIR}/src/firmware/protocol/BootloaderProtocol.h

    # Core / Device
    ${CMAKE_SOURCE_DIR}/src/core/Device.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Device.h
    ${CMAKE_SOURCE_DIR}/src/core/DeviceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/DeviceManager.h
    ${CMAKE_SOURCE_DIR}/src/core/IDevice.h
    ${CMAKE_SOURCE_DIR}/src/core/ICommunication.h
    ${CMAKE_SOURCE_DIR}/src/core/Types.h
    ${CMAKE_SOURCE_DIR}/src/core/IPacketParser.h
    ${CMAKE_SOURCE_DIR}/src/core/IPacketParser.cpp

    # Communication worker (required by Device)
    ${CMAKE_SOURCE_DIR}/src/communication/CommunicationWorker.cpp
    ${CMAKE_SOURCE_DIR}/src/communication/CommunicationWorker.h
    ${CMAKE_SOURCE_DIR}/src/communication/SerialCommunication.h
    ${CMAKE_SOURCE_DIR}/src/communication/SerialCommunication.cpp
    ${CMAKE_SOURCE_DIR}/src/communication/TcpCommunication.h
    ${CMAKE_SOURCE_DIR}/src/communication/TcpCommunication.cpp
    ${CMAKE_SOURCE_DIR}/src/communication/SimplePacketParser.cpp
    ${CMAKE_SOURCE_DIR}/src/communication/SimplePacketParser.h

    # Logging
    ${CMAKE_SOURCE_DIR}/src/logging/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/logging/Logger.h

    # Utils (if DeviceManager uses JsonConfig)
    ${CMAKE_SOURCE_DIR}/src/utils/JsonConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/JsonConfig.h
)

target_include_directories(test_serialbootloader
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_serialbootloader
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::SerialPort
        Qt6::Network
)

add_test(NAME test_serialbootloader COMMAND test_serialbootloader)
