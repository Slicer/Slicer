/*=========================================================================

  Program:   Open IGT Link Library
  Module:    $RCSfile: $
  Language:  C++
  Date:      $Date: $
  Version:   $Revision: $

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "igtlCoordinateMessage.h"

#include "igtlutil/igtl_header.h"
#include "igtlutil/igtl_position.h"

#include <string.h>

namespace igtl {

CoordinateMessage::CoordinateMessage():
  MessageBase()
{
  Init();
  m_DefaultBodyType  = "COORDINATES";
}


CoordinateMessage::~CoordinateMessage()
{
}


void CoordinateMessage::Init()
{
  this->m_PackType      = ALL;

  this->m_Position[0]   = 0.0;
  this->m_Position[1]   = 0.0;
  this->m_Position[2]   = 0.0;

  this->m_Quaternion[0] = 0.0;
  this->m_Quaternion[1] = 0.0;
  this->m_Quaternion[2] = 0.0;
  this->m_Quaternion[3] = 1.0;
}


void CoordinateMessage::SetPackType(int t)
{
  if (t >= POSITION_ONLY && t <= ALL)
    {
    this->m_PackType = t;
    }
}


int CoordinateMessage::SetPackTypeByBodySize(int s)
{

  if (s == IGTL_POSITION_MESSAGE_POSITON_ONLY_SIZE)
    {
    this->m_PackType = POSITION_ONLY;
    }
  else if (s == IGTL_POSITION_MESSAGE_WITH_QUATERNION3_SIZE)
    {
    this->m_PackType = WITH_QUATERNION3;
    }
  else if (s == IGTL_POSITION_MESSAGE_DEFAULT_SIZE)
    {
    this->m_PackType = WITH_QUATERNION;
    }
  else
    {
    this->m_PackType = ALL;
    return 0;
    }

  return this->m_PackType;

}


void CoordinateMessage::SetPosition(const float* pos)
{
  this->m_Position[0] = pos[0];
  this->m_Position[1] = pos[1];
  this->m_Position[2] = pos[2];
}


void CoordinateMessage::SetPosition(float x, float y, float z)
{
  this->m_Position[0] = x;
  this->m_Position[1] = y;
  this->m_Position[2] = z;
}


void CoordinateMessage::SetQuaternion(const float* quat)
{
  this->m_Quaternion[0] = quat[0];
  this->m_Quaternion[1] = quat[1];
  this->m_Quaternion[2] = quat[2];
  this->m_Quaternion[3] = quat[3];
}


void CoordinateMessage::SetQuaternion(float ox, float oy, float oz, float w)
{
  this->m_Quaternion[0] = ox;
  this->m_Quaternion[1] = oy;
  this->m_Quaternion[2] = oz;
  this->m_Quaternion[3] = w;
}


void CoordinateMessage::SetOffset(const float* offset)
{
  this->m_Offset[0] = offset[0];
  this->m_Offset[1] = offset[1];
  this->m_Offset[2] = offset[2];
}


void CoordinateMessage::SetOffset(float x, float y, float z)
{
  this->m_Offset[0] = x;
  this->m_Offset[1] = y;
  this->m_Offset[2] = z;
}


void CoordinateMessage::GetPosition(float* pos)
{
  pos[0] = this->m_Position[0];
  pos[1] = this->m_Position[1];
  pos[2] = this->m_Position[2];
}


void CoordinateMessage::GetPosition(float* x, float* y, float* z)
{
  *x = this->m_Position[0];
  *y = this->m_Position[1];
  *z = this->m_Position[2];
}


void CoordinateMessage::GetQuaternion(float* quat)
{
  quat[0] = this->m_Quaternion[0];
  quat[1] = this->m_Quaternion[1];
  quat[2] = this->m_Quaternion[2];
  quat[3] = this->m_Quaternion[3];
}


void CoordinateMessage::GetQuaternion(float* ox, float* oy, float* oz, float* w)
{
  *ox = this->m_Quaternion[0];
  *oy = this->m_Quaternion[1];
  *oz = this->m_Quaternion[2];
  *w  = this->m_Quaternion[3];
}


void CoordinateMessage::GetOffset(float* offset)
{
  offset[0] = this->m_Offset[0];
  offset[1] = this->m_Offset[1];
  offset[2] = this->m_Offset[2];
}

void CoordinateMessage::GetOffset(float* x, float* y, float* z)
{
  *x = this->m_Offset[0];
  *y = this->m_Offset[1];
  *z = this->m_Offset[2];
}


int CoordinateMessage::SetMessageHeader(const MessageHeader* mb)
{
  int rc = Copy(mb);
  int rt = SetPackTypeByBodySize(this->GetPackBodySize());

  return (rc && rt);
  
}


int CoordinateMessage::GetBodyPackSize()
{
  int ret;

  switch (this->m_PackType)
    {
    case POSITION_ONLY:
      ret = IGTL_POSITION_MESSAGE_POSITON_ONLY_SIZE;
      break;
    case WITH_QUATERNION3:
      ret = IGTL_POSITION_MESSAGE_WITH_QUATERNION3_SIZE;
      break;
    case WITH_QUATERNION:
      ret = IGTL_POSITION_MESSAGE_DEFAULT_SIZE;
      break;
    default:
      ret = IGTL_POSITION_MESSAGE_DEFAULT_SIZE+sizeof(igtlFloat32)*4;
    }

  return ret;

}


int CoordinateMessage::PackBody()
{
  // allocate pack
  AllocatePack();

  igtl_position* p = (igtl_position*)this->m_Body;

  p->position[0]   = this->m_Position[0];
  p->position[1]   = this->m_Position[1];
  p->position[2]   = this->m_Position[2];

  p->quaternion[0] = this->m_Quaternion[0];
  p->quaternion[1] = this->m_Quaternion[1];
  p->quaternion[2] = this->m_Quaternion[2];
  p->quaternion[3] = this->m_Quaternion[3];

  igtl_position_convert_byte_order(p);

  igtl_float32* b = (igtl_float32*)((unsigned char*)this->m_Body + sizeof(igtl_position));
  b[0] = this->m_Offset[0];
  b[1] = this->m_Offset[1];
  b[2] = this->m_Offset[2];
  b[3] = this->m_Insertion;
  
  if (igtl_is_little_endian())
    {
    igtl_uint32* a = (igtl_uint32*)b;
    a[0] = BYTE_SWAP_INT32(a[0]);
    a[1] = BYTE_SWAP_INT32(a[1]);
    a[2] = BYTE_SWAP_INT32(a[2]);
    a[3] = BYTE_SWAP_INT32(a[3]);
    }

  return 1;
}

int CoordinateMessage::UnpackBody()
{
  igtl_position* p = (igtl_position*)this->m_Body;
  
  igtl_position_convert_byte_order(p);

  this->m_Position[0]   = p->position[0];
  this->m_Position[1]   = p->position[1];
  this->m_Position[2]   = p->position[2];
  this->m_Quaternion[0] = p->quaternion[0];
  this->m_Quaternion[1] = p->quaternion[1];
  this->m_Quaternion[2] = p->quaternion[2];
  this->m_Quaternion[3] = p->quaternion[3];

  if (this->GetPackSize() >= static_cast<int>(IGTL_HEADER_SIZE+IGTL_POSITION_MESSAGE_DEFAULT_SIZE
      +sizeof(igtlFloat32)*4))
    {
    unsigned char* off = (unsigned char*) this->m_Body;
    off += sizeof(igtl_position);
    if (igtl_is_little_endian())
      {
      
      igtlUint32 tmp[3];
      memcpy((void*)tmp, (void*)(off), sizeof(igtl_float32)*3);
      tmp[0]  = BYTE_SWAP_INT32(tmp[0]);
      tmp[1]  = BYTE_SWAP_INT32(tmp[1]);
      tmp[2]  = BYTE_SWAP_INT32(tmp[2]);
      memcpy((void*)this->m_Offset, (void*)tmp, sizeof(igtl_float32)*3);
      
      off += sizeof(igtl_float32)*3;
      igtlUint32 tmp2 = 0;
      memcpy((void*)tmp2, (void*)(off), sizeof(igtl_float32));
      this->m_Insertion = BYTE_SWAP_INT32(tmp2);
      }
    else
      {
      memcpy((void*)this->m_Offset, (void*) off, sizeof(igtl_float32)*4);
      }
    }
  
  return 1;

}

} // namespace igtl





