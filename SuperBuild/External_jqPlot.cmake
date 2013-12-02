
superbuild_include_once()

# Set dependency list
set(jqPlot_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(jqPlot)
set(proj jqPlot)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

if(NOT DEFINED jqPlot_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  ExternalProject_Add(${proj}
    URL http://slicer.kitware.com/midas3/download?items=15065&dummy=jquery.jqplot.1.0.4r1120.tar.gz
    URL_MD5 5c5d73730145c3963f09e1d3ca355580
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${jqPlot_DEPENDENCIES}
    )
  set(jqPlot_DIR ${CMAKE_BINARY_DIR}/${proj})
  mark_as_advanced(jqPlot_DIR)

else()
  # The project is provided using jqPlot_DIR, nevertheless since other project may depend on jqPlot,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${jqPlot_DEPENDENCIES}")
endif()

