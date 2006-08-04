/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkScriptedModuleLogic.h"
#include "vtkScriptedModule.h"

#include "vtkMRMLScene.h"

vtkScriptedModuleLogic* vtkScriptedModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkScriptedModuleLogic");
  if(ret)
    {
      return (vtkScriptedModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkScriptedModuleLogic;
}


//----------------------------------------------------------------------------
vtkScriptedModuleLogic::vtkScriptedModuleLogic()
{
  this->ScriptedModuleNode = NULL;
  this->ModuleName = NULL;
}

//----------------------------------------------------------------------------
vtkScriptedModuleLogic::~vtkScriptedModuleLogic()
{
  this->SetScriptedModuleNode(NULL);
}

//----------------------------------------------------------------------------
void vtkScriptedModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << (this->ScriptedModuleNode ? this->ScriptedModuleNode->GetID() : "(none)") << "\n";
}
