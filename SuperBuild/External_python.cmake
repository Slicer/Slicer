
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(python_DEPENDENCIES "")
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    set(python_DEPENDENCIES tcl)
  else()
    set(python_DEPENDENCIES tcl tk)
  endif()
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(python)
set(proj python)

#message(STATUS "${__indent}Adding project ${proj}")

set(python_base ${CMAKE_CURRENT_BINARY_DIR}/${proj})
set(python_build ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
#
# WARNING - If you consider updating the Python version, make sure the patch 
#           step associated with both window and unix are still valid !
#
set(PYVER_SHORT 26)
set(python_URL http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Python-2.6.6.tgz)
set(python_MD5 b2f209df270a33315e62c1ffac1937f0)

get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

if(WIN32)
  set(External_python_win_PROJECT_INCLUDED 1)
  include(${CMAKE_CURRENT_LIST_DIR}/External_python_win.cmake)
  set(External_python_win_PROJECT_INCLUDED 0)
else()
  set(External_python_unix_PROJECT_INCLUDED 1)
  include(${CMAKE_CURRENT_LIST_DIR}/External_python_unix.cmake)
  set(External_python_unix_PROJECT_INCLUDED 0)
endif()

#message(STATUS "slicer_PYTHON_INCLUDE:${slicer_PYTHON_INCLUDE}")
#message(STATUS "slicer_PYTHON_LIBRARY:${slicer_PYTHON_LIBRARY}")
#message(STATUS "slicer_PYTHON_EXECUTABLE:${slicer_PYTHON_EXECUTABLE}")

