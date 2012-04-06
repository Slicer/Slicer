
if(NOT External_python_unix_PROJECT_INCLUDED)
  message(FATAL_ERROR "This file shouldn't be included directly !")
endif()

set(python_SOURCE_DIR python)
set(python_BUILD_IN_SOURCE 1)



# Set slicer_PYTHON_INCLUDE, slicer_PYTHON_LIBRARY and slicer_PYTHON_EXECUTABLE variables
set(python_IMPORT_SUFFIX so)
if(APPLE)
  set(python_IMPORT_SUFFIX dylib)
endif()
set(slicer_PYTHON_INCLUDE ${python_build}/include/python2.6)
set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_build}/lib)
set(slicer_PYTHON_LIBRARY ${slicer_PYTHON_SHARED_LIBRARY_DIR}/libpython2.6.${python_IMPORT_SUFFIX})
set(slicer_PYTHON_EXECUTABLE ${python_build}/bin/customPython)
set(slicer_PYTHON_REAL_EXECUTABLE ${python_build}/bin/python2.6)

configure_file(SuperBuild/python_patch_step_unix.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step.cmake
  @ONLY)

configure_file(SuperBuild/python_configure_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/python_configure_step.cmake
  @ONLY)

configure_file(
  SuperBuild/python_customPython_configure.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
  @ONLY)

configure_file(SuperBuild/python_make_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/python_make_step.cmake
  @ONLY)

configure_file(SuperBuild/python_install_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/python_install_step.cmake
  @ONLY)

set(python_PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step.cmake)
set(python_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_configure_step.cmake)
set(python_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_make_step.cmake)
set(python_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_install_step.cmake)

ExternalProject_Add(${proj}
  URL ${python_URL}
  URL_MD5 ${python_MD5}
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCE_DIR ${python_SOURCE_DIR}
  BUILD_IN_SOURCE ${python_BUILD_IN_SOURCE}
  PATCH_COMMAND ${python_PATCH_COMMAND}
  CONFIGURE_COMMAND ${python_CONFIGURE_COMMAND}
  BUILD_COMMAND ${python_BUILD_COMMAND}
  UPDATE_COMMAND ""
  INSTALL_COMMAND ${python_INSTALL_COMMAND}
  DEPENDS
    ${python_DEPENDENCIES}
  )

#-----------------------------------------------------------------------------
# Since fixup_bundle expects the library to be writable, let's add an extra step
# to make sure it's the case.
if(APPLE)
  ExternalProject_Add_Step(${proj} python_install_chmod_library
    COMMAND chmod u+xw ${slicer_PYTHON_LIBRARY}
    DEPENDEES install
    )
endif()


