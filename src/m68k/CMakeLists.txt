# m68k Module
add_library(MuCoreM68k ${MU_CORE_BUILD_TYPE}
	m68kcpu.c
	m68kdasm.c
	m68kopac.c
	m68kopdm.c
	m68kopnz.c
	m68kops.c)

# Make this position independent so it can be linked into shared libraries
if(MU_FPIC)
	set_property(TARGET MuCoreM68k
		PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Includes for the project
target_include_directories(MuCoreM68k PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include")
