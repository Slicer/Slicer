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

#ifndef __igtlHeaderMessage_h
#define __igtlHeaderMessage_h

#include "igtlWin32Header.h"
#include "igtlObject.h"
#include "igtlMath.h"
#include "igtlMessageBase.h"
#include "igtlTypes.h"

namespace igtl
{

class IGTLCommon_EXPORT HeaderMessage: public MessageBase
{
public:
  enum {
    POSITION_ONLY =  1,
    WITH_QUATERNION3,
    WITH_QUATERNION,
    ALL,
  };


public:
  typedef HeaderMessage                Self;
  typedef MessageBase                    Superclass;
  typedef SmartPointer<Self>             Pointer;
  typedef SmartPointer<const Self>       ConstPointer;

  igtlTypeMacro(igtl::HeaderMessage, igtl::MessageBase);
  igtlNewMacro(igtl::HeaderMessage);


public:

  void         Init();
  virtual int  SetMessageHeader(const MessageHeader* mb);
  int          SetDeviceType(const char* name);
  
protected:
  HeaderMessage();
  ~HeaderMessage();
  
protected:

  virtual int  GetBodyPackSize();
  virtual int  PackBody();
  virtual int  UnpackBody();

};


} // namespace igtl

#endif // _igtlPositionMessage_h



