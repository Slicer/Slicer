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
  install(PROGRAMS ${qt_root_dir}/bin/${tool}${CMAKE_EXECUTABLE_SUFFIX}
    DESTINATION ${Slicer_INSTALL_ROOT}/bin COMPONENT Runtime
    )
  slicerStripInstalledLibrary(
    FILES "${Slicer_INSTALL_ROOT}/bin/${tool}"
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
