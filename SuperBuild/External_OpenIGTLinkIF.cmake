
set(proj OpenIGTLinkIF)

# Set dependency list
set(${proj}_DEPENDENCIES OpenIGTLink)

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED OpenIGTLinkIF_SOURCE_DIR AND NOT EXISTS ${OpenIGTLinkIF_SOURCE_DIR})
  message(FATAL_ERROR "OpenIGTLinkIF_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED OpenIGTLinkIF_SOURCE_DIR)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/openigtlink/OpenIGTLinkIF.git"
    GIT_TAG "b73e6a31119e20ec4793a644d83fc03ab7e716d9"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(OpenIGTLinkIF_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(OpenIGTLinkIF_SOURCE_DIR:PATH)
