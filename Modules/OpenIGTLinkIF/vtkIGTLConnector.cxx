/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.
  
  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$
  
==========================================================================*/

#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

#include "vtkMultiThreader.h"
#include "igtlServerSocket.h"
#include "igtlClientSocket.h"
#include "vtkMutexLock.h"
#include "vtkImageData.h"

#include "igtlOSUtil.h"
#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"

#include "vtkIGTLCircularBuffer.h"
#include "vtkIGTLConnector.h"

vtkStandardNewMacro(vtkIGTLConnector);
vtkCxxRevisionMacro(vtkIGTLConnector, "$Revision$");

//---------------------------------------------------------------------------
vtkIGTLConnector::vtkIGTLConnector()
{
  this->Type   = TYPE_NOT_DEFINED;
  this->State  = STATE_OFF;

  this->Thread = vtkMultiThreader::New();
  this->ServerStopFlag = false;
  this->ThreadID = -1;
  this->ServerHostname = "localhost";
  this->ServerPort = 18944;
  this->Mutex = vtkMutexLock::New();
  this->CircularBufferMutex = vtkMutexLock::New();
  this->RestrictDeviceName = 0;

  this->DeviceInfoList.clear();
  this->IncomingDeviceIDSet.clear();
  this->OutgoingDeviceIDSet.clear();
  this->UnspecifiedDeviceIDSet.clear();

  this->LastID = -1;
}

//---------------------------------------------------------------------------
vtkIGTLConnector::~vtkIGTLConnector()
{
  this->CircularBufferMutex->Lock();
  CircularBufferMap::iterator iter;
  for (iter = this->Buffer.begin(); iter != this->Buffer.end(); iter ++)
    {
    iter->second->Delete();
    }
  this->Buffer.clear();
  this->CircularBufferMutex->Unlock();
  
  if (this->Thread)
    {
    this->Thread->Delete();
    }

  if (this->Mutex)
    {
    this->Mutex->Delete();
    }

  if (this->CircularBufferMutex)
    {
    this->CircularBufferMutex->Delete();
    }
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
    if (this->Socket.IsNotNull())
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
    igtlcon->ServerSocket = igtl::ServerSocket::New();
    igtlcon->ServerSocket->CreateServer(igtlcon->ServerPort);
    }
  
  // Communication -- common to both Server and Client
  while (!igtlcon->ServerStopFlag)
    {
    //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ThreadFunction(): alive.");
    igtlcon->Mutex->Lock();
    //igtlcon->Socket = igtlcon->WaitForConnection();
    igtlcon->WaitForConnection();
    igtlcon->Mutex->Unlock();
    if (igtlcon->Socket.IsNotNull())
      {
      igtlcon->State = STATE_CONNECTED;
      //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ThreadFunction(): Client Connected.");
      igtlcon->ReceiveController();
      igtlcon->State = STATE_WAIT_CONNECTION;
      }
    }

  if (igtlcon->Socket.IsNotNull())
    {
    igtlcon->Socket->CloseSocket();
    }

  if (igtlcon->Type == TYPE_SERVER && igtlcon->ServerSocket.IsNotNull())
    {
    igtlcon->ServerSocket->CloseSocket();
    }
  
  igtlcon->ThreadID = -1;
  igtlcon->State = STATE_OFF;

  return NULL;

}

//---------------------------------------------------------------------------
//igtl::ClientSocket::Pointer vtkIGTLConnector::WaitForConnection()
int vtkIGTLConnector::WaitForConnection()
{
  //igtl::ClientSocket::Pointer socket;

  if (this->Type == TYPE_CLIENT)
    {
    //socket = igtl::ClientSocket::New();
    this->Socket = igtl::ClientSocket::New();
    }

  while (!this->ServerStopFlag)
    {
    if (this->Type == TYPE_SERVER)
      {
      //vtkErrorMacro("vtkIGTLConnector: Waiting for client @ port #" << this->ServerPort);
      this->Socket = this->ServerSocket->WaitForConnection(1000);      
      if (this->Socket.IsNotNull()) // if client connected
        {
        //vtkErrorMacro("vtkIGTLConnector: connected.");
        return 1;
        }
      }
    else if (this->Type == TYPE_CLIENT) // if this->Type == TYPE_CLIENT
      {
      //vtkErrorMacro("vtkIGTLConnector: Connecting to server...");
      int r = this->Socket->ConnectToServer(this->ServerHostname.c_str(), this->ServerPort);
      if (r == 0) // if connected to server
        {
        return 1;
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

  if (this->Socket.IsNotNull())
    {
    //vtkErrorMacro("vtkOpenIGTLinkLogic::WaitForConnection(): Socket Closed.");
    this->Socket->CloseSocket();
    }

  //return NULL;
  return 0;
}

//---------------------------------------------------------------------------
int vtkIGTLConnector::ReceiveController()
{
  //igtl_header header;
  igtl::MessageHeader::Pointer headerMsg;
  headerMsg = igtl::MessageHeader::New();

  if (this->Socket.IsNull())
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
      if (GetDeviceID(headerMsg->GetDeviceName(), headerMsg->GetDeviceType()) < 0) // not found on the list
        {
        this->Skip(headerMsg->GetBodySizeToRead());
        continue;
        }
      }
    else  // if device name is not restricted:
      {
      // search on the list
      if (GetDeviceID(headerMsg->GetDeviceName(), headerMsg->GetDeviceType()) < 0) // not found on the list
        {
        int id = RegisterNewDevice(headerMsg->GetDeviceName(), headerMsg->GetDeviceType());
        RegisterDeviceIO(id, IO_INCOMING);
        }
      }
    
    //----------------------------------------------------------------
    // Search Circular Buffer

    // TODO: 
    // Currently, the circular buffer is selected by device name, but
    // it should be selected by device name and device type.

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
      //std::cerr << "Pushing into the circular buffer." << std::endl;
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
  
  if (this->Socket.IsNull())
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


//---------------------------------------------------------------------------
int vtkIGTLConnector::GetDeviceID(const char* deviceName, const char* deviceType)
{
  // returns -1 if no device found on the list
  int id = -1;

  DeviceInfoMapType::iterator iter;

  for (iter = this->DeviceInfoList.begin(); iter != this->DeviceInfoList.end(); iter ++)
    {
    if (iter->second.name == deviceName && iter->second.type == deviceType)
      {
      id = iter->first;
      }
    }

  return id;
}


//---------------------------------------------------------------------------
int vtkIGTLConnector::RegisterNewDevice(const char* deviceName, const char* deviceType, int io)
{
  int id  = GetDeviceID(deviceName, deviceType);
  
  if (id < 0) // if the device is not on the list
    {
    this->LastID ++;
    id = this->LastID;

    DeviceInfoType info;
    info.name = deviceName;
    info.type = deviceType;
    info.io   = IO_UNSPECIFIED;
    this->UnspecifiedDeviceIDSet.insert(id);
    this->DeviceInfoList[id] = info;
    }

  RegisterDeviceIO(id, io);

  return id;
}


//---------------------------------------------------------------------------
int vtkIGTLConnector::UnregisterDevice(const char* deviceName, const char* deviceType, int io)
{
  // NOTE: If IO_UNSPECIFIED is specified as an 'io' parameter,
  // the method will unregister deivce from both incoming and
  // outgoing data list.
  // return 1, if the device is removed from the device info list.

  int id  = GetDeviceID(deviceName, deviceType);
  DeviceInfoMapType::iterator iter = this->DeviceInfoList.find(id);

  if (iter != this->DeviceInfoList.end())
    {
    if (io == IO_UNSPECIFIED)
      {
      this->OutgoingDeviceIDSet.erase(id);
      this->IncomingDeviceIDSet.erase(id);
      this->UnspecifiedDeviceIDSet.erase(id);
      }
    if (io & IO_INCOMING)
      {
      this->IncomingDeviceIDSet.erase(id);
      }
    if (io & IO_OUTGOING)
      {
      this->OutgoingDeviceIDSet.erase(id);
      }
    // search in device io lists 
    if (this->OutgoingDeviceIDSet.find(id) == this->OutgoingDeviceIDSet.end() &&
        this->IncomingDeviceIDSet.find(id) == this->IncomingDeviceIDSet.end() &&
        this->UnspecifiedDeviceIDSet.find(id) == this->UnspecifiedDeviceIDSet.end())
      {
      this->DeviceInfoList.erase(iter);   // if not found, remove from device info list
      return 1;
      }
    }

  return 0;
}


//---------------------------------------------------------------------------
int vtkIGTLConnector::UnregisterDevice(int id)
{
  DeviceInfoMapType::iterator iter = this->DeviceInfoList.find(id);
  if (iter != this->DeviceInfoList.end())
    {
    this->UnspecifiedDeviceIDSet.erase(id);
    this->IncomingDeviceIDSet.erase(id);
    this->OutgoingDeviceIDSet.erase(id);
    this->DeviceInfoList.erase(iter);
    return 1;
    }
  return 0;
}


//---------------------------------------------------------------------------
int vtkIGTLConnector::RegisterDeviceIO(int id, int io)
{
  DeviceInfoMapType::iterator iter = this->DeviceInfoList.find(id);

  if (iter != this->DeviceInfoList.end()) // if id is on the list
    {
    if (io == IO_UNSPECIFIED)
      {
      iter->second.io = IO_UNSPECIFIED;
      this->UnspecifiedDeviceIDSet.insert(id);
      this->IncomingDeviceIDSet.erase(id);
      this->OutgoingDeviceIDSet.erase(id);
      }
    else
      {
      if (io & IO_INCOMING)
        {
        iter->second.io |= IO_INCOMING;
        this->UnspecifiedDeviceIDSet.erase(id);
        this->IncomingDeviceIDSet.insert(id);
        }
      if (io & IO_OUTGOING)
        {
        iter->second.io |= IO_OUTGOING;
        this->UnspecifiedDeviceIDSet.erase(id);
        this->OutgoingDeviceIDSet.insert(id);
        }
      }
    return 1;
    }

  return 0;
}

//---------------------------------------------------------------------------
vtkIGTLConnector::DeviceInfoType* vtkIGTLConnector::GetDeviceInfo(int id)
{
  DeviceInfoMapType::iterator iter = this->DeviceInfoList.find(id);
  if (iter != this->DeviceInfoList.end())
    {
    return &(iter->second);
    }

  return NULL;
}
