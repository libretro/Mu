# Only include the sub-project if Qt was actually found
find_package(Qt5 COMPONENTS Core Gui Widgets Multimedia Svg)
if(Qt5_FOUND EQUAL 0)
	message("Qt5 or one of their components were not found, check readme.md!")
endif()

# Was this actually found now?
if(NOT Qt5_FOUND EQUAL 0)
	# Information!
	message("Found Qt5, so including the interface as an option.")

	# Where the project really exists
	add_subdirectory(Mu)
endif()