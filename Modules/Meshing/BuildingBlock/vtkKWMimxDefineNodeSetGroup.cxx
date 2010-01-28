/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDefineNodeSetGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.15.2.1 $

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

#include "vtkKWMimxDefineNodeSetGroup.h"

#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkFieldData.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkLinkedListWrapper.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStringArray.h>

#include <vtkKWApplication.h>
#include <vtkKWCheckButton.h>
#include <vtkKWCheckButtonSet.h>
#include <vtkKWComboBox.h>
#include <vtkKWEntry.h>
#include <vtkKWEntryWithLabel.h>
#include <vtkKWFrame.h>
#include <vtkKWIcon.h>
#include <vtkKWMenuButtonWithLabel.h>
#include <vtkKWMessageDialog.h>
#include <vtkKWPushButton.h>
#include <vtkKWPushButtonSet.h>
#include <vtkKWRadioButton.h>
#include <vtkKWRadioButtonSet.h>
#include <vtkKWRenderWidget.h>
#include <vtkKWScale.h>
#include <vtkKWScaleWithLabel.h>

#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxSelectPointsWidget.h"

#include "vtkKWMimxMainWindow.h"

#include "Resources/mimxClear.h"
#include "Resources/mimxDelete.h"
#include "Resources/mimxMiniAdd.h"
#include "Resources/mimxMiniSubtract.h"
#include "Resources/mimxNodeAll.h" 
#include "Resources/mimxNodeFace.h"  
#include "Resources/mimxNodeIndividual.h"
#include "Resources/mimxNodeSurface.h"
#include "Resources/mimxNodeVisible.h"
#include "Resources/mimxSmallApply.h"
#include "Resources/mimxSmallCancel.h"

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDefineNodeSetGroup);
vtkCxxRevisionMacro(vtkKWMimxDefineNodeSetGroup, "$Revision: 1.15.2.1 $");

//----------------------------------------------------------------------------
vtkKWMimxDefineNodeSetGroup::vtkKWMimxDefineNodeSetGroup()
{
  this->ButtonFrame = NULL;
  this->ModeFrame = NULL;
  this->SelectButtonSet = NULL;
  this->SetLabelEntry = NULL;
  this->CancelButton = NULL;
  this->ApplyButton = NULL;
  this->SelectPointsWidget = NULL;
  this->SelectionState = -1;
  this->MeshActor = NULL;
  this->OpacityScale = NULL;
  this->AddSubtractButton = NULL;
  this->OperationFrame = NULL;
  this->ClearSelectionButton = NULL;
  this->NodeSetCombobox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxDefineNodeSetGroup::~vtkKWMimxDefineNodeSetGroup()
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
  if (this->SelectPointsWidget)
    this->SelectPointsWidget->Delete();
  if(this->OpacityScale)
    this->OpacityScale->Delete();
  if(this->AddSubtractButton)
    this->AddSubtractButton->Delete();
  if(this->OperationFrame)
    this->OperationFrame->Delete();
  if(this->ClearSelectionButton)
    this->ClearSelectionButton->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::CreateWidget()
{

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetResizable(1, 1);
  this->SetMinimumSize(250, 125);
  this->ModalOff( );
  this->SetMasterWindow(this->GetMimxMainWindow());
  this->SetTitle("Define Node Set(s)");
  
  if (!this->ModeFrame)
    this->ModeFrame = vtkKWFrame::New();
  this->ModeFrame->SetParent( this );
  this->ModeFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
                                 this->ModeFrame->GetWidgetName());
  
  if (!this->SelectButtonSet)
    this->SelectButtonSet = vtkKWCheckButtonSet::New();
  this->SelectButtonSet->SetParent( this->ModeFrame );
  this->SelectButtonSet->Create();
  this->SelectButtonSet->PackHorizontallyOn();
  this->SelectButtonSet->SetMaximumNumberOfWidgetsInPackingDirection( 5 );
  
                        
  this->SelectButtonSet->AddWidget(0);
  this->SelectButtonSet->GetWidget(0)->SetBalloonHelpString("All Nodes");
  this->SelectButtonSet->GetWidget(0)->SetCommand(this, "SelectNodesThroughCallback");
  vtkKWIcon *allIcon = vtkKWIcon::New();
  allIcon->SetImage( image_mimxNodeAll, 
                     image_mimxNodeAll_width, 
                     image_mimxNodeAll_height, 
                     image_mimxNodeAll_pixel_size);
  this->SelectButtonSet->GetWidget(0)->SetImageToIcon( allIcon );
  this->SelectButtonSet->GetWidget(0)->SetSelectImageToIcon( allIcon );
  this->SelectButtonSet->GetWidget(0)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(1);
  this->SelectButtonSet->GetWidget(1)->SetBalloonHelpString("Surface Nodes");
  this->SelectButtonSet->GetWidget(1)->SetCommand(this, "SelectNodesSurfaceCallback");
  allIcon->Delete();

  vtkKWIcon *surfaceIcon = vtkKWIcon::New();
  surfaceIcon->SetImage(  image_mimxNodeSurface, 
                          image_mimxNodeSurface_width, 
                          image_mimxNodeSurface_height, 
                          image_mimxNodeSurface_pixel_size);
  this->SelectButtonSet->GetWidget(1)->SetImageToIcon( surfaceIcon );
  this->SelectButtonSet->GetWidget(1)->SetSelectImageToIcon( surfaceIcon );
  this->SelectButtonSet->GetWidget(1)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(2);
  this->SelectButtonSet->GetWidget(2)->SetBalloonHelpString("Visible Surface Nodes");
  this->SelectButtonSet->GetWidget(2)->SetCommand(this, "SelectVisibleNodesSurfaceCallback");
  vtkKWIcon *visibleIcon = vtkKWIcon::New();
  visibleIcon->SetImage(  image_mimxNodeVisible, 
                          image_mimxNodeVisible_width, 
                          image_mimxNodeVisible_height, 
                          image_mimxNodeVisible_pixel_size);
  this->SelectButtonSet->GetWidget(2)->SetImageToIcon( visibleIcon );
  this->SelectButtonSet->GetWidget(2)->SetSelectImageToIcon( visibleIcon );
  this->SelectButtonSet->GetWidget(2)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(3);
  this->SelectButtonSet->GetWidget(3)->SetBalloonHelpString("Nodes Associated with a Block Face");
  this->SelectButtonSet->GetWidget(3)->SetCommand(this, "SelectFaceNodeCallback");
  visibleIcon->Delete();

  vtkKWIcon *faceIcon = vtkKWIcon::New();
  faceIcon->SetImage(  image_mimxNodeFace, 
                       image_mimxNodeFace_width, 
                       image_mimxNodeFace_height, 
                       image_mimxNodeFace_pixel_size);
  this->SelectButtonSet->GetWidget(3)->SetImageToIcon( faceIcon );
  this->SelectButtonSet->GetWidget(3)->SetSelectImageToIcon( faceIcon );
  this->SelectButtonSet->GetWidget(3)->SetBorderWidth( 2 );
  
  this->SelectButtonSet->AddWidget(4);
  this->SelectButtonSet->GetWidget(4)->SetBalloonHelpString("Single Node");
  this->SelectButtonSet->GetWidget(4)->SetCommand(this, "SelectSingleNodeCallback");
  faceIcon->Delete();

  vtkKWIcon *elementIcon = vtkKWIcon::New();
  elementIcon->SetImage(  image_mimxNodeIndividual, 
                          image_mimxNodeIndividual_width, 
                          image_mimxNodeIndividual_height, 
                          image_mimxNodeIndividual_pixel_size);
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
  this->AddSubtractButton->SetParent( this->ModeFrame );
  this->AddSubtractButton->Create();
  this->AddSubtractButton->AddWidget(0);
  this->AddSubtractButton->GetWidget(0)->SetImageToIcon( addIcon );
  this->AddSubtractButton->GetWidget(0)->SetSelectImageToIcon( addIcon );
  this->AddSubtractButton->GetWidget(0)->SetValueAsInt(1);
  this->AddSubtractButton->GetWidget(0)->SetBalloonHelpString("Add nodes to the set");
  this->AddSubtractButton->GetWidget(0)->SetCommand(this, "AddNodesCallback");
  this->AddSubtractButton->GetWidget(0)->SetBorderWidth( 2 );
  
  this->AddSubtractButton->AddWidget(1);
  this->AddSubtractButton->GetWidget(1)->SetImageToIcon( subtractIcon );
  this->AddSubtractButton->GetWidget(1)->SetSelectImageToIcon( subtractIcon );
  this->AddSubtractButton->GetWidget(1)->SetValueAsInt(0);
  this->AddSubtractButton->GetWidget(1)->SetBalloonHelpString("Remove nodes from the set");
  this->AddSubtractButton->GetWidget(1)->SetCommand(this, "SubtractNodesCallback");
  this->AddSubtractButton->GetWidget(1)->SetBorderWidth( 2 );
  this->GetApplication()->Script("pack %s -side right -anchor nw -expand n -padx 12 -pady 4", 
                                 this->AddSubtractButton->GetWidgetName());
  addIcon->Delete();
  subtractIcon->Delete();
        
  if (!this->OperationFrame)
    this->OperationFrame = vtkKWFrame::New();
  this->OperationFrame->SetParent( this );
  this->OperationFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
                                 this->OperationFrame->GetWidgetName());
            
  if (!this->ClearSelectionButton)
    this->ClearSelectionButton = vtkKWPushButton::New();
  this->ClearSelectionButton->SetParent( this->OperationFrame );
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
  this->OpacityScale->SetParent(this->OperationFrame);
  this->OpacityScale->Create();
  this->OpacityScale->GetWidget()->SetRange(0.0, 1.0);
  this->OpacityScale->GetWidget()->SetResolution(0.05);
  this->OpacityScale->GetWidget()->SetLength(100);
  this->OpacityScale->SetLabelText("Opacity");
  //this->OpacityScale->SetLabelPositionToLeft();
  this->OpacityScale->GetWidget()->SetValue(1.0);
  this->OpacityScale->GetWidget()->SetCommand(this, "OpacityCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -padx 2 -pady 2", 
    this->OpacityScale->GetWidgetName());
        
        
            
          
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
  this->GetApplication()->Script(
    "pack %s -side left -anchor w -padx 2 -pady 2", 
    this->SetLabelEntry->GetWidgetName()); 
          
          
  if (!this->ApplyButton)
    this->ApplyButton = vtkKWPushButton::New();
  this->ApplyButton->SetParent( this->ButtonFrame );
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat( );
  vtkKWIcon *applyIcon = vtkKWIcon::New();
  applyIcon->SetImage(  image_mimxSmallApply, 
                        image_mimxSmallApply_width, 
                        image_mimxSmallApply_height, 
                        image_mimxSmallApply_pixel_size);
  this->ApplyButton->SetImageToIcon( applyIcon );
  this->ApplyButton->SetCommand(this, "ApplyButtonCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor sw -expand y -padx 2 -pady 6", 
    this->ApplyButton->GetWidgetName());
  applyIcon->Delete();
              
  if (!this->CancelButton)
    this->CancelButton = vtkKWPushButton::New();
  this->CancelButton->SetParent( this->ButtonFrame );
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat( );
  vtkKWIcon *cancelIcon = vtkKWIcon::New();
  cancelIcon->SetImage(  image_mimxSmallCancel, 
                         image_mimxSmallCancel_width, 
                         image_mimxSmallCancel_height, 
                         image_mimxSmallCancel_pixel_size);
  this->CancelButton->SetImageToIcon( cancelIcon );
  this->CancelButton->SetCommand(this, "Withdraw");
  this->GetApplication()->Script(
    "pack %s -side right -anchor se -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName()); 
  cancelIcon->Delete();
        
  this->AddSubtractButton->GetWidget(0)->SetSelectedState(1);  
}
//----------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::SetMimxMainWindow(vtkKWMimxMainWindow *window)
{
  this->MimxMainWindow = window;
}
//----------------------------------------------------------------------------
vtkKWMimxMainWindow* vtkKWMimxDefineNodeSetGroup::GetMimxMainWindow( )
{
  return this->MimxMainWindow;
}
//----------------------------------------------------------------------------
int vtkKWMimxDefineNodeSetGroup::EnableNodeSelection(int mode)
{
  this->SelectionState = mode;
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if ( MeshActor == NULL )
    {
    callback->ErrorMessage("Mesh selection required");
    return 0;
    }

  vtkUnstructuredGrid *ugrid = this->MeshActor->GetDataSet();
  if (this->GetSelectedObject())
    {
    this->GetSelectedObject()->HideMesh();
    }
        
  if (!this->SelectPointsWidget)
    {
    this->SelectPointsWidget = vtkMimxSelectPointsWidget::New();
    this->SelectPointsWidget->SetInteractor(this->GetMimxMainWindow()->
                                            GetRenderWidget()->GetRenderWindowInteractor());
    }
        
  if (!this->SelectPointsWidget->GetEnabled())
    {
    this->SelectPointsWidget->SetInput(ugrid);
    this->SelectPointsWidget->SetEnabled(1);
    }
  this->SelectPointsWidget->SetPointSelectionState(mode);
  switch (this->AddSubtractButton->GetWidget(0)->GetSelectedState())
    {
    case 0: this->SelectPointsWidget->SetBooleanState(1); break;
    case 1: this->SelectPointsWidget->SetBooleanState(0); break;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::DisableNodeSelection(int vtkNotUsed(mode))
{
  this->SelectionState = -1;
  if(this->SelectPointsWidget)
    {
    if(this->SelectPointsWidget->GetEnabled())
      {
      this->SelectPointsWidget->SetEnabled(0);
      }
    this->SelectPointsWidget->Delete();
    this->SelectPointsWidget = NULL;
    }
  if (this->GetSelectedObject())
    {
    this->GetSelectedObject()->ShowMesh();
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
    }
}

//--------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::SelectNodesThroughCallback(int mode)
{
  if ( mode )
    {
    this->EnableNodeSelection(0);
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
void vtkKWMimxDefineNodeSetGroup::SelectNodesSurfaceCallback(int mode)
{
  if ( mode )
    {
    this->EnableNodeSelection(1);
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
void vtkKWMimxDefineNodeSetGroup::SelectVisibleNodesSurfaceCallback(int mode)
{
  if ( mode )
    {
    this->EnableNodeSelection(2);
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
void vtkKWMimxDefineNodeSetGroup::SelectFaceNodeCallback(int mode)
{
  if ( mode )
    {
    this->EnableNodeSelection(4);
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
void vtkKWMimxDefineNodeSetGroup::SelectSingleNodeCallback(int mode)
{
  if ( mode )
    {
    this->EnableNodeSelection(3);
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
vtkMimxMeshActor* vtkKWMimxDefineNodeSetGroup::GetSelectedObject()
{
  return this->MeshActor;
}
//---------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::SetMeshActor( vtkMimxMeshActor *actor )
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
      if(!meshseed) this->SelectButtonSet->GetWidget(3)->SetStateToDisabled();
      else this->SelectButtonSet->GetWidget(3)->SetStateToNormal();
      break;
    case VTK_TETRA:
      this->SelectButtonSet->GetWidget(0)->SetStateToNormal();
      this->SelectButtonSet->GetWidget(3)->SetStateToNormal();
      break;
    case VTK_QUAD:
      if(!meshseed) this->SelectButtonSet->GetWidget(3)->SetStateToDisabled();
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
      this->SelectButtonSet->GetWidget(0)->SetEnabled(1);
      }
    }
  else
    {
    this->SelectButtonSet->GetWidget(0)->SetEnabled(1);
    }
  cellTypes->Delete();      
}
//----------------------------------------------------------------------------
int vtkKWMimxDefineNodeSetGroup::ApplyButtonCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if ( this->MeshActor == NULL )
    {
    callback->ErrorMessage("No mesh was selected");
    return 0;
    }

  vtkUnstructuredGrid *ugrid = this->MeshActor->GetDataSet();
  if (!this->SelectPointsWidget)
    {
    callback->ErrorMessage("No nodes were selected for the element set");
    return 0;
    }
  vtkIdList *idlist = this->SelectPointsWidget->GetSelectedPointIds();
  if(idlist->GetNumberOfIds() == 0)
    {
    callback->ErrorMessage("No nodes were selected for the element set");
    return 0;
    }
  const char *nodeset = this->SetLabelEntry->GetWidget()->GetValue();
  if(!strcmp(nodeset, ""))
    {
    callback->ErrorMessage("No name was provided for the node set");
    return 0;
    }

  if(ugrid->GetPointData()->GetArray(nodeset))
    {
    vtkKWMessageDialog *KWMessageDialog = vtkKWMessageDialog::New();
    KWMessageDialog->SetStyleToOkOtherCancel();
    KWMessageDialog->SetOKButtonText("Overwrite");
    KWMessageDialog->SetOtherButtonText("Append");
    KWMessageDialog->SetApplication(this->GetApplication());
    KWMessageDialog->Create();
    KWMessageDialog->SetTitle("Your Attention Please!");
    KWMessageDialog->SetText("A node set with this name already exists, what would you prefer to do?");
    KWMessageDialog->Invoke();
    if(KWMessageDialog->GetStatus() == vtkKWDialog::StatusCanceled)
      {
      return 0;
      }
    if(KWMessageDialog->GetStatus() == vtkKWMessageDialog::StatusOther)
      {
      this->MeshActor->AppendNodeSet(nodeset, idlist);
      this->RemoveSelectionsCallback();
      this->SetLabelEntry->GetWidget()->SetValue("");
      return 1;
      }
    this->MeshActor->DeleteNodeSet(nodeset);
    }
  int i;
  vtkIntArray *nodearray = vtkIntArray::New();
  nodearray->SetNumberOfValues(ugrid->GetNumberOfPoints());

  for (i=0; i<ugrid->GetNumberOfPoints(); i++) 
    {
    nodearray->SetValue(i, 0);
    }

  for (i=0; i<ugrid->GetNumberOfPoints(); i++)
    {
    if(idlist->IsId(i) != -1)
      {
      nodearray->SetValue(i,1);
      }
    }
  
  this->GetMimxMainWindow()->SetStatusText("Created Node Set");
  
  nodearray->SetName(nodeset);
  ugrid->GetPointData()->AddArray(nodearray);

  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if (stringarray == NULL)
    {
    stringarray = vtkStringArray::New();
    stringarray->SetName( "Node_Set_Names" );
    ugrid->GetFieldData()->AddArray(stringarray);
    stringarray->Delete();
    } 
  stringarray->InsertNextValue(nodeset);
  nodearray->Delete();
  if(this->NodeSetCombobox)
    this->NodeSetCombobox->AddValue(nodeset);

  this->SetLabelEntry->GetWidget()->SetValue("");
  this->RemoveSelectionsCallback();

  return 1;
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::OpacityCallback(double Val)
{
  if(!this->SelectPointsWidget)   return;
  this->SelectPointsWidget->SetInputOpacity(Val);
  if(Val != 1.0)
    {
    this->SelectButtonSet->GetWidget(1)->SetEnabled(0);
    this->SelectButtonSet->GetWidget(2)->SetEnabled(0);
    this->SelectButtonSet->GetWidget(3)->SetEnabled(0);
    }
  else
    {
    this->SelectButtonSet->GetWidget(1)->SetEnabled(1);
    this->SelectButtonSet->GetWidget(2)->SetEnabled(1);
    this->SelectButtonSet->GetWidget(3)->SetEnabled(1);
    }
  if(!this->SelectPointsWidget->GetEnabled())     
    {
    //std::cout << "Select Points Widget is not enabled" << std::endl;
    return;
    }
  //std::cout << "Select Points Widget is enabled" << std::endl;
  this->MimxMainWindow->GetRenderWidget()->Render();
}
//--------------------------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::AddNodesCallback()
{
  if(this->SelectPointsWidget)
    {
    this->SelectPointsWidget->SetBooleanState(vtkMimxSelectPointsWidget::Add);
    }
}
//--------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::SubtractNodesCallback()
{
  if(this->SelectPointsWidget)
    {
    this->SelectPointsWidget->SetBooleanState(vtkMimxSelectPointsWidget::Subtract);
    }
}
//--------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::Withdraw()
{
  this->ResetState();
  this->Superclass::Withdraw();
}
//----------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::RemoveSelectionsCallback( )
{
  this->ResetState();
  if (this->SelectPointsWidget)
    this->SelectPointsWidget->ClearSelections();
        
}
//---------------------------------------------------------------------------------------
void vtkKWMimxDefineNodeSetGroup::ResetState()
{
  if(this->AddSubtractButton)     this->AddSubtractButton->GetWidget(0)->SetSelectedState(1);
  if(this->OpacityScale)  this->OpacityScale->GetWidget()->SetValue(1.0);
  this->DisableNodeSelection(0);
        
  if(this->SelectButtonSet)
    {
    int numwidgets = this->SelectButtonSet->GetNumberOfWidgets();
    int i;
    for (i=0; i<numwidgets; i++)
      {
      if(this->SelectButtonSet->GetWidget(i)->GetSelectedState())
        {
        this->SelectButtonSet->GetWidget(i)->SetSelectedState(0);
        }
      }
    }
        
}
//-------------------------------------------------------------------------------------------
