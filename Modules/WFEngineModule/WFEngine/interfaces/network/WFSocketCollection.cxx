#include "WFSocketCollection.h"

#include <errno.h>

using namespace WFEngine::nmWFSocketCollection;
using namespace WFEngine::nmWFBaseSocket;

WFSocketCollection::WFSocketCollection()
{
 //initialize timeout to 1sec
 seconds = 1;
 max_fd = 0;
}

WFSocketCollection::~WFSocketCollection()
{
}

int WFSocketCollection::addSocket(WFBaseSocket *s)
{
 mySockets.insert(std::make_pair(s->getSocketDescriptor(), s));
// FD_SET(s->getSocketDescriptor(), &internal_fd_set);
 //std::cout<<max_fd<<"  "<<s->getSocketDescriptor()<<std::endl;
 if(s->getSocketDescriptor() > max_fd)
  max_fd = s->getSocketDescriptor();
}

int WFSocketCollection::removeSocket(WFBaseSocket *s)
{
 //std::cout<<mySockets.size()<<std::endl;
 mySockets.erase(s->getSocketDescriptor());
 //std::cout<<mySockets.size()<<std::endl;
}

WFBaseSocket *WFSocketCollection::getSocketByFD(int fd)
{
 return mySockets[fd];
}

void WFSocketCollection::setTimeOut(int sec)
{
 seconds = sec;
}

int WFSocketCollection::selectOnSockets()
{
 fd_set work_set;
 timeout.tv_sec = seconds;
 timeout.tv_usec = 0;
 
// create new internal fd set
 buildFDSet();
 
 for (;;) {
       work_set = this->internal_fd_set;
       int res = select(max_fd+1, &work_set, NULL, NULL, &timeout);
       if (res == 0) {
          // TIME OUT (if so asked)
       //std::cout<<".";
       fflush(stdout);
          return res;
       }
       else if (res > 0) {
          // Execute each sockets own OnReadable() method...
          for (int i=0; i<max_fd+1; i++) {
             if (FD_ISSET(i, &work_set)) {
                this->getSocketByFD(i)->onSelectReadable(this);
             }
          }
          return res;
       }
       else 
          if (errno != EINTR) return -1;
    }
}

void WFSocketCollection::buildFDSet()
{
 FD_ZERO(&internal_fd_set);
 max_fd = 0;
 
 std::map<int, WFBaseSocket*>::iterator iter;
 for(iter = mySockets.begin(); iter != mySockets.end(); iter++)
 {
  FD_SET(iter->first, &internal_fd_set);
  if(iter->first > max_fd)
   max_fd = iter->first;
 }
}
