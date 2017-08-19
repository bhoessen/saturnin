# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.

# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find cppunit
# Find the native CPPUNIT headers and libraries.
#
# CPPUNIT_INCLUDE_DIRS	- where to find cppunit/TestCase.h, etc.
# CPPUNIT_LIBRARIES	- List of libraries when using sqlite.
# CPPUNIT_FOUND	- True if sqlite found.

# Look for the header file.
FIND_PATH(CPPUNIT_INCLUDE_DIR NAMES cppunit/TestCase.h)

# Look for the library.
FIND_LIBRARY(CPPUNIT_LIBRARY NAMES cppunit)

# Handle the QUIETLY and REQUIRED arguments and set CPPUNIT_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CPPUNIT DEFAULT_MSG CPPUNIT_LIBRARY CPPUNIT_INCLUDE_DIR)

# Copy the results to the output variables.
IF(CPPUNIT_FOUND)
	SET(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY})
	SET(CPPUNIT_INCLUDE_DIRS ${CPPUNIT_INCLUDE_DIR})
ELSE(CPPUNIT_FOUND)
	SET(CPPUNIT_LIBRARIES)
	SET(CPPUNIT_INCLUDE_DIRS)
ENDIF(CPPUNIT_FOUND)

MARK_AS_ADVANCED(CPPUNIT_INCLUDE_DIRS CPPUNIT_LIBRARIES)
