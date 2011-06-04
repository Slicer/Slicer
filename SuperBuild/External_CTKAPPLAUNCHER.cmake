#
# CTKAPPLAUNCHER
#
IF(Slicer_USE_CTKAPPLAUNCHER)
  # Sanity checks
  IF(DEFINED CTKAPPLAUNCHER_DIR AND NOT EXISTS ${CTKAPPLAUNCHER_DIR})
    MESSAGE(FATAL_ERROR "CTKAPPLAUNCHER_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()
  
  set(proj CTKAPPLAUNCHER)
  
  IF(NOT DEFINED CTKAPPLAUNCHER_DIR)
    SET(launcher_suffix)
    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux"
        OR CMAKE_SYSTEM_NAME STREQUAL "Windows"
        OR CMAKE_SYSTEM_NAME STREQUAL "Darwin"
        #OR CMAKE_SYSTEM_NAME STREQUAL "Solaris"
        )
      SET(launcher_suffix ${CMAKE_SYSTEM_NAME})
    ELSE()
      MESSAGE(FATAL_ERROR "Failed to download pre-built launcher - Unsupported platform: ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
    ENDIF()
    
    SET(launcher_version "0.1.4")
    ExternalProject_Add(${proj}
      URL http://cloud.github.com/downloads/commontk/AppLauncher/CTKAppLauncher-${launcher_version}-${launcher_suffix}.tar.gz
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
