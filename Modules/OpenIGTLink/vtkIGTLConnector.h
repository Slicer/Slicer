/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkIGTLConnector_h
#define __vtkIGTLConnector_h

#include <string>
#include <map>
#include <vector>

#include "vtkObject.h"
#include "vtkOpenIGTLinkWin32Header.h" 


//class vtkSocketCommunicator;
class vtkMultiThreader;
class vtkClientSocket;
class vtkServerSocket;
class vtkMutexLock;

class vtkImageData;
class vtkMatrix4x4;

class vtkIGTLCircularBuffer;


class VTK_OPENIGTLINK_EXPORT vtkIGTLConnector : public vtkObject
{
 public:  
  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

  //BTX
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
  //ETX
  
 public:
  
  static vtkIGTLConnector *New();
  vtkTypeRevisionMacro(vtkIGTLConnector,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetMacro( ServerPort, int );
  vtkSetMacro( ServerPort, int );
  vtkGetMacro( Type, int );
  vtkSetMacro( Type, int );
  vtkGetMacro( State, int );
  //vtkSetMacro( State, int );

  //BTX
  void SetName (const char* str) { this->Name = str; }
  void SetName (std::string str) { this->Name = str; }
  const char* GetName() { return this->Name.c_str(); }
  void SetServerHostname(const char* str) { this->ServerHostname = str; }
  void SetServerHostname(std::string str) { this->ServerHostname = str; }
  const char* GetServerHostname() { return this->ServerHostname.c_str(); }
  //ETX

  //----------------------------------------------------------------
  // Constructor and Destructor
  //----------------------------------------------------------------

  vtkIGTLConnector();
  virtual ~vtkIGTLConnector();

  //----------------------------------------------------------------
  // Connector configuration
  //----------------------------------------------------------------

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
  vtkClientSocket* WaitForConnection();
  int ReceiveController();


  //----------------------------------------------------------------
  // Circular Buffer
  //----------------------------------------------------------------

  //BTX
  typedef std::vector<std::string> NameListType;
  int GetUpdatedBuffersList(NameListType& nameList);
  vtkIGTLCircularBuffer* GetCircularBuffer(std::string& key);
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
  vtkServerSocket*  ServerSocket;
  vtkClientSocket*  Socket;
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

};

#endif // __vtkIGTLConnector_h


