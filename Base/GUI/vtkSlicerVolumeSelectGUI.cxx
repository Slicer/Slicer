/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerVolumeSelectGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkSlicerVolumeSelectGUI.h"

#include "vtkMRMLVolumeNode.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithSpinButtons.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerVolumeSelectGUI );
vtkCxxRevisionMacro(vtkSlicerVolumeSelectGUI, "$Revision: 1.33 $");


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

  vtkMRMLScene *mrmlscene = static_cast<vtkMRMLScene *>(__mrmlscene); // Not used, since it is ivar

  vtkSlicerVolumeSelectGUI *self = reinterpret_cast<vtkSlicerVolumeSelectGUI *>(__clientData);

  self->UpdateMenu();

  inMRMLCallback = 0;
}


//----------------------------------------------------------------------------
vtkSlicerVolumeSelectGUI::vtkSlicerVolumeSelectGUI()
{
  this->MRMLScene      = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);
}

//----------------------------------------------------------------------------
vtkSlicerVolumeSelectGUI::~vtkSlicerVolumeSelectGUI()
{
  if (this->MRMLScene)
    {
    this->MRMLScene->Delete();
    }
  this->MRMLScene = NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeSelectGUI::SetMRMLScene( vtkMRMLScene *MRMLScene)
{
  if ( this->MRMLScene )
    {
    this->MRMLScene->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene->Delete();
    }
  
  this->MRMLScene = MRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeSelectGUI::UpdateMenu()
{
    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();

    m->DeleteAllMenuItems();

    vtkMRMLVolumeNode *node;
    this->MRMLScene->InitTraversal();
    while ( (node = vtkMRMLVolumeNode::SafeDownCast (this->MRMLScene->GetNextNodeByClass("vtkMRMLVolumeNode"))) != NULL)
      {
      // TODO: figure out how to use the ID instead of the name as the menu indicator
      mb->AddRadioButton ( node->GetName() );
      }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeSelectGUI::SelectVolume()
{
    // do nothing for now - this is the callback for the menu item
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode *vtkSlicerVolumeSelectGUI::GetSelected()
{
    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();

    int idx = m->GetCheckButtonValue(m, "VolumeSelect");
    vtkMRMLNode *n = this->MRMLScene->GetNthNodeByClass (idx, "vtkMRMLVolumeNode");
    return (vtkMRMLVolumeNode::SafeDownCast(n));
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeSelectGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MRMLScene: " << this->MRMLScene << endl;
}

