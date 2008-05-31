project(SlicerBaseGUIPython)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${PYTHON_INCLUDE_PATH}
  ${VTK_INCLUDE_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  )

if(Slicer3_USE_NUMPY)
  include_directories(
    ${PYTHON_NUMPY}
    ${PYTHON_NUMPY}/../
    ${PYTHON_NUMPY}/../../../numarray/numpy
    )
endif(Slicer3_USE_NUMPY)

# --------------------------------------------------------------------------
# Numpy?

option(Slicer3_USE_NUMPY "Enable Numpy/Scipy support in Python" OFF) 

if(Slicer3_USE_NUMPY)
  find_path(PYTHON_NUMPY arrayobject.h
    PATHS
    ${PYTHON_INCLUDE_PATH}
    ${PYTHON_INCLUDE_PATH}/../lib/
    ${PYTHON_INCLUDE_PATH}/../lib/site-packages/numpy/core/include/numpy
    ${PYTHON_INCLUDE_PATH}/../lib/site-packages/numpy/numarray/numpy
    ${PYTHON_INCLUDE_PATH}/../../lib/python2.5/site-packages/numpy/core/include/numpy
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.5\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.4\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]/libs
    PATH_SUFFIXES
    site-packages/numpy/core/include
    site-packages/numpy/core/include/numpy
    site-packages/numpy/numarray/numpy
    DOC "Path to libnumarray.h for numpy module"
    NO_SYSTEM_ENVIRONMENT_PATH
    )
endif(Slicer3_USE_NUMPY)

# --------------------------------------------------------------------------
# Sources

set(SlicerBaseGUIPython_SRCS 
  _mytkinter.c 
  _slicerPython.cxx
)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SlicerBaseGUIPython)
add_library(${lib_name} 
  ${SlicerBaseGUIPython_SRCS}
)
target_link_libraries(${lib_name} 
  SlicerBaseLogic
)
if (WIN32 AND PYTHON_DEBUG_LIBRARIES)
  target_link_libraries( ${lib_name}
    debug ${PYTHON_DEBUG_LIBRARIES}
    optimized ${PYTHON_LIBRARIES})
else (WIN32 AND PYTHON_DEBUG_LIBRARIES)
  target_link_libraries( ${lib_name} ${PYTHON_LIBRARIES})
endif (WIN32 AND PYTHON_DEBUG_LIBRARIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Install support files

# Even though the SlicerBaseGUIPython, let's put it in the parent's project
# subdir, i.e. SlicerBaseGUI, with the other lib files

# Should the install location change (in both the install and/or build tree)
# do update:
# - Applications/GUI: Slicer.cxx, CMakeLists.txt

file(GLOB PYTHONFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.py")
foreach(pythonfile ${PYTHONFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${pythonfile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/SlicerBaseGUI/Python/${pythonfile}
    COPYONLY)
endforeach(pythonfile)

install(
  FILES ${PYTHONFILES}
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}/SlicerBaseGUI/Python
  )
