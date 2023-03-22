
# Get root directory
get_property(_filepath TARGET "Qt5::Core" PROPERTY LOCATION_RELEASE)
get_filename_component(_dir ${_filepath} PATH)
if(APPLE)
  # "_dir" of the form "<qt_root_dir>/lib/QtCore.framework"
  set(qt_root_dir "${_dir}/../..")
else()
  # "_dir" of the form "<qt_root_dir>/lib"
  set(qt_root_dir "${_dir}/..")
endif()

# Sanity checks
set(expected_defined_vars
  Slicer_BUILD_I18N_SUPPORT
  Slicer_INSTALL_BIN_DIR
  )
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

set(expected_existing_vars
  qt_root_dir
  )
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

set(Slicer_INSTALLED_QT_TOOLS)

if(Slicer_BUILD_I18N_SUPPORT)
  # Bundle Qt language tools with the application if internationalization is enabled.
  # These tools allow Slicer modules to update and process Qt translation (.ts) files
  # without requiring installation of Qt.
  list(APPEND Slicer_INSTALLED_QT_TOOLS
    lconvert
    lrelease
    lupdate
  )
endif()

foreach(tool IN LISTS Slicer_INSTALLED_QT_TOOLS)
  set(tool_executable ${qt_root_dir}/bin/${tool}${CMAKE_EXECUTABLE_SUFFIX})
  if(NOT EXISTS "${tool_executable}")
    message(FATAL_ERROR "Qt tool ${tool} not found: ${tool_executable}")
  endif()
  install(PROGRAMS ${tool_executable}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    COMPONENT Runtime
    )
  slicerStripInstalledLibrary(
    FILES "${Slicer_INSTALL_BIN_DIR}/${tool}"
    COMPONENT Runtime
    )
  if(APPLE)
    set(dollar "$")
    install(CODE
      "set(app ${Slicer_INSTALL_BIN_DIR}/${tool})
      set(appfilepath \"${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${dollar}{app}\")
      message(\"CPack: - Adding rpath to ${dollar}{app}\")
      execute_process(COMMAND install_name_tool -add_rpath @loader_path/..  ${dollar}{appfilepath})"
      COMPONENT Runtime
      )
  endif()
endforeach()
