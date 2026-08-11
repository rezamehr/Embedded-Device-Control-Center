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

    ${CMAKE_SOURCE_DIR}/src/firmware/SerialBootloaderTarget.cpp
    ${CMAKE_SOURCE_DIR}/src/firmware/SerialBootloaderTarget.h
    ${CMAKE_SOURCE_DIR}/src/firmware/IMemoryTarget.h
    ${CMAKE_SOURCE_DIR}/src/firmware/protocol/BootloaderProtocol.h

    ${CMAKE_SOURCE_DIR}/src/logging/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/logging/Logger.h
    ${CMAKE_SOURCE_DIR}/src/core/ICommunication.h
    ${CMAKE_SOURCE_DIR}/src/core/Types.h
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
)

add_test(NAME test_serialbootloader COMMAND test_serialbootloader)

