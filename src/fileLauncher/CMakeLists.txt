# m68k Module
add_library(MuCoreFileLauncher STATIC
	launcher.c)

# Make this position independent so it can be linked into shared libraries
if(MU_FPIC)
	set_property(TARGET MuCoreFileLauncher
		PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Includes for the project
target_include_directories(MuCoreFileLauncher PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include")