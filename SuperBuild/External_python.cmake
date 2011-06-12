
# Set dependency list
set(python_DEPENDENCIES "")
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    set(python_DEPENDENCIES tcl)
  else()
    set(python_DEPENDENCIES tk)
  endif()
endif()

set(proj python)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

set(python_base ${CMAKE_CURRENT_BINARY_DIR}/${proj})
set(python_build ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
set(PYVER_SHORT 26)
set(python_URL http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Python-2.6.6.tgz)
set(python_MD5 b2f209df270a33315e62c1ffac1937f0)

get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

if(WIN32)
  include(${CMAKE_CURRENT_LIST_DIR}/External_python_win.cmake)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/External_python_unix.cmake)
endif()

#message(STATUS "slicer_PYTHON_INCLUDE:${slicer_PYTHON_INCLUDE}")
#message(STATUS "slicer_PYTHON_LIBRARY:${slicer_PYTHON_LIBRARY}")
#message(STATUS "slicer_PYTHON_EXECUTABLE:${slicer_PYTHON_EXECUTABLE}")
