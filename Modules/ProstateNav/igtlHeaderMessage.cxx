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

#include "igtlHeaderMessage.h"

#include "igtlutil/igtl_header.h"
#include "igtlutil/igtl_position.h"

#include <string.h>

namespace igtl {

HeaderMessage::HeaderMessage():
  MessageBase()
{
  Init();
  m_DefaultBodyType  = "POSITION";
}


HeaderMessage::~HeaderMessage()
{
}


void HeaderMessage::Init()
{
}


int HeaderMessage::SetMessageHeader(const MessageHeader* mb)
{
  int rc = Copy(mb);

  return (rc);
  
}


int HeaderMessage::SetDeviceType(const char* name)
{
  if (strlen(name) <= IGTL_HEADER_NAMESIZE)
    {
    this->m_DefaultBodyType = name;
    return 1;
    }
  else
    {
    return 0;
    }
}


int HeaderMessage::GetBodyPackSize()
{
  return 0;
}


int HeaderMessage::PackBody()
{
  // allocate pack
  AllocatePack();

  return 1;
}

int HeaderMessage::UnpackBody()
{
  return 1;

}




} // namespace igtl





