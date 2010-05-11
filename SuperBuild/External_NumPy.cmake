
#-----------------------------------------------------------------------------
# Get and build numpy and scipy

#set ::NUMPY_TAG "http://svn.scipy.org/svn/numpy/branches/1.2.x"
#set ::env(PYTHONHOME) $::Slicer3_LIB/python-build
#set ::env(LD_LIBRARY_PATH) $::Slicer3_LIB/python-build/lib:$::env(LD_LIBRARY_PATH)

set(proj numpy)

set(slicer_PYTHON_INTERPRETER)

if(WIN32)
  set(slicer_PYTHON_INTERPRETER ${python_base}/python.exe)
else()
  set(slicer_PYTHON_INTERPRETER ${CMAKE_BINARY_DIR}/python-build/bin/python)
  set(ENV{LD_LIBRARY_PATH} "${CMAKE_BINARY_DIR}/python-build/lib")
endif()

ExternalProject_Add(${proj}
  DEPENDS blas lapack python
  SVN_REPOSITORY "http://svn.scipy.org/svn/numpy/branches/1.3.x"
  SOURCE_DIR python/numpy
  CONFIGURE_COMMAND ""
  BINARY_DIR ${CMAKE_BINARY_DIR}/python/numpy 
  BUILD_COMMAND ${slicer_PYTHON_INTERPRETER} ./setup.py install
  INSTALL_COMMAND ""
)
