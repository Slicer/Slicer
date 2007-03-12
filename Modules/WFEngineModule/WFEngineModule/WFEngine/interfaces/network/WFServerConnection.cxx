#include "WFServerConnection.h"
#include "WFClientConnection.h"
#include "WFSocketCollection.h"

#if defined WIN32
typedef int socklen_t;
#elif defined __linux__
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s);
#endif

using namespace WFEngine::nmWFServerConnection;

//WFConnectionHandler::WFConnectionHandler()
//{
// listening = 0;
// maxConnections = 1;
//}

WFServerConnection::~WFServerConnection()
{
 
}

WFServerConnection *WFServerConnection::New()
{
 return new WFServerConnection;
}

//bool WFConnectionHandler::waitForConnection()
//{
// Superclass::start_server();
// return true;
//}

int WFServerConnection::bindAndListen()
{
 struct sockaddr_in addr, r_addr;
 SOCKET r;
 
 int reuse_addr = 1;
 
 //intialize socket list
 socklen_t len = sizeof(r_addr);
 socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

 if(INVALID_SOCKET == socket_descriptor)
 {
  //std::cout<<"Could not create socket"<<std::endl;
//  return -1;
 }
 
 setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
   sizeof(reuse_addr));

 /* Set socket to non-blocking with our setnonblocking routine */
 setnonblocking(socket_descriptor);
 
 memset((void*)&addr, 0, sizeof(addr));
 addr.sin_family = AF_INET;
 
// if(this->bind_addr == NULL)
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
// else
//  addr.sin_addr.s_addr = htonl(bind_addr);
 addr.sin_port = htons(this->port);
 
 
 r = bind(socket_descriptor, (struct sockaddr*)&addr, sizeof(addr));
 if(SOCKET_ERROR == r)
 {
  //std::cout<<"Could not bind to local socket"<<std::endl;
  return -1;
 }
 
 r = listen(socket_descriptor, 5);
 if(SOCKET_ERROR == r)
 {
  //std::cout<<"Could not listen to local socket"<<std::endl;
  return -1;
 }
 //std::cout<<"intializeSocket: "<<socket_descriptor<<"-"<<port<<std::endl;
// this->socketToPortMap.insert(std::make_pair(sock[curID],port));
// this->portToMaxConnectionMap.insert(std::make_pair(port, maxConnections));
// std::vector<int> conns(maxConnections,0);
 
// std::vector<int> v(maxConnections);
// this->portToConnectlistMap.insert(std::make_pair(port, conns));
 this->highsock = socket_descriptor;
 
 this->connectlist = new int[maxConnections];
 for(int i = 0; i < maxConnections; i++)
 {
  this->connectlist[i] = 0;
 }
}

void WFServerConnection::sendDataToConnection(int socket, std::string &data)
{
 bool success = true;
 //std::cout<<"send_data"<<std::endl;
// if (recv(connectlist[listnum],buffer,size_t(buffer),80) < 0) {
//  /* Connection closed, close this end
//     and free up entry in connectlist */
//  //std::cout<<"Connection lost: FD="<<connectlist[listnum]<<";  Slot="<<listnum<<std::endl;
//  close(connectlist[listnum]);
//  connectlist[listnum] = 0;
// } else {
  /* We got some data, so upper case it
     and send it back. */
 //std::cout<<"Sending: "<<data.size()<<" letters"<<std::endl;
 if(checkSocket(socket) && success)
 {
  send(socket,data.c_str(), data.size(),0); 
 }
 else
 {
  closeConnectionSlot(socket);
  success = false;
 }
 if(checkSocket(socket) && success)
 {
  send(socket,"\n", 3,0); 
 }
 else
 {
  closeConnectionSlot(socket);
  success = false;
 } 
 //std::cout<<"done!"<<std::endl;
// }
}

void WFServerConnection::setPort(int prt)
{
 this->port = prt;
}

void WFServerConnection::onSelectReadable(void* v_ptr) {
 int listnum;      /* Current item in connectlist for for loops */

 //cast pointer down to WFSocketCollection
 
 WFEngine::nmWFSocketCollection::WFSocketCollection *sc = static_cast<WFEngine::nmWFSocketCollection::WFSocketCollection*>(v_ptr);
 
 /* OK, now socks will be set with whatever socket(s)
    are ready for reading.  Lets first check our
    "listening" socket, and then check the sockets
    in connectlist. */
 
 /* If a client is trying to connect() to our listening
  socket, select() will consider that as the socket
  being 'readable'. Thus, if the listening socket is
  part of the fd_set, we need to accept a new connection. */
 
 //std::cout<<"OnReadable:"<<std::endl;
 int fd = handle_new_connection();
 if(fd > 0)
 {
  //create new ClientSocket and add it to the collection
  WFEngine::nmWFClientConnection::WFClientConnection *cc = new WFEngine::nmWFClientConnection::WFClientConnection(fd);
  sc->addSocket(cc);
 }
 else
 {
  //std::cout<<"something went wrong!"<<std::endl;
 }
}

void WFServerConnection::setMaxConnections(int max)
{
 this->maxConnections = max;
 connectlist = new int[max];
 for(int i = 0; i < maxConnections; i++)
 {
  connectlist[i] = 0;
 }
}
