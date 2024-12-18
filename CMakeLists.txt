#cmake build for test/debugging purposes only
cmake_minimum_required(VERSION 3.30)
project(joypad-for-debian)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    joypad.cpp
    config.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

include_directories(${LIBEVDEV_INCLUDE_DIRS} ./)

link_directories(${LIBEVDEV_LIBRARY_DIRS})

add_executable(joypad ${SOURCES})

target_link_libraries(joypad ${LIBEVDEV_LIBRARIES})