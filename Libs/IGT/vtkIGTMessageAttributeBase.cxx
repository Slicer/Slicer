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

#include "vtkIGTMessageAttributeBase.h"

vtkStandardNewMacro(vtkIGTMessageAttributeBase);
vtkCxxRevisionMacro(vtkIGTMessageAttributeBase, "$Revision: 1.0 $");


vtkIGTMessageAttributeBase::vtkIGTMessageAttributeBase()
{
  this->type_name = NULL;
  this->type_id = 0;
  Alloc();
}


vtkIGTMessageAttributeBase::~vtkIGTMessageAttributeBase()
{
  Free();
}

void vtkIGTMessageAttributeBase::PrintSelf(ostream& os, vtkIndent indent)
{
}


const char* vtkIGTMessageAttributeBase::GetType()
{
  //return type_name.c_str();
  return type_name;
}





