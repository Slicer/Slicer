
set(proj SimpleITK)

# Set dependency list
set(${proj}_DEPENDENCIES ITKv4 Swig python python-setuptools)

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED SimpleITK_DIR AND NOT EXISTS ${SimpleITK_DIR})
  message(FATAL_ERROR "SimpleITK_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  include(ExternalProjectForNonCMakeProject)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetPythonSetupEnv_Commands(${_env_script})

  # install step - the working path must be set to the location of the SimpleITK.py
  # file so that it will be picked up by distuils setup, and installed
  set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
  file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}-build/Wrapping\")
ExternalProject_Execute(${proj} \"install\" \"${PYTHON_EXECUTABLE}\" PythonPackage/setup.py install)
")

  set(SimpleITK_REPOSITORY ${git_protocol}://itk.org/SimpleITK.git)
  set(SimpleITK_GIT_TAG f2241a13f6010cb6ea6da1b21c6f2d2e31641b16 ) # master branch 0.9.0.dev381

  ExternalProject_add(SimpleITK
    ${${proj}_EP_ARGS}
    SOURCE_DIR SimpleITK
    BINARY_DIR SimpleITK-build
    GIT_REPOSITORY ${SimpleITK_REPOSITORY}
    GIT_TAG ${SimpleITK_GIT_TAG}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_SHARED_LIBS:BOOL=${Slicer_USE_SimpleITK_SHARED}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DSimpleITK_PYTHON_THREADS:BOOL=ON
      -DSimpleITK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DSimpleITK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DSITK_INT64_PIXELIDS:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DPYTHON_EXECUTABLE:PATH=${PYTHON_EXECUTABLE}
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_DOXYGEN:BOOL=OFF
      -DWRAP_PYTHON:BOOL=ON
      -DWRAP_TCL:BOOL=OFF
      -DWRAP_JAVA:BOOL=OFF
      -DWRAP_RUBY:BOOL=OFF
      -DWRAP_LUA:BOOL=OFF
      -DWRAP_CSHARP:BOOL=OFF
      -DWRAP_R:BOOL=OFF
      -DSWIG_EXECUTABLE:PATH=${SWIG_EXECUTABLE}
      -DSimpleITK_BUILD_DISTRIBUTE:BOOL=ON # Shorten version and install path removing -g{GIT-HASH} suffix.
    #
    INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
    #
    DEPENDS ${${proj}_DEPENDENCIES}
    )
  set(SimpleITK_DIR ${CMAKE_BINARY_DIR}/SimpleITK-build)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS SimpleITK_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
