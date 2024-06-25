add_library(mu_libretro SHARED
	libretro.c
	cursors.c
	miniz.c
	libretro-common/compat/compat_strl.c
	libretro-common/compat/compat_posix_string.c
	libretro-common/compat/fopen_utf8.c
	libretro-common/encodings/encoding_utf.c
	libretro-common/memmap/memmap.c
	libretro-common/streams/file_stream.c
	libretro-common/string/stdstring.c
	libretro-common/vfs/vfs_implementation.c)

# Remove the "lib" prefix always, RetroArch does not name in this way
set_target_properties(mu_libretro PROPERTIES PREFIX "")

# Bring all the sub-modules as needed
target_link_libraries(mu_libretro
	MuCore)

# Include the required includes
target_include_directories(mu_libretro PUBLIC
	"${PROJECT_SOURCE_DIR}/include"
	"libretro-common/include")

# Library location must be placed in the build output for the RetroArch
# build system to pick up the library
# Note that RUNTIME_OUTPUT_DIRECTORY is needed for the Windows build to output
# directories since .DLL files are output there and not where shared libraries
# go??? No idea really.
mu_target_binary_output(mu_libretro
	"${MU_DYLIB_OUTPUT_DIR}")

# Custom launching the core, tries to find RetroArch on the system
## Determine RetroArch directory
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
	set(MU_LIBRETRO_EXTENSION ".exe")

	if (EXISTS "$ENV{APPDATA}/RetroArch")
		set(MU_LIBRETRO_DIR "$ENV{APPDATA}/RetroArch")
	endif()
else()
	find_program(RETROARCH_APP retroarch)
	if(RETROARCH_APP)
		get_filename_component(MU_LIBRETRO_DIR "${RETROARCH_APP}" DIRECTORY)
	endif()

	set(MU_LIBRETRO_EXTENSION "")
endif()

## Target to run RetroArch with the Mu Core
if(DEFINED MU_LIBRETRO_DIR)
	add_custom_target(RetroArch
		DEPENDS mu_libretro
		COMMAND "${MU_LIBRETRO_DIR}/retroarch${MU_LIBRETRO_EXTENSION}" "-v" "-L" "$<TARGET_FILE:mu_libretro>"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMENT "Starting RetroArch with Mu")

	# Do not try running this by default
	set_target_properties(RetroArch PROPERTIES
			EXCLUDE_FROM_ALL TRUE
			EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif()
