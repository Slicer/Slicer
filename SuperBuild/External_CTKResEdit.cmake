
set(proj CTKResEdit)

if(NOT WIN32)
  message(FATAL_ERROR "${proj} is not supported on this platform !")
endif()

# Sanity checks
if(DEFINED CTKResEdit_EXECUTABLE AND NOT EXISTS ${CTKResEdit_EXECUTABLE})
  message(FATAL_ERROR "CTKResEdit_EXECUTABLE variable is defined but corresponds to nonexistent file")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

if(NOT DEFINED CTKResEdit_EXECUTABLE)

  set(CTKResEdit_OS "win")
  set(CTKResEdit_ARCHITECTURE "i386")

  set(CTKResEdit_VERSION "0.1.0")
  set(url "https://github.com/jcfr/ResEdit/releases/download")
  set(url "${url}/v0.1.0-20140331-c157b7c/CTKResEdit-${CTKResEdit_VERSION}-gc157-${CTKResEdit_OS}-${CTKResEdit_ARCHITECTURE}.tar.gz")
  set(md5 "f59547c480420199081b94e96df292ec")

  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL ${url}
    URL_MD5 ${md5}
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${EP_BINARY_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${CTKResEdit_VERSION}
    LICENSE_FILES "https://raw.githubusercontent.com/jcfr/ResEdit/master/LICENSE"
    )

  set(CTKResEdit_EXECUTABLE ${EP_BINARY_DIR}/bin/CTKResEdit.exe)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(CTKResEdit_EXECUTABLE:FILEPATH)
