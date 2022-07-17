cmake_minimum_required(VERSION 3.19)
project(kaleidoscope_llvm)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)
set(MY_CALC_LLVM_CONFIG /opt/homebrew/opt/llvm/bin/llvm-config)

message(STATUS "detected for llvm-config: ${MY_CALC_LLVM_CONFIG}")

# LLVM configurations
execute_process(
        COMMAND
        ${MY_CALC_LLVM_CONFIG} --version
        OUTPUT_VARIABLE
        LLVM_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND
        ${MY_CALC_LLVM_CONFIG} --includedir
        OUTPUT_VARIABLE
        LLVM_INCLUDE_DIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND
        ${MY_CALC_LLVM_CONFIG} --libdir
        OUTPUT_VARIABLE
        LLVM_LIB_DIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND
        ${MY_CALC_LLVM_CONFIG} --ldflags
        OUTPUT_VARIABLE
        LLVM_SYS_LIBS_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX MATCHALL "-l([^ ]+)" LLVM_SYS_LIBS ${LLVM_SYS_LIBS_STRING})

execute_process(
        COMMAND
        ${MY_CALC_LLVM_CONFIG} --libs core
        OUTPUT_VARIABLE
        LLVM_LIBS_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX MATCHALL "-l([^ ]+)" LLVM_LIBS ${LLVM_LIBS_STRING})

set(REQUIRED_LLVM_LIBS)
foreach (l ${LLVM_LIBS} ${LLVM_SYS_LIBS})
    string(REGEX REPLACE "-l([^ ]+)" "\\1" lib_name ${l})
    list(APPEND REQUIRED_LLVM_LIBS ${lib_name})
endforeach (l)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND REQUIRED_LLVM_LIBS "pthread" "curses" "dl")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIB_DIRS})
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)


message(STATUS "LLVM version: ${LLVM_VERSION}")
message(STATUS "LLVM includedir: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libdir: ${LLVM_LIB_DIRS}")

add_definitions("-Wall -std=c++1z")

subdirs(src)