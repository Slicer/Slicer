#ifndef WFSERVERCONNECTION_H_
#define WFSERVERCONNECTION_H_

#include "WFBaseSocket.h"

namespace WFEngine
{
 namespace nmWFServerConnection
 {
  class WFServerConnection : public WFEngine::nmWFBaseSocket::WFBaseSocket
  {
  public:
   int isListening(void) {return listening;}
        
      void setIPAddr(char *ipaddr);
      void setPort(int p);
      void setMaxConnections(int max);
      int bindAndListen(void);
        
      int accept(void);
   static WFServerConnection *New();
   //void addConnection(int port, int maxConnections, void(*handleFunc)(int, char*));
//   bool waitForConnection();
   void sendDataToConnection(int conn, std::string &data);
   void onSelectReadable(void*);
//   void initializeServer();   
  protected:
   WFServerConnection() : WFEngine::nmWFBaseSocket::WFBaseSocket() 
   { 
    listening = 0;
   }
   virtual ~WFServerConnection();   
//   int createSocket();
   
//   char bind_addr[MAX_IPADDR];
      int port;
      int listening; // Boolean value indicating the socket is listening
      
   }; 
 }
}

#endif /*WFSERVERCONNECTION_H_*/
