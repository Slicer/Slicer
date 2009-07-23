/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLIGTLConnectorNode_h
#define __vtkMRMLIGTLConnectorNode_h

#include "vtkOpenIGTLinkIFWin32Header.h"
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include <string>
#include <map>
#include <vector>
#include <set>

#include "vtkObject.h"
#include "vtkOpenIGTLinkIFWin32Header.h" 
#include "igtlServerSocket.h"
#include "igtlClientSocket.h"

class vtkMultiThreader;
class vtkMutexLock;
class vtkIGTLCircularBuffer;

class VTK_OPENIGTLINKIF_EXPORT vtkMRMLIGTLConnectorNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

  //BTX
  // Events
  enum {
    ConnectedEvent        = 118944,
    DisconnectedEvent     = 118945,
    ActivatedEvent        = 118946,
    DeactivatedEvent      = 118947,
    ReceiveEvent          = 118948,
    NewDeviceEvent        = 118949,
  };

  enum {
    TYPE_NOT_DEFINED,
    TYPE_SERVER,
    TYPE_CLIENT,
    NUM_TYPE
  };
  
  enum {
    STATE_OFF,
    STATE_WAIT_CONNECTION,
    STATE_CONNECTED,
    NUM_STATE
  };

  enum {
    IO_UNSPECIFIED = 0x00,
    IO_INCOMING   = 0x01,
    IO_OUTGOING   = 0x02,
  };

  typedef struct {
    std::string   name;
    std::string   type;
    int           io;
    //vtkMRMLNode*  node;
  } DeviceInfoType;

  typedef std::map<int, DeviceInfoType> DeviceInfoMapType;   // Device list:  index is referred as
                                                              // a device id in the connector.
  typedef std::set<int> DeviceIDSetType;
  //ETX

 public:

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLIGTLConnectorNode *New();
  vtkTypeMacro(vtkMRMLIGTLConnectorNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "IGTLConnector";};

  // method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

 protected:
  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLIGTLConnectorNode();
  ~vtkMRMLIGTLConnectorNode();
  vtkMRMLIGTLConnectorNode(const vtkMRMLIGTLConnectorNode&);
  void operator=(const vtkMRMLIGTLConnectorNode&);


 public:
  //----------------------------------------------------------------
  // Connector configuration
  //----------------------------------------------------------------

  vtkGetMacro( ServerPort, int );
  vtkSetMacro( ServerPort, int );
  vtkGetMacro( Type, int );
  vtkSetMacro( Type, int );
  vtkGetMacro( State, int );
  //vtkSetMacro( State, int );
  vtkSetMacro( RestrictDeviceName, int );
  vtkGetMacro( RestrictDeviceName, int );

  // host name
  void SetServerHostname(const char* str) { this->ServerHostname = str; }
  const char* GetServerHostname() { return this->ServerHostname.c_str(); }
  //BTX
  void SetServerHostname(std::string str) { this->ServerHostname = str; }
  //ETX

  int SetTypeServer(int port);
  int SetTypeClient(char* hostname, int port);
  //BTX
  int SetTypeClient(std::string hostname, int port);
  //ETX

  //----------------------------------------------------------------
  // Thread Control
  //----------------------------------------------------------------

  int Start();
  int Stop();
  static void* ThreadFunction(void* ptr);

  //----------------------------------------------------------------
  // OpenIGTLink Message handlers
  //----------------------------------------------------------------
  //BTX
  //igtl::ClientSocket::Pointer WaitForConnection();
  //ETX
  int WaitForConnection();
  int ReceiveController();
  int SendData(int size, unsigned char* data);
  int Skip(int length, int skipFully=1);

  //----------------------------------------------------------------
  // Circular Buffer
  //----------------------------------------------------------------

  //BTX
  typedef std::vector<std::string> NameListType;
  int GetUpdatedBuffersList(NameListType& nameList);
  vtkIGTLCircularBuffer* GetCircularBuffer(std::string& key);
  //ETX

  //----------------------------------------------------------------
  // Device Lists
  //----------------------------------------------------------------

  int GetDeviceID(const char* deviceName, const char* deviceType);
  int RegisterNewDevice(const char* deviceName, const char* deviceType, int io=IO_UNSPECIFIED);
  int UnregisterDevice(const char* deviceName, const char* deviceType, int io=IO_UNSPECIFIED);
  int UnregisterDevice(int id);
  int RegisterDeviceIO(int id, int io);

  //BTX
  DeviceInfoType*     GetDeviceInfo(int id);
  DeviceInfoMapType*  GetDeviceInfoList()    { return &(this->DeviceInfoList);        };
  DeviceIDSetType*    GetIncomingDevice()    { return &(this->IncomingDeviceIDSet);   }
  DeviceIDSetType*    GetOutgoingDevice()    { return &(this->OutgoingDeviceIDSet);   }
  DeviceIDSetType*    GetUnspecifiedDevice() { return &(this->UnspecifiedDeviceIDSet);}
  //ETX

 private:
  //----------------------------------------------------------------
  // Connector configuration 
  //----------------------------------------------------------------
  //BTX
  std::string Name;
  //ETX
  int Type;
  int State;

  //----------------------------------------------------------------
  // Thread and Socket
  //----------------------------------------------------------------

  vtkMultiThreader* Thread;
  vtkMutexLock*     Mutex;
  //BTX
  igtl::ServerSocket::Pointer  ServerSocket;
  igtl::ClientSocket::Pointer  Socket;
  //ETX
  int               ThreadID;
  int               ServerPort;
  int               ServerStopFlag;

  //BTX
  std::string       ServerHostname;
  //ETX

  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------

  //BTX
  typedef std::map<std::string, vtkIGTLCircularBuffer*> CircularBufferMap;
  CircularBufferMap Buffer;
  //ETX

  vtkMutexLock* CircularBufferMutex;
  int     RestrictDeviceName;  // Flag to restrict incoming and outgoing data by device names

  //BTX
  // -- Device Name (same as MRML node) and data type (data type string defined in OpenIGTLink)
  int                LastID;
  DeviceInfoMapType DeviceInfoList;
  DeviceIDSetType   IncomingDeviceIDSet;
  DeviceIDSetType   OutgoingDeviceIDSet;
  DeviceIDSetType   UnspecifiedDeviceIDSet;
  //ETX


  
};

#endif

