
set(proj jqPlot)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

if(NOT DEFINED jqPlot_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL http://slicer.kitware.com/midas3/download?items=15065&dummy=jquery.jqplot.1.0.4r1120.tar.gz
    URL_MD5 5c5d73730145c3963f09e1d3ca355580
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(jqPlot_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS jqPlot_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
