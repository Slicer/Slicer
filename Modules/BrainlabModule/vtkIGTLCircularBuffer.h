/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkIGTLCircularBuffer.h $
  Date:      $Date: 2009-08-12 21:30:38 -0400 (Wed, 12 Aug 2009) $
  Version:   $Revision: 10236 $

==========================================================================*/

#ifndef __vtkIGTLCircularBuffer_h
#define __vtkIGTLCircularBuffer_h

#include "vtkObject.h"
#include "vtkBrainlabModuleWin32Header.h"

#include <string>

#include "igtlMessageBase.h"

#define IGTLCB_CIRC_BUFFER_SIZE    3

class vtkMutexLock;

class VTK_BRAINLABMODULE_EXPORT vtkIGTLCircularBuffer : public vtkObject
{
 public:

  static vtkIGTLCircularBuffer *New();
  vtkTypeRevisionMacro(vtkIGTLCircularBuffer,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  int GetNumberOfBuffer() { return IGTLCB_CIRC_BUFFER_SIZE; }

  int            StartPush();
  void           EndPush();
  //BTX
  igtl::MessageBase::Pointer GetPushBuffer();
  //ETX

  int            StartPull();
  void           EndPull();
  //BTX
  igtl::MessageBase::Pointer GetPullBuffer();
  //ETX

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

  //BTX
  igtl::MessageBase::Pointer Messages[IGTLCB_CIRC_BUFFER_SIZE];
  //ETX
  
};


#endif //__vtkIGTLCircularBuffer_h
