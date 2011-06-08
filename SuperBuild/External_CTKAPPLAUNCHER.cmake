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
    SlicerMacroDiscoverSystemNameAndBits(VAR_PREFIX CTKAPPLAUNCHER)
    SET(launcher_architecture ${CTKAPPLAUNCHER_BUILD})
    # Use 32bits launcher on both win32 and win64 architecture
    STRING(REPLACE "win64" "win32" launcher_architecture ${launcher_architecture})
    SET(launcher_version "0.1.4")
    ExternalProject_Add(${proj}
      URL http://cloud.github.com/downloads/commontk/AppLauncher/CTKAppLauncher-${launcher_version}-${launcher_architecture}.tar.gz
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
