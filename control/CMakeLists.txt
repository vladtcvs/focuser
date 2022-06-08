project(control)
cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

add_library(focuser SHARED focuser.cc)
target_include_directories(focuser PUBLIC ${LIBUSB_INCLUDE_DIRS} .)
target_link_libraries(focuser ${LIBUSB_LIBRARIES})

add_executable(control_example example.cc)
target_link_libraries(control_example focuser)
