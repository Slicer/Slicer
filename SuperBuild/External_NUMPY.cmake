
set(proj NUMPY)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools)

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

  include(ExternalProjectForNonCMakeProject)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetBuildEnv_Commands(${_env_script})
  ExternalProject_Write_SetPythonSetupEnv_Commands(${_env_script} APPEND)
  file(APPEND ${_env_script}
"#------------------------------------------------------------------------------
# Added by '${CMAKE_CURRENT_LIST_FILE}'

set(ENV{F77} \"\")
set(ENV{F90} \"\")
set(ENV{FFLAGS} \"\")

# See http://docs.scipy.org/doc/numpy/user/install.html#disabling-atlas-and-other-accelerated-libraries
# and http://massmail.spl.harvard.edu/public-archives/slicer-devel/2013/011098.html
set(ENV{ATLAS} \"None\")
set(ENV{BLAS} \"None\")
set(ENV{LAPACK} \"None\")
set(ENV{MKL} \"None\")
")

  # configure step
  set(_configure_script ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
  file(WRITE ${_configure_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}\")
file(WRITE \"${CMAKE_BINARY_DIR}/${proj}/site.cfg\" \"\")
ExternalProject_Execute(${proj} \"configure\" \"${PYTHON_EXECUTABLE}\" setup.py config)
")

  # build step
  set(_build_script ${CMAKE_BINARY_DIR}/${proj}_build_step.cmake)
  file(WRITE ${_build_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}\")
ExternalProject_Execute(${proj} \"build\" \"${PYTHON_EXECUTABLE}\" setup.py build --fcompiler=none)
")

  # install step
  set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
  file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}\")
ExternalProject_Execute(${proj} \"install\" \"${PYTHON_EXECUTABLE}\" setup.py install)
")

  #------------------------------------------------------------------------------
  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/numpy-1.4.1.tar.gz"
    URL_MD5 "5c7b5349dc3161763f7f366ceb96516b"
    SOURCE_DIR ${proj}
    BUILD_IN_SOURCE 1
    PATCH_COMMAND ${CMAKE_COMMAND} -DNUMPY_SRC_DIR=${CMAKE_BINARY_DIR}/${proj}
      -P ${CMAKE_CURRENT_LIST_DIR}/${proj}_patch.cmake
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script}
    BUILD_COMMAND ${CMAKE_COMMAND} -P ${_build_script}
    INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
