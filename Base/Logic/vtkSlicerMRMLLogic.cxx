/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerMRMLLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerMRMLLogic.h"

vtkCxxRevisionMacro(vtkSlicerMRMLLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerMRMLLogic);

//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml scene back into the logic layer for further processing
// - this can also end up calling observers of the logic (i.e. in the GUI)
//
static void MRMLCallback(vtkObject *__mrmlscene, unsigned long eid, void *__clientData, void *callData)
{
  static int inMRMLCallback = 0;
  if (inMRMLCallback)
    {
    cout << "*********MRMLCallback called recursively?" << endl;
    return;
    }
  inMRMLCallback = 1;

  cout << "In MRMLCallback" << endl;

  vtkMRMLScene *mrmlscene = static_cast<vtkMRMLScene *>(__mrmlscene);

  vtkSlicerMRMLLogic *mrmllogic = reinterpret_cast<vtkSlicerMRMLLogic *>(__clientData);

  mrmllogic-> SetActiveVolume(
      vtkMRMLVolumeNode::SafeDownCast(mrmlscene->GetNthNodeByClass(0, "vtkMRMLVolumeNode")));

  inMRMLCallback = 0;
}

//----------------------------------------------------------------------------
vtkSlicerMRMLLogic::vtkSlicerMRMLLogic()
{
    this->ActiveVolume = NULL;
    this->MRMLCallbackCommand = vtkCallbackCommand::New();
    this->MRMLCallbackCommand->SetCallback(MRMLCallback);
}

//----------------------------------------------------------------------------
vtkSlicerMRMLLogic::~vtkSlicerMRMLLogic()
{
    this->SetActiveVolume(NULL);

    this->MRMLCallbackCommand->Delete();
}


//----------------------------------------------------------------------------
void vtkSlicerMRMLLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerMRMLLogic:             " << this->GetClassName() << "\n";
}

