PROJECT(SlicerGUIPython)

IF(USE_PYTHON AND PYTHON_INCLUDE_PATH)
  SET(LIB_NAME "SlicerGUIPython")
  SET(LIB_SRCS _mytkinter.c _slicerPython.cxx)
  FIND_PATH(PYTHON_NUMPY arrayobject.h
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

  INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_PATH}
    ${PYTHON_NUMPY}
    ${PYTHON_NUMPY}/../
    ${PYTHON_NUMPY}/../../../numarray/numpy
    ${VTK_INCLUDE_DIR}
    ${SlicerBase_SOURCE_DIR}
    ${SlicerBase_BINARY_DIR}
  )
  ADD_LIBRARY(${LIB_NAME} ${LIB_SRCS})
  TARGET_LINK_LIBRARIES(${LIB_NAME} SlicerBaseLogic)

  IF(BUILD_SHARED_LIBS)
    INSTALL(TARGETS ${LIB_NAME}
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  ENDIF(BUILD_SHARED_LIBS)

  # SET_SOURCE_FILES_PROPERTIES( _mytkinter.c PROPERTIES WRAP_EXCLUDE TRUE )
  # SET_SOURCE_FILES_PROPERTIES( _slicerPython.c PROPERTIES WRAP_EXCLUDE TRUE )

  SET(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH})
  ADD_DEFINITIONS(-DUSE_PYTHON)
  # Python helper class
  # SET(LIB_SRCS _mytkinter.c ${LIB_SRCS})

  IF (WIN32 AND PYTHON_DEBUG_LIBRARIES)
     TARGET_LINK_LIBRARIES( ${LIB_NAME}
                            debug ${PYTHON_DEBUG_LIBRARIES}
                            optimized ${PYTHON_LIBRARIES})
  ELSE (WIN32 AND PYTHON_DEBUG_LIBRARIES)
     TARGET_LINK_LIBRARIES( ${LIB_NAME} ${PYTHON_LIBRARIES})
  ENDIF (WIN32 AND PYTHON_DEBUG_LIBRARIES)

ENDIF(USE_PYTHON AND PYTHON_INCLUDE_PATH)
