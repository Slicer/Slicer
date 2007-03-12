#ifndef WFSOCKETCOLLECTION_H_
#define WFSOCKETCOLLECTION_H_

#include "WFBaseSocket.h"

#include <map>

namespace WFEngine
{

 namespace nmWFSocketCollection
 {

  class WFSocketCollection
  {
  protected:
        std::map<int, WFEngine::nmWFBaseSocket::WFBaseSocket*> mySockets;
        struct timeval timeout;  /* Timeout for select */
        
        fd_set internal_fd_set;
        int max_fd; //highest file descriptor
        int seconds;
        WFEngine::nmWFBaseSocket::WFBaseSocket* getSocketByFD(int fd);
        
        void buildFDSet();
  public:
     WFSocketCollection();
     virtual ~WFSocketCollection();
        int addSocket(WFEngine::nmWFBaseSocket::WFBaseSocket *s);
        int removeSocket(WFEngine::nmWFBaseSocket::WFBaseSocket *s);
        // Here is the part of the code you've been interested:
        int selectOnSockets(void);
        void setTimeOut(int sec);
  };

 }

}

#endif /*WFSOCKETCOLLECTION_H_*/
