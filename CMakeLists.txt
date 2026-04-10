cmake_minimum_required(VERSION 3.16)

project(ghostlayer
        VERSION 0.0.1
        DESCRIPTION "GTK CCSDS analyzer"
        LANGUAGES C
)

include(FetchContent)

FetchContent_Declare(
        ccsds
        GIT_REPOSITORY https://github.com/JahazielLem/libccsds.git
        GIT_TAG main
)

FetchContent_MakeAvailable(ccsds)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

execute_process(
        COMMAND git describe --tags --dirty --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX REPLACE "^v" "" GIT_VERSION ${GIT_VERSION})
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" PROJECT_VERSION_CLEAN ${GIT_VERSION})

if (NOT PROJECT_VERSION_CLEAN)
    set(PROJECT_VERSION_CLEAN "0.0.0")
endif()

configure_file(
        ${CMAKE_SOURCE_DIR}/include/version.h.in
        ${CMAKE_BINARY_DIR}/generated/version.h
        @ONLY
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.c)

add_executable(ghostlayer ${SOURCES})

target_include_directories(ghostlayer
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/generated
        ${GTK_INCLUDE_DIRS}
)

target_link_libraries(ghostlayer
        PRIVATE
        ccsds
        ${GTK_LIBRARIES}
)

target_compile_features(ghostlayer PRIVATE c_std_99)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ghostlayer PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(ghostlayer PRIVATE /W4)
endif()

message(STATUS "Targets: ${ccsds_SOURCE_DIR}")