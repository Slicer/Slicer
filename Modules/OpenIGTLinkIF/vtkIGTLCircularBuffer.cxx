/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkMutexLock.h"
#include "vtkIGTLCircularBuffer.h"

#include <string.h>

#include "igtlMessageBase.h"

vtkStandardNewMacro(vtkIGTLCircularBuffer);
vtkCxxRevisionMacro(vtkIGTLCircularBuffer, "$Revision$");


//---------------------------------------------------------------------------
vtkIGTLCircularBuffer::vtkIGTLCircularBuffer()
{
  this->Mutex = vtkMutexLock::New();
  this->Mutex->Lock();
  // Allocate Circular buffer for the new device
  this->InUse = -1;
  this->Last  = -1;
  for (int i = 0; i < IGTLCB_CIRC_BUFFER_SIZE; i ++)
    {
    this->DeviceType[i] = "";
    this->Size[i]       = 0;
    this->Data[i]       = NULL;
    this->Messages[i] = igtl::MessageBase::New();
    this->Messages[i]->InitPack();
    }

  this->UpdateFlag = 0;
  this->Mutex->Unlock();
}


//---------------------------------------------------------------------------
vtkIGTLCircularBuffer::~vtkIGTLCircularBuffer()
{
  this->Mutex->Lock();
  this->InUse = -1;
  this->Last  = -1;
  this->Mutex->Unlock();

  for (int i = 0; i < IGTLCB_CIRC_BUFFER_SIZE; i ++)
    {
    if (this->Data[i] != NULL)
      {
      delete this->Data[i];
      }
    }
  this->Mutex->Delete();
}


//---------------------------------------------------------------------------
void vtkIGTLCircularBuffer::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
// Functions to push data into the circular buffer (for receiving thread)
// 
//   StartPush() :     Prepare to push data
//   GetPushBuffer():  Get MessageBase buffer from the circular buffer
//   EndPush() :       Finish pushing data. The data becomes ready to
//                     be read by monitor thread.
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
int vtkIGTLCircularBuffer::StartPush()
{
  this->Mutex->Lock();
  this->InPush = (this->Last + 1) % IGTLCB_CIRC_BUFFER_SIZE;
  if (this->InPush == this->InUse)
    {
    this->InPush = (this->Last + 1) % IGTLCB_CIRC_BUFFER_SIZE;
    }
  this->Mutex->Unlock();

  return this->InPush;
}

//---------------------------------------------------------------------------
igtl::MessageBase::Pointer vtkIGTLCircularBuffer::GetPushBuffer()
{
  return this->Messages[this->InPush];
}

//---------------------------------------------------------------------------
void vtkIGTLCircularBuffer::EndPush()
{
  this->Mutex->Lock();
  this->Last = this->InPush;
  this->UpdateFlag = 1;
  this->Mutex->Unlock();
}


//---------------------------------------------------------------------------
// Functions to pull data into the circular buffer (for monitor thread)
// 
//   StartPull() :     Prepare to pull data
//   GetPullBuffer():  Get MessageBase buffer from the circular buffer
//   EndPull() :       Finish pulling data
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
int vtkIGTLCircularBuffer::StartPull()
{
  this->Mutex->Lock();
  this->InUse = this->Last;
  this->UpdateFlag = 0;
  this->Mutex->Unlock();
  return this->Last;   // return -1 if it is not available
}


//---------------------------------------------------------------------------
igtl::MessageBase::Pointer vtkIGTLCircularBuffer::GetPullBuffer()
{
  return this->Messages[this->InUse];
}


//---------------------------------------------------------------------------
void vtkIGTLCircularBuffer::EndPull()
{
  this->Mutex->Lock();
  this->InUse = -1;
  this->Mutex->Unlock();
}
