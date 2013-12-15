
set(proj NUMPY)

# Set dependency list
set(${proj}_DEPENDENCIES python)

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  # XXX - Add a test checking if numpy is available
endif()

if(NOT DEFINED ${CMAKE_PROJECT_NAME}_USE_SYSTEM_NUMPY)
  set(${CMAKE_PROJECT_NAME}_USE_SYSTEM_NUMPY ${${CMAKE_PROJECT_NAME}_USE_SYSTEM_python})
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_NUMPY)

  ExternalProject_Message(${proj} "${proj} - Building without Fortran compiler - Non-optimized code will be built !")

  set(numpy_URL http://svn.slicer.org/Slicer3-lib-mirrors/trunk/numpy-1.4.1.tar.gz)
  set(numpy_MD5 5c7b5349dc3161763f7f366ceb96516b)

  #------------------------------------------------------------------------------
  set(NUMPY_DIR "${CMAKE_BINARY_DIR}/${proj}")

  configure_file(
    SuperBuild/${proj}_environment.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${proj}_environment.cmake @ONLY)

  configure_file(
    SuperBuild/${proj}_configure_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${proj}_configure_step.cmake @ONLY)

  configure_file(
    SuperBuild/NUMPY_make_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${proj}_make_step.cmake @ONLY)

  configure_file(
    SuperBuild/${proj}_install_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${proj}_install_step.cmake @ONLY)

  #------------------------------------------------------------------------------
  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL ${numpy_URL}
    URL_MD5 ${numpy_MD5}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/NUMPY
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/NUMPY
    CONFIGURE_COMMAND ${CMAKE_COMMAND}
      -P ${CMAKE_CURRENT_BINARY_DIR}/${proj}_configure_step.cmake
    BUILD_COMMAND ${CMAKE_COMMAND}
      -P ${CMAKE_CURRENT_BINARY_DIR}/${proj}_make_step.cmake
    INSTALL_COMMAND ${CMAKE_COMMAND}
      -P ${CMAKE_CURRENT_BINARY_DIR}/${proj}_install_step.cmake
    PATCH_COMMAND ${CMAKE_COMMAND}
    -DNUMPY_SRC_DIR=${Slicer_BINARY_DIR}/NUMPY
      -P ${CMAKE_CURRENT_LIST_DIR}/${proj}_patch.cmake
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
