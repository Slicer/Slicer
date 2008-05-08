/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyDisplayLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

#include "vtkSlicerTractographyDisplayLogic.h"

#include <sstream>

vtkCxxRevisionMacro(vtkSlicerTractographyDisplayLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerTractographyDisplayLogic);

//----------------------------------------------------------------------------
vtkSlicerTractographyDisplayLogic::vtkSlicerTractographyDisplayLogic() :
  vtkSlicerFiberBundleLogic()
{
}

//-----------------------------------------------------------------------------
vtkIntArray*
vtkSlicerTractographyDisplayLogic::
NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);

  return events;
}
