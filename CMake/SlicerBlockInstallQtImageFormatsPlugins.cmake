# -------------------------------------------------------------------------
# Find and install Qt imageformats plugins
# -------------------------------------------------------------------------

# See: http://doc.qt.nokia.com/qq/qq10-windows-deployment.html#plugins
#      http://doc.qt.nokia.com/stable/deployment-mac.html#qt-plugins

if(NOT "${QT_PLUGINS_DIR}" STREQUAL "")
  set(qt_plugins_dir "${QT_PLUGINS_DIR}/imageformats")
  file(GLOB plugin_candidates "${qt_plugins_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
  foreach(p ${plugin_candidates})
    if(NOT p MATCHES "(_debug|d[0-9])${CMAKE_SHARED_LIBRARY_SUFFIX}$")
      set(qt_plugins ${qt_plugins} "${p}")
    endif()
  endforeach()
endif()

foreach(qpi ${qt_plugins})
  install(PROGRAMS ${qpi}
    DESTINATION ${Slicer_INSTALL_ROOT}lib/QtPlugins/imageformats
    )
endforeach()
