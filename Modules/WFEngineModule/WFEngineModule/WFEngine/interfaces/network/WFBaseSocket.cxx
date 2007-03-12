#include "WFBaseSocket.h"
#include "WFSocketCollection.h"

#include <errno.h>

using namespace WFEngine::nmWFBaseSocket;
using namespace WFEngine::nmWFSocketCollection;

WFBaseSocket::WFBaseSocket()
{
 socket_descriptor = -1;
 maxConnections = 1;
 connectlist = new int[1];
 connectlist[0] = 0;
}

WFBaseSocket::~WFBaseSocket()
{
 
}


int WFBaseSocket::getSocketDescriptor()
{
 return this->socket_descriptor;
}

void WFBaseSocket::setnonblocking(int socket)
{
 int opts;

 opts = fcntl(socket,F_GETFL);
 if (opts < 0) {
  //std::cout<<"fcntl(F_GETFL)"<<std::endl;
  exit(EXIT_FAILURE);
 }
 opts = (opts | O_NONBLOCK);
 if (fcntl(socket,F_SETFL,opts) < 0) {
  //std::cout<<"fcntl(F_SETFL)"<<std::endl;
  exit(EXIT_FAILURE);
 }
 return;
}

int WFBaseSocket::handle_new_connection() {
 int listnum;      /* Current item in connectlist for for loops */
 int connection; /* Socket file descriptor for incoming connections */
 /* We have a new connection coming in!  We'll
 try to find a spot for it in connectlist. */
 connection = accept(socket_descriptor, NULL, NULL);
 //std::cout<<connection<<" - "<<errno<<std::endl;
 if (connection < 0) {
  //std::cout<<"accept"<<std::endl;
  exit(EXIT_FAILURE);
 }
 setnonblocking(connection);
 int i;
 for (i = 0; i < maxConnections && (connection != -1); i++)
  if (connectlist[i] == 0) {
   //std::cout<<"Connection accepted:   FD="<<connection<<"; Slot="<<i<<std::endl;
   connectlist[i] = connection;
   //std::cout<<connectlist[i]<<std::endl;
   connection = -1;
  }
 if (connection != -1) {
  /* No room left in the queue! */
  //std::cout<<"No room left for new client."<<std::endl;
  char* errorMSG = "Sorry, this server is too busy. Try again later!\r\n";
  send(connection,errorMSG, size_t(errorMSG),0);
  close(connection);
 }
 
 return connectlist[i-1];
}

bool WFBaseSocket::checkSocket(int socket)
{
 int bufsize = 0;
    int e       = 0;
    socklen_t len     = sizeof( bufsize );
    
 e = getsockopt( socket, SOL_SOCKET, SO_ERROR, (char*) &bufsize, &len );
    std::cerr << "DEBUG: send error(SO_ERROR)=" << bufsize << ", errno=" << errno
<< " e=" << e << std::endl;
    
    if(e != 0 || bufsize != 0)
     return false;
    else
     return true;
}

void WFBaseSocket::closeConnectionSlot(int socket)
{
 if(socket != 0)
 {
  int curListNum = -1;
  for(int i = 0; i < maxConnections; i++)
  {
   //std::cout<<connectlist[i]<<std::endl;
   if(connectlist[i] == socket)
   {
    curListNum = i;
    break;
   }
  }
  if(curListNum > -1)
  {
   //std::cout<<"Connection lost: FD="<<connectlist[curListNum]<<";  Slot="<<curListNum<<std::endl;
   //this->callBack(listnum, NULL);
   close(connectlist[curListNum]);
   connectlist[curListNum] = 0; 
  }
 }
}
