# Base Mu library
add_library(MuCore STATIC
	ads7846.c
	dbvz.c
	emulator.c
	flx68000.c
	m5XXBus.c
	pdiUsbD12.c
	sdCard.c
	sed1376.c
	silkscreen.c
	tps65010.c
	tsc2101.c
	w86l488.c)

# Make this position independent so it can be linked into shared libraries
if(MU_FPIC)
	set_property(TARGET MuCore
		PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Bring all the sub-modules as needed
if(MU_ARM)
	target_link_libraries(MuCore
		MuCorePxa260Experimental
		MuCoreFileLauncher
		MuCoreAudio
		MuCoreM68k)
else()
	target_link_libraries(MuCore
		MuCoreFileLauncher
		MuCoreAudio
		MuCoreM68k)
endif()

# Includes for the project
target_include_directories(MuCore PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include")

# Core Mu
add_subdirectory(audio)
add_subdirectory(fileLauncher)

# Classic m68k Core
add_subdirectory(m68k)

# Experimental ARM Cores
if(MU_ARM)
	add_subdirectory(armv5te)
	add_subdirectory(pxa260)
endif()