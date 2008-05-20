project(WFEngine)

# we don't need the network interface right now!
# interfaces/network

set(WFENGINE_SOURCE_DIRS
  engine
  xmlManager
  interfaces/direct
  )

set(WFENGINE_LIBRARY ${LIBRARY_OUTPUT_PATH}/libWFDirectInterface.so)

#option(WFCONFIGEDITOR "Want you to build the WFConfigEditor?")
#if(WFCONFIGEDITOR)
#  set(SOURCE_DIRS ${SOURCE_DIRS} wfConfigEditor)
#endif(WFCONFIGEDITOR)

#option(WFTESTCLIENTS "Want you to build the WFExamples or WFTools?")
#if(WFTESTCLIENTS)
#  set(WFENGINE_SOURCE_DIRS ${WFENGINE_SOURCE_DIRS} test)
#endif(WFTESTCLIENTS)

#we must depend on xerces!
#include(${CMAKE_SOURCE_DIR}/FindXercesC.cmake)
if(XERCESC_FOUND)
  include_directories(${XERCESC_INCLUDE_DIR})
  set(LINK_LIBS ${LINK_LIBS} ${XERCESC_LIBRARY})
endif(XERCESC_FOUND)

subdirs(interfaces/direct)
include_directories(${WFENGINE_SOURCE_DIRS})

#if(LOKI_FOUND)
#  include_directories(${LOKI_INCLUDE_DIR})
#  set(LINK_LIBS ${LINK_LIBS} ${LOKI_LIBRARY})
#endif(LOKI_FOUND)

#include(${CMAKE_SOURCE_DIR}/FindAsio.cmake)
#if(ASIO_FOUND)
#  include_directories(${ASIO_INCLUDE_DIR})
#endif(ASIO_FOUND)

#target_link_libraries(WFBaseEngine ${KWWidgets_LIBRARIES}) 
#add_executable(WFEngine WFEMain.cxx)
#target_link_libraries(WFEngine WFBaseEngine)
