/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLIGTLConnectorNode_h
#define __vtkMRMLIGTLConnectorNode_h

#include "vtkBrainlabModuleWin32Header.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkIGTLToMRMLBase.h"

#include <string>
#include <map>
#include <vector>
#include <set>

#include "vtkObject.h"
#include "igtlServerSocket.h"
#include "igtlClientSocket.h"

class vtkMultiThreader;
class vtkMutexLock;
class vtkIGTLCircularBuffer;

class VTK_BRAINLABMODULE_EXPORT vtkMRMLIGTLConnectorNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

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

  typedef std::map<int, DeviceInfoType>   DeviceInfoMapType;   // Device list:  index is referred as
                                                               // a device id in the connector.
  typedef std::set<int>                   DeviceIDSetType;
  typedef std::list<vtkIGTLToMRMLBase*>   MessageConverterListType;
  typedef std::vector<vtkMRMLNode*>       MRMLNodeListType;
  typedef std::map<std::string, vtkIGTLToMRMLBase*> MessageConverterMapType;

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
  void SetServerHostname(std::string str) { this->ServerHostname = str; }

  int SetTypeServer(int port);
  int SetTypeClient(char* hostname, int port);
  int SetTypeClient(std::string hostname, int port);

  vtkGetMacro( CheckCRC, int );
  void SetCheckCRC(int c);

  
  //----------------------------------------------------------------
  // Thread Control
  //----------------------------------------------------------------

  int Start();
  int Stop();
  static void* ThreadFunction(void* ptr);

  //----------------------------------------------------------------
  // OpenIGTLink Message handlers
  //----------------------------------------------------------------
  int WaitForConnection();
  int ReceiveController();
  int SendData(int size, unsigned char* data);
  int Skip(int length, int skipFully=1);

  //----------------------------------------------------------------
  // Circular Buffer
  //----------------------------------------------------------------

  typedef std::vector<std::string> NameListType;
  unsigned int GetUpdatedBuffersList(NameListType& nameList); // TODO: this will be moved to private
  vtkIGTLCircularBuffer* GetCircularBuffer(std::string& key);     // TODO: Is it OK to use device name as a key?

  //----------------------------------------------------------------
  // Device Lists
  //----------------------------------------------------------------

  // Description:
  // Import received data from the circular buffer to the MRML scne.
  // This is currently called by vtkOpenIGTLinkIFLogic class.
  void ImportDataFromCircularBuffer();

  // Description:
  // Import events from the event buffer to the MRML scene.
  // This is currently called by vtkOpenIGTLinkIFLogic class.
  void ImportEventsFromEventBuffer();

  // Description:
  // Register IGTL to MRML message converter.
  // This is used by vtkOpenIGTLinkIFLogic class.
  int RegisterMessageConverter(vtkIGTLToMRMLBase* converter);

  // Description:
  // Unregister IGTL to MRML message converter.
  // This is used by vtkOpenIGTLinkIFLogic class.
  void UnregisterMessageConverter(vtkIGTLToMRMLBase* converter);
  
  // Description:
  // Set and start observing MRML node for outgoing data.
  int RegisterOutgoingMRMLNode(vtkMRMLNode* node);

  // Description:
  // Stop observing and remove MRML node for outgoing data.
  void UnregisterOutgoingMRMLNode(vtkMRMLNode* node);

  // Description:
  // Register MRML node for incoming data.
  int RegisterIncomingMRMLNode(vtkMRMLNode* node);

  // Description:
  // Unregister MRML node for incoming data.
  void UnregisterIncomingMRMLNode(vtkMRMLNode* node);

  // Description:
  // Get number of registered outgoing MRML nodes:
  unsigned int GetNumberOfOutgoingMRMLNodes();

  // Description:
  // Get Nth outgoing MRML nodes:
  vtkMRMLNode* GetOutgoingMRMLNode(unsigned int i);

  // Description:
  // Get number of registered outgoing MRML nodes:
  unsigned int GetNumberOfIncomingMRMLNodes();

  // Description:
  // Get Nth outgoing MRML nodes:
  vtkMRMLNode* GetIncomingMRMLNode(unsigned int i);
  

 private:

  vtkIGTLToMRMLBase* GetConverterByMRMLTag(const char* tag);
  vtkIGTLToMRMLBase* GetConverterByIGTLDeviceType(const char* type);

  // Description:
  // Inserts the eventId to the EventQueue, and the event will be invoked from the main thread
  void RequestInvokeEvent(unsigned long eventId);

 private:
  //----------------------------------------------------------------
  // Connector configuration 
  //----------------------------------------------------------------
  std::string Name;
  int Type;
  int State;

  //----------------------------------------------------------------
  // Thread and Socket
  //----------------------------------------------------------------

  vtkMultiThreader* Thread;
  vtkMutexLock*     Mutex;
  igtl::ServerSocket::Pointer  ServerSocket;
  igtl::ClientSocket::Pointer  Socket;
  int               ThreadID;
  int               ServerPort;
  int               ServerStopFlag;

  std::string       ServerHostname;

  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------


  typedef std::map<std::string, vtkIGTLCircularBuffer*> CircularBufferMap;
  CircularBufferMap Buffer;

  vtkMutexLock* CircularBufferMutex;
  int           RestrictDeviceName;  // Flag to restrict incoming and outgoing data by device names

  // Event queueing mechanism is needed to send all event notifications from the main thread
  // Events can be pushed to the back of the EventQueue by calling RequestInvoke from any thread,
  // and they will be Invoked in the main thread.
  std::list<unsigned long> EventQueue;
  vtkMutexLock* EventQueueMutex;

  // -- Device Name (same as MRML node) and data type (data type string defined in OpenIGTLink)
  DeviceIDSetType   IncomingDeviceIDSet;
  DeviceIDSetType   OutgoingDeviceIDSet;
  DeviceIDSetType   UnspecifiedDeviceIDSet;
  
  // Message converter (IGTLToMRML)
  MessageConverterListType   MessageConverterList;
  MessageConverterMapType    IGTLNameToConverterMap;
  MessageConverterMapType    MRMLIDToConverterMap;

  // List of nodes that this connector node is observing.
  MRMLNodeListType          OutgoingMRMLNodeList;
  MRMLNodeListType          IncomingMRMLNodeList;
  
  int CheckCRC;
  
};

#endif

