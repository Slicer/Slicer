
superbuild_include_once()

# Set dependency list
set(EMSegment_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(EMSegment)
set(proj EMSegment)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED EMSegment_SOURCE_DIR AND NOT EXISTS ${EMSegment_SOURCE_DIR})
  message(FATAL_ERROR "EMSegment_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED EMSegment_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk/Modules/EMSegment"
    SVN_REVISION -r "17040"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${EMSegment_DEPENDENCIES}
    )
  set(EMSegment_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using EMSegment_DIR, nevertheless since other project may depend on EMSegment,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${EMSegment_DEPENDENCIES}")
endif()
