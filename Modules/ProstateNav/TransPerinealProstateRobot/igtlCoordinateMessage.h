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

#ifndef __igtlCoordinateMessage_h
#define __igtlCoordinateMessage_h

#include "igtlWin32Header.h"
#include "igtlObject.h"
#include "igtlMath.h"
#include "igtlMessageBase.h"
#include "igtlTypes.h"

namespace igtl
{

class IGTLCommon_EXPORT CoordinateMessage: public MessageBase
{
public:
  enum {
    POSITION_ONLY =  1,
    WITH_QUATERNION3,
    WITH_QUATERNION,
    ALL,
  };


public:
  typedef CoordinateMessage                Self;
  typedef MessageBase                    Superclass;
  typedef SmartPointer<Self>             Pointer;
  typedef SmartPointer<const Self>       ConstPointer;

  igtlTypeMacro(igtl::CoordinateMessage, igtl::MessageBase);
  igtlNewMacro(igtl::CoordinateMessage);


public:

  void         Init();

  void         SetPackType(int t); /* POSITION_ONLY / WITH_QUATERNION3 / ALL */

  /** Specify pack type by body size (in most case obtained from general header) */
  int          SetPackTypeByBodySize(int s);

  void         SetPosition(const float* pos);
  void         SetPosition(float x, float y, float z);
  void         SetQuaternion(const float* quat);
  void         SetQuaternion(float ox, float oy, float oz, float w);
  void         SetOffset(const float* offset);
  void         SetOffset(float x, float y, float z);
  void         SetInsertion(float i) { this->m_Insertion = i; };

  void         GetPosition(float* pos);
  void         GetPosition(float* x, float* y, float* z);
  void         GetQuaternion(float* quat);
  void         GetQuaternion(float* ox, float* oy, float* oz, float* w);
  void         GetOffset(float* offset);
  void         GetOffset(float* x, float* y, float* z);
  float        GetInsertion()  { return this->m_Insertion; };


  virtual int  SetMessageHeader(const MessageHeader* mb);
  
protected:
  CoordinateMessage();
  ~CoordinateMessage();
  
protected:

  virtual int  GetBodyPackSize();
  virtual int  PackBody();
  virtual int  UnpackBody();

  igtlInt32    m_PackType;
  
  igtlFloat32  m_Position[3];
  igtlFloat32  m_Quaternion[4];
  igtlFloat32  m_Offset[3];
  igtlFloat32  m_Insertion;

};


} // namespace igtl

#endif // _igtlPositionMessage_h



