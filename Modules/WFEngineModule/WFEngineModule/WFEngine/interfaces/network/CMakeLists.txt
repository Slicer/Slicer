PROJECT(WFEngine)
SET(SRCS
WFBaseSocket.cxx
WFServerConnection.cxx
WFClientConnection.cxx
WFSocketCollection.cxx)

ADD_LIBRARY(WFNetworkInterface ${SRCS})

TARGET_LINK_LIBRARIES(WFNetworkInterface)
