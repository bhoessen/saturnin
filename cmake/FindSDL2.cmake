# - Find SDL2
# Find the native SDL2 headers and libraries.
#
# SDL2_INCLUDE_DIRS	- where to find SDL2/TestCase.h, etc.
# SDL2_LIBRARIES	- List of libraries when using sqlite.
# SDL2_FOUND	- True if sqlite found.

# Look for the header file.
FIND_PATH(SDL2_INCLUDE_DIR NAMES SDL2/SDL.h)

# Look for the library.
FIND_LIBRARY(SDL2_LIBRARY NAMES SDL)

# Handle the QUIETLY and REQUIRED arguments and set SDL2_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR)

# Copy the results to the output variables.
IF(SDL2_FOUND)
	SET(SDL2_LIBRARIES ${SDL2_LIBRARY})
	SET(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}/SDL2")
	MESSAGE(STATUS "Using SDL2 headers: " ${SDL2_INCLUDE_DIRS})
ELSE(SDL2_FOUND)
	SET(SDL2_LIBRARIES)
	SET(SDL2_INCLUDE_DIRS)
ENDIF(SDL2_FOUND)

MARK_AS_ADVANCED(SDL2_INCLUDE_DIRS SDL2_LIBRARIES)
