# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
IF(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  RETURN()
ENDIF()
SET(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(scipy_DEPENDENCIES NUMPY)

set(proj scipy)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

#message(STATUS "Adding project '${proj}'")

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.scipy.org/svn/scipy/branches/0.7.x"
  SOURCE_DIR python/scipy
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS 
    ${scipy_DEPENDENCIES}
  )

