
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(ITKv4_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(ITKv4)
set(proj ITKv4)

if(NOT DEFINED ITK_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  set(ITKv4_WRAP_ARGS)
  #if(foo)
    #set(ITKv4_WRAP_ARGS
    #  -DINSTALL_WRAP_ITK_COMPATIBILITY:BOOL=OFF
    #  -DWRAP_float:BOOL=ON
    #  -DWRAP_unsigned_char:BOOL=ON
    #  -DWRAP_signed_short:BOOL=ON
    #  -DWRAP_unsigned_short:BOOL=ON
    #  -DWRAP_complex_float:BOOL=ON
    #  -DWRAP_vector_float:BOOL=ON
    #  -DWRAP_covariant_vector_float:BOOL=ON
    #  -DWRAP_rgb_signed_short:BOOL=ON
    #  -DWRAP_rgb_unsigned_char:BOOL=ON
    #  -DWRAP_rgb_unsigned_short:BOOL=ON
    #  -DWRAP_ITK_TCL:BOOL=OFF
    #  -DWRAP_ITK_JAVA:BOOL=OFF
    #  -DWRAP_ITK_PYTHON:BOOL=ON
    #  -DPYTHON_EXECUTABLE:PATH=${${CMAKE_PROJECT_NAME}_PYTHON_EXECUTABLE}
    #  -DPYTHON_INCLUDE_DIR:PATH=${${CMAKE_PROJECT_NAME}_PYTHON_INCLUDE}
    #  -DPYTHON_LIBRARY:FILEPATH=${${CMAKE_PROJECT_NAME}_PYTHON_LIBRARY}
    #  )
  #endif()

  # HACK This code fixes a loony problem with HDF5 -- it doesn't
  #      link properly if -fopenmp is used.
  string(REPLACE "-fopenmp" "" ITK_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  string(REPLACE "-fopenmp" "" ITK_CMAKE_CXX_FLAGS "${CMAKE_CX_FLAGS}")

  set(ITKv4_REPOSITORY git://itk.org/ITK.git)
  #set(ITKv4_GIT_TAG v4.1.0)
  set(ITKv4_GIT_TAG ae54e7089a7b12e51586519735c1d6f0af05021a) ## 2012-03-17 The v4.1.0 branch + ITKv4Registration patch set not needed by Slicer proper, but needed by extensions
  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${ITKv4_REPOSITORY}
    GIT_TAG ${ITKv4_GIT_TAG}
    UPDATE_COMMAND ""
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_compiler_args}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DITK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DITK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DITK_LEGACY_REMOVE:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DITK_USE_REVIEW:BOOL=ON
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DITKV3_COMPATIBILITY:BOOL=ON
      -DKWSYS_USE_MD5:BOOL=ON # Required by SlicerExecutionModel
      -DUSE_WRAP_ITK:BOOL=OFF #${BUILD_SHARED_LIBS} ## HACK:  QUICK CHANGE
      ${ITKv4_WRAP_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${ITKv4_DEPENDENCIES}
    )
  set(ITK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

else()
  # The project is provided using ITK_DIR, nevertheless since other project may depend on ITK,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${ITKv4_DEPENDENCIES}")
endif()
