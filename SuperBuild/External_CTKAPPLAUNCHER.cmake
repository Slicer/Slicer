
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

if(Slicer_USE_CTKAPPLAUNCHER)
  # Sanity checks
  if(DEFINED CTKAPPLAUNCHER_DIR AND NOT EXISTS ${CTKAPPLAUNCHER_DIR})
    message(FATAL_ERROR "CTKAPPLAUNCHER_DIR variable is defined but corresponds to non-existing directory")
  endif()

  # Set dependency list
  set(CTKAPPLAUNCHER_DEPENDENCIES "")

  # Include dependent projects if any
  SlicerMacroCheckExternalProjectDependency(CTKAPPLAUNCHER)
  set(proj CTKAPPLAUNCHER)

  if(NOT DEFINED CTKAPPLAUNCHER_DIR)
    SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX CTKAPPLAUNCHER)
    set(launcher_version "0.1.10")
    set(item_id "")
    # On windows, use i386 launcher unconditionally
    if("${CTKAPPLAUNCHER_OS}" STREQUAL "win")
      set(CTKAPPLAUNCHER_ARCHITECTURE "i386")
      set(md5 "fee82ee869cfedb54f48c58d0bad3710")
      set(item_id "53")
    elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "linux")
      set(md5 "7da4bcc68ab09833aebc156a7d0a5f06")
      set(item_id "65")
    elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "macosx")
      set(md5 "de87ffad9d5909c7525c61dac50b0c9e")
      set(item_id "63")
    endif()
    #message(STATUS "${__indent}Adding project ${proj}")
    ExternalProject_Add(${proj}
      URL http://packages.kitware.com/api/rest?method=midas.item.download&id=${item_id}&dummy=CTKAppLauncher-${launcher_version}-${CTKAPPLAUNCHER_OS}-${CTKAPPLAUNCHER_ARCHITECTURE}.tar.gz
      URL_MD5 ${md5}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS
        ${CTKAPPLAUNCHER_DEPENDENCIES}
      )
    set(CTKAPPLAUNCHER_DIR ${CMAKE_BINARY_DIR}/${proj})
  else()
    # The project is provided using CTKAPPLAUNCHER_DIR, nevertheless since other
    # project may depend on CTKAPPLAUNCHER, let's add an 'empty' one
    SlicerMacroEmptyExternalProject(${proj} "${CTKAPPLAUNCHER_DEPENDENCIES}")
  endif()

endif()
