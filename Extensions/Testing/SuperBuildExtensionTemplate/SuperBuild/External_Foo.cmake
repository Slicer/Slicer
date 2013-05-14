
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED Foo_DIR AND NOT EXISTS ${Foo_DIR})
  message(FATAL_ERROR "Foo_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(Foo_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(Foo)
set(proj Foo)

if(NOT DEFINED ${proj}_DIR)
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
    #GIT_REPOSITORY "${git_protocol}://github.com/Foo/Foo.git"
    #GIT_TAG "1e823001cb41c92667299635643f1007876d09f6"
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Remove this line and uncomment GIT_REPOSITORY and GIT_TAG"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_TESTING:BOOL=OFF
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "This CONFIGURE_COMMAND is just here as a placeholder. Remove this line to enable configuring of a real CMake based external project"
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "This BUILD_COMMAND is just here as a placeholder. Remove this line to enable building of a real CMake based external project"
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

else()
  # The project is provided using <proj>_DIR, nevertheless since other project may depend on <proj>,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

