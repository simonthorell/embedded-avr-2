# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.22.2)

# Project name
project("avr-firmware")

# AVR Chip Configuration 
# (used for option of flashing directly on CMake build)
set(F_CPU 16000000UL)      # Clock frequency
set(MCU atmega328p)        # Microcontroller unit
set(BAUD 9600)             # UART baud rate
set(PROG_TYPE arduino)     # Programmer type
set(USB_PORT /dev/tty.usbserial-110) # Serial port for programming

# AVR Fuses - adjust according to your hardware requirements
set(E_FUSE 0xfd)
set(H_FUSE 0xda)
set(L_FUSE 0xfd)
set(LOCK_BIT 0xff)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Compiler and linker flags
add_compile_definitions(F_CPU=${F_CPU} BAUD=${BAUD})
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU}")

add_compile_options(
    -mmcu=${MCU} # specify MCU model
    -Os          # optimize size
    -std=c++17   # C++ standard
    -Wall        # enable warnings
    -Wextra
    -Wundef
    -pedantic
)

# Source and include files
file(GLOB SRC_FILES CMAKE_CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB INC_FILES CMAKE_CONFIGURE_DEPENDS "libs/*.cpp")

# Executable target
add_executable(${PROJECT_NAME} ${SRC_FILES} ${INC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include src libs)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

# Custom targets for AVR programming
add_custom_target(strip ALL avr-strip ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS strip)
add_custom_target(eeprom avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.eep DEPENDS strip)
add_custom_target(upload avrdude -c ${PROG_TYPE} -p ${MCU} -P ${USB_PORT} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(upload_eeprom avrdude -c ${PROG_TYPE} -p ${MCU} -U eeprom:w:${PROJECT_NAME}.eep DEPENDS eeprom)
add_custom_target(fuses avrdude -c ${PROG_TYPE} -p ${MCU} -U lfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m -U efuse:w:${E_FUSE}:m -U lock:w:${LOCK_BIT}:m)

# Clean extra files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eep;${PROJECT_NAME}.lst")