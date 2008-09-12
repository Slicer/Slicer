/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkMutexLock.h"
#include "vtkIGTLCircularBuffer.h"

#include <string.h>

vtkStandardNewMacro(vtkIGTLCircularBuffer);
vtkCxxRevisionMacro(vtkIGTLCircularBuffer, "$Revision: 1.0 $");


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
//   PushDeviceType(): Put device type name
//   PushData() :      Put data itself
//   GetPushDataArea : Get pointer to data area
//                      (alternative way to put data to the buffer)
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
void vtkIGTLCircularBuffer::PushDeviceType(const char* deviceType)
{

  if (this->DeviceType[this->InPush] != deviceType)
    {
    this->DeviceType[this->InPush] = deviceType;
    }

}

//---------------------------------------------------------------------------
void vtkIGTLCircularBuffer::PushData(int size, unsigned char* data)
{
  // Data and its size
  if (this->Size[this->InPush] != size)
    {
    if (this->Data[this->InPush] != NULL)
      {
      delete this->Data[this->InPush];
      }
    this->Data[this->InPush] = new unsigned char[size];
    this->Size[this->InPush] = size;
    }
  
  memcpy(this->Data[this->InPush], data, size);
}


//---------------------------------------------------------------------------
unsigned char* vtkIGTLCircularBuffer::GetPushDataArea(int size)
{
  // Data and its size
  if (this->Size[this->InPush] != size)
    {
    if (this->Data[this->InPush] != NULL)
      {
      delete this->Data[this->InPush];
      }
    this->Data[this->InPush] = new unsigned char[size];
    this->Size[this->InPush] = size;
    }
  
  return this->Data[this->InPush];
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
//   PullDeviceType(): Get device type
//   PullSize() :      Get data size
//   PullData() :      Get data content
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
const char* vtkIGTLCircularBuffer::PullDeviceType()
{
  return this->DeviceType[this->InUse].c_str();
}


//---------------------------------------------------------------------------
int vtkIGTLCircularBuffer::PullSize()
{
  return this->Size[this->InUse];
}

//---------------------------------------------------------------------------
unsigned char* vtkIGTLCircularBuffer::PullData()
{
  return this->Data[this->InUse];
}


//---------------------------------------------------------------------------
void vtkIGTLCircularBuffer::EndPull()
{
  this->Mutex->Lock();
  this->InUse = -1;
  this->Mutex->Unlock();
}
