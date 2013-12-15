
set(proj EMSegment)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED EMSegment_SOURCE_DIR AND NOT EXISTS ${EMSegment_SOURCE_DIR})
  message(FATAL_ERROR "EMSegment_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED EMSegment_SOURCE_DIR)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk/Modules/EMSegment"
    SVN_REVISION -r "17040"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(EMSegment_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(EMSegment_SOURCE_DIR:PATH)
