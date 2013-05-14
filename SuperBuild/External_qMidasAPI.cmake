
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED qMidasAPI_DIR AND NOT EXISTS ${qMidasAPI_DIR})
  message(FATAL_ERROR "qMidasAPI_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(qMidasAPI_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(qMidasAPI)
set(proj qMidasAPI)

if(NOT DEFINED qMidasAPI_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  # Set CMake OSX variable to pass down the external project
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/qMidasAPI.git"
    GIT_TAG "2fae646e963a43d0cae7af52c18e78ce43a75f0c"
    "${${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288}"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      #-DCMAKE_C_FLAGS:STRING=${ep_common_c_flags} # Unused
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_TESTING:BOOL=OFF
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${qMidasAPI_DEPENDENCIES}
    )
  set(qMidasAPI_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
else()
  # The project is provided using qMidasAPI_DIR, nevertheless since other project may depend on qMidasAPI,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${qMidasAPI_DEPENDENCIES}")
endif()

