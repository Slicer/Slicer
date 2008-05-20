project(WFEngine)

set(SRCS
  WFBaseSocket.cxx
  WFServerConnection.cxx
  WFClientConnection.cxx
  WFSocketCollection.cxx
  )

add_library(WFNetworkInterface ${SRCS})

target_link_libraries(WFNetworkInterface)
