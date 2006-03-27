/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtGradientAnisotropicDiffusionFilterModule.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtGradientAnisotropicDiffusionFilterModule.h"
#include "vtkGradientAnisotropicDiffusionFilterWidget.h"
#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"

//------------------------------------------------------------------------------
vtGradientAnisotropicDiffusionFilterModule* vtGradientAnisotropicDiffusionFilterModule::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtGradientAnisotropicDiffusionFilterModule");
  if(ret)
    {
      return (vtGradientAnisotropicDiffusionFilterModule*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtGradientAnisotropicDiffusionFilterModule;
}


//----------------------------------------------------------------------------
vtGradientAnisotropicDiffusionFilterModule::vtGradientAnisotropicDiffusionFilterModule()
{

}

//----------------------------------------------------------------------------
vtGradientAnisotropicDiffusionFilterModule::~vtGradientAnisotropicDiffusionFilterModule()
{

}

//----------------------------------------------------------------------------
void vtGradientAnisotropicDiffusionFilterModule::BuildGUI(vtkKWFrame* frame);
{
  Superclass::BuildGUI(frame);
  vtkMRMLGradientAnisotropicDiffusionFilterWidget widget = vtkMRMLGradientAnisotropicDiffusionFilterWidget::New();
  

}

//----------------------------------------------------------------------------
void vtGradientAnisotropicDiffusionFilterModule::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}

