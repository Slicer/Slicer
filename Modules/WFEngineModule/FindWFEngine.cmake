find_path(WFENGINE_INCLUDE_DIR WFEMain.h 
  /usr/include 
  /usr/local/include
  )
mark_as_advanced(WFENGINE_INCLUDE_DIR)

find_library(WFENGINE_LIBRARY 
  NAMES WFDirectInterface 
  PATH 
  /usr/lib 
  /usr/local/lib
  )

mark_as_advanced(WFENGINE_LIBRARY)

if(WFENGINE_INCLUDE_DIR)
  set(WFENGINE_FOUND TRUE)
  set(WFENGINE_INCLUDE_DIR 
    ${WFENGINE_INCLUDE_DIR} 
    ${WFENGINE_INCLUDE_DIR}/engine
    ${WFENGINE_INCLUDE_DIR}/interfaces 
    ${WFENGINE_INCLUDE_DIR}/interfaces/direct 
    ${WFENGINE_INCLUDE_DIR}/xmlManager
    )
endif(WFENGINE_INCLUDE_DIR)


if(WFENGINE_FOUND)
  if(NOT WFENGINE_FIND_QUIETLY)
    message(STATUS "Found WFENGINE: ${WFENGINE_INCLUDE_DIR}")
  endif(NOT WFENGINE_FIND_QUIETLY)
else(WFENGINE_FOUND)
  if(WFENGINE_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find WFEngine")
  endif(WFENGINE_FIND_REQUIRED)
endif(WFENGINE_FOUND)
