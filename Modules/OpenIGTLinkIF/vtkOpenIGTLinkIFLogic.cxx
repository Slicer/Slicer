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

  this->SliceDriver[0] = vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER;
  this->SliceDriver[1] = vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER;
  this->SliceDriver[2] = vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER;

  // If the following code doesn't work, slice nodes should be obtained from application GUI
  this->SliceNode[0] = NULL;
  this->SliceNode[1] = NULL;
  this->SliceNode[2] = NULL;

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
  
  this->SliceOrientation[0] = SLICE_RTIMAGE_PERP;
  this->SliceOrientation[1] = SLICE_RTIMAGE_INPLANE90;
  this->SliceOrientation[2] = SLICE_RTIMAGE_INPLANE;

  this->LastConnectorID = -1;
  this->ConnectorMap.clear();
  this->ConnectorPrevStateList.clear();

  this->IDToMRMLNodeMap.clear();
  this->MessageConverterList.clear();

  //this->OutTransformMsg = igtl::TransformMessage::New();
  //this->OutImageMsg = igtl::ImageMessage::New();


  // register default data types
  this->LinearTransformConverter = vtkIGTLToMRMLLinearTransform::New();
  this->ImageConverter = vtkIGTLToMRMLImage::New();
  RegisterMessageConverter(this->LinearTransformConverter);
  RegisterMessageConverter(this->ImageConverter);

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
    if (strcmp(node->GetNodeTagName(), converter->GetMRMLName()) == 0)
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
  vtkMRMLNode *node = NULL; // TODO: is this OK?
  vtkSetAndObserveMRMLNodeEventsMacro(node,srcNode,nodeEvents);

  // TODO: node should be stored somewhere to stop event monitoring after deleting the MRML node.
  nodeEvents->Delete();
  list->push_back(con);

  return 1;
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::UnRegisterDeviceEvent(vtkIGTLConnector* con, const char* deviceName, const char* deviceType)
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
        if (strcmp(buffer->GetDeviceType(), (*iter)->GetIGTLName()) == 0)
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
              if (strcmp(node->GetNodeTagName(), (*iter)->GetMRMLName()) == 0)
                {
                found = i;
                i = nCol;
                }
              }
            if (found == -1) // if the same type is not found
              {
              node = (*iter)->CreateNewNode(this->GetMRMLScene(), buffer->GetDeviceName());
              }
            }
          (*iter)->IGTLToMRML(buffer, node);
          }
        }

      circBuffer->EndPull();
      }
    }
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
        UnRegisterDeviceEvent(connector, deviceName, deviceType);
        }
      connector->UnregisterDevice(deviceName, deviceType, io);
      }

    /*
    int param;
    if (io == vtkIGTLConnector::IO_INCOMING)         param = vtkIGTLConnector::IO_INCOMING;
    else if (io == vtkIGTLConnector::IO_OUTGOING)    param = vtkIGTLConnector::IO_OUTGOING;
    else if (io == vtkIGTLConnector::IO_UNSPECIFIED) param = vtkIGTLConnector::IO_UNSPECIFIED;
    */
    //
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
    if (strcmp(converter->GetIGTLName(), (*iter)->GetIGTLName()) == 0 &&
        strcmp(converter->GetMRMLName(), (*iter)->GetMRMLName()) == 0)
      {
      found = 1;
      }
    }
  if (found)
    {
    return 0;
    }

  this->MessageConverterList.push_back(converter);
  return 1;
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
        if (strcmp(node->GetNodeTagName(), (*iter)->GetMRMLName()) == 0)
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
    if (strcmp((*iter)->GetIGTLName(), deviceType) == 0)
      {
      converter = *iter;
      break;
      }
    }

  return converter;
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateMRMLScalarVolumeNode(igtl::MessageBase::Pointer ptr)
{
  /*
  px = px + cx;
  py = py + cy;
  pz = pz + cz;
  */

  //volumeNode->SetAndObserveImageData(imageData);

  //----------------------------------------------------------------
  // Slice Orientation
  //----------------------------------------------------------------
  
  /*
  for (int i = 0; i < 3; i ++)
    {
    if (this->SliceDriver[i] == SLICE_DRIVER_RTIMAGE)
      {
      UpdateSliceNode(i, nx, ny, nz, tx, ty, tz, px, py, pz);
      }
    else if (this->SliceDriver[i] == SLICE_DRIVER_LOCATOR)
      {
      UpdateSliceNodeByTransformNode(i, "Tracker");
      }
    }
  */
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateMRMLLinearTransformNode(igtl::MessageBase::Pointer ptr)
{
  /*
  if (strcmp(transMsg->GetDeviceName(), "Tracker") == 0)
    {
    for (int i = 0; i < 3; i ++)
      {
      if (this->SliceDriver[i] == SLICE_DRIVER_LOCATOR)
        {
        UpdateSliceNodeByTransformNode(i, "Tracker");
        }
      }
    }
  */

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateSliceNode(int sliceNodeNumber,
                                         float nx, float ny, float nz,
                                         float tx, float ty, float tz,
                                         float px, float py, float pz)
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

  if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Axial") == 0)
    {
    if (this->EnableOblique) // perpendicular
      {
      this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_PERP;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
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
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
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
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
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
        this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE)
        {
        this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE90)
        {
        this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
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
        this->SliceNode[sliceNodeNumber]->SetOrientationToCoronal();
        }
      else if (this->SliceOrientation[sliceNodeNumber] == SLICE_RTIMAGE_INPLANE90)
        {
        this->SliceNode[sliceNodeNumber]->SetOrientationToSagittal();
        }

      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }

  this->SliceNode[sliceNodeNumber]->UpdateMatrices();
}


//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::UpdateSliceNodeByTransformNode(int sliceNodeNumber, const char* nodeName)
{

  if (this->FreezePlane)
    {
    return 1;
    }

  vtkMRMLLinearTransformNode* transformNode;
  vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(nodeName);

  if (collection != NULL && collection->GetNumberOfItems() == 0)
    {
    // the node name does not exist in the MRML tree
    return 0;
    }

  transformNode = vtkMRMLLinearTransformNode::SafeDownCast(collection->GetItemAsObject(0));

  vtkMatrix4x4* transform;
  //transform = transformNode->GetMatrixTransformToParent();
  transform = transformNode->GetMatrixTransformToParent();

  if (transform)
    {
    // set volume orientation
    float tx = transform->GetElement(0, 0);
    float ty = transform->GetElement(1, 0);
    float tz = transform->GetElement(2, 0);
    //float sx = transform->GetElement(0, 1);
    //float sy = transform->GetElement(1, 1);
    //float sz = transform->GetElement(2, 1);
    float nx = transform->GetElement(0, 2);
    float ny = transform->GetElement(1, 2);
    float nz = transform->GetElement(2, 2);
    float px = transform->GetElement(0, 3);
    float py = transform->GetElement(1, 3);
    float pz = transform->GetElement(2, 3);

    UpdateSliceNode(sliceNodeNumber, nx, ny, nz, tx, ty, tz, px, py, pz);

    }

  return 1;

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::CheckSliceNode()
{

  for (int i = 0; i < 3; i ++)
    {
    if (this->SliceNode[i] == NULL)
      {
      char nodename[36];
      sprintf(nodename, "vtkMRMLSliceNode%d", i+1);
      this->SliceNode[i] = vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodename));
      }
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
  cylinder->SetCenter(0, 50, 0);
  cylinder->Update();
  
  // Sphere represents the locator tip 
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetRadius(3.0);
  sphere->SetCenter(0, 0, 0);
  sphere->Update();
  
  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(sphere->GetOutput());
  apd->AddInput(cylinder->GetOutput());
  apd->Update();
  
  locatorModel->SetAndObservePolyData(apd->GetOutput());
  
  double color[3];
  color[0] = r;
  color[1] = g;
  color[2] = b;
  locatorDisp->SetPolyData(locatorModel->GetPolyData());
  locatorDisp->SetColor(color);
  
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
//void vtkOpenIGTLinkIFLogic::GetDeviceNamesFromMrml(std::vector<char*> &list)
void vtkOpenIGTLinkIFLogic::GetDeviceNamesFromMrml(IGTLMrmlNodeListType &list)
{

  list.clear();

  MessageConverterListType::iterator mcliter;
  for (mcliter = this->MessageConverterList.begin();
       mcliter != this->MessageConverterList.end();
       mcliter ++)
    {
    const char* className = this->GetMRMLScene()->GetClassNameByTag((*mcliter)->GetMRMLName());
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
      list.push_back(nodeInfo);
      }
    }

}




