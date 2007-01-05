/*=========================================================================

  Module:    $RCSfile: vtkKWMatrix4x4.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCommand.h"

#include "vtkKWMatrix4x4.h"

#include "vtkKWMultiColumnList.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMatrix4x4 );
vtkCxxRevisionMacro(vtkKWMatrix4x4, "$Revision: 1.49 $");

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
#ifdef _DEBUG
    vtkDebugWithObjectMacro (__mrmlscene, << "*********MRMLCallback called recursively?" << endl);
#endif
    return;
    }
  inMRMLCallback = 1;

  vtkKWMatrix4x4 *self = reinterpret_cast<vtkKWMatrix4x4 *>(__clientData);

  self->UpdateWidget();

  inMRMLCallback = 0;
}


//----------------------------------------------------------------------------
vtkKWMatrix4x4::vtkKWMatrix4x4()
{
  this->Command = NULL;
  //this->StartCommand = NULL;
  //this->EndCommand   = NULL;

  this->Matrix4x4 = NULL;

  this->MultiColumnList = vtkKWMultiColumnList::New();

  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);

  this->UpdateWidget();
}

//----------------------------------------------------------------------------
vtkKWMatrix4x4::~vtkKWMatrix4x4()
{
  if (this->Command)
    {
    delete [] this->Command;
    }
  if ( this->Matrix4x4 )
    {
    this->Matrix4x4->RemoveObservers( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
  if (this->MRMLCallbackCommand)
    {
    this->MRMLCallbackCommand->Delete();
    this->MRMLCallbackCommand = NULL;
    }
  this->SetMatrix4x4(NULL);
  this->MultiColumnList->SetParent(NULL);
  this->MultiColumnList->Delete();

}

//----------------------------------------------------------------------------
void vtkKWMatrix4x4::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->MultiColumnList->SetParent(this);
  this->MultiColumnList->Create();
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->SetSelectionTypeToCell();

  int col;
  for (col = 0; col < 4; col++)
    {
    this->MultiColumnList->AddColumn(" ");
    this->MultiColumnList->SetColumnWidth(col, 7);
    this->MultiColumnList->SetColumnAlignmentToCenter(col);
    this->MultiColumnList->ColumnEditableOn(col);
    }

  // avoid the empty row of labels (and avoid accidental sorting!)
  this->MultiColumnList->SetColumnLabelsVisibility(0);

  this->Script("pack %s -fill both -expand true",
                 this->MultiColumnList->GetWidgetName());

  this->MultiColumnList->SetCellUpdatedCommand(this, "UpdateElement");

  // Override the column sorting behavior by always updating 
}

//----------------------------------------------------------------------------
void vtkKWMatrix4x4::UpdateWidget()
{
  this->MultiColumnList->DeleteAllRows();

  if (this->Matrix4x4 == NULL)
    {
    this->MultiColumnList->InsertCellText(0, 0, "NULL");
    return;
    }


  int row, col;
  for (row=0; row<4; row++)
    {
    for (col=0; col<4; col++)
      {
      this->MultiColumnList->InsertCellTextAsDouble(row, col, this->Matrix4x4->GetElement(row, col));
      this->MultiColumnList->SetCellEditWindowToSpinBox(row, col);
      }
    }
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkKWMatrix4x4::UpdateElement (int row, int col, double val)
{
  UpdateVTK();
}

//----------------------------------------------------------------------------
void vtkKWMatrix4x4::UpdateVTK()
{
  if (this->Matrix4x4 == NULL)
    {
    return;
    }


  int row, col;
  for (row=0; row<4; row++)
    {
    for (col=0; col<4; col++)
      {
      this->Matrix4x4->SetElement(row, col, this->MultiColumnList->GetCellTextAsDouble(row, col));
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMatrix4x4::SetCommand(vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->Command, object, method);
}


//----------------------------------------------------------------------------
void vtkKWMatrix4x4::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Matrix4x4: " << this->GetMatrix4x4() << endl;
}

//----------------------------------------------------------------------------
void vtkKWMatrix4x4::SetAndObserveMatrix4x4(vtkMatrix4x4 *matrix)
{
  if ( this->Matrix4x4 )
    {
    this->Matrix4x4->RemoveObserver( this->MRMLCallbackCommand );
    this->Matrix4x4->Delete ( );
    this->Matrix4x4 = NULL;
    }
  
  this->Matrix4x4 = matrix;

  if ( this->Matrix4x4 )
    {
    this->Matrix4x4->Register(this);
    this->Matrix4x4->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  this->UpdateWidget();
}
