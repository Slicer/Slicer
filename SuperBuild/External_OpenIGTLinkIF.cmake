
superbuild_include_once()

# Set dependency list
set(OpenIGTLinkIF_DEPENDENCIES OpenIGTLink)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(OpenIGTLinkIF)
set(proj OpenIGTLinkIF)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED OpenIGTLinkIF_SOURCE_DIR AND NOT EXISTS ${OpenIGTLinkIF_SOURCE_DIR})
  message(FATAL_ERROR "OpenIGTLinkIF_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED OpenIGTLinkIF_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/openigtlink/OpenIGTLinkIF.git"
    GIT_TAG "b73e6a31119e20ec4793a644d83fc03ab7e716d9"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${OpenIGTLinkIF_DEPENDENCIES}
    )
  set(OpenIGTLinkIF_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using OpenIGTLinkIF_SOURCE_DIR, nevertheless since other project may depend on OpenIGTLinkIF,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${OpenIGTLinkIF_DEPENDENCIES}")
endif()
