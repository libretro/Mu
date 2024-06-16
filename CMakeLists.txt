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

# Main project sources
add_subdirectory(src)

# LibRetro Build
add_subdirectory(libretroBuildSystem)

# QT Build
add_subdirectory(qtBuildSystem)