
if(Slicer_USE_CTKAPPLAUNCHER)

  set(proj CTKAPPLAUNCHER)

  # Sanity checks
  if(DEFINED CTKAPPLAUNCHER_DIR AND NOT EXISTS ${CTKAPPLAUNCHER_DIR})
    message(FATAL_ERROR "CTKAPPLAUNCHER_DIR variable is defined but corresponds to nonexistent directory")
  endif()

  # Set dependency list
  set(${proj}_DEPENDENCIES "")
  if(WIN32)
    set(${proj}_DEPENDENCIES CTKResEdit)
  endif()

  # Include dependent projects if any
  ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

  if(Slicer_USE_SYSTEM_${proj})
    message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
  endif()

  if(NOT DEFINED CTKAppLauncher_DIR)

    SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX CTKAPPLAUNCHER)
    set(launcher_version "0.1.33")
    # On windows, use i386 launcher unconditionally
    if("${CTKAPPLAUNCHER_OS}" STREQUAL "win")
      set(CTKAPPLAUNCHER_ARCHITECTURE "i386")
      set(sha256 "df72ae06e70b7c01d09c4fd4cdf2aa6e35724ae793bd5026c47b46ca494ca0c9")
    elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "linux")
      set(sha256 "ea9a8fe540aa0614439e0c61e2efa4de273ef38f969e48c2bcb570934c1dc1fc")
    elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "macosx")
      set(sha256 "355e39904138921466213330a5f7af127702bc28a8dc9b25115461db0b3546af")
    endif()

    set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj})

    set(CTKAppLauncherFileName CTKAppLauncher-${launcher_version}-${CTKAPPLAUNCHER_OS}-${CTKAPPLAUNCHER_ARCHITECTURE}.tar.gz)
    ExternalProject_Add(${proj}
      ${${proj}_EP_ARGS}
      URL https://github.com/commontk/AppLauncher/releases/download/v${launcher_version}/${CTKAppLauncherFileName}
      URL_HASH SHA256=${sha256}
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
      SOURCE_DIR ${EP_BINARY_DIR}
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS
        ${${proj}_DEPENDENCIES}
      )

    ExternalProject_GenerateProjectDescription_Step(${proj}
      VERSION ${launcher_version}
      LICENSE_FILES "https://raw.githubusercontent.com/commontk/AppLauncher/v${launcher_version}/LICENSE_Apache_20"
      )

    set(CTKAppLauncher_DIR ${EP_BINARY_DIR})

  else()
    ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
  endif()

  mark_as_superbuild(
    VARS
      CTKAppLauncher_DIR:PATH
    LABELS "FIND_PACKAGE"
    )

endif()
