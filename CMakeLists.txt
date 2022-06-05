project(focuser)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

set(CMAKE_C_FLAGS "-Os -mmcu=attiny88")

add_subdirectory(core)

add_executable(firmware.elf main.c uln2003.c)
target_include_directories(firmware.elf PUBLIC .)
target_link_libraries(firmware.elf focuser_core)

add_custom_target(firmware.bin avr-objcopy -O binary firmware.elf firmware.bin DEPENDS firmware.elf)
add_custom_target(flash avrdude -pt88 -cusbasp -U flash:w:firmware.bin DEPENDS firmware.bin)
