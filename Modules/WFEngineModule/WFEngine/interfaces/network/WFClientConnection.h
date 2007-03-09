#ifndef WFCLIENTCONNECTION_H_
#define WFCLIENTCONNECTION_H_

#include "WFBaseSocket.h"

namespace WFEngine
{
 namespace nmWFClientConnection
 {
  class WFClientConnection : public WFEngine::nmWFBaseSocket::WFBaseSocket 
  {
  protected:
        // A client socket has a destination host and port
        // Probably it's easier to keep this in a sockaddr_in struct
        //char dest_host[MAX_HOSTNAME];
        int port;
        int connected; // Boolean value indicating the socket is connected
        bool deal_with_data();
        
        virtual ~WFClientConnection();
     public:
        // Simple constructor
        WFClientConnection();
        // Constructor that receives an existing FD
        WFClientConnection(int FD);
        // Here you can add additional constructors, e.g. a constr. with destination host and port
        int isConnected(void) {return connected;}

        void setHost(char *h);
        void setPort(int p);

        int connect(void);
                
        void onSelectReadable(void*);
  };

 }

}

#endif /*WFCLIENTCONNECTION_H_*/
