project(WFDirectInterface)

set(SRCS
  WFDirectInterface.cxx)

set(BUILD_WFDIRECTINTERFACE TRUE)

subdirs(
  ${WFENGINE_BASE_DIRECTORY}/engine 
  ${WFENGINE_BASE_DIRECTORY}/xmlManager
  )

include_directories(
  ${WFENGINE_BASE_DIRECTORY}/engine 
  ${WFENGINE_BASE_DIRECTORY}/xmlManager
  )

add_library(WFDirectInterface ${SRCS})

set(LINK_LIBS ${LINK_LIBS} 
  WFDIBaseEngine 
  WFXmlManager
)

target_link_libraries(WFDirectInterface ${LINK_LIBS})

