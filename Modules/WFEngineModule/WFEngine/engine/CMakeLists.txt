project(WFEngine)

set(SRCS
  ${WFENGINE_BASE_DIRECTORY}/engine/WFBaseEngine.cxx
  ${WFENGINE_BASE_DIRECTORY}/engine/WFEngineOptions.cxx
  ${WFENGINE_BASE_DIRECTORY}/engine/WFFactory.cxx
  ${WFENGINE_BASE_DIRECTORY}/engine/WFWorkflowManager.cxx
  )

#if(LOKI_FOUND)
#  set(LINK_LIBS ${LINK_LIBS} ${LOKI_LIBRARY})
#endif(LOKI_FOUND)

if(BUILD_WFDIRECTINTERFACE)
  set_source_files_properties(
    ${WFENGINE_BASE_DIRECTORY}/engine/WFBaseEngine.cxx
    COMPILE_FLAGS -DWFDIRECTINTERFACE
    )
  set(LINK_LIBS 
    WFXmlManager
    )
  add_library(WFDIBaseEngine ${SRCS})
  target_link_libraries(WFDIBaseEngine ${LINK_LIBS})
else(BUILD_WFDIRECTINTERFACE)
  set(LINK_LIBS 
    WFNetworkInterface 
    WFXmlManager
    )
  add_library(WFBaseEngine ${SRCS})
  target_link_libraries(WFBaseEngine ${LINK_LIBS})
endif(BUILD_WFDIRECTINTERFACE)




