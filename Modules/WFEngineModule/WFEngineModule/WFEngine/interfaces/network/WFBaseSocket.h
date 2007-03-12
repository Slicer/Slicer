#ifndef WFBASESOCKET_H_
#define WFBASESOCKET_H_

#if defined WIN32
#include <winsock.h>
#elif defined __linux__
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#include <iostream>
#include <string>
#include <map>
#include <vector>

//#include "WFSocketCollection.h"

//#include <boost/t
//#include <pthread.h>

namespace WFEngine
{
 namespace nmWFBaseSocket
 {
  class WFBaseSocket
  {
  public:
   int getSocketDescriptor();
   
   virtual void onSelectReadable(void*) = 0;
  protected:
   WFBaseSocket();
   virtual ~WFBaseSocket();
   
   int maxConnections;
   int socket_descriptor;
   
   bool listening;
   
   int* connectlist;  /* Array of connected sockets so we know who we are talking to */
   fd_set socks;        /* Socket file descriptors we want to wake up for, using select() */
   int highsock;      /* Highest #'d file descriptor, needed for select() */
  
   void setnonblocking(int socket); 
   bool checkSocket(int socket);
   void closeConnectionSlot(int socket);
   int handle_new_connection();
  private:
  #if defined WIN32
   WSADATA wsa_data;
   WSAStartup(MAKEWORD(1,1), &wsa_data);
  #endif
   
  #if defined WIN32
   WSACleanup();
  #endif
   
   bool runFromInside;
   
//   void build_select_list();
//   void deal_with_data(int listnum);
  };
 }
}
#endif /*WFBASESOCKET_H_*/
