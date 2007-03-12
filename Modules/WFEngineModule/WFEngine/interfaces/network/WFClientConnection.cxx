#include "WFClientConnection.h"
#include "WFSocketCollection.h"

using namespace WFEngine::nmWFClientConnection;
using namespace WFEngine::nmWFBaseSocket;

WFClientConnection::WFClientConnection() : WFBaseSocket()
{ 
 connected = 0;
}

WFClientConnection::WFClientConnection(int fd) : WFBaseSocket()
{ 
 socket_descriptor = fd;
 connected = 1; 
};

WFClientConnection::~WFClientConnection()
{
}

bool WFClientConnection::deal_with_data()
{
 //std::cout<<"deal_with_data"<<std::endl;
 if(!socket_descriptor)
 {
  //std::cout<<"Client socket_descriptor not set!"<<std::endl;
  return false;
 }
 char buffer[80];     /* Buffer for socket reads */
 char *cur_char = 0;      /* Used in processing buffer */
 
 bool success = true;

 if (recv(socket_descriptor,buffer,size_t(buffer),80) < 0) {
  /* Connection closed, close this end
     and free up entry in connectlist */
  //std::cout<<"Connection lost: FD="<<socket_descriptor<<std::endl;
//  this->callBack(listnum, NULL);
  close(socket_descriptor);
//  socket_descriptor = 0;
 } else {
  /* We got some data, so upper case it
     and send it back. */
  //std::cout<<"Received: "<<buffer<<std::endl;
  cur_char = &buffer[0];
//  this->callBack(listnum,&buffer[0]);
  int i = 0;
  while (cur_char[0] != 0) {
   cur_char[0] = toupper(cur_char[0]);
   cur_char++;
   i++;
  }
  //std::cout<<"buffer length: "<<i<<std::endl;
//  i = recv(connectlist[listnum], , 0);
  try
  {
   if(checkSocket(socket_descriptor) && success)
   {
    i = send(socket_descriptor,buffer, i,0);
   }
   else
   {
    closeConnectionSlot(socket_descriptor);
    success = false;
   }
   if(checkSocket(socket_descriptor) && success)
   {   
       i = send(socket_descriptor,"\n", 3,0);
       //std::cout<<i<<std::endl;
      }
      else
      {
    closeConnectionSlot(socket_descriptor);
    success = false;
   }
  }
  catch(...)
  {
   //std::cout<<"error"<<std::endl;
  }
  //std::cout<<"responded: "<<buffer<<std::endl;
 }
 
 return success;
}

void WFClientConnection::onSelectReadable(void* v_ptr)
{
 //cast pointer down to WFSocketCollection
 
 WFEngine::nmWFSocketCollection::WFSocketCollection *sc = static_cast<WFEngine::nmWFSocketCollection::WFSocketCollection*>(v_ptr);
 
 if(!deal_with_data()) //something went wrong
 {
  //std::cout<<"Socket is closed!"<<std::endl;
  sc->removeSocket(this);
 }  
}
