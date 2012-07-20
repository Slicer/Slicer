
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED SimpleITK_DIR AND NOT EXISTS ${SimpleITK_DIR})
  message(FATAL_ERROR "SimpleITK_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(SimpleITK_DEPENDENCIES ITKv4 Swig python)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(SimpleITK)

#
#  SimpleITK externalBuild
#
include(ExternalProject)

if(APPLE)
  set(SIMPLEITK_PYTHON_ARGS
    -DPYTHON_EXECUTABLE:PATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_FRAMEWORKS:PATH=${slicer_PYTHON_FRAMEWORK}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    )
else()
  set(SIMPLEITK_PYTHON_ARGS
    -DPYTHON_EXECUTABLE:PATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    )
endif()

configure_file(SuperBuild/SimpleITK_install_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/SimpleITK_install_step.cmake
  @ONLY)

set(SimpleITK_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/SimpleITK_install_step.cmake)


ExternalProject_add(SimpleITK
  SOURCE_DIR SimpleITK
  BINARY_DIR SimpleITK-build
  GIT_REPOSITORY http://itk.org/SimpleITK.git
  # This is the tag for the "master" branch as of June 8th, 2012
  GIT_TAG f65f370185eaca75a7a95d8deb235e1b51ef5c88
  "${slicer_external_update}"
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  # SimpleITK does not work with shared libs turned on
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
  -DITK_DIR:PATH=${ITK_DIR}
  -DBUILD_EXAMPLES:BOOL=OFF
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_DOXYGEN:BOOL=OFF
  -DWRAP_PYTHON:BOOL=ON
  -DWRAP_TCL:BOOL=OFF
  -DWRAP_JAVA:BOOL=OFF
  -DWRAP_RUBY:BOOL=OFF
  -DWRAP_LUA:BOOL=OFF
  -DWRAP_CSHARP:BOOL=OFF
  -DWRAP_R:BOOL=OFF
  ${SIMPLEITK_PYTHON_ARGS}
  -DSWIG_EXECUTABLE:PATH=${SWIG_EXECUTABLE}
  #
  INSTALL_COMMAND ${SimpleITK_INSTALL_COMMAND}
  #
  DEPENDS ${SimpleITK_DEPENDENCIES}
)
