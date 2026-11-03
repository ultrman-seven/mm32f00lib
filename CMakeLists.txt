cmake_minimum_required(VERSION 3.22)
enable_language(C CXX ASM)

# generated symbols (macros)
set(Hal_pp_Defines_Syms
)

set(Hal_Source
    ./lib/cppHalReg.cpp
    ./lib/system_mm32f0010.c
    ./lib/mm32CppWyHAL/wyTimer.cpp
    ./lib/mm32CppWyHAL/wyAdc.cpp
    ./lib/mm32CppWyHAL/wySys.cpp
    ./lib/mm32CppWyHAL/wySpi.cpp
    ./lib/mm32CppWyHAL/wyI2c.cpp
    ./lib/mm32CppWyHAL/wyGpio.cpp
    ./lib/mm32CppWyHAL/wyUart.cpp
    ./lib/mm32CppWyHAL/wyFlash.cpp
    ./lib/mm32CppWyHAL/wyIstream.cpp
    ./lib/mm32CppWyHAL/wyOstream.cpp
)

set(Hal_Inc
    ./lib/
    ./lib/ADT
    ./lib/CMSIS
    ./lib/Include
    ./lib/mm32CppWyHAL
)

# Interface library for includes and symbols
add_library(mm32 INTERFACE)

target_include_directories(mm32 INTERFACE
${Hal_Inc}
)
target_sources(mm32 INTERFACE
./lib/startup_mm32f0010_gcc.s
${Hal_Source}
)

target_compile_definitions(mm32 INTERFACE
${Hal_pp_Defines_Syms}
)

add_library(mm32g0001 INTERFACE)

target_include_directories(mm32g0001 INTERFACE
${Hal_Inc}
)
target_sources(mm32g0001 INTERFACE
./lib/startup_mm32f0010_gcc.s
${Hal_Source}
)

target_compile_definitions(mm32g0001 INTERFACE
${Hal_pp_Defines_Syms}
__Chip_MM32G0001
)


# Add the map file to the list of files to be removed with 'clean' target
# set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)
if((CMAKE_C_STANDARD EQUAL 90) OR(CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
