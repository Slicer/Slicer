
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
IF(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  RETURN()
ENDIF()
SET(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

IF(Slicer_USE_CTKAPPLAUNCHER)
  # Sanity checks
  IF(DEFINED CTKAPPLAUNCHER_DIR AND NOT EXISTS ${CTKAPPLAUNCHER_DIR})
    MESSAGE(FATAL_ERROR "CTKAPPLAUNCHER_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()
  
  # Set dependency list
  set(CTKAPPLAUNCHER_DEPENDENCIES "")
  
  # Include dependent projects if any
  SlicerMacroCheckExternalProjectDependency(CTKAPPLAUNCHER)
  set(proj CTKAPPLAUNCHER)
  
  IF(NOT DEFINED CTKAPPLAUNCHER_DIR)
    SlicerMacroGetPlatformArchitectureBitness(VAR_PREFIX CTKAPPLAUNCHER)
    # On windows, use i386 launcher unconditionally
    IF("${CTKAPPLAUNCHER_PLATFORM}" STREQUAL "win")
      SET(CTKAPPLAUNCHER_ARCHITECTURE "i386")
    ENDIF()
    SET(launcher_version "0.1.4")
    #message(STATUS "${__indent}Adding project ${proj}")
    ExternalProject_Add(${proj}
      URL http://cloud.github.com/downloads/commontk/AppLauncher/CTKAppLauncher-${launcher_version}-${CTKAPPLAUNCHER_PLATFORM}-${CTKAPPLAUNCHER_ARCHITECTURE}.tar.gz
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS 
        ${CTKAPPLAUNCHER_DEPENDENCIES}
      )
    SET(CTKAPPLAUNCHER_DIR ${CMAKE_BINARY_DIR}/${proj})
  ELSE()
    # The project is provided using CTKAPPLAUNCHER_DIR, nevertheless since other 
    # project may depend on CTKAPPLAUNCHER, let's add an 'empty' one
    SlicerMacroEmptyExternalProject(${proj} "${CTKAPPLAUNCHER_DEPENDENCIES}")
  ENDIF()
  
ENDIF()

