/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerSliceControlGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkCallbackCommand.h"

#include "vtkKWEntry.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithSpinButtons.h"

#include "vtkSlicerSliceControlGUI.h"

#include "vtkMRMLVolumeNode.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerSliceControlGUI );
vtkCxxRevisionMacro(vtkSlicerSliceControlGUI, "$Revision: 1.33 $");


//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
static void MRMLCallback(vtkObject *__mrmlslice, unsigned long eid, void *__clientData, void *callData)
{
  static int inMRMLCallback = 0;

  if (inMRMLCallback)
    {
    cout << "*********MRMLCallback called recursively?" << endl;
    return;
    }
  inMRMLCallback = 1;

  cout << "In MRMLCallback" << endl;

  vtkMRMLSliceNode *mrmlslice = static_cast<vtkMRMLSliceNode *>(__mrmlslice); // Not used, since it is ivar

  vtkSlicerSliceControlGUI *self = reinterpret_cast<vtkSlicerSliceControlGUI *>(__clientData);

  self->UpdateWidgets();

  inMRMLCallback = 0;
}

//----------------------------------------------------------------------------
// Description:
// the WidgetCallback is a static function to relay modified events from the 
// observed widgets back into the mrml layer for further processing
//
static void WidgetCallback(vtkObject *__kwwidget, unsigned long eid, void *__clientData, void *callData)
{
  static int inWidgetCallback = 0;

  if (inWidgetCallback)
    {
    cout << "*********WidgetCallback called recursively?" << endl;
    return;
    }
  inWidgetCallback = 1;

  cout << "In WidgetCallback" << endl;

  vtkKWWidget *kwwidget = static_cast<vtkKWWidget*>(__kwwidget); // Not used, since it is ivar

  vtkSlicerSliceControlGUI *self = reinterpret_cast<vtkSlicerSliceControlGUI *>(__clientData);

  self->Apply();

  inWidgetCallback = 0;
}


//----------------------------------------------------------------------------
vtkSlicerSliceControlGUI::vtkSlicerSliceControlGUI()
{
  // Set the observer on the mrml node
  this->SliceNode      = NULL;
  this->MRMLScene      = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);

  this->WidgetCallbackCommand = vtkCallbackCommand::New();
  this->WidgetCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->WidgetCallbackCommand->SetCallback(WidgetCallback);

  // create the sub widgets
  this->OffsetScale = vtkKWScaleWithEntry::New();
  this->FieldOfViewEntry = vtkKWEntryWithLabel::New();
  this->OrientationMenu = vtkKWMenuButtonWithLabel::New();

}

//----------------------------------------------------------------------------
vtkSlicerSliceControlGUI::~vtkSlicerSliceControlGUI()
{
  if (this->SliceNode)
    {
    this->SliceNode->Delete();
    }
  this->SliceNode = NULL;

  if (this->OffsetScale)
    {
    this->OffsetScale->Delete();
    this->OffsetScale = NULL;
    }

  if (this->FieldOfViewEntry)
    {
    this->FieldOfViewEntry->Delete();
    this->FieldOfViewEntry = NULL;
    }

  if (this->OrientationMenu)
    {
    this->OrientationMenu->Delete();
    this->OrientationMenu = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::Create()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create();

  this->OffsetScale->SetParent(this);
  this->OffsetScale->SetLabelText("Offset: ");
  this->OffsetScale->Create();

  this->FieldOfViewEntry->SetParent(this);
  this->FieldOfViewEntry->SetLabelText("FOV: ");
  this->FieldOfViewEntry->Create();

  this->OrientationMenu->SetParent(this);
  this->OrientationMenu->SetLabelText("Orientation: ");
  vtkKWMenuButton *mb = this->OrientationMenu->GetWidget();
  this->OrientationMenu->Create();

  vtkKWMenu *m = mb->GetMenu();
  int index;

  m->DeleteAllItems();

  index = m->AddRadioButton ("Axial", this, "SetOrientationFromMenu");
  m->SetItemHelpString(index, "Axial view (parallel to the floor)");
  
  index = m->AddRadioButton ("Sagittal", this, "SetOrientationFromMenu");
  m->SetItemHelpString(index, "Sagittal view (side view)" );
  
  index = m->AddRadioButton ("Coronal", this, "SetOrientationFromMenu");
  m->SetItemHelpString(index, "Coronal view (front view)");

  this->Script("pack %s -side bottom -expand true -fill x", this->OffsetScale->GetWidgetName());
  this->Script("pack %s -side right -expand false", this->FieldOfViewEntry->GetWidgetName());
  this->Script("pack %s -side right -expand false", this->OrientationMenu->GetWidgetName());

  // TODO: there is no start event (needed for undo)
  //this->OffsetScale->GetWidget()->AddObserver( vtkKWScale::ScaleValueChangedEvent, this->WidgetCallbackCommand );
  // TODO: the entry doesn't invoke events, it only has a Command
  //this->FieldOfViewEntry->AddObserver( vtkCommand::ModifiedEvent, this->WidgetCallbackCommand );
  // TODO: the menu doesn't send an event either
  this->OrientationMenu->AddObserver( vtkCommand::ModifiedEvent, this->WidgetCallbackCommand );

  // TODO: should we make these events for consistency or stick with the Commands?
  this->OffsetScale->GetWidget()->SetStartCommand( this, "Apply" );
  this->OffsetScale->GetWidget()->SetCommand( this, "TransientApply" );
  this->FieldOfViewEntry->GetWidget()->SetCommand( this, "Apply" );
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::SetSliceNode ( vtkMRMLSliceNode  *SliceNode )
{
  if ( this->SliceNode  )
    {
    this->SliceNode->RemoveObserver( this->MRMLCallbackCommand );
    this->SliceNode->Delete();
    }
  
  this->SliceNode  = SliceNode ;

  if ( this->SliceNode  )
    {
    this->SliceNode->Register(this);
    this->SliceNode->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::SetMRMLScene ( vtkMRMLScene  *MRMLScene )
{
  if ( this->MRMLScene  )
    {
    this->MRMLScene ->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene ->Delete();
    }
  
  this->MRMLScene  = MRMLScene ;

  if ( this->MRMLScene  )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::UpdateWidgets()
{
  double fov = this->SliceNode->GetFieldOfView()[0];
  char fovstring[80];
  sprintf (fovstring, "%g", fov);
  this->FieldOfViewEntry->GetWidget()->SetValue(fovstring);
  
  double fovover2 = this->SliceNode->GetFieldOfView()[2] / 2.;
  this->OffsetScale->SetRange(-fovover2, fovover2);


  // TODO: set the scale value from the translation part of the matrix with rotation
  // Set the Scale from the Offest in the matrix
  vtkMatrix4x4 *m = this->SliceNode->GetSliceToRAS();
  this->OffsetScale->SetValue( m->GetElement( 2, 3 ) );

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::SetOrientationFromMenu()
{

  // TODO: set the SliceToRAS matrix from the menu value

}


//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::TransientApply()
{

  // Set the Field of View from the Entry
  double val = this->FieldOfViewEntry->GetWidget()->GetValueAsDouble();
  if ( val != 0 )
    {
    this->SliceNode->SetFieldOfView(val, val, val);
    }

  // Set the Offset from the Scale
  vtkMatrix4x4 *m = this->SliceNode->GetSliceToRAS();
  // TODO: this should be set by the Orientation menu
  m->SetElement( 2, 3, this->OffsetScale->GetValue() );
  this->SliceNode->UpdateMatrices();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::Apply()
{

  // TODO: observe scale start and end states and only set undo on
  // button down
  this->MRMLScene->SaveStateForUndo(this->SliceNode);

  this->TransientApply();

}


//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SliceNode: " << this->SliceNode << endl;
}

