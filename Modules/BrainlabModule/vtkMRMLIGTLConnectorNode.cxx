/*=auto=========================================================================

Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include "vtkObjectFactory.h"

#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLScene.h"
#include "vtkIGTLCircularBuffer.h"

#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"
#include "vtkImageData.h"

#include "igtlServerSocket.h"
#include "igtlClientSocket.h"
#include "igtlOSUtil.h"
#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"




//------------------------------------------------------------------------------
vtkMRMLIGTLConnectorNode* vtkMRMLIGTLConnectorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLIGTLConnectorNode"); if(ret)
    {
      return (vtkMRMLIGTLConnectorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLIGTLConnectorNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLIGTLConnectorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLIGTLConnectorNode");
  if(ret)
    {
      return (vtkMRMLIGTLConnectorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLIGTLConnectorNode;
}

//----------------------------------------------------------------------------
vtkMRMLIGTLConnectorNode::vtkMRMLIGTLConnectorNode()
{
  this->HideFromEditors = false;

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

  this->EventQueueMutex = vtkMutexLock::New();

  this->IncomingDeviceIDSet.clear();
  this->OutgoingDeviceIDSet.clear();
  this->UnspecifiedDeviceIDSet.clear();

  this->MessageConverterList.clear();
  this->IGTLNameToConverterMap.clear();
  this->MRMLIDToConverterMap.clear();

  this->OutgoingMRMLNodeList.clear();
  this->IncomingMRMLNodeList.clear();

  this->CheckCRC = 1;

}

//----------------------------------------------------------------------------
vtkMRMLIGTLConnectorNode::~vtkMRMLIGTLConnectorNode()
{

  this->Stop();
  if (this->Thread)
    {
    this->Thread->Delete();
    }

  if (this->Mutex)
    {
    this->Mutex->Delete();
    }

  this->CircularBufferMutex->Lock();
  CircularBufferMap::iterator iter;
  for (iter = this->Buffer.begin(); iter != this->Buffer.end(); iter ++)
    {
    iter->second->Delete();
    }
  this->Buffer.clear();
  this->CircularBufferMutex->Unlock();
  
  if (this->CircularBufferMutex)
    {
    this->CircularBufferMutex->Delete();
    }

  if (this->EventQueueMutex)
    {
    this->EventQueueMutex->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  switch (this->Type)
    {
    case TYPE_SERVER:
      of << " connectorType=\"" << "SERVER" << "\" ";
      break;
    case TYPE_CLIENT:
      of << " connectorType=\"" << "CLIENT" << "\" ";
      of << " serverHostname=\"" << this->ServerHostname << "\" ";
      break;
    default:
      of << " connectorType=\"" << "NOT_DEFINED" << "\" ";
      break;
    }

  of << " serverPort=\"" << this->ServerPort << "\" ";
  of << " restrictDeviceName=\"" << this->RestrictDeviceName << "\" ";

}


//----------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  const char* serverHostname = "";
  int port = 0;
  int type = -1;
  int restrictDeviceName = 0;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "connectorType"))
      {
      if (!strcmp(attValue, "SERVER"))
        {
        type = TYPE_SERVER;
        }
      else if (!strcmp(attValue, "CLIENT"))
        {
        type = TYPE_CLIENT;
        }
      else
        {
        type = TYPE_NOT_DEFINED;
        }
      }
    if (!strcmp(attName, "serverHostname"))
      {
      serverHostname = attValue;
      }
    if (!strcmp(attName, "serverPort"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> port;
      }
    if (!strcmp(attName, "restrictDeviceName"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> restrictDeviceName;;
      }
    }

  switch(type)
    {
    case TYPE_SERVER:
      this->SetTypeServer(port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    case TYPE_CLIENT:
      this->SetTypeClient(serverHostname, port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    default: // not defined
      // do nothing
      break;
    }

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLIGTLConnectorNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  vtkMRMLIGTLConnectorNode *node = (vtkMRMLIGTLConnectorNode *) anode;

  int type = node->GetType();
  
  switch(type)
    {
    case TYPE_SERVER:
      this->SetType(TYPE_SERVER);
      this->SetTypeServer(node->GetServerPort());
      this->SetRestrictDeviceName(node->GetRestrictDeviceName());
      break;
    case TYPE_CLIENT:
      this->SetType(TYPE_CLIENT);
      this->SetTypeClient(node->GetServerHostname(), node->GetServerPort());
      this->SetRestrictDeviceName(node->GetRestrictDeviceName());
      break;
    default: // not defined
      // do nothing
      this->SetType(TYPE_NOT_DEFINED);
      break;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  MRMLNodeListType::iterator iter;
  for (iter = this->OutgoingMRMLNodeList.begin(); iter != this->OutgoingMRMLNodeList.end(); iter ++)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
    if (node == *iter)
      {
      int size;
      void* igtlMsg;
      vtkIGTLToMRMLBase* converter = this->MRMLIDToConverterMap[node->GetID()];
      if (converter->MRMLToIGTL(event, node, &size, &igtlMsg))
        {
        int r = this->SendData(size, (unsigned char*)igtlMsg);
        if (r == 0)
          {
          // TODO: error handling
          //std::cerr << "ERROR: send data." << std::endl;
          }
        return;
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::SetTypeServer(int port)
{
  this->Type = TYPE_SERVER;
  this->ServerPort = port;
  this->Modified();
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::SetTypeClient(char* hostname, int port)
{
  this->Type = TYPE_CLIENT;
  this->ServerPort = port;
  this->ServerHostname = hostname;
  this->Modified();
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::SetTypeClient(std::string hostname, int port)
{
  this->Type = TYPE_CLIENT;
  this->ServerPort = port;
  this->ServerHostname = hostname;
  this->Modified();
  return 1;
}


//----------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::SetCheckCRC(int c)
{

  if (c == 0)
    {
    this->CheckCRC = 0;
    }
  else
    {
    this->CheckCRC = 1;
    }

  // Set CheckCRC flag in each converter
  MessageConverterListType::iterator iter;
  for (iter = this->MessageConverterList.begin();
       iter != this->MessageConverterList.end();
       iter ++)
    {
    (*iter)->SetCheckCRC(this->CheckCRC);
    }

}


//---------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::Start()
{
  // Check if type is defined.
  if (this->Type == vtkMRMLIGTLConnectorNode::TYPE_NOT_DEFINED)
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
  this->ThreadID = this->Thread->SpawnThread((vtkThreadFunctionType) &vtkMRMLIGTLConnectorNode::ThreadFunction, this);

  // Following line is necessary in some Linux environment,
  // since it takes for a while for the thread to update
  // this->State to non STATE_OFF value. This causes error
  // after calling vtkMRMLIGTLConnectorNode::Start() in ProcessGUIEvent()
  // in vtkOpenIGTLinkIFGUI class.
  this->State = STATE_WAIT_CONNECTION;
  this->InvokeEvent(vtkMRMLIGTLConnectorNode::ActivatedEvent);

  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::Stop()
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
    this->Thread->TerminateThread(this->ThreadID);
    this->ThreadID = -1;
    return 1;
    }
  else
    {
    return 0;
    }
}


//---------------------------------------------------------------------------
void* vtkMRMLIGTLConnectorNode::ThreadFunction(void* ptr)
{

  //vtkMRMLIGTLConnectorNode* igtlcon = static_cast<vtkMRMLIGTLConnectorNode*>(ptr);
  vtkMultiThreader::ThreadInfo* vinfo = 
    static_cast<vtkMultiThreader::ThreadInfo*>(ptr);
  vtkMRMLIGTLConnectorNode* igtlcon = static_cast<vtkMRMLIGTLConnectorNode*>(vinfo->UserData);
  
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
      igtlcon->RequestInvokeEvent(vtkMRMLIGTLConnectorNode::ConnectedEvent); // need to Request the InvokeEvent, because we are not on the main thread now
      //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ThreadFunction(): Client Connected.");
      igtlcon->ReceiveController();
      igtlcon->State = STATE_WAIT_CONNECTION;
      igtlcon->RequestInvokeEvent(vtkMRMLIGTLConnectorNode::DisconnectedEvent); // need to Request the InvokeEvent, because we are not on the main thread now
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
  igtlcon->RequestInvokeEvent(vtkMRMLIGTLConnectorNode::DeactivatedEvent); // need to Request the InvokeEvent, because we are not on the main thread now

  return NULL;

}

//----------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::RequestInvokeEvent(unsigned long eventId)
{
  this->EventQueueMutex->Lock();
  this->EventQueue.push_back(eventId);
  this->EventQueueMutex->Unlock();
}

//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::WaitForConnection()
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
      //vtkErrorMacro("vtkMRMLIGTLConnectorNode: Waiting for client @ port #" << this->ServerPort);
      this->Socket = this->ServerSocket->WaitForConnection(1000);      
      if (this->Socket.IsNotNull()) // if client connected
        {
        //vtkErrorMacro("vtkMRMLIGTLConnectorNode: connected.");
        return 1;
        }
      }
    else if (this->Type == TYPE_CLIENT) // if this->Type == TYPE_CLIENT
      {
      //vtkErrorMacro("vtkMRMLIGTLConnectorNode: Connecting to server...");
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


//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::ReceiveController()
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
      // Check if the node has already been registered.
      int registered = 0;
      MRMLNodeListType::iterator iter;
      for (iter = this->IncomingMRMLNodeList.begin(); iter != this->IncomingMRMLNodeList.end(); iter ++)
        {
        if (strcmp((*iter)->GetName(), headerMsg->GetDeviceName()) == 0)
          {
          // Find converter for this message's device name to find out the MRML node type
          vtkIGTLToMRMLBase* converter = GetConverterByIGTLDeviceType(headerMsg->GetDeviceType());
          if (converter)
            {
            const char* mrmlName = converter->GetMRMLName();
            if (strcmp((*iter)->GetNodeTagName(), mrmlName) == 0)
              {
              registered = 1;
              break; // for (;;)
              }
            }
          }
        }
      if (registered == 0)
        {
        this->Skip(headerMsg->GetBodySizeToRead());
        continue; //  while (!this->ServerStopFlag)
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


//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::SendData(int size, unsigned char* data)
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


//----------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::Skip(int length, int skipFully)
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


//----------------------------------------------------------------------------
unsigned int vtkMRMLIGTLConnectorNode::GetUpdatedBuffersList(NameListType& nameList)
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


//----------------------------------------------------------------------------
vtkIGTLCircularBuffer* vtkMRMLIGTLConnectorNode::GetCircularBuffer(std::string& key)
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
void vtkMRMLIGTLConnectorNode::ImportDataFromCircularBuffer()
{

  vtkMRMLIGTLConnectorNode::NameListType nameList;
  GetUpdatedBuffersList(nameList);
  
  vtkMRMLIGTLConnectorNode::NameListType::iterator nameIter;
  for (nameIter = nameList.begin(); nameIter != nameList.end(); nameIter ++)
    {
    vtkIGTLCircularBuffer* circBuffer = GetCircularBuffer(*nameIter);
    circBuffer->StartPull();
    
    igtl::MessageBase::Pointer buffer = circBuffer->GetPullBuffer();

    MessageConverterMapType::iterator conIter = 
      this->IGTLNameToConverterMap.find(buffer->GetDeviceType());
    if (conIter == this->IGTLNameToConverterMap.end()) // couldn't find from the map
      {
      continue;
      }
    vtkIGTLToMRMLBase* converter = conIter->second;

    vtkMRMLScene* scene = this->GetScene();

    // look up the incoming MRML node list
    MRMLNodeListType::iterator inIter;
    for (inIter = this->IncomingMRMLNodeList.begin();
         inIter != this->IncomingMRMLNodeList.end();
         inIter ++)
      {
      if (strcmp((*inIter)->GetNodeTagName(), converter->GetMRMLName()) == 0 && 
          strcmp((*inIter)->GetName(), (*nameIter).c_str()) == 0)
        {
        vtkMRMLNode* node = (*inIter);
        converter->IGTLToMRML(buffer, node);
        node->Modified();
        continue;
        }
      }

    // if the incoming data is not restricted by name and type, search the scene as well.
    if (!this->RestrictDeviceName)
      {
      const char* classname = scene->GetClassNameByTag(converter->GetMRMLName());
      vtkCollection* collection = scene->GetNodesByClassByName(classname, buffer->GetDeviceName());
      int nCol = collection->GetNumberOfItems();
      if (nCol == 0)
        {
        vtkMRMLNode* node = converter->CreateNewNode(this->GetScene(), buffer->GetDeviceName());
        RegisterIncomingMRMLNode(node);
        converter->IGTLToMRML(buffer, node);
        node->Modified();
        }
      else
        {
        for (int i = 0; i < nCol; i ++)
          {
          vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
          RegisterIncomingMRMLNode(node);
          converter->IGTLToMRML(buffer, node);
          node->Modified();
          continue;
          }
        }
      }
    circBuffer->EndPull();
    }

}

//---------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::ImportEventsFromEventBuffer()
{
  // Invoke all events in the EventQueue

  bool emptyQueue=true;
  unsigned long eventId=0;
  do
  {
    emptyQueue=true;
    this->EventQueueMutex->Lock();
    if (this->EventQueue.size()>0)
    {
      eventId=this->EventQueue.front();
      this->EventQueue.pop_front();
      emptyQueue=false;
    }
    this->EventQueueMutex->Unlock();

    // Invoke the event
    this->InvokeEvent(eventId);

  } while (!emptyQueue);

}


//---------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::RegisterMessageConverter(vtkIGTLToMRMLBase* converter)
{
  if (converter == NULL)
    {
      return 0;
    }
  
  // Check if the same converter has already been registered.
  MessageConverterListType::iterator iter;
  for (iter = this->MessageConverterList.begin();
       iter != this->MessageConverterList.end();
       iter ++)
    {
    if ((converter->GetIGTLName() && strcmp(converter->GetIGTLName(), (*iter)->GetIGTLName()) == 0) &&
        (converter->GetMRMLName() && strcmp(converter->GetMRMLName(), (*iter)->GetMRMLName()) == 0))
      {
      return 0;
      }
    }
  
  // Register the converter
  if (converter->GetIGTLName() && converter->GetMRMLName())
    {
    // check the converter type (single IGTL name or multiple IGTL names?)
    if (converter->GetConverterType() == vtkIGTLToMRMLBase::TYPE_NORMAL)
      {
      const char* name = converter->GetIGTLName();

      // Check if the name already exists.
      MessageConverterMapType::iterator citer;
      citer = this->IGTLNameToConverterMap.find(name);
      if (citer != this->IGTLNameToConverterMap.end()) // exists
        {
        std::cerr << "The converter with the same IGTL name has already been registered." << std::endl;
        return 0;
        }
      else
        {
        // Add converter to the map
        this->IGTLNameToConverterMap[name] = converter;
        }
      }

    else // vtkIGTLToMRMLBase::TYPE_MULTI_IGTL_NAMES
      { 
      int numNames = converter->GetNumberOfIGTLNames();

      // Check if one of the names already exists.
      for (int i = 0; i < numNames; i ++)
        {
        const char* name = converter->GetIGTLName(i);
        MessageConverterMapType::iterator citer;
        citer = this->IGTLNameToConverterMap.find(name);
        if (citer != this->IGTLNameToConverterMap.end()) // exists
          {
          std::cerr << "The converter with the same IGTL name has already been registered." << std::endl;
          return 0;
          }
        }

      for (int i = 0; i < numNames; i ++)
        {
        // Add converter to the map
        const char* name = converter->GetIGTLName(i);
        this->IGTLNameToConverterMap[name] = converter;
        }
      
      }

    // Set CRC check flag
    converter->SetCheckCRC(this->CheckCRC);

    // Add the converter to the list
    this->MessageConverterList.push_back(converter);

    return 1;
    }
  else
    {
    return 0;
    }
}


//---------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::UnregisterMessageConverter(vtkIGTLToMRMLBase* converter)
{
  
  if (converter)
    {
    MessageConverterListType::iterator iter;
    for (iter = this->MessageConverterList.begin(); iter != this->MessageConverterList.end(); iter ++)
      {
      if (*iter == converter)
        {
        this->MessageConverterList.erase(iter); // TODO: Does this work properly? Does the iterator need to be incremented?
        }
      }
    }

}


//---------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::RegisterOutgoingMRMLNode(vtkMRMLNode* node)
{

  if (!node)
    {
    return 0;
    }

  // Find a converter for the node
  const char* tag = node->GetNodeTagName();
  vtkIGTLToMRMLBase* converter = GetConverterByMRMLTag(tag);
  if (!converter)
    {
    return 0;
    }

  // Check if the node has already been reagistered.
  MRMLNodeListType::iterator iter;
  for (iter = this->OutgoingMRMLNodeList.begin(); iter != this->OutgoingMRMLNodeList.end(); iter ++)
    {
    if (*iter == node) // the node has been already registered.
      {
      // Unregister events
      vtkIntArray* nodeEvents = converter->GetNodeEvents();
      int n = nodeEvents->GetNumberOfTuples();
      for (int i = 0; i < n; i ++)
        {
        int event = nodeEvents->GetValue(i);
        vtkEventBroker::GetInstance()
          ->RemoveObservations(*iter, event, this, this->MRMLCallbackCommand );
        }
      this->OutgoingMRMLNodeList.erase(iter);
      break;
      }
    }
  
  this->MRMLIDToConverterMap[node->GetID()] = converter;    
  this->OutgoingMRMLNodeList.push_back(node);
  
  vtkIntArray* nodeEvents = converter->GetNodeEvents();
  int n = nodeEvents->GetNumberOfTuples();
  for (int i = 0; i < n; i ++)
    {
    // regiser events
    int event = nodeEvents->GetValue(i);
    vtkEventBroker::GetInstance()
      ->AddObservation(node, event, this, this->MRMLCallbackCommand );
    }

  this->Modified();

  return 1;

}


//---------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::UnregisterOutgoingMRMLNode(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }

  // Check if the node has already been reagistered.
  MRMLNodeListType::iterator iter;
  for (iter = this->OutgoingMRMLNodeList.begin(); iter != this->OutgoingMRMLNodeList.end(); iter ++)
    {
    if (*iter == node) // the node has been already registered.
      {
      vtkEventBroker::GetInstance()
        ->RemoveObservations(*iter, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
      this->OutgoingMRMLNodeList.erase(iter);

      // Search converter from MRMLIDToConverterMap
      MessageConverterMapType::iterator citer = this->MRMLIDToConverterMap.find(node->GetID());
      if (citer != this->MRMLIDToConverterMap.end())
        {
        this->MRMLIDToConverterMap.erase(citer);
        break;
        }
      break;
      }
    }
}


//---------------------------------------------------------------------------
int vtkMRMLIGTLConnectorNode::RegisterIncomingMRMLNode(vtkMRMLNode* node)
{

  if (!node)
    {
    return 0;
    }

  // Check if the node has already been registered.
  MRMLNodeListType::iterator iter;
  for (iter = this->IncomingMRMLNodeList.begin(); iter != this->IncomingMRMLNodeList.end(); iter ++)
    {
    if (*iter == node) // the node has been already registered.
      {
      this->IncomingMRMLNodeList.erase(iter);
      break;
      }
    }
  this->IncomingMRMLNodeList.push_back(node);
  this->Modified();

  return 1;
}


//---------------------------------------------------------------------------
void vtkMRMLIGTLConnectorNode::UnregisterIncomingMRMLNode(vtkMRMLNode* node)
{

  std::cerr << "void vtkMRMLIGTLConnectorNode::UnregisterIncomingMRMLNode(vtkMRMLNode* node)" << std::endl;
  

  if (!node)
    {
    return;
    }

  // Check if the node has already been reagistered.
  // TODO: MRMLNodeListType can be reimplemented as a std::list
  // so that the converter can be removed by 'remove()' method.
  MRMLNodeListType::iterator iter;
  for (iter = this->IncomingMRMLNodeList.begin(); iter != this->IncomingMRMLNodeList.end(); iter ++)
    {
    if (*iter == node) // the node has been already registered.
      {
      this->IncomingMRMLNodeList.erase(iter);
      break;
      }
    }
  
}


//---------------------------------------------------------------------------
unsigned int vtkMRMLIGTLConnectorNode::GetNumberOfOutgoingMRMLNodes()
{
  return this->OutgoingMRMLNodeList.size();
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLIGTLConnectorNode::GetOutgoingMRMLNode(unsigned int i)
{
  if (i < this->OutgoingMRMLNodeList.size())
    {
    return this->OutgoingMRMLNodeList[i];
    }
  else
    {
    return NULL;
    }
}


//---------------------------------------------------------------------------
unsigned int vtkMRMLIGTLConnectorNode::GetNumberOfIncomingMRMLNodes()
{
  return this->IncomingMRMLNodeList.size();
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLIGTLConnectorNode::GetIncomingMRMLNode(unsigned int i)
{

  if (i < this->IncomingMRMLNodeList.size())
    {
    return this->IncomingMRMLNodeList[i];
    }
  else
    {
    return NULL;
    }

}
  

//---------------------------------------------------------------------------
vtkIGTLToMRMLBase* vtkMRMLIGTLConnectorNode::GetConverterByMRMLTag(const char* tag)
{

  MessageConverterListType::iterator iter;
  for (iter = this->MessageConverterList.begin();
       iter != this->MessageConverterList.end();
       iter ++)
    {
    if ((*iter) && strcmp((*iter)->GetMRMLName(), tag) == 0)
      {
      return *iter;
      }
    }

  // if no converter is found.
  return NULL;

}


//---------------------------------------------------------------------------
vtkIGTLToMRMLBase* vtkMRMLIGTLConnectorNode::GetConverterByIGTLDeviceType(const char* type)
{
  MessageConverterListType::iterator iter;

  for (iter = this->MessageConverterList.begin();
       iter != this->MessageConverterList.end();
       iter ++)
    {
    vtkIGTLToMRMLBase* converter = *iter;
    if (converter->GetConverterType() == vtkIGTLToMRMLBase::TYPE_NORMAL)
      {
      if (strcmp(converter->GetMRMLName(), type) == 0)
        {
        return converter;
        }
      }
    else // The converter has multiple IGTL device names
      {
      int n = converter->GetNumberOfIGTLNames();
      for (int i = 0; i < n; i ++)
        {
        if (strcmp(converter->GetIGTLName(i), type) == 0)
          {
          return converter;
          }
        }
      }
    }
  
  // if no converter is found.
  return NULL;
  
}



