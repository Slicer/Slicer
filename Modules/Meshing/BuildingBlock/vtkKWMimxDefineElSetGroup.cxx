/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDefineElSetGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.30.2.1 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkKWMimxDefineElSetGroup.h"

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkIntArray.h>
#include <vtkFieldData.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

#include <vtkKWApplication.h>
#include <vtkKWComboBox.h>
#include <vtkKWCheckButton.h>
#include <vtkKWCheckButtonSet.h>
#include <vtkKWEntry.h>
#include <vtkKWEntryWithLabel.h>
#include <vtkKWFrame.h>
#include <vtkKWIcon.h>
#include <vtkKWMenu.h>
#include <vtkKWMenuButton.h>
#include <vtkKWMenuButtonWithLabel.h>
#include <vtkKWMessageDialog.h>
#include <vtkKWPushButton.h>
#include <vtkKWPushButtonSet.h>
#include <vtkKWRadioButton.h>
#include <vtkKWRadioButtonSet.h>
#include <vtkKWRenderWidget.h>
#include <vtkKWScale.h>
#include <vtkKWScaleWithLabel.h>

#include "vtkMimxCreateElementSetWidgetFEMesh.h"
#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"

#include "vtkKWMimxCreateFEMeshFromBBGroup.h"

#include <vtkLinkedListWrapper.h>

#include "Resources/mimxClearSelect.h"
#include "Resources/mimxClear.h"
#include "Resources/mimxElementAll.h"   
#include "Resources/mimxElementIndividual.h"
#include "Resources/mimxElementFace.h"  
#include "Resources/mimxElementSurface.h"
#include "Resources/mimxElementVisible.h"
#include "Resources/mimxMiniAdd.h"
#include "Resources/mimxMiniSubtract.h"
#include "Resources/mimxSmallCancel.h"
#include "Resources/mimxSmallApply.h"

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDefineElSetGroup);
vtkCxxRevisionMacro(vtkKWMimxDefineElSetGroup, "$Revision: 1.30.2.1 $");

//----------------------------------------------------------------------------
vtkKWMimxDefineElSetGroup::vtkKWMimxDefineElSetGroup()
{
  this->ButtonFrame = NULL;
  this->ModeFrame = NULL;
  this->SelectButtonSet = NULL;
  this->SetLabelEntry = NULL;
  this->CancelButton = NULL;
  this->ApplyButton = NULL;
  this->SelectCellsWidget = NULL;
  this->SelectionState = -1;
  this->MeshActor = NULL;
  this->AddSubtractButton = NULL;
  this->ElementSetCombobox = NULL;
  this->OperationFrame = NULL;
  this->ClearSelectionButton = NULL;
  this->OpacityScale = NULL;
  this->DimensionState = 0;
  this->WithdrawOnApply = 0;
  this->ExtrudeFrame = NULL;
  this->ExtrudeLengthEntry = NULL;
  this->NumberOfDivisionsEntry = NULL;
  this->CreateFEMeshFromBBGroup = NULL;
  this->SurfaceOnlyMode = false;
}

//----------------------------------------------------------------------------
vtkKWMimxDefineElSetGroup::~vtkKWMimxDefineElSetGroup()
{
  if (this->SelectButtonSet)
    this->SelectButtonSet->Delete();
  if (this->SetLabelEntry)
    this->SetLabelEntry->Delete();
  if (this->CancelButton)
    this->CancelButton->Delete();
  if (this->ApplyButton)
    this->ApplyButton->Delete();
  if (this->ButtonFrame)
    this->ButtonFrame->Delete();
  if (this->ModeFrame)
    this->ModeFrame->Delete();
  if (this->AddSubtractButton)
    this->AddSubtractButton->Delete();
  if (this->OperationFrame)
    this->OperationFrame->Delete();
  if (this->ClearSelectionButton)
    this->ClearSelectionButton->Delete();
  if(this->OpacityScale)
    this->OpacityScale->Delete();
  if(this->ExtrudeFrame)
    this->ExtrudeFrame->Delete();
  if(this->ExtrudeLengthEntry)
    this->ExtrudeLengthEntry->Delete();
  if(this->NumberOfDivisionsEntry)
    this->NumberOfDivisionsEntry->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::CreateWidget()
{

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetResizable(1, 1);
  this->SetMinimumSize(275, 150);
  this->ModalOff( );
  this->SetMasterWindow(this->GetMimxMainWindow());
  this->SetTitle("Define Element Set(s)");
   
  if (!this->OperationFrame)
    this->OperationFrame = vtkKWFrame::New();
  this->OperationFrame->SetParent( this );
  this->OperationFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
                                 this->OperationFrame->GetWidgetName());

  if (!this->SelectButtonSet)
    this->SelectButtonSet = vtkKWCheckButtonSet::New();
  this->SelectButtonSet->SetParent( this->OperationFrame );
  this->SelectButtonSet->Create();
  this->SelectButtonSet->PackHorizontallyOn();
  this->SelectButtonSet->SetMaximumNumberOfWidgetsInPackingDirection( 6 );
  
                        
  this->SelectButtonSet->AddWidget(0);
  this->SelectButtonSet->GetWidget(0)->SetBalloonHelpString("All Elements");
  this->SelectButtonSet->GetWidget(0)->SetCommand(this, "SelectElementsThroughCallback");
  vtkKWIcon *allIcon = vtkKWIcon::New();
  allIcon->SetImage( image_mimxElementAll, 
                     image_mimxElementAll_width, 
                     image_mimxElementAll_height, 
                     image_mimxElementAll_pixel_size);
  this->SelectButtonSet->GetWidget(0)->SetImageToIcon( allIcon );
  this->SelectButtonSet->GetWidget(0)->SetSelectImageToIcon( allIcon );
  this->SelectButtonSet->GetWidget(0)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(1);
  this->SelectButtonSet->GetWidget(1)->SetBalloonHelpString("Surface Elements");
  this->SelectButtonSet->GetWidget(1)->SetCommand(this, "SelectElementsSurfaceCallback");
  allIcon->Delete();

  vtkKWIcon *surfaceIcon = vtkKWIcon::New();
  surfaceIcon->SetImage(  image_mimxElementSurface, 
                          image_mimxElementSurface_width, 
                          image_mimxElementSurface_height, 
                          image_mimxElementSurface_pixel_size);
  this->SelectButtonSet->GetWidget(1)->SetImageToIcon( surfaceIcon );
  this->SelectButtonSet->GetWidget(1)->SetSelectImageToIcon( surfaceIcon );
  this->SelectButtonSet->GetWidget(1)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(2);
  this->SelectButtonSet->GetWidget(2)->SetBalloonHelpString("Visible Surface Elements");
  this->SelectButtonSet->GetWidget(2)->SetCommand(this, "SelectVisibleElementsSurfaceCallback");
  surfaceIcon->Delete();

  vtkKWIcon *visibleIcon = vtkKWIcon::New();
  visibleIcon->SetImage(  image_mimxElementVisible, 
                          image_mimxElementVisible_width, 
                          image_mimxElementVisible_height, 
                          image_mimxElementVisible_pixel_size);
  this->SelectButtonSet->GetWidget(2)->SetImageToIcon( visibleIcon );
  this->SelectButtonSet->GetWidget(2)->SetSelectImageToIcon( visibleIcon );
  this->SelectButtonSet->GetWidget(2)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(3);
  this->SelectButtonSet->GetWidget(3)->SetBalloonHelpString("Elements Associated with a Block Face");
  this->SelectButtonSet->GetWidget(3)->SetCommand(this, "SelectSingleElementCallback");
  vtkKWIcon *faceIcon = vtkKWIcon::New();
  faceIcon->SetImage(  image_mimxElementFace, 
                       image_mimxElementFace_width, 
                       image_mimxElementFace_height, 
                       image_mimxElementFace_pixel_size);
  this->SelectButtonSet->GetWidget(3)->SetImageToIcon( faceIcon );
  this->SelectButtonSet->GetWidget(3)->SetSelectImageToIcon( faceIcon );
  this->SelectButtonSet->GetWidget(3)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(4);
  this->SelectButtonSet->GetWidget(4)->SetBalloonHelpString("Single Element");
  this->SelectButtonSet->GetWidget(4)->SetCommand(this, "SelectMultipleElementsCallback");
  visibleIcon->Delete();

  vtkKWIcon *elementIcon = vtkKWIcon::New();
  elementIcon->SetImage(  image_mimxElementIndividual, 
                          image_mimxElementIndividual_width, 
                          image_mimxElementIndividual_height, 
                          image_mimxElementIndividual_pixel_size);
  this->SelectButtonSet->GetWidget(4)->SetImageToIcon( elementIcon );
  this->SelectButtonSet->GetWidget(4)->SetSelectImageToIcon( elementIcon );
  this->SelectButtonSet->GetWidget(4)->SetBorderWidth( 2 );
 
  this->GetApplication()->Script("pack %s -side left -anchor nw -padx 2 -pady 4", 
                                 this->SelectButtonSet->GetWidgetName());
  elementIcon->Delete();
  
  vtkKWIcon *addIcon = vtkKWIcon::New();
  addIcon->SetImage( image_mimxMiniAdd, 
                     image_mimxMiniAdd_width, 
                     image_mimxMiniAdd_height, 
                     image_mimxMiniAdd_pixel_size);
  vtkKWIcon *subtractIcon = vtkKWIcon::New();
  subtractIcon->SetImage(  image_mimxMiniSubtract, 
                           image_mimxMiniSubtract_width, 
                           image_mimxMiniSubtract_height,
                           image_mimxMiniSubtract_pixel_size);
                           
  if (!this->AddSubtractButton)
    this->AddSubtractButton = vtkKWRadioButtonSet::New();
  this->AddSubtractButton->SetParent( this->OperationFrame );
  this->AddSubtractButton->Create();
  this->AddSubtractButton->AddWidget(0);
  this->AddSubtractButton->GetWidget(0)->SetImageToIcon( addIcon );
  this->AddSubtractButton->GetWidget(0)->SetSelectImageToIcon( addIcon );
  this->AddSubtractButton->GetWidget(0)->SetValueAsInt(1);
  this->AddSubtractButton->GetWidget(0)->SetBalloonHelpString("Add elements to the set");
  this->AddSubtractButton->GetWidget(0)->SetCommand(this, "AddElementsCallback");
  this->AddSubtractButton->GetWidget(0)->SetBorderWidth( 2 );
  
  this->AddSubtractButton->AddWidget(1);
  this->AddSubtractButton->GetWidget(1)->SetImageToIcon( subtractIcon );
  this->AddSubtractButton->GetWidget(1)->SetSelectImageToIcon( subtractIcon );
  this->AddSubtractButton->GetWidget(1)->SetValueAsInt(0);
  this->AddSubtractButton->GetWidget(1)->SetBalloonHelpString("Remove elements from the set");
  this->AddSubtractButton->GetWidget(1)->SetCommand(this, "SubtractElementsCallback");
  this->AddSubtractButton->GetWidget(1)->SetBorderWidth( 2 );
  this->GetApplication()->Script("pack %s -side right -anchor nw -expand n -padx 12 -pady 4", 
                                 this->AddSubtractButton->GetWidgetName());

  addIcon->Delete();
  subtractIcon->Delete();

  if (!this->ModeFrame)
    this->ModeFrame = vtkKWFrame::New();
  this->ModeFrame->SetParent( this );
  this->ModeFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
                                 this->ModeFrame->GetWidgetName());
          
  if (!this->ClearSelectionButton)
    this->ClearSelectionButton = vtkKWPushButton::New();
  this->ClearSelectionButton->SetParent( this->ModeFrame );
  this->ClearSelectionButton->Create();
  this->ClearSelectionButton->SetCommand(this, "RemoveSelectionsCallback");
  this->ClearSelectionButton->SetReliefToFlat( );
  vtkKWIcon *clearIcon = vtkKWIcon::New();
  clearIcon->SetImage(  image_mimxClear, 
                        image_mimxClear_width, 
                        image_mimxClear_height, 
                        image_mimxClear_pixel_size);
  this->ClearSelectionButton->SetImageToIcon( clearIcon );
  this->GetApplication()->Script("pack %s -side right -anchor ne -padx 2 -pady 2", 
                                 this->ClearSelectionButton->GetWidgetName());
  clearIcon->Delete();
 
  if(!this->OpacityScale)
    this->OpacityScale = vtkKWScaleWithLabel::New();
  this->OpacityScale->SetParent(this->ModeFrame);
  this->OpacityScale->Create();
  this->OpacityScale->GetWidget()->SetRange(0.0, 1.0);
  this->OpacityScale->GetWidget()->SetResolution(0.05);
  this->OpacityScale->GetWidget()->SetLength(100);
  this->OpacityScale->SetLabelText("Opacity");
  this->OpacityScale->SetLabelPositionToLeft();
  this->OpacityScale->GetWidget()->SetValue(1.0);
  this->OpacityScale->GetWidget()->SetCommand(this, "OpacityCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor e -padx 2 -pady 2", 
    this->OpacityScale->GetWidgetName());

  if (!this->ExtrudeFrame)      
    this->ExtrudeFrame = vtkKWFrame::New();
  this->ExtrudeFrame->SetParent(this);
  this->ExtrudeFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
    this->ExtrudeFrame->GetWidgetName());


  if (!this->ExtrudeLengthEntry)
    this->ExtrudeLengthEntry = vtkKWEntryWithLabel::New();
  this->ExtrudeLengthEntry->SetParent(this->ExtrudeFrame);
  this->ExtrudeLengthEntry->Create();
  this->ExtrudeLengthEntry->SetWidth(5); 
  this->ExtrudeLengthEntry->SetLabelText("Extrusion Length : ");
  this->ExtrudeLengthEntry->SetLabelPositionToTop();
  this->ExtrudeLengthEntry->GetWidget()->SetRestrictValueToDouble();
  this->ExtrudeLengthEntry->GetWidget()->SetValueAsDouble(1.0);
  this->ExtrudeLengthEntry->GetWidget()->SetWidth(10); 
  this->GetApplication()->Script(
    "pack %s -side left -anchor n -padx 2 -pady 2", 
    this->ExtrudeLengthEntry->GetWidgetName());

  if (!this->NumberOfDivisionsEntry)
    this->NumberOfDivisionsEntry = vtkKWEntryWithLabel::New();
  this->NumberOfDivisionsEntry->SetParent(this->ExtrudeFrame);
  this->NumberOfDivisionsEntry->Create();
  this->NumberOfDivisionsEntry->SetWidth(5); 
  this->NumberOfDivisionsEntry->SetLabelText("Number of Divisions : ");
  this->NumberOfDivisionsEntry->SetLabelPositionToTop();
  this->NumberOfDivisionsEntry->GetWidget()->SetRestrictValueToInteger();
  this->NumberOfDivisionsEntry->GetWidget()->SetValueAsInt(1);
  this->NumberOfDivisionsEntry->GetWidget()->SetWidth(10); 
  this->GetApplication()->Script(
    "pack %s -side left -anchor n -padx 2 -pady 2", 
    this->NumberOfDivisionsEntry->GetWidgetName());

  if (!this->ButtonFrame)
    this->ButtonFrame = vtkKWFrame::New();
  this->ButtonFrame->SetParent( this );
  this->ButtonFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
                                 this->ButtonFrame->GetWidgetName());
        
  if (!this->SetLabelEntry)
    this->SetLabelEntry = vtkKWEntryWithLabel::New();
  this->SetLabelEntry->SetParent( this->ButtonFrame );
  this->SetLabelEntry->Create();
  this->SetLabelEntry->SetLabelText("Set Label");
  this->SetLabelEntry->SetLabelPositionToLeft();
  this->GetApplication()->Script("pack %s -side left -anchor n -padx 2 -pady 4", 
                                 this->SetLabelEntry->GetWidgetName());
          
          
  if (!this->ApplyButton)
    this->ApplyButton = vtkKWPushButton::New();
  this->ApplyButton->SetParent( this->ButtonFrame );
  this->ApplyButton->Create();
  vtkKWIcon *applyIcon = vtkKWIcon::New();
  applyIcon->SetImage(  image_mimxSmallApply, 
                        image_mimxSmallApply_width, 
                        image_mimxSmallApply_height, 
                        image_mimxSmallApply_pixel_size);
  this->ApplyButton->SetImageToIcon( applyIcon );
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetCommand(this, "ApplyButtonCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor sw -expand y -padx 2 -pady 0", 
    this->ApplyButton->GetWidgetName());
  applyIcon->Delete();
              
  if (!this->CancelButton)
    this->CancelButton = vtkKWPushButton::New();
  this->CancelButton->SetParent( this->ButtonFrame );
  this->CancelButton->Create();
  vtkKWIcon *cancelIcon = vtkKWIcon::New();
  cancelIcon->SetImage(  image_mimxSmallCancel, 
                         image_mimxSmallCancel_width, 
                         image_mimxSmallCancel_height, 
                         image_mimxSmallCancel_pixel_size);
  this->CancelButton->SetImageToIcon( cancelIcon );
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetCommand(this, "Withdraw");
  this->GetApplication()->Script(
    "pack %s -side right -anchor se -expand y -padx 2 -pady 0", 
    this->CancelButton->GetWidgetName()); 
  cancelIcon->Delete();
        
  this->AddSubtractButton->GetWidget(0)->SetSelectedState(1);
        
  if(this->DimensionState)
    {
    this->SelectButtonSet->GetWidget(0)->SetEnabled(0);
    this->SelectButtonSet->Modified();
    }
  else
    {
    this->GetApplication()->Script("pack forget %s", this->ExtrudeFrame->GetWidgetName());
    }
    
}
//----------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
  if(this->DimensionState)
    {
    if (this->SurfaceOnlyMode == false)
      this->SelectButtonSet->GetWidget(0)->SetEnabled(0);
    this->SelectButtonSet->Modified();
    }
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SetMimxMainWindow(vtkKWMimxMainWindow *window)
{
  this->MimxMainWindow = window;
}
//----------------------------------------------------------------------------
vtkKWMimxMainWindow* vtkKWMimxDefineElSetGroup::GetMimxMainWindow( )
{
  return this->MimxMainWindow;
}
//----------------------------------------------------------------------------
int vtkKWMimxDefineElSetGroup::EnableElementSelection(int mode)
{
  this->SelectionState = mode;
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if ( MeshActor == NULL )
    {
    callback->ErrorMessage("Mesh selection required");
    return 0;
    }

  vtkUnstructuredGrid *ugrid = this->MeshActor->GetDataSet();
  if(!this->SelectCellsWidget)
    {
    this->SelectCellsWidget = vtkMimxCreateElementSetWidgetFEMesh::New();
    this->SelectCellsWidget->SetInteractor(this->GetMimxMainWindow()->
                                           GetRenderWidget()->GetRenderWindowInteractor());
    if(this->DimensionState)
      {
      this->SelectCellsWidget->SetDimensionState(this->DimensionState);
      if (this->SurfaceOnlyMode == false)
        this->SelectButtonSet->GetWidget(0)->SetEnabled(0);
      }
    this->SelectCellsWidget->SetInput(ugrid);
    }
        
  if(this->AddSubtractButton->GetWidget(0)->GetSelectedState())
    {
    this->SelectCellsWidget->SetBooleanState(vtkMimxCreateElementSetWidgetFEMesh::Add);
    }
  else
    {
    this->SelectCellsWidget->SetBooleanState(vtkMimxCreateElementSetWidgetFEMesh::Subtract);
    }
  if(this->OpacityScale)
    {
    this->SelectCellsWidget->SetInputOpacity(this->OpacityScale->GetWidget()->GetValue());
    }

  if(this->GetSelectedObject())
    {
    this->GetSelectedObject()->HideMesh();
    }
  this->SelectCellsWidget->SetEnabled(1);
  this->SelectCellsWidget->SetCellSelectionState(mode);
        
  vtkIntArray *meshseedarray = vtkIntArray::SafeDownCast(ugrid->GetFieldData()->GetArray("Mesh_Seed"));
  if(meshseedarray)
    {
    this->SelectButtonSet->GetWidget(3)->SetEnabled(1);
    }
  else
    this->SelectButtonSet->GetWidget(3)->SetEnabled(0);
  // check for the type of mesh. if mixed mesh remove surface elements, if all surface elements retain the
  // elements. 
  vtkCellTypes *cellTypes = vtkCellTypes::New();
  this->MeshActor->GetDataSet()->GetCellTypes(cellTypes);
  if(cellTypes->GetNumberOfTypes() == 1)
    {
    if(cellTypes->GetCellType(0) == VTK_QUAD ||
       cellTypes->GetCellType(0) == VTK_TRIANGLE)
      {
      this->SelectButtonSet->GetWidget(0)->SetEnabled(0);
      }
    else
      {
      if (this->SurfaceOnlyMode == false)
        this->SelectButtonSet->GetWidget(0)->SetEnabled(1);
      }
    }
  else
    {
    if (this->SurfaceOnlyMode == false)
      this->SelectButtonSet->GetWidget(0)->SetEnabled(1);
    }
  cellTypes->Delete();
  return 1;
}

//----------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::DisableElementSelection(int vtkNotUsed(mode))
{
  this->SelectionState = -1;
  if(this->SelectCellsWidget)
    {
    if(this->SelectCellsWidget->GetEnabled())
      {
      this->SelectCellsWidget->SetEnabled(0);
      }
    this->SelectCellsWidget->Delete();
    this->SelectCellsWidget = NULL;
    }
  if(this->GetSelectedObject())
    {
    this->GetSelectedObject()->ShowMesh();
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
    }
}


//--------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SelectElementsThroughCallback(int mode)
{
  if ( mode )
    {
    this->EnableElementSelection(0);
    for (int i=0;i<5;i++)
      {
      if ((i != 0) && (this->SelectButtonSet->GetWidget(i)->GetSelectedState()))
        {
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
        }
      }
    }
}
//---------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SelectElementsSurfaceCallback(int mode)
{
  if ( mode )
    {
    this->EnableElementSelection(1);
    for (int i=0;i<5;i++)
      {
      if ((i != 1) && (this->SelectButtonSet->GetWidget(i)->GetSelectedState()))
        {
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
        }
      }
    }
}
//---------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SelectVisibleElementsSurfaceCallback(int mode)
{
  if ( mode )
    {
    this->EnableElementSelection(2);
    for (int i=0;i<5;i++)
      {
      if ((i != 2) && (this->SelectButtonSet->GetWidget(i)->GetSelectedState()))
        {
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
        }
      }
    }
}
//---------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SelectSingleElementCallback(int mode)
{
  if ( mode )
    {
    this->EnableElementSelection(3);
    for (int i=0;i<5;i++)
      {
      if ((i != 3) && (this->SelectButtonSet->GetWidget(i)->GetSelectedState()))
        {
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
        }
      }
    }
}
//---------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SelectMultipleElementsCallback(int mode)
{
  if ( mode )
    {
    this->EnableElementSelection(4);
    for (int i=0;i<5;i++)
      {
      if ((i != 4) && (this->SelectButtonSet->GetWidget(i)->GetSelectedState()))
        {
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
        }
      }
    }
}
//---------------------------------------------------------------------------------
vtkMimxMeshActor* vtkKWMimxDefineElSetGroup::GetSelectedObject()
{
  return this->MeshActor;
}
//---------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SetMeshActor( vtkMimxMeshActor *actor )
{
  this->MeshActor = actor;
        
  vtkUnstructuredGrid *ugrid = actor->GetDataSet();
           
  int meshType = VTK_HEXAHEDRON;
  if (ugrid->GetNumberOfCells() > 0 )
    meshType = ugrid->GetCellType( 0 );

  vtkFieldData *fielddata = ugrid->GetFieldData();
  vtkIntArray *meshseed = vtkIntArray::SafeDownCast(fielddata->GetArray("Mesh_Seed"));
  switch ( meshType )
    {
    case VTK_HEXAHEDRON:
      if(!meshseed)   this->SelectButtonSet->GetWidget(3)->SetStateToDisabled();
      else this->SelectButtonSet->GetWidget(3)->SetStateToNormal();
      break;
    case VTK_TETRA:
      this->SelectButtonSet->GetWidget(0)->SetStateToNormal();
      this->SelectButtonSet->GetWidget(3)->SetStateToNormal();
      break;
    case VTK_QUAD:
      if(!meshseed)   this->SelectButtonSet->GetWidget(3)->SetStateToDisabled();
      else this->SelectButtonSet->GetWidget(3)->SetStateToNormal();
      break;
    case VTK_TRIANGLE:
      this->SelectButtonSet->GetWidget(0)->SetStateToDisabled();
      this->SelectButtonSet->GetWidget(3)->SetStateToDisabled();
      break;   
    }
  vtkCellTypes *cellTypes = vtkCellTypes::New();
  this->MeshActor->GetDataSet()->GetCellTypes(cellTypes);
  if(cellTypes->GetNumberOfTypes() == 1)
    {
    if(cellTypes->GetCellType(0) == VTK_QUAD ||
       cellTypes->GetCellType(0) == VTK_TRIANGLE)
      {
      this->SelectButtonSet->GetWidget(0)->SetEnabled(0);
      }
    else
      {
      if (this->SurfaceOnlyMode == false)
        this->SelectButtonSet->GetWidget(0)->SetEnabled(1);
      }
    }
  else
    {
    if (this->SurfaceOnlyMode == false)
      this->SelectButtonSet->GetWidget(0)->SetEnabled(1);
    }
  cellTypes->Delete();     
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::AddElementsCallback()
{
  if(this->SelectCellsWidget)
    {
    this->SelectCellsWidget->SetBooleanState(vtkMimxCreateElementSetWidgetFEMesh::Add);
    }
}
//--------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SubtractElementsCallback()
{
  if(this->SelectCellsWidget)
    {
    this->SelectCellsWidget->SetBooleanState(vtkMimxCreateElementSetWidgetFEMesh::Subtract);
    }
}
//--------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::CancelButtonCallback()
{

}
//---------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::RemoveSelectionsCallback( )
{
  this->ResetState();
  if (this->SelectCellsWidget)
    this->SelectCellsWidget->ClearSelections();
}
//---------------------------------------------------------------------------------------
int vtkKWMimxDefineElSetGroup::ApplyButtonCallback()
{
  if(this->DimensionState)
    {
    if(!this->CreateFEMeshFromBBGroup)      return 0;
    this->CreateFEMeshFromBBGroup->CreateFEMeshFromBBApplyCallback();
    return 1;
    }
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        
  vtkUnstructuredGrid *ugrid = this->MeshActor->GetDataSet();

  if (!this->SelectCellsWidget)
    {
    callback->ErrorMessage("Selection of elements should be made");
    return 0;
    }
  vtkIdList *idlist = this->SelectCellsWidget->GetSelectedCellIds();
  if(idlist->GetNumberOfIds() == 0)
    {
    callback->ErrorMessage("Number of elements selected is 0");
    return 0;
    }
  const char *elementset = this->SetLabelEntry->GetWidget()->GetValue();
  if(!strcmp(elementset, ""))
    {
    callback->ErrorMessage("Enter the element set name");
    return 0;
    }

  if(ugrid->GetCellData()->GetArray(elementset))
    {
    vtkKWMessageDialog *KWMessageDialog = vtkKWMessageDialog::New();
    KWMessageDialog->SetStyleToOkOtherCancel();
    KWMessageDialog->SetOKButtonText("Over Write");
    KWMessageDialog->SetOtherButtonText("Append");
    KWMessageDialog->SetApplication(this->GetApplication());
    KWMessageDialog->Create();
    KWMessageDialog->SetTitle("Your Attention Please!");
    KWMessageDialog->SetText("An element set with this name already exists, what would you prefer to do?");
    KWMessageDialog->Invoke();
    if(KWMessageDialog->GetStatus() == vtkKWDialog::StatusCanceled)
      {
      return 0;
      }
    if(KWMessageDialog->GetStatus() == vtkKWMessageDialog::StatusOther)
      {
      this->MeshActor->AppendElementSet(elementset, idlist);
      this->RemoveSelectionsCallback();
      this->SetLabelEntry->GetWidget()->SetValue("");
      return 1;
      }

    this->MeshActor->DeleteElementSet(elementset);
    if(this->ElementSetCombobox)
      this->ElementSetCombobox->DeleteValue(
        this->ElementSetCombobox->GetValueIndex(elementset));
    }
  int i;
  vtkIntArray *elementarray = vtkIntArray::New();
  elementarray->SetNumberOfValues(ugrid->GetNumberOfCells());

  for (i=0; i<ugrid->GetNumberOfCells(); i++) {
  elementarray->SetValue(i, 0);
  }

  for (i=0; i<ugrid->GetNumberOfCells(); i++)
    {
    if(idlist->IsId(i) != -1)
      {
      elementarray->SetValue(i,1);
      }
    }
  elementarray->SetName(elementset);
  ugrid->GetCellData()->AddArray(elementarray);

  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));
  if (! stringarray )
    {
    stringarray = vtkStringArray::New();
    stringarray->SetName("Element_Set_Names");
    ugrid->GetFieldData()->AddArray( stringarray );
    stringarray->Delete();
    }
  stringarray->InsertNextValue(elementset);
  this->MeshActor->AddElementSetListItem(elementset);
  elementarray->Delete();
  if(this->ElementSetCombobox)
    this->ElementSetCombobox->AddValue(elementset);
//      this->MeshActor->CreateElementSetList();
  this->SetLabelEntry->GetWidget()->SetValue("");
  this->RemoveSelectionsCallback();
  this->GetMimxMainWindow()->SetStatusText("Created Element Set");
        
  if (this->WithdrawOnApply)
    this->Withdraw();
  return 1;
}
//---------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::Withdraw()
{
  this->ResetState();
  this->Superclass::Withdraw();
}
//----------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::OpacityCallback(double Val)
{
  if(!this->SelectCellsWidget)    return;
  this->SelectCellsWidget->SetInputOpacity(Val);
  if(Val != 1.0)
    {
    this->SelectButtonSet->GetWidget(3)->SetEnabled(0);
    }
  else{
  this->SelectButtonSet->GetWidget(3)->SetEnabled(1);
  }

  this->MimxMainWindow->GetRenderWidget()->Render();
}
//-------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::ResetState()
{
  if(this->AddSubtractButton)     this->AddSubtractButton->GetWidget(0)->SetSelectedState(1);
  if(this->OpacityScale)  this->OpacityScale->GetWidget()->SetValue(1.0);
  this->DisableElementSelection(0);
  if(this->SelectButtonSet)
    for (int i=0; i<this->SelectButtonSet->GetNumberOfWidgets(); i++)
      {
      if(this->SelectButtonSet->GetWidget(i)->GetSelectedState())
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
      }
}
//-------------------------------------------------------------------------------------------
vtkUnstructuredGrid* vtkKWMimxDefineElSetGroup::GetSelectedMesh()
{
  if(this->SelectCellsWidget)
    return this->SelectCellsWidget->GetSelectedGrid();
  else
    return  NULL;
}
//-------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SetDialogTitle(const char *title)
{
  this->SetTitle( title );
}
//-------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SetModeSurfaceOnly( )
{
  this->SurfaceOnlyMode = true;
  this->SelectButtonSet->GetWidget(0)->SetStateToDisabled();
}
//-------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SetModeAll( )
{
  this->SurfaceOnlyMode = false;
  for (int i=0; i<this->SelectButtonSet->GetNumberOfWidgets(); i++)
    {
    this->SelectButtonSet->GetWidget(i)->SetEnabled(1);
    }
}
//-------------------------------------------------------------------------------------------
double vtkKWMimxDefineElSetGroup::GetExtrusionLength()
{
  double length = 0.0;
  if(this->ExtrudeLengthEntry)
    {
    length = this->ExtrudeLengthEntry->GetWidget()->GetValueAsDouble();
    }
  return length;
}
//-------------------------------------------------------------------------------------------
int vtkKWMimxDefineElSetGroup::GetNumberOfDivisions()
{
  int numdivisions = 0;
  if(this->NumberOfDivisionsEntry)
    {
    numdivisions = this->NumberOfDivisionsEntry->GetWidget()->GetValueAsInt();
    }
  return numdivisions;
}
//--------------------------------------------------------------------------------------------
void vtkKWMimxDefineElSetGroup::SetCreateFEMeshFromBBGroup(vtkKWMimxCreateFEMeshFromBBGroup *Group)
{
  this->CreateFEMeshFromBBGroup = Group;
}
//---------------------------------------------------------------------------------------------
