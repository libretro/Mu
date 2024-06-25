# Experimental ARM V5TE Module
add_library(MuCoreArmV5TEExperimental STATIC
	uArm/CPU_2.c
	uArm/icache.c
	uArm/uArmGlue.cpp
	arm_interpreter.cpp
	asmcode.c
	coproc.cpp
	cpu.cpp
	disasm.c
	emuVarPool.c
	mem.c
	mmu.c
	thumb_interpreter.cpp)

# Make this position independent so it can be linked into shared libraries
if(MU_FPIC)
	set_property(TARGET MuCoreArmV5TEExperimental
		PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Currently do not enable the dynamic recompiler
set(NO_TRANSLATION)

# Dynamic Recompiler not supported
if(NOT DEFINED NO_TRANSLATION)
target_compile_definitions(MuCoreArmV5TEExperimental

	# Disable Dynamic Recompiler (non-portable)
	PUBLIC NO_TRANSLATION=1)
else()
	# Include Translators
	if(CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)
		target_sources(MuCoreArmV5TEExperimental PUBLIC
			translate_aarch64.cpp)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES arm)
		target_sources(MuCoreArmV5TEExperimental PUBLIC
			translate_arm.cpp)
	elseif(MU_IS_X86_32 OR CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(i386)")
		target_sources(MuCoreArmV5TEExperimental PUBLIC
			translate_x86.c)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(x86-64)|(amd64)|(AMD64)")
		target_sources(MuCoreArmV5TEExperimental PUBLIC
			translate_x86_64.c)
	endif()
endif()

# Some defines are required for this to function properly
target_compile_definitions(MuCoreArmV5TEExperimental
	# Enable PalmOS Support
	PUBLIC EMU_SUPPORT_PALM_OS5=1

	# forces the dynarec to use accurate mode and disable Nspire OS hacks
	PUBLIC SUPPORT_LINUX=1)

# Windows?
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
	target_sources(MuCoreArmV5TEExperimental PUBLIC
		os/os-win32.c)

# Everything else?
else()
	target_sources(MuCoreArmV5TEExperimental PUBLIC
		os/os-linux.c)
endif()

# Includes for the project
target_include_directories(MuCoreArmV5TEExperimental PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include")
