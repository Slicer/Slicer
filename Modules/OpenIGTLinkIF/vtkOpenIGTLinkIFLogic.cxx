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
#include "vtkCallbackCommand.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"

#include "vtkOpenIGTLinkIFLogic.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerColorLogic.h"

#include "vtkCylinderSource.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"


#include "vtkMultiThreader.h"

#include "vtkIGTLConnector.h"
#include "vtkIGTLCircularBuffer.h"

//#include "igtl_header.h"
//#include "igtl_image.h"
//#include "igtl_transform.h"

vtkCxxRevisionMacro(vtkOpenIGTLinkIFLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkOpenIGTLinkIFLogic);

//---------------------------------------------------------------------------
vtkOpenIGTLinkIFLogic::vtkOpenIGTLinkIFLogic()
{
  // initialize slice driver information
  for (int i = 0; i < 3; i ++)
    {
    this->SliceDriver[i] = vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER;
    //this->SliceDriverConnectorID[i] = -1;
    //this->SliceDriverDeviceID[i] = -1;
    //this->SliceDriverNodeID[i] = "";
    }
  this->LocatorDriverFlag = 0;
  //this->LocatorDriver = NULL;
  this->LocatorDriverNodeID = "";
  this->RealTimeImageSourceNodeID = "";

  // If the following code doesn't work, slice nodes should be obtained from application GUI
  this->SliceNode[0] = NULL;
  this->SliceNode[1] = NULL;
  this->SliceNode[2] = NULL;

  this->SliceOrientation[0] = SLICE_RTIMAGE_PERP;
  this->SliceOrientation[1] = SLICE_RTIMAGE_INPLANE90;
  this->SliceOrientation[2] = SLICE_RTIMAGE_INPLANE;

  this->NeedRealtimeImageUpdate0 = 0;
  this->NeedRealtimeImageUpdate1 = 0;
  this->NeedRealtimeImageUpdate2 = 0;
  this->ImagingControl = 0;

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkOpenIGTLinkIFLogic::DataCallback);

  this->ConnectorMap.clear();
  this->ConnectorPrevStateList.clear();

  this->EnableOblique = false;
  this->FreezePlane   = false;

  this->Initialized   = 0;
  this->RestrictDeviceName = 0;
  
  this->LastConnectorID = -1;
  this->ConnectorMap.clear();
  this->ConnectorPrevStateList.clear();

  this->IDToMRMLNodeMap.clear();
  this->MessageConverterList.clear();

  //this->OutTransformMsg = igtl::TransformMessage::New();
  //this->OutImageMsg = igtl::ImageMessage::New();


  // register default data types
  this->LinearTransformConverter = vtkIGTLToMRMLLinearTransform::New();
  this->ImageConverter           = vtkIGTLToMRMLImage::New();
  this->PositionConverter        = vtkIGTLToMRMLPosition::New();
  RegisterMessageConverter(this->LinearTransformConverter);
  RegisterMessageConverter(this->ImageConverter);
  RegisterMessageConverter(this->PositionConverter);

}


//---------------------------------------------------------------------------
vtkOpenIGTLinkIFLogic::~vtkOpenIGTLinkIFLogic()
{
  if (this->LinearTransformConverter)
    {
    UnregisterMessageConverter(this->LinearTransformConverter);
    this->LinearTransformConverter->Delete();
    }

  if (this->ImageConverter)
    {
    UnregisterMessageConverter(this->ImageConverter);
    this->ImageConverter->Delete();
    }

  if (this->PositionConverter)
    {
    UnregisterMessageConverter(this->PositionConverter);

    this->PositionConverter->Delete();
    }

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkOpenIGTLinkIFLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::Initialize()
{

  // -----------------------------------------
  // Register MRML event handler
  
  if (this->Initialized == 0)
    {
    // MRML Event Handling
    vtkIntArray* events = vtkIntArray::New();
    //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    //events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
    //events->InsertNextValue(vtkMRMLScene::Node
    if (this->GetMRMLScene() != NULL)
      {
      //this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
      }

    events->Delete();
    this->Initialized = 1;

    }

  return 1;

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkOpenIGTLinkIFLogic *self = reinterpret_cast<vtkOpenIGTLinkIFLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkOpenIGTLinkIFLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateAll()
{

}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::CheckConnectorsStatusUpdates()
{

  //----------------------------------------------------------------
  // Find state change in the connectors

  //int nCon = GetNumberOfConnectors();
  int updated = 0;

  ConnectorMapType::iterator iter;
  //for (int i = 0; i < nCon; i ++)
  for (iter = this->ConnectorMap.begin(); iter != this->ConnectorMap.end(); iter ++)
    {
    int id = iter->first;
    if (this->ConnectorPrevStateList[id] != this->ConnectorMap[id]->GetState())
      {
      updated = 1;
      this->ConnectorPrevStateList[id] = this->ConnectorMap[id]->GetState();
      }
    }

  return updated;

}



//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddConnector()
{
  //this->AddConnector("connector");
  this->AddConnector(NULL);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddConnector(const char* name)
{
  vtkIGTLConnector* connector = vtkIGTLConnector::New();
  this->LastConnectorID ++;
  int newID = this->LastConnectorID;

  if (name == NULL)
    {
    char newname[128];
    //sprintf(newname, "Connector%d", (int)this->ConnectorMap.size() + 1);
    sprintf(newname, "Connector%d", newID + 1);
    connector->SetName(newname);
    }
  else
    {
    connector->SetName(name);
    }
  //this->ConnectorMap.push_back(connector);
  this->ConnectorMap[newID] = connector;
  //this->ConnectorPrevStateList.push_back(-1);
  this->ConnectorPrevStateList[newID] = -1;
  connector->SetRestrictDeviceName(this->RestrictDeviceName);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddServerConnector(const char* name, int port)
{
  vtkIGTLConnector* connector = vtkIGTLConnector::New();
  this->LastConnectorID ++;
  int newID = this->LastConnectorID;

  connector->SetName(name);
  connector->SetType(vtkIGTLConnector::TYPE_SERVER);
  connector->SetServerPort(port);
  //this->ConnectorMap.push_back(connector);
  this->ConnectorMap[newID] = connector;
  //this->ConnectorPrevStateList.push_back(-1);
  this->ConnectorPrevStateList[newID] = -1;
  connector->SetRestrictDeviceName(this->RestrictDeviceName);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddClientConnector(const char* name, const char* svrHostName, int port)
{
  vtkIGTLConnector* connector = vtkIGTLConnector::New();
  this->LastConnectorID ++;
  int newID = this->LastConnectorID;

  connector->SetName(name);
  connector->SetType(vtkIGTLConnector::TYPE_CLIENT);
  connector->SetServerPort(port);
  connector->SetServerHostname(svrHostName);
  //this->ConnectorMap.push_back(connector);
  this->ConnectorMap[newID] = connector;
  //this->ConnectorPrevStateList.push_back(-1);
  this->ConnectorPrevStateList[newID] = -1;
  connector->SetRestrictDeviceName(this->RestrictDeviceName);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::DeleteConnector(int id)
{
  ConnectorMapType::iterator iter = this->ConnectorMap.find(id);

  //if (id >= 0 && id < (int)this->ConnectorMap.size())
  if (iter != this->ConnectorMap.end()) // if id is on the list
    {
    this->ConnectorMap[id]->Stop();
    this->ConnectorMap[id]->Delete();
    //this->ConnectorMap.erase(this->ConnectorMap.begin() + id);
    this->ConnectorMap.erase(iter);
    //this->ConnectorPrevStateList.erase(this->ConnectorPrevStateList.begin() + id);
    ConnectorStateMapType::iterator iter2 = this->ConnectorPrevStateList.find(id);
    if (iter2 != this->ConnectorPrevStateList.find(id))
      {
      this->ConnectorPrevStateList.erase(iter2);
      }
    }
}

//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::GetNumberOfConnectors()
{
  return this->ConnectorMap.size();
}

//---------------------------------------------------------------------------
vtkIGTLConnector* vtkOpenIGTLinkIFLogic::GetConnector(int id)
{
  ConnectorMapType::iterator iter = this->ConnectorMap.find(id);

  //if (id >= 0 && id < GetNumberOfConnectors())
  if (iter != this->ConnectorMap.end())
    {
    return this->ConnectorMap[id];
    }
  else
    {
    return NULL;
    }
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::GetConnectorID(vtkIGTLConnector* con)
{
  ConnectorMapType::iterator cmiter;
  for (cmiter = this->ConnectorMap.begin(); cmiter != this->ConnectorMap.end(); cmiter ++)
    {
    if (cmiter->second == con)
      {
      return cmiter->first;
      }
    }
  return -1;
}


//---------------------------------------------------------------------------
vtkOpenIGTLinkIFLogic::ConnectorMapType* vtkOpenIGTLinkIFLogic::GetConnectorMap()
{
  return &(this->ConnectorMap);
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::RegisterDeviceEvent(vtkIGTLConnector* con, const char* deviceName, const char* deviceType)
{
  if (con == NULL)
    {
    return 0;
    }

  // check if the connector exists in the table
  if (GetConnectorID(con) < 0)
    {
    return 0;
    }
  
  // find converter
  vtkIGTLToMRMLBase* converter = GetConverterByDeviceType(deviceType);
  if (converter == NULL)
    {
    return 0;
    }

  // check if the device name exists in the MRML tree
  vtkMRMLNode* srcNode = NULL;   // Event Source MRML node 
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName(deviceName);
  int nItems = collection->GetNumberOfItems();
  for (int i = 0; i < nItems; i ++)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
    if (converter->GetMRMLName() && strcmp(node->GetNodeTagName(), converter->GetMRMLName()) == 0)
      {
      srcNode = node;
      break;
      }
    }
  
  if (srcNode == NULL) // couldn't find a device with the specified name and type.
    {
    srcNode = converter->CreateNewNode(this->GetMRMLScene(), deviceName);
    }

  // check if events have already been registered
  ConnectorListType* list = &MRMLEventConnectorMap[srcNode];
  ConnectorListType::iterator iter;
  int found = 0;
  for (iter = list->begin(); iter != list->end(); iter ++)
    {
    if (*iter == con)
      {
      found = 1;
      break;
      }
    }
  if (found) // the events has already been registered
    {
    return 0;
    }
  
  // register events
  vtkIntArray* nodeEvents = converter->GetNodeEvents();
  if (nodeEvents)
    {
    vtkMRMLNode *node = NULL; // TODO: is this OK?
    vtkSetAndObserveMRMLNodeEventsMacro(node,srcNode,nodeEvents);
    }

  // TODO: node should be stored somewhere to stop event monitoring after deleting the MRML node.
  nodeEvents->Delete();
  list->push_back(con);

  return 1;
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::UnregisterDeviceEvent(vtkIGTLConnector* con, const char* deviceName, const char* deviceType)
{
  if (con == NULL)
    {
    return 0;
    }

  // check if the connector exists in the table
  if (GetConnectorID(con) < 0)
    {
    return 0;
    }

  // check if the device name exists in the MRML tree
  vtkMRMLNode* srcNode = NULL;   // Event Source MRML node 
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName(deviceName);
  int nItems = collection->GetNumberOfItems();
  for (int i = 0; i < nItems; i ++)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
    if (strcmp(node->GetNodeTagName(), deviceType) == 0)
      {
      srcNode = node;
      break;
      }
    }

  if (srcNode == NULL) // not found
    {
    return 0;
    }

  this->MRMLObserverManager->RemoveObjectEvents(srcNode);

  // unregister from event connector map
  ConnectorListType* list = &MRMLEventConnectorMap[srcNode];
  ConnectorListType::iterator iter;
  for (iter = list->begin(); iter != list->end(); iter ++)
    {
    if (*iter == con)
      {
      list->erase(iter);
      }
    }

  return 1;
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::ImportFromCircularBuffers()
{
  ConnectorMapType::iterator cmiter;

  for (cmiter = this->ConnectorMap.begin(); cmiter != this->ConnectorMap.end(); cmiter ++)
    {
    vtkIGTLConnector::NameListType nameList;
    //(*iter)->GetUpdatedBuffersList(nameList);
    cmiter->second->GetUpdatedBuffersList(nameList);
    vtkIGTLConnector::NameListType::iterator nameIter;
    for (nameIter = nameList.begin(); nameIter != nameList.end(); nameIter ++)
      {
      //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ImportFromCircularBuffers(): Import Image from : " << *nameIter);
      //vtkIGTLCircularBuffer* circBuffer = (*iter)->GetCircularBuffer(*nameIter);

      vtkIGTLCircularBuffer* circBuffer = cmiter->second->GetCircularBuffer(*nameIter);
      circBuffer->StartPull();

      igtl::MessageBase::Pointer buffer = circBuffer->GetPullBuffer();
      MessageConverterListType::iterator iter;
      for (iter = this->MessageConverterList.begin();
           iter != this->MessageConverterList.end();
           iter ++)
        {
        vtkMRMLNode* node = NULL;
        if ((*iter)->GetIGTLName() && strcmp(buffer->GetDeviceType(), (*iter)->GetIGTLName()) == 0)
          {
          vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
          vtkCollection* collection = scene->GetNodesByName(buffer->GetDeviceName());
          int nCol = collection->GetNumberOfItems();
          if (nCol == 0)
            {
            node = (*iter)->CreateNewNode(this->GetMRMLScene(), buffer->GetDeviceName());
            }
          else
            {
            int found = -1;
            // if the same name is found in the scene, check the type
            for (int i = 0; i < nCol; i ++)
              {
              node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
              // check if the node type is same
              if ((*iter)->GetMRMLName() && strcmp(node->GetNodeTagName(), (*iter)->GetMRMLName()) == 0)
                {
                found = i;
                i = nCol; // loop end
                }
              }
            if (found == -1) // if the same type is not found
              {
              node = (*iter)->CreateNewNode(this->GetMRMLScene(), buffer->GetDeviceName());
              }
            }
          (*iter)->IGTLToMRML(buffer, node);
          //int devID = cmiter->second->GetDeviceID(buffer->GetDeviceName(), (*iter)->GetIGTLName());
          //if (devID >= 0)
          //  {
          //  PostImportProcess(cmiter->first, devID, node);
          //  }
          }
        }

      circBuffer->EndPull();
      }
    }
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::SetLocatorDriver(const char* nodeID)
{
  vtkMRMLLinearTransformNode* node =
    vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));

  if (node && strcmp(node->GetNodeTagName(), "LinearTransform") == 0)
    {
    this->LocatorDriverNodeID = nodeID;
    if (this->LocatorDriverFlag)
      {
      EnableLocatorDriver(1);
      }
    return 1;
    }

  return 0;
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::EnableLocatorDriver(int sw)
{
  if (sw == 1)  // turn on
    {
    this->LocatorDriverFlag = 1;
    vtkMRMLModelNode* mnode = 
      SetVisibilityOfLocatorModel("IGTLLocator", 1);
    vtkMRMLLinearTransformNode *tnode = 
      vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->LocatorDriverNodeID));
    if (!tnode)
      {
      return 0;
      }
    mnode->SetAndObserveTransformNodeID(tnode->GetID());
    mnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    }
  else  // turn off
    {
    this->LocatorDriverFlag = 0;
    //vtkMRMLModelNode* mnode = 
    //  SetVisibilityOfLocatorModel("IGTLLocator", 0);
    //mnode->SetAndObserveTransformNodeID(NULL);
    SetVisibilityOfLocatorModel("IGTLLocator", 0);
    }
  return 1;

}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::SetRealTimeImageSource(const char* nodeID)
{
  vtkMRMLVolumeNode* volNode =
    vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));

  if (volNode && strcmp(volNode->GetNodeTagName(), "Volume") == 0)
    {
    // register the volume node in event observer
    vtkMRMLNode *node = NULL; // TODO: is this OK?
    vtkIntArray* nodeEvents = vtkIntArray::New();
    nodeEvents->InsertNextValue(vtkMRMLVolumeNode::ImageDataModifiedEvent); 
    vtkSetAndObserveMRMLNodeEventsMacro(node,volNode,nodeEvents);
    nodeEvents->Delete();
    this->RealTimeImageSourceNodeID = nodeID;
    return 1;
    }

  return 0;
  
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::SetSliceDriver(int index, int v)
{
  if (index < 0 || index >= 3)
    {
    return 0;
    }

  this->SliceDriver[index] = v;
  if (v == SLICE_DRIVER_LOCATOR)
    {
    vtkMRMLLinearTransformNode* transNode =
      vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->LocatorDriverNodeID));
    if (transNode)
      {
      vtkMRMLNode *node = NULL; // TODO: is this OK?
      vtkIntArray* nodeEvents = vtkIntArray::New();
      nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
      vtkSetAndObserveMRMLNodeEventsMacro(node,transNode,nodeEvents);
      nodeEvents->Delete();
      }
    transNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    }

  return 1;
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::GetSliceDriver(int index)
{
  if (index < 0 || index >= 3)
    {
    return -1;
    }

  return this->SliceDriver[index];
    
}

//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::SetRestrictDeviceName(int f)
{

  if (f != 0) f = 1; // make sure that f is either 0 or 1.
  this->RestrictDeviceName = f;

  ConnectorMapType::iterator iter;
  for (iter = this->ConnectorMap.begin(); iter != this->ConnectorMap.end(); iter ++)
    {
    //(*iter)->SetRestrictDeviceName(f);
    iter->second->SetRestrictDeviceName(f);
    }

  return this->RestrictDeviceName;
}


//---------------------------------------------------------------------------
int  vtkOpenIGTLinkIFLogic::AddDeviceToConnector(int conID, const char* deviceName, const char* deviceType, int io)
// io -- vtkIGTLConnector::IO_INCOMING : incoming, vtkIGTLConnector::IO_OUTGOING: outgoing
{

  vtkIGTLConnector* connector = GetConnector(conID);

  if (connector)
    {
    if (io == vtkIGTLConnector::IO_INCOMING)
      {
      connector->RegisterNewDevice(deviceName, deviceType, vtkIGTLConnector::IO_INCOMING);
      }
    else if (io == vtkIGTLConnector::IO_OUTGOING)
      {
      connector->RegisterNewDevice(deviceName, deviceType, vtkIGTLConnector::IO_OUTGOING);
      RegisterDeviceEvent(connector,deviceName, deviceType);
      }
    else
      {
      connector->RegisterNewDevice(deviceName, deviceType);
      }
    return 1;
    }
  else
    {
    return 0;
    }
  
  return 1;

}


//---------------------------------------------------------------------------
int  vtkOpenIGTLinkIFLogic::DeleteDeviceFromConnector(int conID, const char* deviceName, const char* deviceType, int io)
{
  vtkIGTLConnector* connector = GetConnector(conID);

  if (connector)
    {
    int devid = connector->GetDeviceID(deviceName, deviceType);
    if (devid >= 0) // the device is found in the list
      {
      if (io == vtkIGTLConnector::IO_OUTGOING)
        {
        UnregisterDeviceEvent(connector, deviceName, deviceType);
        }
      connector->UnregisterDevice(deviceName, deviceType, io);
      }
    }

  return 1;

}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::DeleteDeviceFromConnector(int conID, int devID, int io)
{
  vtkIGTLConnector* connector = GetConnector(conID);

  vtkIGTLConnector::DeviceInfoType* devInfo = connector->GetDeviceInfo(devID);
  if (devInfo)
    {
    DeleteDeviceFromConnector(conID, devInfo->name.c_str(), devInfo->type.c_str(), io);
    }
  return 1;

}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::RegisterMessageConverter(vtkIGTLToMRMLBase* converter)
{
  if (converter == NULL)
    {
    return 0;
    }

  // Search the list and check if the same converter has already been registered.
  int found = 0;

  MessageConverterListType::iterator iter;
  for (iter = this->MessageConverterList.begin();
       iter != this->MessageConverterList.end();
       iter ++)
    {
    if ((converter->GetIGTLName() && strcmp(converter->GetIGTLName(), (*iter)->GetIGTLName()) == 0) &&
        (converter->GetMRMLName() && strcmp(converter->GetMRMLName(), (*iter)->GetMRMLName()) == 0))
      {
      found = 1;
      }
    }
  if (found)
    {
    return 0;
    }
  
  if (converter->GetIGTLName() || converter->GetMRMLName())
    {
    this->MessageConverterList.push_back(converter);
    return 1;
    }
  else
    {
    return 0;
    }
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::UnregisterMessageConverter(vtkIGTLToMRMLBase* converter)
{
  if (converter == NULL)
    {
    return 0;
    }

  this->MessageConverterList.remove(converter);
  return 1;
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData)
{
  if (caller != NULL)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);

    //---------------------------------------------------------------------------
    // Outgoing data
    // TODO: should check the type of the node here
    ConnectorListType* list = &MRMLEventConnectorMap[node];
    ConnectorListType::iterator cliter;
    for (cliter = list->begin(); cliter != list->end(); cliter ++)
      {
      vtkIGTLConnector* connector = *cliter;

      MessageConverterListType::iterator iter;
      for (iter = this->MessageConverterList.begin();
           iter != this->MessageConverterList.end();
           iter ++)
        {
        if ((*iter)->GetMRMLName() && strcmp(node->GetNodeTagName(), (*iter)->GetMRMLName()) == 0)
          {
          // check if the name-type combination is on the list
          if (connector->GetDeviceID(node->GetName(), (*iter)->GetIGTLName()) >= 0)
            {
            int size;
            void* igtlMsg;
            (*iter)->MRMLToIGTL(event, node, &size, &igtlMsg);
            int r = connector->SendData(size, (unsigned char*)igtlMsg);
            if (r == 0)
              {
              // TODO: error handling
              //std::cerr << "ERROR: send data." << std::endl;
              }
            }
          }
        } // for (iter)
      } // for (cliter)

    //---------------------------------------------------------------------------
    // Slice Driven by Locator
    if (node && strcmp(node->GetID(), this->LocatorDriverNodeID.c_str()) == 0)
      {
      vtkMatrix4x4* transform = NULL;
      for (int i = 0; i < 3; i ++)
        {
        if (this->SliceDriver[i] == SLICE_DRIVER_LOCATOR)
          {
          if (!transform)
            {
            vtkMRMLLinearTransformNode* transNode =    
              vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->LocatorDriverNodeID));
            if (transNode)
              {
              transform = transNode->GetMatrixTransformToParent();
              }
            }
          if (transform)
            {
            UpdateSliceNode(i, transform);
            }
          }
        }
      }

      
    //---------------------------------------------------------------------------
    // Slice Driven by Real-time image
    if (strcmp(node->GetID(), this->RealTimeImageSourceNodeID.c_str()) == 0)
      {
      for (int i = 0; i < 3; i ++)
        {
        if (this->SliceDriver[i] == SLICE_DRIVER_RTIMAGE)
          {
          UpdateSliceNodeByImage(i);
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLBase* vtkOpenIGTLinkIFLogic::GetConverterByDeviceType(const char* deviceType)
{
  vtkIGTLToMRMLBase* converter = NULL;

  MessageConverterListType::iterator iter;
  for (iter = this->MessageConverterList.begin();
       iter != this->MessageConverterList.end();
       iter ++)
    {
    if ((*iter)->GetConverterType() == vtkIGTLToMRMLBase::TYPE_NORMAL)
      {
      if (strcmp((*iter)->GetIGTLName(), deviceType) == 0)
        {
        converter = *iter;
        break;
        }
      }
    else
      {
      int n = (*iter)->GetNumberOfIGTLNames();
      for (int i = 0; i < n; i ++)
        {
        if (strcmp((*iter)->GetIGTLName(i), deviceType) == 0)
          {
          converter = *iter;
          break;
          }
        }
      }
    }

  return converter;
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateSliceNode(int sliceNodeNumber, vtkMatrix4x4* transform)
{

  // NOTES: In Slicer3 ver. 3.2 and higher, a slice orientation information in
  // a slice nodes classes are automaticall set to "Reformat", whenever
  // SetSliceToRASByNTP() function is called.
  // The OpenIGTLinkIF module saves the slice orientations in SliceOrientation[]
  // before the slice nodes update slice orientation information.

  if (this->FreezePlane)
    {
    return;
    }

  CheckSliceNode();

  float tx = transform->GetElement(0, 0);
  float ty = transform->GetElement(1, 0);
  float tz = transform->GetElement(2, 0);
  /*
  float sx = transform->GetElement(0, 1);
  float sy = transform->GetElement(1, 1);
  float sz = transform->GetElement(2, 1);
  */
  float nx = transform->GetElement(0, 2);
  float ny = transform->GetElement(1, 2);
  float nz = transform->GetElement(2, 2);
  float px = transform->GetElement(0, 3);
  float py = transform->GetElement(1, 3);
  float pz = transform->GetElement(2, 3);

  if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Axial") == 0)
    {
    if (this->EnableOblique) // perpendicular
      {
      this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_PERP;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
      }
    else
      {
      this->SliceNode[sliceNodeNumber]->SetOrientationToAxial();
      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }
  else if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Sagittal") == 0)
    {
    if (this->EnableOblique) // In-Plane
      {
      this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_INPLANE;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
      }
    else
      {
      this->SliceNode[sliceNodeNumber]->SetOrientationToSagittal();
      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }
  else if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Coronal") == 0)
    {
    if (this->EnableOblique)  // In-Plane 90
      {
      this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_INPLANE90;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
      }
    else
      {
      this->SliceNode[sliceNodeNumber]->SetOrientationToCoronal();
      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }

  else if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Reformat") == 0)
    {
    if (this->EnableOblique)  // In-Plane 90
      {
      if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_PERP)
        {
        this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE)
        {
        this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE90)
        {
        this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
        }
      }
    else
      {
      if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_PERP)
        {
        this->SliceNode[sliceNodeNumber]->SetOrientationToAxial();
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE)
        {
        this->SliceNode[sliceNodeNumber]->SetOrientationToSagittal();
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE90)
        {
        this->SliceNode[sliceNodeNumber]->SetOrientationToCoronal();
        }

      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }

  this->SliceNode[sliceNodeNumber]->UpdateMatrices();
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateSliceNodeByImage(int sliceNodeNumber)
{

  vtkMRMLVolumeNode* volumeNode =
    vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->RealTimeImageSourceNodeID));

  if (volumeNode == NULL)
    {
    return;
    }

  vtkMatrix4x4* rtimgTransform = vtkMatrix4x4::New();
  //volumeNode->GetRASToIJKMatrix(rtimgTransform);
  volumeNode->GetIJKToRASMatrix(rtimgTransform);
  //rtimgTransform->Invert();
  
  float tx = rtimgTransform->GetElement(0, 0);
  float ty = rtimgTransform->GetElement(1, 0);
  float tz = rtimgTransform->GetElement(2, 0);
  float sx = rtimgTransform->GetElement(0, 1);
  float sy = rtimgTransform->GetElement(1, 1);
  float sz = rtimgTransform->GetElement(2, 1);
  float nx = rtimgTransform->GetElement(0, 2);
  float ny = rtimgTransform->GetElement(1, 2);
  float nz = rtimgTransform->GetElement(2, 2);
  float px = rtimgTransform->GetElement(0, 3);
  float py = rtimgTransform->GetElement(1, 3);
  float pz = rtimgTransform->GetElement(2, 3);

  vtkImageData* imageData;
  imageData = volumeNode->GetImageData();
  int size[3];
  imageData->GetDimensions(size);
  
  // normalize
  float psi = sqrt(tx*tx + ty*ty + tz*tz);
  float psj = sqrt(sx*sx + sy*sy + sz*sz);
  float psk = sqrt(nx*nx + ny*ny + nz*nz);
  float ntx = tx / psi;
  float nty = ty / psi;
  float ntz = tz / psi;
  float nsx = sx / psj;
  float nsy = sy / psj;
  float nsz = sz / psj;
  float nnx = nx / psk;
  float nny = ny / psk;
  float nnz = nz / psk;

  // Shift the center
  // NOTE: The center of the image should be shifted due to different
  // definitions of image origin between VTK (Slicer) and OpenIGTLink;
  // OpenIGTLink image has its origin at the center, while VTK image
  // has one at the corner.

  float hfovi = psi * size[0] / 2.0;
  float hfovj = psj * size[1] / 2.0;
  //float hfovk = psk * imgheader->size[2] / 2.0;
  float hfovk = 0;

  float cx = ntx * hfovi + nsx * hfovj + nnx * hfovk;
  float cy = nty * hfovi + nsy * hfovj + nny * hfovk;
  float cz = ntz * hfovi + nsz * hfovj + nnz * hfovk;

  rtimgTransform->SetElement(0, 0, ntx);
  rtimgTransform->SetElement(1, 0, nty);
  rtimgTransform->SetElement(2, 0, ntz);
  rtimgTransform->SetElement(0, 1, nsx);
  rtimgTransform->SetElement(1, 1, nsy);
  rtimgTransform->SetElement(2, 1, nsz);
  rtimgTransform->SetElement(0, 2, nnx);
  rtimgTransform->SetElement(1, 2, nny);
  rtimgTransform->SetElement(2, 2, nnz);
  rtimgTransform->SetElement(0, 3, px + cx);
  rtimgTransform->SetElement(1, 3, py + cy);
  rtimgTransform->SetElement(2, 3, pz + cz);
  
  UpdateSliceNode(sliceNodeNumber, rtimgTransform);

  rtimgTransform->Delete();
  //volumeNode->Delete();

} 


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::CheckSliceNode()
{
  
  if (this->SliceNode[0] == NULL)
    {
    this->SliceNode[0] = this->GetApplicationLogic()
      ->GetSliceLogic("Red")->GetSliceNode();
    }
  if (this->SliceNode[1] == NULL)
    {
    this->SliceNode[1] = this->GetApplicationLogic()
      ->GetSliceLogic("Yellow")->GetSliceNode();
    }
  if (this->SliceNode[2] == NULL)
    {
    this->SliceNode[2] = this->GetApplicationLogic()
      ->GetSliceLogic("Green")->GetSliceNode();
    }
}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkOpenIGTLinkIFLogic::SetVisibilityOfLocatorModel(const char* nodeName, int v)
{
  vtkMRMLModelNode*   locatorModel;
  vtkMRMLDisplayNode* locatorDisp;

  // Check if any node with the specified name exists
  vtkMRMLScene*  scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(nodeName);

  if (collection != NULL && collection->GetNumberOfItems() == 0)
    {
    // if a node doesn't exist
    locatorModel = AddLocatorModel(nodeName, 0.0, 1.0, 1.0);
    }
  else
    {
    locatorModel = vtkMRMLModelNode::SafeDownCast(collection->GetItemAsObject(0));
    }

  if (locatorModel)
    {
    locatorDisp = locatorModel->GetDisplayNode();
    locatorDisp->SetVisibility(v);
    locatorModel->Modified();
    this->GetApplicationLogic()->GetMRMLScene()->Modified();
    }

  return locatorModel;
}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkOpenIGTLinkIFLogic::AddLocatorModel(const char* nodeName, double r, double g, double b)
{

  vtkMRMLModelNode           *locatorModel;
  vtkMRMLModelDisplayNode    *locatorDisp;

  locatorModel = vtkMRMLModelNode::New();
  locatorDisp = vtkMRMLModelDisplayNode::New();
  
  GetMRMLScene()->SaveStateForUndo();
  GetMRMLScene()->AddNode(locatorDisp);
  GetMRMLScene()->AddNode(locatorModel);  
  
  locatorDisp->SetScene(this->GetMRMLScene());
  
  locatorModel->SetName(nodeName);
  locatorModel->SetScene(this->GetMRMLScene());
  locatorModel->SetAndObserveDisplayNodeID(locatorDisp->GetID());
  locatorModel->SetHideFromEditors(0);
  
  // Cylinder represents the locator stick
  vtkCylinderSource *cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.5);
  cylinder->SetHeight(100);
  cylinder->SetCenter(0, 0, 0);
  cylinder->Update();

  // Rotate cylinder
  vtkTransformPolyDataFilter *tfilter = vtkTransformPolyDataFilter::New();
  vtkTransform* trans =   vtkTransform::New();
  trans->RotateX(90.0);
  trans->Translate(0.0, -50.0, 0.0);
  trans->Update();
  tfilter->SetInput(cylinder->GetOutput());
  tfilter->SetTransform(trans);
  tfilter->Update();
  
  // Sphere represents the locator tip 
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetRadius(3.0);
  sphere->SetCenter(0, 0, 0);
  sphere->Update();
  
  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(sphere->GetOutput());
  //apd->AddInput(cylinder->GetOutput());
  apd->AddInput(tfilter->GetOutput());
  apd->Update();
  
  locatorModel->SetAndObservePolyData(apd->GetOutput());
  
  double color[3];
  color[0] = r;
  color[1] = g;
  color[2] = b;
  locatorDisp->SetPolyData(locatorModel->GetPolyData());
  locatorDisp->SetColor(color);
  
  trans->Delete();
  tfilter->Delete();
  cylinder->Delete();
  sphere->Delete();
  apd->Delete();

  //locatorModel->Delete();
  locatorDisp->Delete();

  return locatorModel;
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::ProcCommand(const char* nodeName, int size, unsigned char* data)
{
}

//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::GetDeviceNamesFromMrml(IGTLMrmlNodeListType &list)
{

  list.clear();

  MessageConverterListType::iterator mcliter;
  for (mcliter = this->MessageConverterList.begin();
       mcliter != this->MessageConverterList.end();
       mcliter ++)
    {
    if ((*mcliter)->GetMRMLName())
      {
      std::string className = this->GetMRMLScene()->GetClassNameByTag((*mcliter)->GetMRMLName());
      std::string deviceTypeName;
      if ((*mcliter)->GetIGTLName() != NULL)
        {
        deviceTypeName = (*mcliter)->GetIGTLName();
        }
      else
        {
        deviceTypeName = (*mcliter)->GetMRMLName();
        }
      std::vector<vtkMRMLNode*> nodes;
      this->GetApplicationLogic()->GetMRMLScene()->GetNodesByClass(className.c_str(), nodes);
      std::vector<vtkMRMLNode*>::iterator iter;
      for (iter = nodes.begin(); iter != nodes.end(); iter ++)
        {
        IGTLMrmlNodeInfoType nodeInfo;
        nodeInfo.name = (*iter)->GetName();
        nodeInfo.type = deviceTypeName.c_str();
        nodeInfo.io   = vtkIGTLConnector::IO_UNSPECIFIED;
        nodeInfo.nodeID = (*iter)->GetID();
        list.push_back(nodeInfo);
        }
      }
    }

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::GetDeviceNamesFromMrml(IGTLMrmlNodeListType &list, const char* mrmlTagName)
{

  list.clear();

  MessageConverterListType::iterator mcliter;
  for (mcliter = this->MessageConverterList.begin();
       mcliter != this->MessageConverterList.end();
       mcliter ++)
    {
    if ((*mcliter)->GetMRMLName() && strcmp(mrmlTagName, (*mcliter)->GetMRMLName()) == 0)
      {
      const char* className = this->GetMRMLScene()->GetClassNameByTag(mrmlTagName);
      const char* deviceTypeName = (*mcliter)->GetIGTLName();
      std::vector<vtkMRMLNode*> nodes;
      this->GetMRMLScene()->GetNodesByClass(className, nodes);
      std::vector<vtkMRMLNode*>::iterator iter;
      for (iter = nodes.begin(); iter != nodes.end(); iter ++)
        {
        IGTLMrmlNodeInfoType nodeInfo;
        nodeInfo.name = (*iter)->GetName();
        nodeInfo.type = deviceTypeName;
        nodeInfo.io   = vtkIGTLConnector::IO_UNSPECIFIED;
        nodeInfo.nodeID = (*iter)->GetID();
        list.push_back(nodeInfo);
        }
      }
    }

}


