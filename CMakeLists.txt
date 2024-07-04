cmake_minimum_required(VERSION 3.5)

project(dirtyInject)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCE_FILES
    src/windows/winInject.cpp
    src/linux/linInject.cpp
    src/macOS/macInject.cpp
    src/dirtyInject.cpp
    src/utils/baddll.cpp
    src/utils/badSo.cpp
)

if (APPLE)
    list(APPEND SOURCE_FILES src/utils/badDylib.mm)
endif()

if (WIN32)
    add_library(baddll SHARED src/utils/baddll.cpp)
    add_executable(dirtyInject ${SOURCE_FILES})
    target_link_libraries(dirtyInject baddll)
elseif (UNIX AND NOT APPLE)
    add_library(badSo SHARED src/utils/badSo.cpp)
    target_compile_definitions(badSo PRIVATE LINUX=1)
    set_target_properties(badSo PROPERTIES OUTPUT_NAME "badSo")
    set_target_properties(badSo PROPERTIES PREFIX "")
elseif (APPLE)
    add_library(badDylib SHARED src/utils/badDylib.mm)
    target_compile_definitions(badDylib PRIVATE MACOS=1)
    set_target_properties(badDylib PROPERTIES OUTPUT_NAME "badDylib")
    set_target_properties(badDylib PROPERTIES PREFIX "")
endif()

