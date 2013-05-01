
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
set(ITKv4_DEPENDENCIES "zlib")
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND ITKv4_DEPENDENCIES DCMTK)
endif()

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

  set(ITKv4_DCMTK_ARGS)
  if(Slicer_BUILD_DICOM_SUPPORT)
    set(ITKv4_DCMTK_ARGS
      -DITK_USE_SYSTEM_DCMTK:BOOL=ON
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      -DModule_ITKIODCMTK:BOOL=ON
      )
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

  if(NOT DEFINED git_protocol)
      set(git_protocol "git")
  endif()

  set(ITKv4_REPOSITORY ${git_protocol}://itk.org/ITK.git)
  set(ITKv4_GIT_TAG 1866ef42887df677a6197ad11ed0ef6e9b239567) #2013-04-03

  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${ITKv4_REPOSITORY}
    GIT_TAG ${ITKv4_GIT_TAG}
    "${${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288}"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DITK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DITK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DITK_LEGACY_REMOVE:BOOL=OFF
      -DITKV3_COMPATIBILITY:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DITK_USE_REVIEW:BOOL=ON
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DITK_INSTALL_NO_DEVELOPMENT:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DKWSYS_USE_MD5:BOOL=ON # Required by SlicerExecutionModel
      -DITK_WRAPPING:BOOL=OFF #${BUILD_SHARED_LIBS} ## HACK:  QUICK CHANGE
      -DITK_USE_SYSTEM_DCMTK:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
      ${ITKv4_DCMTK_ARGS}
      ${ITKv4_WRAP_ARGS}
      -DITK_USE_SYSTEM_ZLIB:BOOL=ON
      -DZLIB_ROOT:PATH=${SLICER_ZLIB_ROOT}
      -DZLIB_INCLUDE_DIR:PATH=${SLICER_ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${SLICER_ZLIB_LIBRARY}
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
