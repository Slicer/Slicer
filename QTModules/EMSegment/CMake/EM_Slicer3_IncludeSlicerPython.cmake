#
# Include Slicer3 Python to add support for the AtlasCreator
#

#-----------------------------------------------------------------------------
## Python, needed to access the AtlasCreator
if(Slicer3_USE_PYTHON)
  find_package(PythonLibs)
endif(Slicer3_USE_PYTHON)

if(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)
  set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH})
  if(WIN32)
    set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH}/../PC)
  endif(WIN32)
endif(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)

include_directories(${dir_list})
#-----------------------------------------------------------------------------
