find_path(XERCESC_INCLUDE_DIR xercesc/dom/DOM.hpp 
  /usr/include 
  /usr/local/include
  )
mark_as_advanced(XERCESC_INCLUDE_DIR)

find_library(XERCESC_LIBRARY 
  NAMES xerces-c 
  PATH /usr/lib /usr/local/lib
  ) 
mark_as_advanced(XERCESC_LIBRARY)

if(XERCESC_INCLUDE_DIR)
  set(XERCESC_FOUND TRUE)
  message(STATUS "Found XercesC: ${XERCESC_INCLUDE_DIR}")
endif(XERCESC_INCLUDE_DIR)

if(XERCESC_FOUND)
  if(NOT XERCESC_FIND_QUIETLY)
    message(STATUS "Found XercesC: ${XERCESC_LIBRARY}")
  endif(NOT XERCESC_FIND_QUIETLY)
else(XERCESC_FOUND)
  if(XERCESC_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find XercesC-lib")
  endif(XERCESC_FIND_REQUIRED)
endif(XERCESC_FOUND)
