cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(CMAKE_CROSSCOMPILING)
message(STATUS "Cross compiling for board...")

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME fly_drone)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

add_compile_options(
    -Os
    -DUSE_HAL_DRIVER 
    -DSTM32F103xB 
    -ffunction-sections 
    -fdata-sections 
    -fno-exceptions 
    -fno-rtti 
    -flto
)
add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    # -specs=nano.specs
    -specs=nosys.specs
)
# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_SOURCE_DIR}/Core/App/MotorControl.c
    ${CMAKE_SOURCE_DIR}/Core/App/mavlink_handler.c
    ${CMAKE_SOURCE_DIR}/Core/App/params.c
    ${CMAKE_SOURCE_DIR}/Core/App/spi.c
    ${CMAKE_SOURCE_DIR}/Core/App/system.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/MPU9250.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/MahonyAHRS.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/VL53L0X.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/fast_atan.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/imu.c
    
    ${CMAKE_SOURCE_DIR}/Drivers/modem/ModemControl.c
    ${CMAKE_SOURCE_DIR}/Drivers/modem/cb.c
    ${CMAKE_SOURCE_DIR}/Drivers/modem/si4463.c

)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_SOURCE_DIR}/Core/App
    ${CMAKE_SOURCE_DIR}/Drivers/imu
    ${CMAKE_SOURCE_DIR}/Drivers/mavlink
    ${CMAKE_SOURCE_DIR}/Drivers/mavlink/common
    ${CMAKE_SOURCE_DIR}/Drivers/modem

)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
)


set(CUBE_PROGR "/home/mbaranov/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI")

set(ELF_FILE ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin)
set(BIN_BL_FILE ${PROJECT_BINARY_DIR}/bootloader.bin)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        # COMMAND ${CMAKE_OBJCOPY} -Oihex ${ELF_FILE} ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_FILE} ${BIN_FILE}
        COMMENT "Building ${BIN_FILE}"
        )


            
else () # CMAKE_CROSSCOMPILING

    message(STATUS "Compiling for PC...")

    project(host C CXX)

# Set the project name
set(CMAKE_PROJECT_NAME host)

set(CMAKE_C_FLAGS_RELEASE "-O0")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")


add_executable( ${CMAKE_PROJECT_NAME}
                # ${SRC_PATH}/common.c

                ${CMAKE_SOURCE_DIR}/tests/host/host.c
                ${CMAKE_SOURCE_DIR}/tests/host/log.c
                ${CMAKE_SOURCE_DIR}/tests/host/tty.c
                # ${CMAKE_SOURCE_DIR}/tests/host/cb.c
                ${CMAKE_SOURCE_DIR}/tests/host/network.c


                
              )

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_SOURCE_DIR}/Core/App/MotorControl.c
    ${CMAKE_SOURCE_DIR}/Core/App/mavlink_handler.c
    ${CMAKE_SOURCE_DIR}/Core/App/params.c
    ${CMAKE_SOURCE_DIR}/Core/App/spi.c
    ${CMAKE_SOURCE_DIR}/Core/App/system.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/MPU9250.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/MahonyAHRS.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/VL53L0X.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/fast_atan.c
    ${CMAKE_SOURCE_DIR}/Drivers/imu/imu.c
    
    ${CMAKE_SOURCE_DIR}/Drivers/modem/ModemControl.c
    ${CMAKE_SOURCE_DIR}/Drivers/modem/cb.c
    ${CMAKE_SOURCE_DIR}/Drivers/modem/si4463.c


    ${CMAKE_SOURCE_DIR}/Drivers/imu/vector_math.c
    
                ${CMAKE_SOURCE_DIR}/tests/host/imu_data_generate.c

)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_SOURCE_DIR}/tests/host
    ${CMAKE_SOURCE_DIR}/Core/App
    ${CMAKE_SOURCE_DIR}/Drivers/imu
    ${CMAKE_SOURCE_DIR}/Drivers/mavlink
    ${CMAKE_SOURCE_DIR}/Drivers/mavlink/common
    ${CMAKE_SOURCE_DIR}/Drivers/modem

)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE m)

set(TEST_SRC 
                ${CMAKE_SOURCE_DIR}/tests/unity/unity.c
                ${CMAKE_SOURCE_DIR}/Drivers/imu/vector_math.c
                ${CMAKE_SOURCE_DIR}/Core//App/system.c
                ${CMAKE_SOURCE_DIR}/tests/host/log.c
                ${CMAKE_SOURCE_DIR}/tests/host/imu_data_generate.c)

set(TEST_INC
    ${CMAKE_SOURCE_DIR}/Drivers/imu
    ${CMAKE_SOURCE_DIR}/tests/unity
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/tests/host
    ${CMAKE_SOURCE_DIR}/Core/App) 

add_executable( test_gyro_calibrate

                ${CMAKE_SOURCE_DIR}/tests/test_gyro_calibrate.c
                ${CMAKE_SOURCE_DIR}/Drivers/imu/gyro_calibration.c
                ${TEST_SRC}
              )



# Add include paths
target_include_directories(test_gyro_calibrate PRIVATE
    # Add user defined include paths
    ${TEST_INC}
)

# Add project symbols (macros)
target_compile_definitions(test_gyro_calibrate PRIVATE
    # Add user defined symbols
    UNIT_TEST
    TEST
)

add_executable( test_acc_calibrate

                ${CMAKE_SOURCE_DIR}/tests/test_acc_calibrate.c
                ${CMAKE_SOURCE_DIR}/Drivers/imu/acc_calibration.c
                ${TEST_SRC}
              )


# Add include paths
target_include_directories(test_acc_calibrate PRIVATE
    # Add user defined include paths
    ${TEST_INC}
)

# Add project symbols (macros)
target_compile_definitions(test_acc_calibrate PRIVATE
    # Add user defined symbols
    UNIT_TEST
    TEST
)

# link_directories(/system/lib64 /system/lib)
# target_link_libraries(test_rearrange PRIVATE m)

# target_link_libraries(test_rearrange PRIVATE /system/lib64/libm.so)

# target_link_libraries(test_rearrange PRIVATE c)

add_executable( test_rearrange

                ${CMAKE_SOURCE_DIR}/tests/test_rearrange.c
                ${CMAKE_SOURCE_DIR}/Drivers/imu/imu_rearrange.c
                ${CMAKE_SOURCE_DIR}/tests/host/imu_data_generate.c
                ${TEST_SRC}
              )


# Add include paths
target_include_directories(test_rearrange PRIVATE
    # Add user defined include paths
    ${TEST_INC}
)

# Add project symbols (macros)
target_compile_definitions(test_rearrange PRIVATE
    # Add user defined symbols
    UNIT_TEST
    TEST
)

# set_target_properties(test_rearrange PROPERTIES 
#    LINK_FLAGS "-lm"
#)

target_link_libraries(test_rearrange PRIVATE m)

# // # Пример Makefile для тестов
# // test: test_gyro_calibration
# // 	./test_gyro_calibration --verbose

# // test_gyro_calibration: test_gyro_calibration.c gyro_calibration.c
# // 	gcc -DUNIT_TEST -DTEST -Wall -o $@ $^ -lm
# // 	./$@
	



# set(CMAKE_REQUIRED_LIBRARIES m)

# add_compile_options(
#     -lm
# )
# add_link_options(
#     -lm
# )

# add_library( ${CMAKE_PROJECT_NAME} 
#     libm
# )
# target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE m)

endif ()  # CMAKE_CROSSCOMPILING
