# -------------------------------------------------------------------------
# Find and install Qt plugins
# -------------------------------------------------------------------------

# See: http://doc.qt.nokia.com/qq/qq10-windows-deployment.html#plugins
#      http://doc.qt.nokia.com/stable/deployment-mac.html#qt-plugins

# "SlicerBlockInstallQtPlugins_subdirectories" variable should set externally to contain
# the list of plugins subdirectory to install.
# Usually this list contains subdirectories like: iconengines, imageformats, sqldrivers ...

if("${SlicerBlockInstallQtPlugins_subdirectories}" STREQUAL "")
  message(FATAL_ERROR "error: SlicerBlockInstallQtPlugins_subdirectories should be set to a list"
                      " of subdirectories like: iconengines, imageformats, sqldrivers ...")
endif()

if(NOT "${QT_PLUGINS_DIR}" STREQUAL "")

  foreach(plugins_subdirectory ${SlicerBlockInstallQtPlugins_subdirectories})

    set(plugins_pattern)
    string(REPLACE ":" ";" plugins_subdirectory ${plugins_subdirectory})
    list(LENGTH plugins_subdirectory _len)
    if(_len EQUAL 2)
      list(GET plugins_subdirectory 1 plugins_pattern)
      list(GET plugins_subdirectory 0 plugins_subdirectory)
      set(plugins_pattern "*${plugins_pattern}")
    endif()

    set(qt_plugins)
    set(qt_plugins_dir "${QT_PLUGINS_DIR}/${plugins_subdirectory}")
    file(GLOB plugin_candidates "${qt_plugins_dir}/${plugins_pattern}*${CMAKE_SHARED_LIBRARY_SUFFIX}")
    foreach(p ${plugin_candidates})
      if(NOT p MATCHES "(_debug|d[0-9])${CMAKE_SHARED_LIBRARY_SUFFIX}$")
        list(APPEND qt_plugins ${p})
      endif()
    endforeach()

    foreach(qpi ${qt_plugins})
      get_filename_component(qpi_libname ${qpi} NAME)
      # Some Qt deployments (notably Homebrew) expose the plugins directory as a
      # tree of symlinks pointing into the versioned install location (e.g.
      # ".../share/qt/plugins/platforms/libqcocoa.dylib" -> "../../../../Cellar/...").
      # install(PROGRAMS) copies such a symlink verbatim, so the installed plugin
      # is a relative link that dangles as soon as the bundle is moved off the
      # build machine, and the application then aborts at startup because the Qt
      # platform plugin ("cocoa") cannot be loaded. Resolve to the real file so an
      # actual binary is installed and can be fixed up and embedded in the bundle.
      get_filename_component(qpi_realpath "${qpi}" REALPATH)
      install(PROGRAMS ${qpi_realpath}
        DESTINATION ${Slicer_INSTALL_QtPlugins_DIR}/${plugins_subdirectory}
        RENAME ${qpi_libname}
        COMPONENT RuntimePlugins
        )
      slicerStripInstalledLibrary(
        FILES "${Slicer_INSTALL_QtPlugins_DIR}/${plugins_subdirectory}/${qpi_libname}"
        COMPONENT Runtime
        )
    endforeach()

  endforeach()
endif()
