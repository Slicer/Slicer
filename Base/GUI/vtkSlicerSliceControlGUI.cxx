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
#include "vtkKWSpinBox.h"
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
    vtkErrorWithObjectMacro (__mrmlslice, << "*********MRMLCallback called recursively?" << endl);
    return;
    }
  inMRMLCallback = 1;

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
  this->FieldOfViewEntry = vtkKWSpinBoxWithLabel::New();
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

  if (this->MRMLCallbackCommand)
    {
    this->MRMLCallbackCommand->Delete();
    this->MRMLCallbackCommand = NULL;
    }
  if (this->WidgetCallbackCommand)
    {
    this->WidgetCallbackCommand->Delete();
    this->WidgetCallbackCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->OffsetScale->SetParent(this);
  this->OffsetScale->SetLabelText("Offset: ");
  this->OffsetScale->Create();

  this->FieldOfViewEntry->SetParent(this);
  this->FieldOfViewEntry->SetLabelText("FOV: ");
  this->FieldOfViewEntry->Create();
  this->FieldOfViewEntry->GetWidget()->SetIncrement (1);
  this->FieldOfViewEntry->GetWidget()->SetRange (0., 100000.);

  this->OrientationMenu->SetParent(this);
  this->OrientationMenu->SetLabelText("Orientation: ");
  this->OrientationMenu->Create();
  
  vtkKWMenuButton *mb = this->OrientationMenu->GetWidget();
  vtkKWMenu *m = mb->GetMenu();
  m->DeleteAllItems ( );

  int index;
  m->AddRadioButton ( "Axial" );
  index = m->AddRadioButton ("Axial", this, "SetOrientationFromMenu");
  m->SetItemHelpString(index, "Axial view (parallel to the floor)");
  
  m->AddRadioButton ("Sagittal");
  index = m->AddRadioButton ("Sagittal", this, "SetOrientationFromMenu");
  m->SetItemHelpString(index, "Sagittal view (side view)" );
  
  m->AddRadioButton ("Coronal");
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
  
  // TODO: this should probably be done in kwwidgets
  // (make the Return key trigger the callback)
  char *command = NULL;
  this->SetObjectMethodCommand(&command, this->FieldOfViewEntry->GetWidget(), "CommandCallback");
  this->Script("bind %s <Return> \"%s\"", 
          this->FieldOfViewEntry->GetWidget()->GetWidgetName(), command);
  delete [] command;
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
  if ( !this->SliceNode )
    {
    return;
    }
  
  int modified = 0;

  double fov = this->SliceNode->GetFieldOfView()[0];
  if ( fov != this->FieldOfViewEntry->GetWidget()->GetValue() )
    {
    this->FieldOfViewEntry->GetWidget()->SetValue(fov);
    modified = 1;
    }
  
  double fovover2 = this->SliceNode->GetFieldOfView()[2] / 2.;
  double min, max;
  this->OffsetScale->GetRange(min, max);
  if ( min != fovover2 || max != fovover2 )
    {
    this->OffsetScale->SetRange(-fovover2, fovover2);
    modified = 1;
    }



  // Since translation is a scalar multiple of the Pz column of the
  // SliceToRAS upper 3x3, find a non-zero entry in that column
  // and calculate the scalar from that.
  vtkMatrix4x4 *m = this->SliceNode->GetSliceToRAS();
  int i;
  double s;
  for (i = 0; i < 3; i++)
    {
    if ( m->GetElement( i, 2 ) != 0.0 )
      { 
      s = m->GetElement( i, 3 ) / m->GetElement( i, 2 ); 
      }
    }

  if ( s != this->OffsetScale->GetValue() )
  {
    this->OffsetScale->SetValue( s );
    modified = 1;
    }

  if ( modified )
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::SetOrientationFromMenu()
{

  // TODO: set the SliceToRAS matrix from the menu value

}


//----------------------------------------------------------------------------
void vtkSlicerSliceControlGUI::TransientApply()
{

  if ( !this->SliceNode )
    {
    return;
    }
  
  int modified = 0;

  // Set the Field of View from the Entry
  double val = this->FieldOfViewEntry->GetWidget()->GetValue();
  double nodeval[3];
  this->SliceNode->GetFieldOfView(nodeval[0], nodeval[1], nodeval[2]);
  if ( val != 0 && (val != nodeval[0] || val != nodeval[1] || val != nodeval[2]) )
    {
    this->SliceNode->SetFieldOfView(val, val, val);
    modified = 1;
    }

  // Set the Offset from the Scale
  // - transform the value by SliceToRAS so that the 
  //   slice's 'z' coordinate corresponds to the direction
  //   perpendicular to the current slice orientation
  // - basically, multiply the scale value times the z column vector of the
  //   rotation matrix
  vtkMatrix4x4 *newm = vtkMatrix4x4::New();
  newm->DeepCopy( this->SliceNode->GetSliceToRAS() );
  double in[4], out[4];
  in[0] = in[1] = in[3] = 0.;
  in[2] = (double) this->OffsetScale->GetValue();
  newm->MultiplyPoint(in, out);
  newm->SetElement( 0, 3, out[0] );
  newm->SetElement( 1, 3, out[1] );
  newm->SetElement( 2, 3, out[2] );
 
  if ( newm->GetElement( 0, 3 ) != this->SliceNode->GetSliceToRAS()->GetElement( 0, 3 ) ||
       newm->GetElement( 1, 3 ) != this->SliceNode->GetSliceToRAS()->GetElement( 1, 3 ) ||
       newm->GetElement( 2, 3 ) != this->SliceNode->GetSliceToRAS()->GetElement( 2, 3 ) )
    {
    this->SliceNode->GetSliceToRAS()->DeepCopy( newm );
    this->SliceNode->UpdateMatrices();
    modified = 1;
    }

  if ( modified )
    {
    this->Modified();
    }
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

