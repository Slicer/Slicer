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

  this->ConnectorList.clear();
  this->ConnectorPrevStateList.clear();

  this->EnableOblique = false;
  this->FreezePlane   = false;

  this->Initialized   = 0;
  this->RestrictDeviceName = 0;
  
  this->SliceOrientation[0] = SLICE_RTIMAGE_PERP;
  this->SliceOrientation[1] = SLICE_RTIMAGE_INPLANE90;
  this->SliceOrientation[2] = SLICE_RTIMAGE_INPLANE;

}


//---------------------------------------------------------------------------
vtkOpenIGTLinkIFLogic::~vtkOpenIGTLinkIFLogic()
{

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

#ifdef BRP_DEVELOPMENT

    this->AddServerConnector("Robot", 18945);
    AddDeviceToConnector(GetNumberOfConnectors()-1, "Robot",   "TRANSFORM", DEVICE_IN);

    this->AddServerConnector("Scanner", 18944);
    AddDeviceToConnector(GetNumberOfConnectors()-1, "Robot",   "TRANSFORM", DEVICE_OUT);
    AddDeviceToConnector(GetNumberOfConnectors()-1, "Scanner", "IMAGE", DEVICE_IN);

#endif //BRP_DEVELOPMENT

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

  int nCon = GetNumberOfConnectors();
  int updated = 0;

  for (int i = 0; i < nCon; i ++)
    {
    if (this->ConnectorPrevStateList[i] != this->ConnectorList[i]->GetState())
      {
      updated = 1;
      this->ConnectorPrevStateList[i] = this->ConnectorList[i]->GetState();
      }
    }

  return updated;

}



//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddConnector()
{
  this->AddConnector("connector");
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddConnector(const char* name)
{
  vtkIGTLConnector* connector = vtkIGTLConnector::New();
  connector->SetName(name);
  this->ConnectorList.push_back(connector);
  this->ConnectorPrevStateList.push_back(-1);
  connector->SetRestrictDeviceName(this->RestrictDeviceName);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddServerConnector(const char* name, int port)
{
  vtkIGTLConnector* connector = vtkIGTLConnector::New();
  connector->SetName(name);
  connector->SetType(vtkIGTLConnector::TYPE_SERVER);
  connector->SetServerPort(port);
  this->ConnectorList.push_back(connector);
  this->ConnectorPrevStateList.push_back(-1);
  connector->SetRestrictDeviceName(this->RestrictDeviceName);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::AddClientConnector(const char* name, const char* svrHostName, int port)
{
  vtkIGTLConnector* connector = vtkIGTLConnector::New();
  connector->SetName(name);
  connector->SetType(vtkIGTLConnector::TYPE_CLIENT);
  connector->SetServerPort(port);
  connector->SetServerHostname(svrHostName);
  this->ConnectorList.push_back(connector);
  this->ConnectorPrevStateList.push_back(-1);
  connector->SetRestrictDeviceName(this->RestrictDeviceName);
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::DeleteConnector(int id)
{
  if (id >= 0 && id < this->ConnectorList.size())
    {
    this->ConnectorList[id]->Stop();
    this->ConnectorList[id]->Delete();
    this->ConnectorList.erase(this->ConnectorList.begin() + id);
    this->ConnectorPrevStateList.erase(this->ConnectorPrevStateList.begin() + id);
    }
}

//---------------------------------------------------------------------------
int vtkOpenIGTLinkIFLogic::GetNumberOfConnectors()
{
  return this->ConnectorList.size();
}

vtkIGTLConnector* vtkOpenIGTLinkIFLogic::GetConnector(int id)
{
  if (id >= 0 && id < GetNumberOfConnectors())
    {
    return this->ConnectorList[id];
    }
  else
    {
    return NULL;
    }
}


//---------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkOpenIGTLinkIFLogic::AddVolumeNode(const char* volumeNodeName)
{

  //vtkErrorMacro("AddVolumeNode(): called.");

  vtkMRMLVolumeNode *volumeNode = NULL;

  if (volumeNode == NULL)  // if real-time volume node has not been created
    {

    //vtkMRMLVolumeDisplayNode *displayNode = NULL;
    vtkMRMLScalarVolumeDisplayNode *displayNode = NULL;
    vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
    vtkImageData* image = vtkImageData::New();

    float fov = 256.0;
    image->SetDimensions(256, 256, 1);
    image->SetExtent(0, 255, 0, 255, 0, 0 );
    image->SetSpacing( fov/256, fov/256, 10 );
    image->SetOrigin( -fov/2, -fov/2, -0.0 );
    image->SetScalarTypeToShort();
    image->AllocateScalars();
        
    short* dest = (short*) image->GetScalarPointer();
    if (dest)
      {
      memset(dest, 0x00, 256*256*sizeof(short));
      image->Update();
      }
        
    /*
      vtkSlicerSliceLayerLogic *reslice = vtkSlicerSliceLayerLogic::New();
      reslice->SetUseReslice(0);
    */
    scalarNode->SetAndObserveImageData(image);

        
    /* Based on the code in vtkSlicerVolumeLogic::AddHeaderVolume() */
    //displayNode = vtkMRMLVolumeDisplayNode::New();
    displayNode = vtkMRMLScalarVolumeDisplayNode::New();
    scalarNode->SetLabelMap(0);
    volumeNode = scalarNode;
        
    if (volumeNode != NULL)
      {
      volumeNode->SetName(volumeNodeName);
      this->GetMRMLScene()->SaveStateForUndo();
            
      vtkDebugMacro("Setting scene info");
      volumeNode->SetScene(this->GetMRMLScene());
      displayNode->SetScene(this->GetMRMLScene());
            
            
      double range[2];
      vtkDebugMacro("Set basic display info");
      volumeNode->GetImageData()->GetScalarRange(range);
      range[0] = 0.0;
      range[1] = 256.0;
      displayNode->SetLowerThreshold(range[0]);
      displayNode->SetUpperThreshold(range[1]);
      displayNode->SetWindow(range[1] - range[0]);
      displayNode->SetLevel(0.5 * (range[1] - range[0]) );
            
      vtkDebugMacro("Adding node..");
      this->GetMRMLScene()->AddNode(displayNode);
            
      //displayNode->SetDefaultColorMap();
      vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
      //colorLogic->Delete();
            
      volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
            
      vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
      vtkDebugMacro("Display node "<<displayNode->GetClassName());
            
      this->GetMRMLScene()->AddNode(volumeNode);
      vtkDebugMacro("Node added to scene");
      }

    //scalarNode->Delete();
    /*
      if (displayNode)
      {
      displayNode->Delete();
      }
    */

    }
  return volumeNode;
}


//---------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkOpenIGTLinkIFLogic::AddTransformNode(const char* nodeName)
{
  
  vtkMRMLLinearTransformNode *transformNode = vtkMRMLLinearTransformNode::New();
  transformNode->SetName(nodeName);
  transformNode->SetDescription("Created by Open IGT Link Module");

  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  matrix->Identity();
  transformNode->ApplyTransform(matrix);
  //transformNode->SetAndObserveMatrixTransformToParent(matrix);
  matrix->Delete();
  
  this->GetMRMLScene()->AddNode(transformNode);  

  return transformNode;
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::RegisterDeviceEvent(vtkIGTLConnector* con, const char* deviceName, const char* deviceType)
{

  // check if the device name exists in the MRML tree
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName(deviceName);
  int nItems = collection->GetNumberOfItems();

  vtkMRMLNode* srcNode = NULL;   // Event Source MRML node 

  if (nItems > 0) // if nodes with the same name as the device name are found in the MRML tree
    {
    if (strcmp(deviceType, "TRANSFORM") == 0)
      {
      for (int i = 0; i < nItems; i ++)
        {
        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
        if (strcmp(node->GetNodeTagName(), "LinearTransform") == 0)
          {
          srcNode = node;
          }
        }
      }
    if (strcmp(deviceType, "IMAGE") == 0)
      {
      for (int i = 0; i < nItems; i ++)
        {
        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
        if (strcmp(node->GetNodeTagName(), "Volume") == 0)
          {
          srcNode = node;
          }
        }
      }
    }
  if (nItems == 0 || srcNode == NULL) // if not
    {
    if (strcmp(deviceType, "TRANSFORM") == 0)
      {
      srcNode = AddTransformNode(deviceName);
      }
    else if (strcmp(deviceType, "IMAGE") == 0)
      {
      srcNode = AddVolumeNode(deviceName);
      }
    }
  
  // check if the connector exists in the table

  ConnectorListType* list = &MRMLEventConnectorTable[srcNode];
  ConnectorListType::iterator iter;
  int found = 0;
  for (iter = list->begin(); iter != list->end(); iter ++)
    {
    if (*iter == con)
      {
      found = 1;
      }
    }

  if (!found)  // register as a MRML Node Event
    {
    //std::cerr << "NODE REGISTERED............................." << std::endl;
    vtkIntArray* nodeEvents = vtkIntArray::New();
    //nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
    if (strcmp(deviceType, "TRANSFORM") == 0)
      {
      nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
      }
    else if (strcmp(deviceType, "IMAGE") == 0)
      {
      nodeEvents->InsertNextValue(vtkMRMLVolumeNode::ImageDataModifiedEvent); 
          // NOTE: it hasn't been tested yet.
      }

    vtkMRMLLinearTransformNode *node = NULL;
    vtkSetAndObserveMRMLNodeEventsMacro(node,srcNode,nodeEvents);


    // NOTE: node should be stored somewhere to stop event monitoring after deleting the MRML node.


    nodeEvents->Delete();

    list->push_back(con);
    }

}

//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UnRegisterDeviceEvent(vtkIGTLConnector* con, const char* deviceName, const char* deviceType)
{

  // check if the device name exists in the MRML tree
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName(deviceName);
  int nItems = collection->GetNumberOfItems();

  vtkMRMLNode* srcNode = NULL;   // Event Source MRML node 

  if (nItems > 0) // if nodes with the same name as the device name are found in the MRML tree
    {
    if (strcmp(deviceType, "TRANSFORM") == 0)
      {
      for (int i = 0; i < nItems; i ++)
        {
        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
        if (strcmp(node->GetNodeTagName(), "LinearTransform") == 0)
          {
          srcNode = node;
          }
        }
      }
    if (strcmp(deviceType, "IMAGE") == 0)
      {
      for (int i = 0; i < nItems; i ++)
        {
        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
        if (strcmp(node->GetNodeTagName(), "Volume") == 0)
          {
          srcNode = node;
          }
        }
      }
    }

  if (nItems == 0 || srcNode == NULL) // not found
    {
    return;
    }

  this->MRMLObserverManager->RemoveObjectEvents(srcNode);

}



//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::ImportFromCircularBuffers()
{
  ConnectorListType::iterator iter;

  for (iter = this->ConnectorList.begin(); iter != this->ConnectorList.end(); iter ++)
    {
    vtkIGTLConnector::NameListType nameList;
    (*iter)->GetUpdatedBuffersList(nameList);
    vtkIGTLConnector::NameListType::iterator nameIter;
    for (nameIter = nameList.begin(); nameIter != nameList.end(); nameIter ++)
      {
      //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ImportFromCircularBuffers(): Import Image from : " << *nameIter);
      vtkIGTLCircularBuffer* circBuffer = (*iter)->GetCircularBuffer(*nameIter);
      circBuffer->StartPull();
      
      igtl::MessageBase::Pointer buffer = circBuffer->GetPullBuffer();

      //const char* type = circBuffer->PullDeviceType();
      //vtkErrorMacro("vtkOpenIGTLinkIFLogic::ImportFromCircularBuffers(): TYPE = " << type);
      if (strcmp(buffer->GetDeviceType(), "IMAGE") == 0)
        {
        UpdateMRMLScalarVolumeNode(buffer);
        //UpdateMRMLScalarVolumeNode((*nameIter).c_str(), buffer->PullSize(), buffer->PullData());
        }
      else if (strcmp(buffer->GetDeviceType(), "TRANSFORM") == 0)
        {
        UpdateMRMLLinearTransformNode(buffer);
        //UpdateMRMLLinearTransformNode((*nameIter).c_str(), buffer->PullSize(), buffer->PullData());
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

  ConnectorListType::iterator iter;
  for (iter = this->ConnectorList.begin(); iter != this->ConnectorList.end(); iter ++)
    {
    (*iter)->SetRestrictDeviceName(f);
    }

  return this->RestrictDeviceName;
}


//---------------------------------------------------------------------------
int  vtkOpenIGTLinkIFLogic::AddDeviceToConnector(int id, const char* deviceName, const char* deviceType, int io)
// io -- DEVICE_IN : incoming, DEVICE_OUT: outgoing
{

  vtkIGTLConnector* connector = GetConnector(id);

  if (connector)
    {
    vtkIGTLConnector::DeviceNameList* devList;

    if (io == DEVICE_IN)             // incoming
      {
      devList = connector->GetIncomingDeviceList();
      }
    else if (io == DEVICE_OUT)       // outgoing
      {
      devList = connector->GetOutgoingDeviceList();
      }
    else //if (io == DEVICE_UNSPEC)  // unspecified
      {
      devList = connector->GetUnspecifiedDeviceList();
      }

    if ((*devList)[std::string(deviceName)] != deviceType)
      {
      (*devList)[std::string(deviceName)] = std::string(deviceType);
      if (io == DEVICE_OUT)
        {
        //std::cerr << "registering device : " << deviceName << ", " << deviceType << std::endl;
        RegisterDeviceEvent(connector,deviceName, deviceType);
        }
      return 1;
      }
    else
      {
      return 0;
      }
    }

  return 1;

}

//---------------------------------------------------------------------------
int  vtkOpenIGTLinkIFLogic::DeleteDeviceFromConnector(int id, const char* deviceName, const char* deviceType, int io)
{
  vtkIGTLConnector* connector = GetConnector(id);

  if (connector)
    {
    vtkIGTLConnector::DeviceNameList* devList;
    
    if (io == DEVICE_IN)             // incoming
      {
      devList = connector->GetIncomingDeviceList();
      }
    else if (io == DEVICE_OUT)       // outgoing
      {
      devList = connector->GetOutgoingDeviceList();
      }
    else //if (io == DEVICE_UNSPEC)  // unspecified
      {
      devList = connector->GetUnspecifiedDeviceList();
      }

    // find the device name from the list
    vtkIGTLConnector::DeviceNameList::iterator iter; 
    iter = devList->find(std::string(deviceName));
    if (iter != devList->end()) // TODO: check device type here 
      {
      UnRegisterDeviceEvent(connector, deviceName, deviceType);
      devList->erase(iter);
      }
    }

  return 1;

}

//---------------------------------------------------------------------------
int  vtkOpenIGTLinkIFLogic::SetDeviceType(int id, const char* deviceName, const char* deviceType, int io)
{
  vtkIGTLConnector* connector = GetConnector(id);

  if (connector)
    {
    vtkIGTLConnector::DeviceNameList* devList;
    
    if (io == DEVICE_IN)             // incoming
      {
      devList = connector->GetIncomingDeviceList();
      }
    else if (io == DEVICE_OUT)       // outgoing
      {
      devList = connector->GetOutgoingDeviceList();
      }
    else //if (io == DEVICE_UNSPEC)  // unspecified
      {
      devList = connector->GetUnspecifiedDeviceList();
      }

    // find the device name from the list
    vtkIGTLConnector::DeviceNameList::iterator iter; 
    iter = devList->find(std::string(deviceName));
    if (iter != devList->end()) // TODO: check device type here 
      {
      const char* origDeviceType = iter->second.c_str();
      if (io == DEVICE_OUT)
        {
        UnRegisterDeviceEvent(connector, deviceName, origDeviceType); 
        }
      
      iter->second = std::string(deviceType);

      if (io == DEVICE_OUT)
        {
        RegisterDeviceEvent(connector, deviceName, deviceType);
        }
      }
    }

  return 1;

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData)
{
  //std::cerr << "void vtkOpenIGTLinkIFLogic::ProcessMRMLEvents() is called" << std::endl;

  if (caller != NULL)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
    ConnectorListType* list = &MRMLEventConnectorTable[node];
    ConnectorListType::iterator iter;

    for (iter = list->begin(); iter != list->end(); iter ++)
      {
      vtkIGTLConnector* connector = *iter;

      //std::cerr << "void vtkOpenIGTLinkIFLogic::ProcessMRMLEvents() Connector: "
      //<< connector->GetName() << std::endl;

      // NOTE: should add more strict device name and device type check here.
      
      if (strcmp(node->GetNodeTagName(), "LinearTransform") == 0)
        {
        vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
        vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();

        igtl::TransformMessage::Pointer transMsg;
        transMsg = igtl::TransformMessage::New();
        transMsg->SetDeviceName(node->GetName());

        igtl::Matrix4x4 igtlmatrix;

        igtlmatrix[0][0]  = matrix->GetElement(0, 0);
        igtlmatrix[1][0]  = matrix->GetElement(1, 0);
        igtlmatrix[2][0]  = matrix->GetElement(2, 0);
        igtlmatrix[3][0]  = matrix->GetElement(3, 0);

        igtlmatrix[0][1]  = matrix->GetElement(0, 1);
        igtlmatrix[1][1]  = matrix->GetElement(1, 1);
        igtlmatrix[2][1]  = matrix->GetElement(2, 1);
        igtlmatrix[3][1]  = matrix->GetElement(3, 1);

        igtlmatrix[0][2]  = matrix->GetElement(0, 2);
        igtlmatrix[1][2]  = matrix->GetElement(1, 2);
        igtlmatrix[2][2]  = matrix->GetElement(2, 2);
        igtlmatrix[3][2]  = matrix->GetElement(3, 2);

        igtlmatrix[0][3]  = matrix->GetElement(0, 3);
        igtlmatrix[1][3]  = matrix->GetElement(1, 3);
        igtlmatrix[2][3]  = matrix->GetElement(2, 3);
        igtlmatrix[3][3]  = matrix->GetElement(3, 3);

        transMsg->SetMatrix(igtlmatrix);
        transMsg->Pack();

        /*
        // build message body
        igtl_float32 transform[12];

        transform[0]  = matrix->GetElement(0, 0);
        transform[1]  = matrix->GetElement(1, 0);
        transform[2]  = matrix->GetElement(2, 0);
        transform[3]  = matrix->GetElement(0, 1);
        transform[4]  = matrix->GetElement(1, 1);
        transform[5]  = matrix->GetElement(2, 1);
        transform[6]  = matrix->GetElement(0, 2);
        transform[7]  = matrix->GetElement(1, 2);
        transform[8]  = matrix->GetElement(2, 2);
        transform[9]  = matrix->GetElement(0, 3);
        transform[10] = matrix->GetElement(1, 3);
        transform[11] = matrix->GetElement(2, 3);

        // build header
        igtl_header header;
        igtl_uint64 crc = crc64(0, 0, 0LL); // initial crc

        header.version   = IGTL_HEADER_VERSION;
        header.timestamp = 0;
        header.body_size = IGTL_TRANSFORM_SIZE;
        header.crc       = crc64((unsigned char*)transform, IGTL_TRANSFORM_SIZE, crc);
        strncpy(header.name, "TRANSFORM", 12);
        strncpy(header.device_name, node->GetName(), 20);

        igtl_transform_convert_byte_order(transform);
        igtl_header_convert_byte_order(&header);
        */

        int r; 
        r = connector->SendData(transMsg->GetPackSize(), (unsigned char*)transMsg->GetPackPointer());
        /*
        r = connector->SendData(IGTL_HEADER_SIZE, (unsigned char*) &header);
        r = connector->SendData(IGTL_TRANSFORM_SIZE, (unsigned char*) transform);
        */
        
        }
      else if (strcmp(node->GetNodeTagName(), "Volume") == 0)
        {
        //igtl_header header;
        
        //connector->SendData();
        }
      }
    }

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateMRMLScalarVolumeNode(igtl::MessageBase::Pointer ptr)
{
  //vtkErrorMacro("vtkIGTLConnector::UpdateMRMLScalarVolumeNode()  is called  ");

  // Create a message buffer to receive transform data
  igtl::ImageMessage::Pointer imgMsg;
  imgMsg = igtl::ImageMessage::New();
  imgMsg->Copy(ptr);

  // Deserialize the transform data
  // If you want to skip CRC check, call Unpack() without argument.
  int c = imgMsg->Unpack(1);

  if (c & igtl::MessageHeader::UNPACK_BODY == 0) // if CRC check fails
    {
    return;
    }

  // Retrive the image data
  int   size[3];          // image dimension
  float spacing[3];       // spacing (mm/pixel)
  int   svsize[3];        // sub-volume size
  int   svoffset[3];      // sub-volume offset
  int   scalarType;       // scalar type
  igtl::Matrix4x4 matrix; // Image origin and orientation matrix

  scalarType = imgMsg->GetScalarType();
  imgMsg->GetDimensions(size);
  imgMsg->GetSpacing(spacing);
  imgMsg->GetSubVolume(svsize, svoffset);
  imgMsg->GetMatrix(matrix);

  float tx = matrix[0][0];
  float ty = matrix[1][0];
  float tz = matrix[2][0];
  float sx = matrix[0][1];
  float sy = matrix[1][1];
  float sz = matrix[2][1];
  float nx = matrix[0][2];
  float ny = matrix[1][2];
  float nz = matrix[2][2];
  float px = matrix[0][3];
  float py = matrix[1][3];
  float pz = matrix[2][3];

  /*
  vtkErrorMacro("matrix = ");
  vtkErrorMacro( << tx << ", " << ty << ", " << tz);
  vtkErrorMacro( << sx << ", " << sy << ", " << sz);
  vtkErrorMacro( << nx << ", " << ny << ", " << nz);
  vtkErrorMacro( << px << ", " << py << ", " << pz);
  */
  
  vtkMRMLScalarVolumeNode* volumeNode;
  vtkImageData* imageData;
  
  vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(imgMsg->GetDeviceName());
  
  if (collection->GetNumberOfItems() == 0)
    {
    volumeNode = vtkMRMLScalarVolumeNode::New();
    volumeNode->SetName(imgMsg->GetDeviceName());
    volumeNode->SetDescription("Received by OpenIGTLink");
    
    imageData = vtkImageData::New();
    
    imageData->SetDimensions(size[0], size[1], size[2]);
    imageData->SetNumberOfScalarComponents(1);
    
    // Scalar type
    //  TBD: Long might not be 32-bit in some platform.
    switch (scalarType)
      {
      case igtl::ImageMessage::TYPE_INT8:
        imageData->SetScalarTypeToChar();
        break;
      case igtl::ImageMessage::TYPE_UINT8:
        imageData->SetScalarTypeToUnsignedChar();
        break;
      case igtl::ImageMessage::TYPE_INT16:
        imageData->SetScalarTypeToShort();
        break;
      case igtl::ImageMessage::TYPE_UINT16:
        imageData->SetScalarTypeToUnsignedShort();
        break;
      case igtl::ImageMessage::TYPE_INT32:
        imageData->SetScalarTypeToUnsignedLong();
        break;
      case igtl::ImageMessage::TYPE_UINT32:
        imageData->SetScalarTypeToUnsignedLong();
        break;
      case igtl::ImageMessage::TYPE_FLOAT32:
        imageData->SetScalarTypeToFloat();
        break;
      case igtl::ImageMessage::TYPE_FLOAT64:
        imageData->SetScalarTypeToDouble();
        break;
      default:
        //vtkErrorMacro ("Invalid Scalar Type\n");
        break;
    }

    imageData->AllocateScalars();
    volumeNode->SetAndObserveImageData(imageData);
    imageData->Delete();
    
    scene->AddNode(volumeNode);
    this->GetApplicationLogic()->GetSelectionNode()->SetReferenceActiveVolumeID(volumeNode->GetID());
    this->GetApplicationLogic()->PropagateVolumeSelection();
    
    }
  else
    {
    vtkCollection* collection = scene->GetNodesByName(imgMsg->GetDeviceName());
    volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(collection->GetItemAsObject(0));
    }
  
  // Get vtk image from MRML node
  imageData = volumeNode->GetImageData();


  // TODO:
  // It should be checked here if the dimension of vtkImageData
  // and arrived data is same.

  //vtkErrorMacro("IGTL image size = " << bytes);
  if (imgMsg->GetImageSize() == imgMsg->GetSubVolumeImageSize())
    {
    // In case that volume size == sub-volume size,
    // image is read directly to the memory area of vtkImageData
    // for better performance. 
    memcpy(imageData->GetScalarPointer(),
           imgMsg->GetScalarPointer(), imgMsg->GetSubVolumeImageSize());
    }
  else
    {
    // In case of volume size != sub-volume size,
    // image is loaded into ImageReadBuffer, then copied to
    // the memory area of vtkImageData.
    
    // Check scalar size
    int scalarSize = imgMsg->GetScalarSize();
    
    char* imgPtr = (char*) imageData->GetScalarPointer();
    char* bufPtr = (char*) imgMsg->GetScalarPointer();
    int sizei = size[0];
    int sizej = size[1];
    int sizek = size[2];
    int subsizei = svsize[0];
    
    int bg_i = svoffset[0];
    int ed_i = bg_i + svsize[0];
    int bg_j = svoffset[1];
    int ed_j = bg_j + svsize[1];
    int bg_k = svoffset[2];
    int ed_k = bg_k + svsize[2];
    
    for (int k = bg_k; k < ed_k; k ++)
      {
      for (int j = bg_j; j < ed_j; j ++)
        {
        memcpy(&imgPtr[(sizei*sizej*k + sizei*j + bg_i)*scalarSize],
               bufPtr, subsizei*scalarSize);
        bufPtr += subsizei*scalarSize;
        }
      }
    }
  
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
  
  float hfovi = psi * size[0] / 2.0;
  float hfovj = psj * size[1] / 2.0;
  //float hfovk = psk * imgheader->size[2] / 2.0;
  float hfovk = 0;

  float cx = ntx * hfovi + nsx * hfovj + nnx * hfovk;
  float cy = nty * hfovi + nsy * hfovj + nny * hfovk;
  float cz = ntz * hfovi + nsz * hfovj + nnz * hfovk;

  px = px - cx;
  py = py - cy;
  pz = pz - cz;


  // set volume orientation
  vtkMatrix4x4* rtimgTransform = vtkMatrix4x4::New();
  rtimgTransform->Identity();
  rtimgTransform->SetElement(0, 0, tx);
  rtimgTransform->SetElement(1, 0, ty);
  rtimgTransform->SetElement(2, 0, tz);
  
  rtimgTransform->SetElement(0, 1, sx);
  rtimgTransform->SetElement(1, 1, sy);
  rtimgTransform->SetElement(2, 1, sz);
  
  rtimgTransform->SetElement(0, 2, nx);
  rtimgTransform->SetElement(1, 2, ny);
  rtimgTransform->SetElement(2, 2, nz);

  rtimgTransform->SetElement(0, 3, px);
  rtimgTransform->SetElement(1, 3, py);
  rtimgTransform->SetElement(2, 3, pz);


  rtimgTransform->Invert();
  volumeNode->SetRASToIJKMatrix(rtimgTransform);
  volumeNode->Modified();

//  if (lps) { // LPS coordinate
//    vtkMatrix4x4* lpsToRas = vtkMatrix4x4::New();
//    lpsToRas->Identity();
//    lpsToRas->SetElement(-1, 0,  0);
//    lpsToRas->SetElement(0, -1,  0);
//    lpsToRas->SetElement(0,  0,  1);
//    lpsToRas->Multiply4x4(lpsToRas, rtimgTransform, rtimgTransform);
//    lpsToRas->Delete();
//  }

  px = px + cx;
  py = py + cy;
  pz = pz + cz;

  //volumeNode->SetAndObserveImageData(imageData);

  //----------------------------------------------------------------
  // Slice Orientation
  //----------------------------------------------------------------

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
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::UpdateMRMLLinearTransformNode(igtl::MessageBase::Pointer ptr)
{
  //vtkErrorMacro("vtkIGTLConnector::UpdateMRMLLinearTransformNode()  is called  ");

  // Create a message buffer to receive transform data
  igtl::TransformMessage::Pointer transMsg;
  transMsg = igtl::TransformMessage::New();

  transMsg->Copy(ptr);  // !! TODO: copy makes performance issue.

  // Deserialize the transform data
  // If you want to skip CRC check, call Unpack() without argument.
  int c = transMsg->Unpack(1);

  vtkMRMLLinearTransformNode* transformNode;
  vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(transMsg->GetDeviceName());

  if (collection->GetNumberOfItems() == 0)
    {
    transformNode = vtkMRMLLinearTransformNode::New();
    transformNode->SetName(transMsg->GetDeviceName());
    transformNode->SetDescription("Received by OpenIGTLink");
    
    vtkMatrix4x4* transform = vtkMatrix4x4::New();
    transform->Identity();
    //transformNode->SetAndObserveImageData(transform);
    transformNode->ApplyTransform(transform);
    transform->Delete();
    
    scene->AddNode(transformNode);
    }
  else
    {
    vtkCollection* collection = scene->GetNodesByName(transMsg->GetDeviceName());
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(collection->GetItemAsObject(0));
    }
  
  igtl::Matrix4x4 matrix;
  transMsg->GetMatrix(matrix);

  float tx = matrix[0][0];
  float ty = matrix[1][0];
  float tz = matrix[2][0];
  float sx = matrix[0][1];
  float sy = matrix[1][1];
  float sz = matrix[2][1];
  float nx = matrix[0][2];
  float ny = matrix[1][2];
  float nz = matrix[2][2];
  float px = matrix[0][3];
  float py = matrix[1][3];
  float pz = matrix[2][3];

  //vtkErrorMacro("matrix = ");
  //vtkErrorMacro( << tx << ", " << ty << ", " << tz);
  //vtkErrorMacro( << sx << ", " << sy << ", " << sz);
  //vtkErrorMacro( << nx << ", " << ny << ", " << nz);
  //vtkErrorMacro( << px << ", " << py << ", " << pz);
  
  // set volume orientation
  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();

  transform->Identity();
  transform->SetElement(0, 0, tx);
  transform->SetElement(1, 0, ty);
  transform->SetElement(2, 0, tz);

  transform->SetElement(0, 1, sx);
  transform->SetElement(1, 1, sy);
  transform->SetElement(2, 1, sz);

  transform->SetElement(0, 2, nx);
  transform->SetElement(1, 2, ny);
  transform->SetElement(2, 2, nz);

  transform->SetElement(0, 3, px);
  transform->SetElement(1, 3, py);
  transform->SetElement(2, 3, pz);

  transformToParent->DeepCopy(transform);
  transform->Delete();

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
    float sx = transform->GetElement(0, 1);
    float sy = transform->GetElement(1, 1);
    float sz = transform->GetElement(2, 1);
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

  char* deviceTypeNames[] = {
    "TRANSFORM",
    "IMAGE",
  };

  char* classNames[] = 
    {
      "vtkMRMLLinearTransformNode",
      "vtkMRMLVolumeNode"
    };

  for (int i = 0; i < 2; i ++)
    {
    std::vector<vtkMRMLNode*> nodes;
    this->GetMRMLScene()->GetNodesByClass(classNames[i], nodes);
    std::vector<vtkMRMLNode*>::iterator iter;
    for (iter = nodes.begin(); iter != nodes.end(); iter ++)
      {
      IGTLMrmlNodeInfoType nodeInfo;
      nodeInfo.name = (*iter)->GetName();
      nodeInfo.type = deviceTypeNames[i];
      nodeInfo.io   = DEVICE_UNSPEC;
      list.push_back(nodeInfo);
      }
    }
  
}

/*
//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFLogic::GetDeviceTypes(std::vector<char*> &list)
{
  char* deviceTypes[] = {
    "TRANSFORM",
    "IMAGE",
  };

  list.clear();
  
  for (int i = 0; i < 2; i ++)
    {
    list.push_back(deviceTypes[i]);
    }
}

*/





