# Please see README.md for build instructions

cmake_minimum_required(VERSION 3.16.4)

set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_SYSTEM_PROCESSOR arm)

# Use add_library() with the STATIC option to name the source file in the generated project.
# This avoids running the linker and is intended for use with cross-compiling toolchains that
# cannot link without custom flags or linker scripts
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo MinSizeRel)

if(BUILD_KIT)
	message(NOTICE "Build Kit is defined as ${BUILD_KIT}")
endif()

message(NOTICE "CMAKE_CURRENT_BINARY_DIR is defined as ${CMAKE_CURRENT_BINARY_DIR}")

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    message(NOTICE "Multi-config generator(${CMAKE_GENERATOR}) is active")
else()
    message(NOTICE "Single-config generator(${CMAKE_GENERATOR}) is active")
	if(DEFINED CMAKE_BUILD_TYPE)
	 	message(NOTICE "CMAKE_BUILD_TYPE is defined as ${CMAKE_BUILD_TYPE}")
	elseif(DEFINED BUILD_TYPE)
		set(CMAKE_BUILD_TYPE ${BUILD_TYPE})
		message(NOTICE "CMAKE_BUILD_TYPE is set to BUILD_TYPE ENV ${CMAKE_BUILD_TYPE}")
	else()
		message(FATAL_ERROR "Please specify the CMAKE_BUILD_TYPE with -DCMAKE_BUILD_TYPE=")
	endif()
    #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
	message(NOTICE "CMAKE_RUNTIME_OUTPUT_DIRECTORY is defined as ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

cmake_minimum_required(VERSION 3.16.4)

# Project configuration
#
# Derived from the containing folder name rather than hardcoded, so it always
# matches the project name (.project / ${ProjName}) after a copy/
# rename - the CMake build and the build must produce the same
# artifact name, since everything downstream uses ${PROJECT_NAME}.
get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJ_NAME}
	VERSION 1.0.0
	LANGUAGES C CXX ASM)

set(CMAKE_CROSSCOMPILING "TRUE")
set(CMAKE_EXPORT_COMPILE_COMMANDS "TRUE")

include(${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedCfg.cmake)

# If required user can amend RASC generated flags here, e.g. list(REMOVE_ITEM RASC_CMAKE_C_FLAGS "-Wall")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc.cmake)

# Include paths for the app/RS sources under src/,
# which #include headers via paths relative to these directories (e.g. "gatt_db.h",
# "profile_cmn/r_ble_servs_if.h").
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/ble
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/rm_icm42670
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/rm_icm42670/data_ready_mode
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rs_ble_sensor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT)

# Comment out to compile out all RTT debug code
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE RTT_DEBUG_ON)


# Vendor/FSP code is not warning-clean under RASC's -Wconversion/-Wall/-Wextra flags
file(GLOB_RECURSE _VENDOR_NO_WARN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/renesas/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/aws/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/arm/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/fsp/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/intel/tinycrypt/lib/source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT/*.c)

set_source_files_properties(${_VENDOR_NO_WARN_SRC} PROPERTIES COMPILE_OPTIONS -w)