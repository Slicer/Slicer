/*=========================================================================

  Module:    $RCSfile: vtkContainer.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkContainer.h"

#include "vtkDebugLeaks.h"

vtkContainer::vtkContainer() 
{ 
}

vtkContainer::~vtkContainer() 
{
}

int vtkContainerCompareMethod(const char* d1, const char* d2)
{ 
  if ( !d1 && !d2 ) 
    { 
    return 0;
    }
  if ( !d1 )
    {
    return 1;
    }
  if ( !d2 )
    {
    return -1;
    }
  return strcmp(d1, d2); 
}

int vtkContainerCompareMethod(char* d1, char* d2)
{ 
  if ( !d1 && !d2 ) 
    { 
    return 0;
    }
  if ( !d1 )
    {
    return 1;
    }
  if ( !d2 )
    {
    return -1;
    }
  return strcmp(d1, d2); 
}

const char* vtkContainerCreateMethod(const char* d1)
{ 
  if ( !d1 )
    {
    return 0;
    }
  char* str = new char[ strlen(d1) + 1];
  strcpy(str, d1);
  return str;
}

char* vtkContainerCreateMethod(char* d1)
{ 
  if ( !d1 )
    {
    return 0;
    }
  char* str = new char[ strlen(d1) + 1];
  strcpy(str, d1);
  return str;
}



