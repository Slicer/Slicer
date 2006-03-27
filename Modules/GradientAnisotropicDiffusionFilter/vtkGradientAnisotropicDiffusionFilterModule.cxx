/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterModule.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkGradientAnisotropicDiffusionFilterModule.h"
#include "vtkGradientAnisotropicDiffusionFilterWidget.h"
#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"

//------------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterModule* vtkGradientAnisotropicDiffusionFilterModule::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGradientAnisotropicDiffusionFilterModule");
  if(ret)
    {
      return (vtkGradientAnisotropicDiffusionFilterModule*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkGradientAnisotropicDiffusionFilterModule;
}


//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterModule::vtkGradientAnisotropicDiffusionFilterModule()
{

}

//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterModule::~vtkGradientAnisotropicDiffusionFilterModule()
{

}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::BuildGUI(vtkKWFrame* frame)
{
  Superclass::BuildGUI(frame);
  vtkGradientAnisotropicDiffusionFilterWidget *widget = vtkGradientAnisotropicDiffusionFilterWidget::New();
  widget->Create();

  widget->SetParent(frame);
  

}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

