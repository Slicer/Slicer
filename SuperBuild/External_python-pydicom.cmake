set(proj python-pydicom)

#------------------------------------------------------------------------------
#
# cmake -DSOURCE_DIR:PATH=/path/to/${proj} -DPATCH_${proj}:BOOL=ON -P /path/to/External_${proj}.cmake
#
if(PATCH_${proj})
  # See issue #3763
  cmake_minimum_required(VERSION 2.8.9)
  set(_file ${SOURCE_DIR}/setup.py)

  message(STATUS "Computing offset for ${_file}")
  file(STRINGS ${_file} _lines LIMIT_COUNT 3)
  string(LENGTH "${_lines}" _offset)

  message(STATUS "Reading  ${_file} using offset: ${_offset}")
  file(READ ${_file} _content OFFSET ${_offset})

  message(STATUS "Writting ${_file}")
  file(WRITE ${_file} ${_content})

  return()
endif()

#------------------------------------------------------------------------------
# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools)

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  # XXX - Add a test checking if <proj> is available
endif()

if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_python})
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://pypi.python.org/packages/5d/1d/dd9716ef3a0ac60c23035a9b333818e34dec2e853733d03f502533af9b84/pydicom-0.9.9.tar.gz"
    URL_MD5 "a66ca6728e69ba565ab9c8a21740eee8"
    SOURCE_DIR ${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${PYTHON_EXECUTABLE} setup.py install
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
