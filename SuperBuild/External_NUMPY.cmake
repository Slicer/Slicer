
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
    URL "http://slicer.kitware.com/midas3/download/item/159598/numpy-1.4.1-patched-2014-09-18.tar.gz"
    URL_MD5 "620e5bbc454a6a3d8c5089be57b9bc83"
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

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(_pythonhome ${CMAKE_BINARY_DIR}/python-install)
  set(pythonpath_subdir lib/python2.7)
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(pythonpath_subdir Lib)
  endif()

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    ${_pythonhome}/${pythonpath_subdir}/site-packages/numpy/core
    ${_pythonhome}/${pythonpath_subdir}/site-packages/numpy/lib
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy/core
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy/lib
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
    LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
