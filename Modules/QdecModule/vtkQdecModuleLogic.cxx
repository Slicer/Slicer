/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkQdecModuleLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObjectFactory.h"

#include "vtkQdecModuleLogic.h"
#include "vtkQdecModule.h"

#include "vtkSlicerApplication.h"


vtkQdecModuleLogic* vtkQdecModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkQdecModuleLogic");
  if(ret)
    {
      return (vtkQdecModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkQdecModuleLogic;
}


//----------------------------------------------------------------------------
vtkQdecModuleLogic::vtkQdecModuleLogic()
{
  // have to check this here, or will get seg fault when try to instantiate a QdecProject
  if (NULL == getenv( "SUBJECTS_DIR" ) )
    {
    vtkErrorMacro("Unable to init QDECProject: please set SUBJECTS_DIR environment variable");
    this->QDECProject = NULL;
    }
  else
    {
    this->QDECProject = new QdecProject();
    }
}

//----------------------------------------------------------------------------
vtkQdecModuleLogic::~vtkQdecModuleLogic()
{
  if (QDECProject)
    {
    delete this->QDECProject;
    this->QDECProject = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkQdecModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//----------------------------------------------------------------------------
void vtkQdecModuleLogic::Apply(vtkSlicerVolumesGUI *volsGUI)
{


    return;
}

//----------------------------------------------------------------------------
int vtkQdecModuleLogic::LoadDataTable(const char* fileName)
{
  vtkDebugMacro("LoadDataTable: trying to load " << fileName);

  if (this->QDECProject)
    {
    // returns -1 on error, 0 on success
    int err = this->QDECProject->LoadDataTable(fileName);
    vtkDebugMacro("Return from LoadDataTable call on QDECProject = " << err);
    if (err == 0)
      {
      return 1;
      }
    else
      {
      vtkErrorMacro("LoadDataTable: Failed to load data file " << fileName);
      }
    }
  else
    {
    vtkErrorMacro("LoadDataTable: QDEC Project is unintialised, cannot use it to load data file " << fileName);
    }
  return 0;
}

//----------------------------------------------------------------------------
//void vtkQdecModuleLogic::CreateGLMDesign(const char* name)
