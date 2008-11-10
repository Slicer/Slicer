/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

// .NAME vtkOpenIGTLinkIFLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkOpenIGTLinkIFLogic_h
#define __vtkOpenIGTLinkIFLogic_h

#include <vector>

#include "vtkOpenIGTLinkIFWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMultiThreader.h"

#include "igtlImageMessage.h"
#include "igtlTransformMessage.h"

// switch to activate testing code for development  -- J.T. 06.17.2008
//#define BRP_DEVELOPMENT      1

class vtkIGTLConnector;


class VTK_OPENIGTLINKIF_EXPORT vtkOpenIGTLinkIFLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
  enum WorkPhase {
    StartUp = 0,
    Planning,
    Calibration,
    Targeting,
    Manual,
    Emergency,
    NumPhases,
  };

  enum {
    SLICE_DRIVER_USER    = 0,
    SLICE_DRIVER_LOCATOR = 1,
    SLICE_DRIVER_RTIMAGE = 2
  };

  enum ImageOrient{
    SLICE_RTIMAGE_NONE      = 0,
    SLICE_RTIMAGE_PERP      = 1,
    SLICE_RTIMAGE_INPLANE90 = 2,
    SLICE_RTIMAGE_INPLANE   = 3
  };

  enum {  // Events
    LocatorUpdateEvent      = 50000,
    StatusUpdateEvent       = 50001,
    //SliceUpdateEvent        = 50002,
  };

  enum {    // Device IO
    DEVICE_UNSPEC           = 0,  // unspecified
    DEVICE_IN               = 1,  // incoming
    DEVICE_OUT              = 2   // outgoing
  };

  //ETX
  
  // Work phase keywords used in NaviTrack (defined in BRPTPRInterface.h)

 public:
  
  static vtkOpenIGTLinkIFLogic *New();
  
  vtkTypeRevisionMacro(vtkOpenIGTLinkIFLogic,vtkObject);

  vtkSetMacro ( NeedRealtimeImageUpdate0, int );
  vtkGetMacro ( NeedRealtimeImageUpdate0, int );
  vtkSetMacro ( NeedRealtimeImageUpdate1, int );
  vtkGetMacro ( NeedRealtimeImageUpdate1, int );
  vtkSetMacro ( NeedRealtimeImageUpdate2, int );
  vtkGetMacro ( NeedRealtimeImageUpdate2, int );

  vtkSetMacro ( NeedUpdateLocator,       bool );
  vtkGetMacro ( NeedUpdateLocator,       bool );

  void SetSliceDriver0(int v) { this->SliceDriver[0] = v; };
  void SetSliceDriver1(int v) { this->SliceDriver[1] = v; };
  void SetSliceDriver2(int v) { this->SliceDriver[2] = v; };
  int  SetSliceDriver0() { return this->SliceDriver[0]; };
  int  SetSliceDriver1() { return this->SliceDriver[1]; };
  int  SetSliceDriver2() { return this->SliceDriver[2]; };

  vtkGetMacro ( Connection,              bool );

  vtkSetMacro ( EnableOblique,           bool );
  vtkGetMacro ( EnableOblique,           bool );
  vtkSetMacro ( FreezePlane,             bool );
  vtkGetMacro ( FreezePlane,              bool );

  vtkGetObjectMacro ( LocatorTransform, vtkTransform );
  vtkGetObjectMacro ( LocatorMatrix,    vtkMatrix4x4 );

  void PrintSelf(ostream&, vtkIndent);
  //void AddRealtimeVolumeNode(const char* name);

  //----------------------------------------------------------------
  // Start up the class
  //----------------------------------------------------------------

  int Initialize();

  //----------------------------------------------------------------
  // Connector Management
  //----------------------------------------------------------------

  // Add connector
  void AddConnector();
  void AddConnector(const char* name);
  void AddServerConnector(const char* name, int port);
  void AddClientConnector(const char* name, const char* svrHostName, int port);

  // Delete connector
  void DeleteConnector(int id);

  int  GetNumberOfConnectors();
  vtkIGTLConnector* GetConnector(int id);
  int  CheckConnectorsStatusUpdates();
  //int  ReadCircularBuffers();

  void ImportFromCircularBuffers();

  // Device Name management          // io -- 0: unspecified, 1: incoming, 2: outgoing
  int  SetRestrictDeviceName(int f);
  int  AddDeviceToConnector(int id, const char* deviceName, const char* deviceType, int io);
  int  DeleteDeviceFromConnector(int id, const char* deviceName, const char* deviceType, int io);
  int  SetDeviceType(int id, const char* deviceName, const char* deviceType, int io);


  //----------------------------------------------------------------
  // MRML Management
  //----------------------------------------------------------------

  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData);

  //BTX
  void UpdateMRMLScalarVolumeNode(igtl::MessageBase::Pointer ptr);
  void UpdateMRMLLinearTransformNode(igtl::MessageBase::Pointer ptr);
  //ETX
  
  void UpdateSliceNode(int sliceNodeNumber,
                       float nx, float ny, float nz,
                       float tx, float ty, float tz,
                       float px, float py, float pz);
  int  UpdateSliceNodeByTransformNode(int sliceNodeNumber, const char* nodeName);
  void CheckSliceNode();

  vtkMRMLModelNode* SetVisibilityOfLocatorModel(const char* nodeName, int v);
  vtkMRMLModelNode* AddLocatorModel(const char* nodeName, double r, double g, double b);

  void ProcCommand(const char* nodeName, int size, unsigned char* data);

  //BTX
  typedef struct {
    std::string name;
    std::string type;
    int io;
  } IGTLMrmlNodeInfoType;

  typedef std::vector<IGTLMrmlNodeInfoType> IGTLMrmlNodeListType;
  
  void GetDeviceNamesFromMrml(IGTLMrmlNodeListType &list);
  //void GetDeviceTypes(std::vector<char*> &list);
  //ETX

 protected:
  
  //----------------------------------------------------------------
  // Constructor, destructor etc.
  //----------------------------------------------------------------

  vtkOpenIGTLinkIFLogic();
  ~vtkOpenIGTLinkIFLogic();
  vtkOpenIGTLinkIFLogic(const vtkOpenIGTLinkIFLogic&);
  void operator=(const vtkOpenIGTLinkIFLogic&);

  
  static void DataCallback(vtkObject*, unsigned long, void *, void *);

  void UpdateAll();
  void UpdateSliceDisplay();
  void UpdateLocator();

  vtkMRMLVolumeNode*          AddVolumeNode(const char*);
  vtkMRMLLinearTransformNode* AddTransformNode(const char*);
  void                        RegisterDeviceEvent(vtkIGTLConnector* con,
                                                  const char* deviceName,
                                                  const char* deviceType);
  void                        UnRegisterDeviceEvent(vtkIGTLConnector* con,
                                                    const char* deviceName,
                                                    const char* deviceType);
  vtkCallbackCommand *DataCallbackCommand;


 private:

  int Initialized;

  //----------------------------------------------------------------
  // Connector Management
  //----------------------------------------------------------------

  //BTX
  typedef std::vector<vtkIGTLConnector*> ConnectorListType;
  ConnectorListType              ConnectorList;
  std::vector<int>               ConnectorPrevStateList;

  typedef std::map<vtkMRMLNode*, ConnectorListType> MRMLNodeAndConnectorTable;
  MRMLNodeAndConnectorTable      MRMLEventConnectorTable;

  //ETX

  int RestrictDeviceName;

  //----------------------------------------------------------------
  // Monitor Timer
  //----------------------------------------------------------------

  int MonitorFlag;
  int MonitorInterval;
  

  //----------------------------------------------------------------
  // Real-time image
  //----------------------------------------------------------------
  
  vtkMRMLVolumeNode     *RealtimeVolumeNode;

  int   NeedRealtimeImageUpdate0;
  int   NeedRealtimeImageUpdate1;
  int   NeedRealtimeImageUpdate2;

  vtkMRMLSliceNode *SliceNode[3];

  int   SliceDriver[3];
  
  bool  ImagingControl;
  bool  NeedUpdateLocator;

  bool  EnableOblique;
  bool  FreezePlane;

  long  RealtimeImageTimeStamp;
  //int   RealtimeImageSerial;
  //int   RealtimeImageOrient;

  int   SliceOrientation[3];
  


  //----------------------------------------------------------------
  // Locator
  //----------------------------------------------------------------

  // What's a difference between LocatorMatrix and Locator Transform???
  vtkMatrix4x4*         LocatorMatrix;
  vtkTransform*         LocatorTransform;

  bool  Connection;  
  
};

#endif


  
