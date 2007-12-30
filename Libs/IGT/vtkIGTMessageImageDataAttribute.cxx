/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#include "vtkObjectFactory.h"

#include "vtkIGTMessageImageDataAttribute.h"

vtkStandardNewMacro(vtkIGTMessageImageDataAttribute);
vtkCxxRevisionMacro(vtkIGTMessageImageDataAttribute, "$Revision: 1.0 $");


vtkIGTMessageImageDataAttribute::vtkIGTMessageImageDataAttribute()
{
  Alloc();
  this->type_name = typeid(vtkImageData).name();
}

vtkIGTMessageImageDataAttribute::~vtkIGTMessageImageDataAttribute()
{
  Free();
}

void vtkIGTMessageImageDataAttribute::PrintSelf(ostream& os, vtkIndent indent)
{
}

/*
template <typename T> 
int vtkIGTMessageImageDataAttribute::SetAttribute(T* ptr)
{
  if (typeid(T) == typeid(this->data))
    {
      this->data = *ptr;
      return 1;
    }
  return 0;
}

template <typename T>
int vtkIGTMessageImageDataAttribute::GetAttribute(T* ptr)
{
  if (typeid(T) == typeid(this->data))
    {
      *ptr = this->data;
      return 1;
    }
  return 0;
}
*/

int vtkIGTMessageImageDataAttribute::SetAttribute(void* ptr)
{
  if (ptr)
    {
    vtkImageData* attr = static_cast<vtkImageData*>(ptr);
    this->data->ShallowCopy(attr);
    }

  return 1;
}

int vtkIGTMessageImageDataAttribute::GetAttribute(void* ptr)
{
  if (ptr)
    {
    vtkImageData* attr = static_cast<vtkImageData*>(ptr);
    attr->ShallowCopy(this->data);
    return 1;
    }
  else
    {
    return 0;
    }
}


int vtkIGTMessageImageDataAttribute::SetAttribute(vtkImageData* ptr)
{
  if (ptr)
    {
    this->data->ShallowCopy(ptr);
    return 1;
    }
  else
    {
    return 0;
    }
}


int vtkIGTMessageImageDataAttribute::GetAttribute(vtkImageData* ptr)
{
  if (ptr)
    {
    ptr->ShallowCopy(this->data);
    return 1;
    }
  else
    {
    return 0;
    }

}


void vtkIGTMessageImageDataAttribute::ClearAttribute()
{
  if (this->data)
    {
    this->data->Delete();
    }
  this->data = NULL;
}


int vtkIGTMessageImageDataAttribute::Alloc()
{
  this->data = vtkImageData::New();
}

int vtkIGTMessageImageDataAttribute::Free()
{
  this->data->Delete();
}

