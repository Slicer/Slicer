/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.
  
  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $
  
==========================================================================*/

#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

//#include "vtkSocketCommunicator.h"
#include "vtkMultiThreader.h"
#include "vtkServerSocket.h"
#include "vtkClientSocket.h"
#include "vtkMutexLock.h"
#include "vtkImageData.h"

//#include "igtl_util.h"
//#include "igtl_header.h"
//#include "igtl_image.h"
//#include "igtl_transform.h"

#include "igtlOSUtil.h"
#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"
#include "igtlClientSocket.h"

#include "vtkIGTLCircularBuffer.h"
#include "vtkIGTLConnector.h"

vtkStandardNewMacro(vtkIGTLConnector);
vtkCxxRevisionMacro(vtkIGTLConnector, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkIGTLConnector::vtkIGTLConnector()
{
  this->Type   = TYPE_NOT_DEFINED;
  this->State  = STATE_OFF;

  //this->Communicator = vtkSocketCommunicator::New();
  this->Thread = vtkMultiThreader::New();
  this->ServerStopFlag = false;
  this->ThreadID = -1;
  this->ServerSocket = NULL;
  this->ServerHostname = "localhost";
  this->ServerPort = 18944;
  this->Mutex = vtkMutexLock::New();
  this->CircularBufferMutex = vtkMutexLock::New();
  this->RestrictDeviceName = 0;
  this->IncomingDeviceList.clear();
  this->OutgoingDeviceList.clear();
  this->UnspecifiedDeviceList.clear();
}

//---------------------------------------------------------------------------
vtkIGTLConnector::~vtkIGTLConnector()
{
}

//---------------------------------------------------------------------------
void vtkIGTLConnector::PrintSelf(ostream& os, vtkIndent indent)
{
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::SetTypeServer(int port)
{
  this->Type = TYPE_SERVER;
  this->ServerPort = port;
  return 1;
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::SetTypeClient(char* hostname, int port)
{
  this->Type = TYPE_CLIENT;
  this->ServerPort = port;
  this->ServerHostname = hostname;
  return 1;
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::SetTypeClient(std::string hostname, int port)
{
  this->Type = TYPE_CLIENT;
  this->ServerPort = port;
  this->ServerHostname = hostname;
  return 1;
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::Start()
{
  // Check if type is defined.
  if (this->Type == vtkIGTLConnector::TYPE_NOT_DEFINED)
    {
      //vtkErrorMacro("Connector type is not defined.");
    return 0;
    }

  // Check if thread is detached
  if (this->ThreadID >= 0)
    {
      //vtkErrorMacro("Thread exists.");
    return 0;
    }

  this->ServerStopFlag = false;
  this->ThreadID = this->Thread->SpawnThread((vtkThreadFunctionType) &vtkIGTLConnector::ThreadFunction, this);

  // Following line is necessary in some Linux environment,
  // since it takes for a while for the thread to update
  // this->State to non STATE_OFF value. This causes error
  // after calling vtkIGTLConnector::Start() in ProcessGUIEvent()
  // in vtkOpenIGTLinkIFGUI class.
  this->State = STATE_WAIT_CONNECTION;

  return 1;
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::Stop()
{
  // Check if thread exists
  if (this->ThreadID >= 0)
    {
    // NOTE: Thread should be killed by activating ServerStopFlag.
    this->ServerStopFlag = true;
    this->Mutex->Lock();
    if (this->Socket)
      {
      this->Socket->CloseSocket();
      }
    this->Mutex->Unlock();
    //this->Thread->TerminateThread(this->ThreadID);
    //this->ThreadID = -1;
    return 1;
    }
  else
    {
    return 0;
    }
}


//---------------------------------------------------------------------------
void* vtkIGTLConnector::ThreadFunction(void* ptr)
{

  //vtkIGTLConnector* igtlcon = static_cast<vtkIGTLConnector*>(ptr);
  vtkMultiThreader::ThreadInfo* vinfo = 
    static_cast<vtkMultiThreader::ThreadInfo*>(ptr);
  vtkIGTLConnector* igtlcon = static_cast<vtkIGTLConnector*>(vinfo->UserData);
  
  igtlcon->State = STATE_WAIT_CONNECTION;
  
  if (igtlcon->Type == TYPE_SERVER)
    {
    igtlcon->ServerSocket = vtkServerSocket::New();
    igtlcon->ServerSocket->CreateServer(igtlcon->ServerPort);
    }
  
  // Communication -- common to both Server and Client
  while (!igtlcon->ServerStopFlag)
    {
    //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ThreadFunction(): alive.");
    igtlcon->Mutex->Lock();
    igtlcon->Socket = igtlcon->WaitForConnection();
    igtlcon->Mutex->Unlock();
    if (igtlcon->Socket != NULL)
      {
      igtlcon->State = STATE_CONNECTED;
      //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ThreadFunction(): Client Connected.");
      igtlcon->ReceiveController();
      igtlcon->State = STATE_WAIT_CONNECTION;
      }
    }

  if (igtlcon->ServerSocket)
    {
    igtlcon->ServerSocket->CloseSocket();
    igtlcon->ServerSocket->Delete();
    igtlcon->ServerSocket = NULL;
    }
  if (igtlcon->Socket)
    {
    igtlcon->Socket->CloseSocket();
    igtlcon->Socket->Delete();
    igtlcon->Socket = NULL;
    }
  igtlcon->ThreadID = -1;
  igtlcon->State = STATE_OFF;

  return NULL;

}

//---------------------------------------------------------------------------
vtkClientSocket* vtkIGTLConnector::WaitForConnection()
{
  vtkClientSocket* socket = NULL;

  if (this->Type == TYPE_CLIENT)
    {
    socket = vtkClientSocket::New();
    }

  while (!this->ServerStopFlag)
    {
    if (this->Type == TYPE_SERVER)
      {
      //vtkErrorMacro("vtkIGTLConnector: Waiting for client @ port #" << this->ServerPort);
      socket = this->ServerSocket->WaitForConnection(1000);
      if (socket != NULL) // if client connected
        {
        //vtkErrorMacro("vtkIGTLConnector: connected.");
        return socket;
        }
      }
    else if (this->Type == TYPE_CLIENT) // if this->Type == TYPE_CLIENT
      {
      //vtkErrorMacro("vtkIGTLConnector: Connecting to server...");
      int r = socket->ConnectToServer(this->ServerHostname.c_str(), this->ServerPort);
      if (r == 0) // if connected to server
        {
        return socket;
        }
      else
        {
        break;
        }
      }
    else
      {
      this->ServerStopFlag = true;
      }
    }

  if (socket != NULL)
    {
    //vtkErrorMacro("vtkOpenIGTLinkLogic::WaitForConnection(): Socket Closed.");
    socket->CloseSocket();
    socket->Delete();
    }

  return NULL;
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::ReceiveController()
{
  //igtl_header header;
  igtl::MessageHeader::Pointer headerMsg;
  headerMsg = igtl::MessageHeader::New();

  if (!this->Socket)
    {
    return 0;
    }
  
  while (!this->ServerStopFlag)
    {
    
    // check if connection is alive
    if (!this->Socket->GetConnected())
      {
      break;
      }

    //----------------------------------------------------------------
    // Receive Header
    headerMsg->InitPack();

    int r = this->Socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
    if (r != headerMsg->GetPackSize())
      {
      vtkErrorMacro("Irregluar size.");
      break;
      }

    // Deserialize the header
    headerMsg->Unpack();

    //----------------------------------------------------------------
    // Check Device Name if device name is restricted
    if (this->RestrictDeviceName)
      {
      if (this->IncomingDeviceList[std::string(headerMsg->GetDeviceName())] != headerMsg->GetDeviceType())
        {
        // The conbination of device name and type doesn't exist on the list
        // just read and discad
        this->Skip(headerMsg->GetBodySizeToRead());
        continue;
        
        }
      }
    else
      {
      // if device name is not restricted:
      if (this->IncomingDeviceList[std::string(headerMsg->GetDeviceName())] != headerMsg->GetDeviceType())
        this->IncomingDeviceList[std::string(headerMsg->GetDeviceName())] = headerMsg->GetDeviceType();
      }
    
    //----------------------------------------------------------------
    // Search Circular Buffer
    
    std::string key = headerMsg->GetDeviceName();
    CircularBufferMap::iterator iter = this->Buffer.find(key);
    if (iter == this->Buffer.end()) // First time to refer the device name
      {
      this->CircularBufferMutex->Lock();
      this->Buffer[key] = vtkIGTLCircularBuffer::New();
      this->CircularBufferMutex->Unlock();
      }
    
    //----------------------------------------------------------------
    // Load to the circular buffer
    
    vtkIGTLCircularBuffer* circBuffer = this->Buffer[key];
    
    if (circBuffer && circBuffer->StartPush() != -1)
      {
      circBuffer->StartPush();
      
      igtl::MessageBase::Pointer buffer = circBuffer->GetPushBuffer();
      buffer->SetMessageHeader(headerMsg);
      buffer->AllocatePack();

      int read = this->Socket->Receive(buffer->GetPackBodyPointer(), buffer->GetPackBodySize());
      if (read != buffer->GetPackBodySize())
        {
        vtkErrorMacro ("Only read " << read << " but expected to read "
                       << buffer->GetPackBodySize() << "\n");
        continue;
        }
      
      circBuffer->EndPush();
      
      }
    else
      {
      break;
      }
    
    } // while (!this->ServerStopFlag)
  
  this->Socket->CloseSocket();
  
  return 0;
    
}
  

//---------------------------------------------------------------------------
int vtkIGTLConnector::SendData(int size, unsigned char* data)
{
  
  if (!this->Socket)
    {
    return 0;
    }
  
  // check if connection is alive
  if (!this->Socket->GetConnected())
    {
    return 0;
    }

  return this->Socket->Send(data, size);  // return 1 on success, otherwise 0.

}


//---------------------------------------------------------------------------
int vtkIGTLConnector::Skip(int length, int skipFully/*=1*/)
{
  unsigned char dummy[256];
  int block  = 256;
  int n      = 0;
  int remain = length;
  
  do
    {
    if (remain < block)
      {
      block = remain;
      }
    
    n = this->Socket->Receive(dummy, block, skipFully);
    remain -= n;
    }
  while (remain > 0 || (skipFully && n < block));

  return (length - remain);


}


//---------------------------------------------------------------------------
int vtkIGTLConnector::GetUpdatedBuffersList(NameListType& nameList)
{
  nameList.clear();

  CircularBufferMap::iterator iter;
  for (iter = this->Buffer.begin(); iter != this->Buffer.end(); iter ++)
    {
    if (iter->second->IsUpdated())
      {
      nameList.push_back(iter->first);
      }
    }
  return nameList.size();
}


//---------------------------------------------------------------------------
vtkIGTLCircularBuffer* vtkIGTLConnector::GetCircularBuffer(std::string& key)
{
  CircularBufferMap::iterator iter = this->Buffer.find(key);
  if (iter != this->Buffer.end())
    {
    return this->Buffer[key]; // the key has been found in the list
    }
  else
    {
    return NULL;  // nothing found
    }
}



