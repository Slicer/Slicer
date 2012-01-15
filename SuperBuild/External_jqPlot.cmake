
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

#message(STATUS "${__indent}Adding project ${proj}")

# Note: Both jqPlot_configure_step.cmake and jqPlot_make_step.cmake expects
#       this variable to be defined.
set(jqPlot_DIR "${CMAKE_BINARY_DIR}/jqPlot" CACHE STRING "jqPlot directory" FORCE)

# Git repository
#
#set(${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY "github.com/Slicer/jqPlot.git" CACHE STRING "Repository from which to get jqPlot" FORCE)
#set(${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG "1.0.0b2_r1012" CACHE STRING "jqPlot git tag to use" FORCE)
set(${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY "github.com/Slicer/jqPlot.git")
set(${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG "1.0.0b2_r1012")


# To configure jqPlot we run a cmake -P script
# The configure script just encapsulate jqPlot into a Qt resource
#configure_file(
#  SuperBuild/jqPlot_configure_step.cmake.in
#  ${CMAKE_CURRENT_BINARY_DIR}/jqPlot_configure_step.cmake @ONLY)

# To build jqplot we also run a cmake -P script.
# 
# 
#configure_file(
#  SuperBuild/jqPlot_make_step.cmake.in
#  ${CMAKE_CURRENT_BINARY_DIR}/jqPlot_make_step.cmake @ONLY)

# Create an external project to download jqplot
# and configure and build it
ExternalProject_Add(${proj}
  URL http://cloud.github.com/downloads/Slicer/jqPlot/jquery.jqplot.1.0.0b2_r1012.tar.gz
  URL_MD5 2afa87db609446d568b79a9ae5c07523
#  GIT_REPOSITORY "${git_protocol}://${${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY}"
#  GIT_TAG ${${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG}
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/jqPlot-build
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
#  CONFIGURE_COMMAND ${CMAKE_COMMAND}
#    -DCONFIG_TYPE=${CMAKE_CFG_INTDIR} -P ${CMAKE_CURRENT_BINARY_DIR}/jqPlot_configure_step.cmake
#  BUILD_COMMAND ${CMAKE_COMMAND}
#    -P ${CMAKE_CURRENT_BINARY_DIR}/jqPlot_make_step.cmake
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${jqPlot_DEPENDENCIES}
  )

