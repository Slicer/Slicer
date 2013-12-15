
if(Slicer_USE_CTKAPPLAUNCHER)

  set(proj CTKAPPLAUNCHER)

  # Sanity checks
  if(DEFINED CTKAPPLAUNCHER_DIR AND NOT EXISTS ${CTKAPPLAUNCHER_DIR})
    message(FATAL_ERROR "CTKAPPLAUNCHER_DIR variable is defined but corresponds to non-existing directory")
  endif()

  # Set dependency list
  set(${proj}_DEPENDENCIES "")

  # Include dependent projects if any
  ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

  if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
    message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
  endif()

  if(NOT DEFINED CTKAPPLAUNCHER_DIR)
    SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX CTKAPPLAUNCHER)
    set(launcher_version "0.1.11")
    set(item_id "")
    # On windows, use i386 launcher unconditionally
    if("${CTKAPPLAUNCHER_OS}" STREQUAL "win")
      set(CTKAPPLAUNCHER_ARCHITECTURE "i386")
      set(md5 "bd39716da9c2d5455ba95db561d3d883")
      set(item_id "4165")
    elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "linux")
      set(md5 "fcbd0ca4eb01cc66d4691c08d8fa194c")
      set(item_id "4166")
    elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "macosx")
      set(md5 "0af14c017ff7953b6fcca76f4f88c952")
      set(item_id "4164")
    endif()

    ExternalProject_Add(${proj}
      ${${proj}_EP_ARGS}
      URL http://packages.kitware.com/api/rest?method=midas.item.download&id=${item_id}&dummy=CTKAppLauncher-${launcher_version}-${CTKAPPLAUNCHER_OS}-${CTKAPPLAUNCHER_ARCHITECTURE}.tar.gz
      URL_MD5 ${md5}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS
        ${${proj}_DEPENDENCIES}
      )
    set(CTKAPPLAUNCHER_DIR ${CMAKE_BINARY_DIR}/${proj})

  else()
    ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
  endif()

  mark_as_superbuild(CTKAPPLAUNCHER_DIR:PATH)

endif()
