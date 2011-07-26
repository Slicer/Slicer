/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkAtlasCreatorLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// Slicer includes
#include "vtkAtlasCreatorLogic.h"

// VTKITK includes

// MRML includes

// VTK includes

// STD includes

vtkAtlasCreatorLogic* vtkAtlasCreatorLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkAtlasCreatorLogic");
  if(ret)
    {
      return (vtkAtlasCreatorLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkAtlasCreatorLogic;
}


//----------------------------------------------------------------------------
vtkAtlasCreatorLogic::vtkAtlasCreatorLogic()
{
  this->AtlasCreatorNode = NULL;
}

//----------------------------------------------------------------------------
vtkAtlasCreatorLogic::~vtkAtlasCreatorLogic()
{
  vtkSetMRMLNodeMacro(this->AtlasCreatorNode, NULL);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

void vtkAtlasCreatorLogic::Apply()
{

}
