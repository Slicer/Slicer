/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkWFEngineModuleLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkWFEngineModuleLogic.h"


vtkCxxRevisionMacro(vtkWFEngineModuleLogic, "$Revision: 1.9.12.1 $");

vtkWFEngineModuleLogic* vtkWFEngineModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkWFEngineModuleLogic");
  if(ret)
    {
      return (vtkWFEngineModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkWFEngineModuleLogic;
}


//----------------------------------------------------------------------------
vtkWFEngineModuleLogic::vtkWFEngineModuleLogic()
{
  this->WFEngineModuleNode = NULL;
}

//----------------------------------------------------------------------------
vtkWFEngineModuleLogic::~vtkWFEngineModuleLogic()
{
    if(this->WFEngineModuleNode)
    {
        this->WFEngineModuleNode->Delete();
        this->SetWFEngineModuleNode(NULL);
    }  
}


//----------------------------------------------------------------------------
void vtkWFEngineModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{

}

