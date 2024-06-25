cmake_minimum_required (VERSION 3.13)
project(Mu
	VERSION 1.3.2
	DESCRIPTION "Classic Palm OS Emulator."
	HOMEPAGE_URL https://github.com/libretro/Mu
	LANGUAGES C CXX)

# Requires C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# To Emily...
message("******************************")
message("The continuation of the Mu along with the RetroArch Core is dedicated to")
message("Emily (1998-2020), your friendship was very important to me and I hope")
message("that you are resting well.")
message(" -- Your friend, Stephanie")
message("******************************")

# Is this x86_32?
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "X86" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_32" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86-32" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i486" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i586" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686" OR
	"${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "ia32" OR
	"$ENV{ARCH}" STREQUAL "x86")
	set(MU_IS_X86_32 YES)
endif()

# Some platforms cannot use FPIC code
if(DJGPP OR PLATFORM_PSP OR PSP)
	set(MU_FPIC OFF)
else()
	set(MU_FPIC ON)
endif()

# Do not include experimental ARM core for some platforms
if(DJGPP OR PLATFORM_PSP OR PSP OR (APPLE AND MU_IS_X86_32))
	set(MU_ARM OFF)
else()
	set(MU_ARM ON)
endif()

# RetroArch must be static
if(DJGPP OR PLATFORM_PSP OR PSP)
	set(MU_LIBRETRO_OBJECT_LIB ON)
else()
	set(MU_LIBRETRO_OBJECT_LIB OFF)
endif()

if(MU_LIBRETRO_OBJECT_LIB)
	set(MU_CORE_BUILD_TYPE OBJECT)
else()
	set(MU_CORE_BUILD_TYPE STATIC)
endif()

# Where should dynamic libraries go when output?
if(NOT DEFINED MU_DYLIB_OUTPUT_DIR)
	set(MU_DYLIB_OUTPUT_DIR
		"${CMAKE_BINARY_DIR}")
endif()

# Force a specific name for the output resultant binary
macro(mu_target_binary_name target what)
	# Base properties
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_NAME "${what}"
		LIBRARY_OUTPUT_NAME "${what}"
		ARCHIVE_OUTPUT_NAME "${what}")

	# Then for each configuration
	foreach(outputConfig ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER "${outputConfig}" outputConfig)

		set_target_properties(${target} PROPERTIES
			RUNTIME_OUTPUT_NAME_${outputConfig} "${what}"
			LIBRARY_OUTPUT_NAME_${outputConfig} "${what}"
			ARCHIVE_OUTPUT_NAME_${outputConfig} "${what}")
	endforeach()
endmacro()

# Need to set specific locations for output libraries?
# Note that RUNTIME_OUTPUT_DIRECTORY is needed for the Windows build to output
# directories since .DLL files are output there and not where shared libraries
# go??? No idea really.
macro(mu_target_binary_output target where)
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${where}"
		LIBRARY_OUTPUT_DIRECTORY "${where}"
		ARCHIVE_OUTPUT_DIRECTORY "${where}")

	foreach(outputConfig ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER "${outputConfig}" outputConfig)

		set_target_properties(${target} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${outputConfig} "${where}"
			LIBRARY_OUTPUT_DIRECTORY_${outputConfig} "${where}"
			ARCHIVE_OUTPUT_DIRECTORY_${outputConfig} "${where}")
	endforeach()
endmacro()

# Main project sources
add_subdirectory(src)

# LibRetro Build
add_subdirectory(libretroBuildSystem)

# QT Build
add_subdirectory(qtBuildSystem)