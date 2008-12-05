/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditFEMeshLaplacianSmoothGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.36.4.2 $

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

#include "vtkKWMimxEditFEMeshLaplacianSmoothGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"
#include "vtkMimxErrorCallback.h"

#include "vtkUnstructuredGridWriter.h"
#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxSmoothUnstructuredGridFilter.h"
#include "vtkMimxRecalculateInteriorNodes.h"

//#include "vtkUnstructuredGridWriter.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkRenderer.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkMimxMeshActor.h"


#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxEditFEMeshLaplacianSmoothGroup);
vtkCxxRevisionMacro(vtkKWMimxEditFEMeshLaplacianSmoothGroup, "$Revision: 1.36.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxEditFEMeshLaplacianSmoothGroup::vtkKWMimxEditFEMeshLaplacianSmoothGroup()
{
  this->FEMeshListComboBox = NULL;
  this->BBListComboBox = NULL;
  this->SurfaceListComboBox = NULL;
  this->SmoothIterations = vtkKWEntryWithLabel::New();
  this->RecalculateIterations = NULL;
  this->RecalculateCheckButton = NULL;
  this->OriginalPosition = NULL;
  this->EntryFrame = vtkKWFrameWithLabel::New();
  this->RecalculateCheckButtonBB = vtkKWCheckButtonWithLabel::New();
  this->ComponentFrame = NULL;
  this->InterpolationMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxEditFEMeshLaplacianSmoothGroup::~vtkKWMimxEditFEMeshLaplacianSmoothGroup()
{
  if(this->FEMeshListComboBox)
     this->FEMeshListComboBox->Delete();
  if(this->BBListComboBox)
    this->BBListComboBox->Delete();
  if(this->SurfaceListComboBox)
          this->SurfaceListComboBox->Delete();
  if(this->SmoothIterations)
    this->SmoothIterations->Delete();
  if(this->RecalculateIterations)
    this->RecalculateIterations->Delete();
  if(this->SurfaceListComboBox)
    this->RecalculateCheckButton->Delete();
  if(this->OriginalPosition)
          this->OriginalPosition->Delete();
  this->EntryFrame->Delete();
  this->RecalculateCheckButtonBB->Delete();
  if (this->ComponentFrame)
    this->ComponentFrame->Delete();
  if (this->InterpolationMenuButton)
    this->InterpolationMenuButton->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  if(!this->FEMeshListComboBox) 
  {
     this->FEMeshListComboBox = vtkKWComboBoxWithLabel::New();
  }
  if(!this->BBListComboBox)     
  {
    this->BBListComboBox = vtkKWComboBoxWithLabel::New();
  }

  if(!this->SurfaceListComboBox)        
  {
          this->SurfaceListComboBox = vtkKWComboBoxWithLabel::New();
  }

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Laplacian Smoothing");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());
  
  if ( !this->ComponentFrame )
    this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent( this->MainFrame );
  this->ComponentFrame->Create();
  this->ComponentFrame->SetLabelText("Mesh Components");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->ComponentFrame->GetWidgetName());
  
/*
  this->EntryFrame->SetParent(this->MainFrame->GetFrame());
  this->EntryFrame->Create();
  this->EntryFrame->SetLabelText("Edit FE Mesh - Smooth laplacian");

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->EntryFrame->GetWidgetName());
*/
  FEMeshListComboBox->SetParent(this->ComponentFrame->GetFrame());
  FEMeshListComboBox->Create();
  FEMeshListComboBox->SetLabelText("Mesh : ");
  FEMeshListComboBox->SetLabelWidth( 15 );
  FEMeshListComboBox->GetWidget()->ReadOnlyOn();
  FEMeshListComboBox->GetWidget()->SetBalloonHelpString(
          "Laplacian smoothing of Mesh surface nodes");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    FEMeshListComboBox->GetWidgetName());

  // surface list
  SurfaceListComboBox->SetParent(this->ComponentFrame->GetFrame());
  SurfaceListComboBox->Create();
  SurfaceListComboBox->SetLabelText("Surface : ");
  SurfaceListComboBox->SetLabelWidth( 15 );
  SurfaceListComboBox->GetWidget()->ReadOnlyOn();
  SurfaceListComboBox->GetWidget()->SetBalloonHelpString(
          "Surface from which mesh is created");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
          SurfaceListComboBox->GetWidgetName());

  BBListComboBox->SetParent(this->ComponentFrame->GetFrame());
  BBListComboBox->Create();
  BBListComboBox->SetLabelText("Building Block : ");
  BBListComboBox->SetLabelWidth( 15 );
  BBListComboBox->GetWidget()->ReadOnlyOn();
  BBListComboBox->GetWidget()->SetBalloonHelpString(
          "Building Block from which FE Mesh was generated");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    BBListComboBox->GetWidgetName());

  this->ComponentFrame->CollapseFrame();
  
  if (!this->InterpolationMenuButton)   
                this->InterpolationMenuButton = vtkKWMenuButtonWithLabel::New();
        this->InterpolationMenuButton->SetParent(this->MainFrame);
        this->InterpolationMenuButton->Create();
        this->InterpolationMenuButton->SetLabelText("Interpolation: ");
        this->InterpolationMenuButton->SetLabelWidth( 25 );
        this->InterpolationMenuButton->GetWidget()->SetBorderWidth(2);
        this->InterpolationMenuButton->GetWidget()->SetReliefToGroove();
        this->InterpolationMenuButton->GetWidget()->SetPadX(2);
        this->InterpolationMenuButton->GetWidget()->SetPadY(2);
        this->InterpolationMenuButton->GetWidget()->SetWidth(12);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 6 -fill x", 
                this->InterpolationMenuButton->GetWidgetName());
        this->InterpolationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Elliptical",this, "InterpolationModeCallback");
  this->InterpolationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Transfinite",this, "InterpolationModeCallback");
  this->InterpolationMenuButton->GetWidget()->SetValue( "Elliptical" );   
  
  // number of iterations for laplacian smoothing
  this->SmoothIterations->SetParent( this->MainFrame );
  this->SmoothIterations->Create();
  this->SmoothIterations->GetWidget()->SetWidth(4);
  this->SmoothIterations->GetWidget()->SetValueAsInt(1);
  this->SmoothIterations->SetLabelText("Smoothing Iterations : ");
  this->SmoothIterations->SetLabelWidth( 25 );

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
          this->SmoothIterations->GetWidgetName());
 
  // number of iterations for recalculating interior nodes
  /*
  this->RecalculateIterations->SetParent( this->MainFrame );
  this->RecalculateIterations->Create();
  this->RecalculateIterations->GetWidget()->SetWidth(4);
  this->RecalculateIterations->GetWidget()->SetValueAsInt(10);
  this->RecalculateIterations->SetLabelText("Recalculation Iterations: ");
  this->RecalculateIterations->SetLabelWidth( 25 );
  
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 1 -fill x", 
          this->RecalculateIterations->GetWidgetName());
  */
  // check button for enabling and disabling iteration
/*
  this->RecalculateCheckButton->SetParent(this->MainFrame->GetFrame());
  this->RecalculateCheckButton->Create();
  this->RecalculateCheckButton->SetCommand(this, "RecalculateCheckButtonCallback");
  this->RecalculateCheckButton->Select();
*/
//  this->RecalculateCheckButton->SetStateToDisabled(); 
  //this->GetApplication()->Script(
         // "pack %s -side top -anchor nw -expand n -padx 1 -pady 6", 
         // this->RecalculateCheckButton->GetWidgetName());

  
  
  
  this->ApplyButton->SetParent( this->MainFrame );
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "EditFEMeshLaplacianSmoothApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand n -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent( this->MainFrame );
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "EditFEMeshLaplacianSmoothCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand n -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());
  
}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxEditFEMeshLaplacianSmoothGroup::EditFEMeshLaplacianSmoothApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        callback->SetState(0);
  if(!strcmp(this->SurfaceListComboBox->GetWidget()->GetValue(),""))
  {
        callback->ErrorMessage("Choose the surface onto which the nodes should be constrained");
        return 0;
  }
  if( !strcmp(this->FEMeshListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("Choose the FE mesh that needs to be smoothed");
          return 0;
  }
  
  int smoothingIterations = this->SmoothIterations->GetWidget()->GetValueAsInt();
  if (smoothingIterations < 1)
  {
          callback->ErrorMessage("Number of Smoothing iterations should be > 0");
          return 0;
  }

  vtkKWComboBox *combobox = this->SurfaceListComboBox->GetWidget();
  const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Surface");
                combobox->SetValue("");
                return 0;
        }
        
        /***VAM - Hack - Essentially disables Elliptical smoothing by setting to zero ***/
  int recalcIterations = 0;
        if (strcmp(this->InterpolationMenuButton->GetWidget()->GetValue(), "Elliptical") == 0)
  {
    recalcIterations = 10;
  }
  

  vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

  combobox = this->FEMeshListComboBox->GetWidget();
  name = combobox->GetValue();

        num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid FE Mesh");
                combobox->SetValue("");
                return 0;
        }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
      this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  int meshType = ugrid->GetCellType( 0 );
  if (meshType == VTK_TETRA)
  {
    callback->ErrorMessage("Laplacian smoothing is currently not supported for tet meshes.");
                return 0;
  }
  
  vtkMimxSmoothUnstructuredGridFilter *smooth = vtkMimxSmoothUnstructuredGridFilter::New();
        smooth->SetSource(polydata);
        smooth->SetInput(ugrid);
        smooth->SetNumberOfIterations( smoothingIterations );
        smooth->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
  smooth->Update();
  if (!callback->GetState())
  {
        ugrid->GetPoints()->DeepCopy(smooth->GetOutput()->GetPoints());
        ugrid->GetPoints()->Modified();
        this->GetMimxMainWindow()->GetRenderWidget()->Render();
        this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
        smooth->RemoveObserver(callback);
        smooth->Delete();               
  }
        else    
        {               
                smooth->RemoveObserver(callback);
                smooth->Delete();               
                return 0;
        }
        
        
  
  if ( meshType == VTK_HEXAHEDRON )
  {
                if(!strcmp(this->BBListComboBox->GetWidget()->GetValue(),""))
                {
                        callback->ErrorMessage("Please select the building block from which the FE mesh was created.");
                        return 0;
                }
                

    combobox = this->BBListComboBox->GetWidget();
    /*const char **/name = combobox->GetValue();
    num = combobox->GetValueIndex(name);
    if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
          callback->ErrorMessage("Choose valid Building-block structure");
          combobox->SetValue("");
          return 0;
    }

    vtkUnstructuredGrid *bbox = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(num))->GetDataSet();

    combobox = this->FEMeshListComboBox->GetWidget();
    name = combobox->GetValue();
    /*vtkUnstructuredGrid **/ugrid = vtkMimxMeshActor::SafeDownCast(
        this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
    if(!bbox->GetCellData()->GetArray("Mesh_Seed"))
    {
         callback->ErrorMessage("Choose a building-block structure with mesh seeds assigned");
         return 0;
    }

    vtkMimxRecalculateInteriorNodes *recalc = vtkMimxRecalculateInteriorNodes::New();
    recalc->SetBoundingBox(bbox);
    recalc->SetRecalculateBBInteriorVertices( 1 );
    recalc->SetInput(ugrid);
    recalc->SetNumberOfIterations( recalcIterations );
    recalc->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    recalc->Update();
    
    if (!callback->GetState())
    {
          ugrid->GetPoints()->DeepCopy(recalc->GetOutput()->GetPoints());
          ugrid->GetPoints()->Modified();
          ugrid->BuildLinks();
          this->GetMimxMainWindow()->GetRenderWidget()->Render();
          this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
    }
    recalc->RemoveObserver(callback);
    recalc->Delete();
  
  this->GetMimxMainWindow()->SetStatusText("Smoothed Mesh");
  
  
  return 0;
        }
        else
        {
                return 1;
        }
}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::EditFEMeshLaplacianSmoothCancelCallback()
{
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        this->MenuGroup->SetMenuButtonsEnabled(1);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::UpdateObjectLists()
{
  this->UpdateSurfaceComboBox( this->SurfaceListComboBox->GetWidget() );
  this->UpdateBuildingBlockComboBox( this->BBListComboBox->GetWidget() );
  this->UpdateMeshComboBox( this->FEMeshListComboBox->GetWidget() );
  /*
  this->SurfaceListComboBox->GetWidget()->DeleteAllValues();
  this->BBListComboBox->GetWidget()->DeleteAllValues();
  this->FEMeshListComboBox->GetWidget()->DeleteAllValues();
  */
  if(this->OriginalPosition)
          this->OriginalPosition->Initialize();
         
        /*********** Update the Surface Combo Box ***********/ 
/*
  int defaultItem = -1;
  for (int i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    SurfaceListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
      
    int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->SurfaceList->GetItem(i)->GetActor());
                if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if (( defaultItem == -1) && (this->SurfaceList->GetNumberOfItems() > 0) )
    defaultItem = this->SurfaceList->GetNumberOfItems() - 1;
    
  if (defaultItem != -1)
  {
    SurfaceListComboBox->GetWidget()->SetValue(
          this->SurfaceList->GetItem(defaultItem)->GetFileName());
  }
*/  
  /*********** Update the Building Block Combo Box ***********/ 
/*
  defaultItem = -1;
  for (int i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
          vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
                  SafeDownCast(this->BBoxList->GetItem(i));
          vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
          if (ugrid->GetCellData()->GetArray("Mesh_Seed"))
          {
                  if(!this->OriginalPosition)
                          this->OriginalPosition = vtkIntArray::New();
                  BBListComboBox->GetWidget()->AddValue(
                          this->BBoxList->GetItem(i)->GetFileName());
                  this->OriginalPosition->InsertNextValue(i);
                  
                  int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                          this->BBoxList->GetItem(i)->GetActor());
                        / *
                  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(i));  
      bool viewedItem = meshActor->GetMeshVisibility();
      * /    
                if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
          }
  }
  
  if (( defaultItem == -1) && (this->BBoxList->GetNumberOfItems() > 0) )
    defaultItem = this->BBoxList->GetNumberOfItems() - 1;
    
  if (defaultItem != -1)
  {
    BBListComboBox->GetWidget()->SetValue(
          this->BBoxList->GetItem(defaultItem)->GetFileName());
  }
*/
   
  /*********** Update the Mesh Combo Box ***********/ 
/*
  defaultItem = -1;
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
          FEMeshListComboBox->GetWidget()->AddValue(
                  this->FEMeshList->GetItem(i)->GetFileName());
                  
                vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(i));  
    bool viewedItem = meshActor->GetMeshVisibility();
      
                if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if (( defaultItem == -1) && (this->FEMeshList->GetNumberOfItems() > 0) )
    defaultItem = this->FEMeshList->GetNumberOfItems() - 1;
    
  if (defaultItem != -1)
  {
    FEMeshListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }  
*/
}
//------------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::RecalculateCheckButtonCallback(int State)
{
        this->RecalculateIterations->SetEnabled(State);
}
//------------------------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::EditFEMeshLaplacianSmoothDoneCallback()
{
        if(this->EditFEMeshLaplacianSmoothApplyCallback())
                this->EditFEMeshLaplacianSmoothCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::InterpolationModeCallback( )
{

}
//---------------------------------------------------------------------------------
