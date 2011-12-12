
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED SkullStripper_SOURCE_DIR AND NOT EXISTS ${SkullStripper_SOURCE_DIR})
  message(FATAL_ERROR "SkullStripper_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(SkullStripper_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(SkullStripper)
set(proj SkullStripper)

if(NOT DEFINED SkullStripper_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    #SVN_REPOSITORY "http://svn.na-mic.org/NAMICSandBox/trunk/SkullStripper"
    #SVN_REVISION -r "7735"
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/SkullStripper.git"
    GIT_TAG "6ed556c206eff9a008030047c613b40cff8b7480"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${SkullStripper_DEPENDENCIES}
    )
  set(SkullStripper_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using SkullStripper_SOURCE_DIR, nevertheless since other project may depend on SkullStripper,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${SkullStripper_DEPENDENCIES}")
endif()

