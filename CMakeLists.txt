cmake_minimum_required(VERSION 3.5)

project(dirtyInject)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCE_FILES
        src/dirtyInject.cpp
        src/utils/badDll.cpp
        src/utils/badSo.cpp
)

if (WIN32)
    list(APPEND SOURCE_FILES src/windows/winInject.cpp)
elseif (UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES src/linux/linInject.cpp)
elseif (APPLE)
    list(APPEND SOURCE_FILES src/macOS/macInject.cpp)
    list(APPEND SOURCE_FILES src/utils/badDylib.mm)
endif()

if (WIN32)
    add_library(badDll SHARED src/utils/badDll.cpp)
    add_executable(dirtyInject ${SOURCE_FILES})
    target_link_libraries(dirtyInject badDll)
elseif (UNIX AND NOT APPLE)
    add_library(badSo SHARED src/utils/badSo.cpp)
    target_compile_definitions(badSo PRIVATE LINUX=1)
    set_target_properties(badSo PROPERTIES OUTPUT_NAME "badSo")
    set_target_properties(badSo PROPERTIES PREFIX "")
    add_executable(dirtyInject ${SOURCE_FILES})
    target_link_libraries(dirtyInject badSo)
elseif (APPLE)
    add_library(badDylib SHARED src/utils/badDylib.mm)
    add_executable(dirtyInject ${SOURCE_FILES})
    target_link_libraries(badDylib "-framework Cocoa")
    target_link_libraries(dirtyInject badDylib)
endif()
