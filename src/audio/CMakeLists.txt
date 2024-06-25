# m68k Module
add_library(MuCoreAudio ${MU_CORE_BUILD_TYPE}
	blip_buf.c)

# Make this position independent so it can be linked into shared libraries
if(MU_FPIC)
	set_property(TARGET MuCoreAudio
		PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Includes for the project
target_include_directories(MuCoreAudio PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include")