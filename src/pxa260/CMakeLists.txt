# PXA260 Module
add_library(MuCorePxa260Experimental STATIC
	pxa260.c
	pxa260_DMA.c
	pxa260_DSP.c
	pxa260_GPIO.c
	pxa260_IC.c
	pxa260_LCD.c
	pxa260_PwrClk.c
	pxa260_RTC.c
	pxa260_TIMR.c
	pxa260_UART.c
	pxa260I2c.c
	pxa260Memctrl.c
	pxa260Ssp.c
	pxa260Timing.c
	pxa260Udc.c)

# Make this position independent so it can be linked into shared libraries
if(MU_FPIC)
	set_property(TARGET MuCorePxa260Experimental
		PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Bring all the sub-modules as needed
target_link_libraries(MuCorePxa260Experimental
	MuCoreArmV5TEExperimental)

# Includes for the project
target_include_directories(MuCorePxa260Experimental PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/include")
