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

    set(qt_plugins)
    set(qt_plugins_dir "${QT_PLUGINS_DIR}/${plugins_subdirectory}")
    file(GLOB plugin_candidates "${qt_plugins_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
    foreach(p ${plugin_candidates})
      if(NOT p MATCHES "(_debug|d[0-9])${CMAKE_SHARED_LIBRARY_SUFFIX}$")
        list(APPEND qt_plugins ${p})
      endif()
    endforeach()

    foreach(qpi ${qt_plugins})
      install(PROGRAMS ${qpi}
        DESTINATION ${Slicer_INSTALL_QtPlugins_DIR}/${plugins_subdirectory}
        )
    endforeach()

  endforeach()
endif()
