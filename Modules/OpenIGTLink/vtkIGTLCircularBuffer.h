/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkIGTLCircularBuffer_h
#define __vtkIGTLCircularBuffer_h

#include "vtkObject.h"
#include "vtkOpenIGTLinkWin32Header.h" 

#include <string>

#define IGTLCB_CIRC_BUFFER_SIZE    3


class vtkMutexLock;

class VTK_OPENIGTLINK_EXPORT vtkIGTLCircularBuffer : public vtkObject
{
 public:

  static vtkIGTLCircularBuffer *New();
  vtkTypeRevisionMacro(vtkIGTLCircularBuffer,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  int GetNumberOfBuffer() { return IGTLCB_CIRC_BUFFER_SIZE; }

  int            StartPush();
  void           PushDeviceType(const char* deviceType);
  void           PushData(int size, unsigned char* data);
  unsigned char* GetPushDataArea(int size);
  void           EndPush();
  int            StartPull();
  const char*    PullDeviceType();
  int            PullSize();
  unsigned char* PullData();
  void           EndPull();
  int            IsUpdated() { return this->UpdateFlag; };

 protected:
  vtkIGTLCircularBuffer();
  virtual ~vtkIGTLCircularBuffer();

 protected:

  vtkMutexLock*      Mutex;
  int                Last;        // updated by connector thread
  int                InPush;      // updated by connector thread
  int                InUse;       // updated by main thread

  int                UpdateFlag;  // non-zero if updated since StartPull() has called

  //BTX
  std::string        DeviceType[IGTLCB_CIRC_BUFFER_SIZE];
  //ETX
  long long          Size[IGTLCB_CIRC_BUFFER_SIZE];
  unsigned char*     Data[IGTLCB_CIRC_BUFFER_SIZE];
  
};


#endif //__vtkIGTLCircularBuffer_h
