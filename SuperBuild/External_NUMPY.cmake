## NUMPY requires fortran, so set up the fortran compiler properly
## http://www.cmake.org/Wiki/CMakeForFortranExample

enable_language(Fortran OPTIONAL)

if(CMAKE_Fortran_COMPILER_WORKS)
  # FFLAGS depend on the compiler
  get_filename_component(Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)

  if(Fortran_COMPILER_NAME STREQUAL "gfortran")
    # gfortran
    set(CMAKE_Fortran_FLAGS_RELEASE "-funroll-all-loops -fno-f2c -O3")
    set(CMAKE_Fortran_FLAGS_DEBUG   "-fno-f2c -O0 -g")
  elseif(Fortran_COMPILER_NAME STREQUAL "ifort")
    # ifort (untested)
    set(CMAKE_Fortran_FLAGS_RELEASE "-f77rtl -O3")
    set(CMAKE_Fortran_FLAGS_DEBUG   "-f77rtl -O0 -g")
  elseif(Fortran_COMPILER_NAME STREQUAL "g77")
    # g77
    message(FATAL_ERROR "The g77 compiler should not be used as specified on NUMPY page http://www.scipy.org/Installing_SciPy/Linux")
  else()
    message(STATUS "CMAKE_Fortran_COMPILER full path: " ${CMAKE_Fortran_COMPILER})
    message(STATUS "Fortran compiler: " ${Fortran_COMPILER_NAME})
    message(STATUS "No optimized Fortran compiler flags are known, we just try -O2...")
    set(CMAKE_Fortran_FLAGS_RELEASE "-O2")
    set(CMAKE_Fortran_FLAGS_DEBUG   "-O0 -g")
  endif()
else()
  message(STATUS "No Fortran compiler found - Non-optimized code will be built !")
endif()

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(NUMPY_DEPENDENCIES CLAPACK python)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(NUMPY)
set(proj NUMPY)

#message(STATUS "${__indent}Adding project ${proj}")

set(numpy_URL http://svn.slicer.org/Slicer3-lib-mirrors/trunk/numpy-1.4.1.tar.gz)
set(numpy_MD5 5c7b5349dc3161763f7f366ceb96516b)

# Note: Both NUMPY_configure_step.cmake and NUMPY_make_step.cmake expects
#       this variable to be defined.
set(NUMPY_DIR "${CMAKE_BINARY_DIR}/NUMPY")

# To configure numpy we run a cmake -P script
# the script will create a site.cfg file
# then run python setup.py config to verify setup
configure_file(
  SuperBuild/NUMPY_configure_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_configure_step.cmake @ONLY)

# To build numpy we also run a cmake -P script.
# the script will set LD_LIBRARY_PATH so that
# python can run after it is built on linux
configure_file(
  SuperBuild/NUMPY_make_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_make_step.cmake @ONLY)

ExternalProject_Add(${proj}
  URL ${numpy_URL}
  URL_MD5 ${numpy_MD5}
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/NUMPY
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/NUMPY
  CONFIGURE_COMMAND ${CMAKE_COMMAND}
    -DCONFIG_TYPE=${CMAKE_CFG_INTDIR} -P ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_configure_step.cmake
  BUILD_COMMAND ${CMAKE_COMMAND}
    -P ${CMAKE_CURRENT_BINARY_DIR}/NUMPY_make_step.cmake
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${NUMPY_DEPENDENCIES}
  )

