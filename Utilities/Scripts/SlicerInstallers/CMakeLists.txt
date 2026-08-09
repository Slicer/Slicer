#-----------------------------------------------------------------------------
# Installer scripts
#
# Two kinds of script live in this directory:
#
#  * The *outer* installers, install.sh and install.ps1. They are fetched and
#    piped to a shell to install Slicer on a machine that does not have it yet,
#    so there is nothing for them to be installed into: they are source only,
#    served over HTTPS from the repository.
#
#  * The *inner* scripts handled below. They do the things that are not
#    installing Slicer -- installing the system libraries Slicer needs from a
#    Linux distribution, and installing an interface language -- and are run by
#    the user afterwards, on demand. Those belong inside the package.
#
# The inner scripts locate Slicer relative to their own path: the launcher sits
# one directory above bin/ in an installed package, in a build tree, and (as
# ../MacOS/Slicer) inside the macOS bundle. So they need no configuration, and
# behave identically wherever they are run from.
#
# Until every supported package ships these, the outer installers carry their
# own embedded copies and write them out only when the package does not already
# contain them -- so once this rule is in place, the package's copy wins.
#-----------------------------------------------------------------------------

# These scripts are specific to Slicer itself: they look for a launcher named
# "Slicer", install the SlicerLanguagePacks extension, and talk to Slicer's own
# download and translation servers. A custom application built on Slicer wants
# none of that, so ship them only when Slicer is the application being built.
if(NOT Slicer_MAIN_PROJECT_APPLICATION_NAME STREQUAL "Slicer")
  return()
endif()

set(installer_scripts)

if(UNIX)
  # Linux and macOS share one POSIX sh implementation.
  list(APPEND installer_scripts slicer-language)
endif()

if(UNIX AND NOT APPLE)
  # Installs distribution packages, so it is Linux-only: on macOS and Windows
  # everything Slicer links against is bundled.
  list(APPEND installer_scripts slicer-deps)
endif()

if(WIN32)
  list(APPEND installer_scripts slicer-language.ps1)
endif()

foreach(script IN LISTS installer_scripts)
  # Copy into the build tree's bin/ as well, so the scripts are usable from a
  # build tree exactly as from an installed package.
  configure_file(
    ${script}
    ${Slicer_BINARY_DIR}/bin/${script}
    COPYONLY
    USE_SOURCE_PERMISSIONS
    )

  install(PROGRAMS ${Slicer_BINARY_DIR}/bin/${script}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    COMPONENT Runtime
    )
endforeach()

if(BUILD_TESTING)
  # The outer installers each carry a copy of the inner scripts, for setting up
  # a Slicer package built before those scripts shipped in it. Those copies are
  # generated upstream and vendored here, so fail the build if editing one side
  # here has made the two disagree. Checks source files only, on every platform,
  # regardless of which scripts this platform installs.
  add_test(NAME slicer-installers-embedded-copies-test
    COMMAND ${CMAKE_COMMAND}
      -DSOURCE_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckEmbeddedCopies.cmake
    )
endif()
