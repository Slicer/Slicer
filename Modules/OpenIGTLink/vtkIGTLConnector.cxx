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

#include "igtl_util.h"
#include "igtl_header.h"
#include "igtl_image.h"
#include "igtl_transform.h"

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
  // in vtkOpenIGTLinkGUI class.
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
    //vtkErrorMacro("vtkOpenIGTLinkLogic::ThreadFunction(): alive.");
    igtlcon->Mutex->Lock();
    igtlcon->Socket = igtlcon->WaitForConnection();
    igtlcon->Mutex->Unlock();
    if (igtlcon->Socket != NULL)
      {
      igtlcon->State = STATE_CONNECTED;
      //vtkErrorMacro("vtkOpenIGTLinkLogic::ThreadFunction(): Client Connected.");
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
  igtl_header header;

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

    int r = this->Socket->Receive(&header, IGTL_HEADER_SIZE);

    if (r != IGTL_HEADER_SIZE)
      {
      //vtkErrorMacro("Irregluar size.");
      }

    igtl_header_convert_byte_order(&header);  
    char deviceType[13];
    deviceType[12] = 0;
    memcpy((void*)deviceType, header.name, 12);
      
    char deviceName[21];
    deviceName[20] = 0;
    memcpy((void*)deviceName, header.device_name, 20);
      
    //vtkErrorMacro("deviceType  = " << deviceType);
    //vtkErrorMacro("deviceName  = " << deviceName);
    //vtkErrorMacro("size = "        << header.body_size);
      
    if (header.version != IGTL_HEADER_VERSION)
      {
      //vtkErrorMacro("Unsupported OpenIGTLink version.");
      break;
      }


    //----------------------------------------------------------------
    // Search Circular Buffer

    std::string key = deviceName;
    CircularBufferMap::iterator iter = this->Buffer.find(key);
    if (iter == this->Buffer.end()) // First time to refer the device name
      {
      this->CircularBufferMutex->Lock();
      this->Buffer[key] = vtkIGTLCircularBuffer::New();
      this->CircularBufferMutex->Unlock();
      }


    //----------------------------------------------------------------
    // Load to the circular buffer

    vtkIGTLCircularBuffer* buffer = this->Buffer[key];

    if (buffer && buffer->StartPush() != -1)
      {
      buffer->StartPush();
      buffer->PushDeviceType(deviceType);

      unsigned char* dataPtr = buffer->GetPushDataArea(header.body_size);
      int read = this->Socket->Receive(dataPtr, header.body_size);
      if (read != header.body_size)
        {
        //vtkErrorMacro ("Only read " << read << " but expected to read " << IGTL_IMAGE_HEADER_SIZE << "\n");
        continue;
        }

      // check CRC here
      buffer->EndPush();
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



