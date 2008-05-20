project(WFEngine)

set(SRCS
  WFXmlManager.cxx 
  DOMTreeErrorReporter.cxx
  WFXmlConfigManager.cxx 
  WFXmlWriter.cxx
  WFXmlWorkflowManager.cxx 
  WFStepObject.cxx
  )

if(XERCESC_LIBRARY)
  set(LINK_LIBS ${XERCESC_LIBRARY})
endif(XERCESC_LIBRARY)

add_library(WFXmlManager ${SRCS})
target_link_libraries(WFXmlManager ${LINK_LIBS})
