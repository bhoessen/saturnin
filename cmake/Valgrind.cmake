include(CMakeParseArguments)

find_program(VALGRIND_PATH valgrind)

IF (VALGRIND_PATH)
  SET(VALGRIND_FOUND VALGRIND_PATH)
  message(STATUS "Using valgrind: " ${VALGRIND_PATH})
ENDIF (VALGRIND_PATH)

# SETUP_TARGET_FOR_VALGRIND(
#     NAME testrunner_coverage                    # New target name
#     EXECUTABLE testrunner -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES testrunner                     # Dependencies to build first
# )
function(SETUP_TARGET_FOR_VALGRIND)
  set(options NONE)
  set(oneValueArgs NAME)
  set(multiValueArgs EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES)
  cmake_parse_arguments(VALGRIND "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if (NOT VALGRIND_ARGUMENTS)
    set(VALGRIND_ARGUMENTS "--db-attach=yes" "--trace-children=yes" "--child-silent-after-fork=yes" "--leak-check=full" "--show-reachable=yes" "--track-origins=yes" "--error-exitcode=1")
  endif ()
  
  message(STATUS "Using valgrind arguments: " ${VALGRIND_ARGUMENTS})
  
  # Setup target
  add_custom_target(${VALGRIND_NAME}
    # Capturing lcov counters and generating report
    COMMAND ${VALGRIND_PATH} ${VALGRIND_ARGUMENTS} ${VALGRIND_EXECUTABLE} ${VALGRIND_EXECUTABLE_ARGS}

    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    DEPENDS ${Coverage_DEPENDENCIES}
  )
  
endfunction()