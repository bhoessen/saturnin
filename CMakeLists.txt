cmake_minimum_required (VERSION 3.1)
project (saturnin)
set (CMAKE_CXX_STANDARD 11)

#Try to find all libraries that could be used for this project
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (sqlite3)
find_package (cppunit)

#specify the include directory
include_directories("include")

#create the library
file(GLOB saturnin_sources ./src/libsaturnin/*.cpp)
file(GLOB saturnin_headers ./include/saturnin/*.h)
add_library(saturnin SHARED ${saturnin_sources} ${saturnin_headers})
target_compile_definitions(saturnin PRIVATE SATURNIN_LIBRARY_COMPILE)

#create the binary
file(GLOB saturnin_bin ./src/bin/*.cpp)
add_executable(saturnin-bin ${saturnin_sources} ${saturnin_bin})
set_target_properties(saturnin-bin PROPERTIES OUTPUT_NAME saturnin)
target_compile_definitions(saturnin-bin PRIVATE SATURNIN_LIBRARY_COMPILE)

#create the parallel version of saturnin
add_executable(saturnins-bin ${saturnin_sources} ${saturnin_bin})
target_compile_definitions(saturnins-bin PRIVATE SATURNIN_PARALLEL)
target_compile_definitions(saturnins-bin PRIVATE SATURNIN_LIBRARY_COMPILE)
set_target_properties(saturnins-bin PROPERTIES OUTPUT_NAME saturnins)

add_library(saturnins SHARED ${saturnin_sources})
target_compile_definitions(saturnins PRIVATE SATURNIN_LIBRARY_COMPILE)
target_compile_definitions(saturnins PRIVATE SATURNIN_PARALLEL)

#check if sqlite exist and if so, add the unsaturnin targets
if (SQLITE3_FOUND)
  add_executable(unsaturnin-bin ${saturnin_sources} ${saturnin_bin})
  target_include_directories(unsaturnin-bin PRIVATE ${SQLITE3_INCLUDE_DIRS})
  target_compile_definitions(unsaturnin-bin PRIVATE SATURNIN_DB)
  target_compile_definitions(unsaturnin-bin PRIVATE SATURNIN_LIBRARY_COMPILE)
  target_link_libraries (unsaturnin-bin ${SQLITE3_LIBRARIES})
  set_target_properties(unsaturnin-bin PROPERTIES OUTPUT_NAME unsaturnin)
  
  add_library(unsaturnin SHARED ${saturnin_sources})
  target_include_directories(unsaturnin PRIVATE ${SQLITE3_INCLUDE_DIRS})
  target_compile_definitions(unsaturnin PRIVATE SATURNIN_DB)
  target_compile_definitions(unsaturnin PRIVATE SATURNIN_LIBRARY_COMPILE)
  
  add_library(unsaturnins SHARED ${saturnin_sources})
  target_include_directories(unsaturnins PRIVATE ${SQLITE3_INCLUDE_DIRS})
  target_compile_definitions(unsaturnins PRIVATE SATURNIN_DB)
  target_compile_definitions(unsaturnins PRIVATE SATURNIN_PARALLEL)
  target_compile_definitions(unsaturnins PRIVATE SATURNIN_LIBRARY_COMPILE)
  
endif (SQLITE3_FOUND)

if (WIN32)
  file(GLOB gsaturnin_sources ./src/gsaturnin/*.cpp)
  file(GLOB imgui_sources ./src/imgui/*.cpp)
  add_executable(gsaturnin-bin ${saturnin_sources} ${gsaturnin_sources} ${imgui_sources})
  set_target_properties(gsaturnin-bin PROPERTIES OUTPUT_NAME gsaturnin)
  target_compile_definitions(gsaturnin-bin PRIVATE SATURNIN_LIBRARY_COMPILE)
  target_compile_definitions(gsaturnin-bin PRIVATE PROFILE)
  target_link_libraries(gsaturnin-bin d3d9)
endif (WIN32)

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_definitions(-DDEBUG)
  add_definitions(-DPROFILE)
  if (CMAKE_COMPILER_IS_GNUCXX)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
  endif (CMAKE_COMPILER_IS_GNUCXX)
	
  #check if we can create the unit tests
  if (CPPUNIT_FOUND)
    file(GLOB saturnin_test_sources test/src/*.cpp)
    add_executable(saturnin-test ${saturnin_test_sources})
	#make sure we test the parallel code
	target_compile_definitions(saturnin-test PRIVATE SATURNIN_PARALLEL)
	#add the cppunit dependency
	target_include_directories(saturnin-test PRIVATE ${CPPUNIT_INCLUDE_DIRS})
	target_link_libraries (saturnin-test ${CPPUNIT_LIBRARIES})
    if (SQLITE3_FOUND)
	  target_compile_definitions(saturnin-test PRIVATE SATURNIN_DB)
	  target_include_directories(saturnin-test PRIVATE ${SQLITE3_INCLUDE_DIRS})
	  target_link_libraries (saturnin-test ${SQLITE3_LIBRARIES})
	  add_dependencies(saturnin-test unsaturnins)
	  target_link_libraries (saturnin-test unsaturnins)
	else (SQLITE3_FOUND)
	  add_dependencies(saturnin-test saturnins)
	  target_link_libraries (saturnin-test saturnins)
	endif (SQLITE3_FOUND)
	
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/all.used-as.sat04-986.simplified.cnf ${CMAKE_CURRENT_BINARY_DIR}/instances/all.used-as.sat04-986.simplified.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/dp04s04.shuffled.cnf                 ${CMAKE_CURRENT_BINARY_DIR}/instances/dp04s04.shuffled.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/dp04u03.shuffled.cnf                 ${CMAKE_CURRENT_BINARY_DIR}/instances/dp04u03.shuffled.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/dp10s10.shuffled.cnf                 ${CMAKE_CURRENT_BINARY_DIR}/instances/dp10s10.shuffled.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/manol-pipe-c9.cnf                    ${CMAKE_CURRENT_BINARY_DIR}/instances/manol-pipe-c9.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/openstacks-cnfdd.cnf                 ${CMAKE_CURRENT_BINARY_DIR}/instances/openstacks-cnfdd.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/presumed.cnf                         ${CMAKE_CURRENT_BINARY_DIR}/instances/presumed.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/schup-l2s-abp4-1-k31.cnf             ${CMAKE_CURRENT_BINARY_DIR}/instances/schup-l2s-abp4-1-k31.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/simple.cnf                           ${CMAKE_CURRENT_BINARY_DIR}/instances/simple.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/stupidUnsat.cnf                      ${CMAKE_CURRENT_BINARY_DIR}/instances/stupidUnsat.cnf)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/instances/velev-engi-uns-1.0-4nd.cnf           ${CMAKE_CURRENT_BINARY_DIR}/instances/velev-engi-uns-1.0-4nd.cnf)
	
  endif (CPPUNIT_FOUND)
  
  #Set the highest warning level
  if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
    if(CMAKE_CXX_FLAGS MATCHES "/WX")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    endif ()
  elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Werror")
  endif()
  
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
  if (CMAKE_COMPILER_IS_GNUCXX)
    #we should set the visibility only on the binary targets
	set_target_properties(saturnin-bin PROPERTIES COMPILE_FLAGS "-flto -fno-rtti -fvisibility=hidden"
	                                              LINK_FLAGS    "-fno-exceptions -fomit-frame-pointer -s -ffunction-sections -fdata-sections -Wl,--gc-sections")
    set_target_properties(saturnins-bin PROPERTIES COMPILE_FLAGS "-flto -fno-rtti -fvisibility=hidden"
	                                               LINK_FLAGS    "-fno-exceptions -fomit-frame-pointer -s -ffunction-sections -fdata-sections -Wl,--gc-sections")
	if (SQLITE3_FOUND)
	  set_target_properties(unsaturnin-bin PROPERTIES COMPILE_FLAGS "-flto -fno-rtti -fvisibility=hidden"
                                                      LINK_FLAGS    "-fno-exceptions -fomit-frame-pointer -s -ffunction-sections -fdata-sections -Wl,--gc-sections")
	endif (SQLITE3_FOUND)
  endif (CMAKE_COMPILER_IS_GNUCXX)
endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

#Make the doxygen documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
                    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating API documentation with Doxygen" VERBATIM
                    )
endif(DOXYGEN_FOUND)


