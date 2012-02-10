
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(jqPlot_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(jqPlot)
set(proj jqPlot)

if(NOT DEFINED jqPlot_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  ExternalProject_Add(${proj}
    URL http://cloud.github.com/downloads/Slicer/jqPlot/jquery.jqplot.1.0.0b2_r1012.tar.gz
    URL_MD5 2afa87db609446d568b79a9ae5c07523
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    UPDATE_COMMAND ""
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

