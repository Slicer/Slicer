/*=========================================================================
Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshFromBBGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.74.2.4 $


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


/* VTK Headers */
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkLinkedListWrapper.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCellTypes.h"
#include "vtkConnectivityFilter.h"
#include "vtkDataSetCollection.h"
#include "vtkDataSetTriangleFilter.h"
#include "vtkExtractCells.h"
#include "vtkGeometryFilter.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkStringArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkProperty.h"
#include "vtkExtractCells.h"
#include "vtkIdList.h"
#include "vtkCellTypes.h"
#include "vtkDataSetTriangleFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkConnectivityFilter.h"
#include "vtkDataSetCollection.h"
#include "vtkDataSetCollection.h"
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include "vtkRenderer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"

/* KWWidgets Headers */
//#include "vtkKWApplication.h"
//#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWIcon.h"


/* MIMX Headers */
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWTkUtilities.h"

#include "vtkMimxApplyNodeElementNumbers.h"
#include "vtkMimxEquivalancePoints.h"
#include "vtkMimxErrorCallback.h"
#include "vtkMimxExtrudePolyData.h"
#include "vtkMimxGeometryFilter.h"
#include "vtkMimxMergeNodesPointAndCellData.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxRecalculateInteriorNodes.h"
#include "vtkMimxSmoothUnstructuredGridFilter.h"
#include "vtkMimxRecalculateInteriorNodes.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxUnstructuredGridFromBoundingBox.h"

#include "vtkKWMimxDefineElSetGroup.h"
#include "vtkMimxExtrudePolyData.h"
#include "vtkMimxMergeNodesPointAndCellData.h"
#include "vtkKWMimxCreateFEMeshFromBBGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkMimxApplyNodeElementNumbers.h"
#include "vtkMimxGeometryFilter.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxUnstructuredGridFromBoundingBox.h"
#include "vtkMimxEquivalancePoints.h"
#include "vtkKWMimxNodeElementNumbersGroup.h"
#include "vtkMimxMeshActor.h"

#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxNodeElementNumbersGroup.h"
#include "vtkKWMimxMainUserInterfacePanel.h"

#include "vtkLinkedListWrapper.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/list>


#include "Resources/mimxElementFace.h"        
#include "Resources/mimxElementIndividual.h"  
#include "Resources/mimxElementSurface.h"
#include "Resources/mimxExtractElements.h"
#include "Resources/mimxExtrudeElements.h"
#include "Resources/mimxHex.h"
#include "Resources/mimxQuad.h"
#include "Resources/mimxTet.h"
#include "Resources/mimxTri.h"

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

#define MESH_INTERNAL_EXTRUDE_SET "IAFEMESH_INTERNAL_EXTRUDE"
#define MESH_INTERNAL_EXTRACT_SET "IAFEMESH_INTERNAL_EXTRACT"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateFEMeshFromBBGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateFEMeshFromBBGroup, "$Revision: 1.74.2.4 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshFromBBGroup::vtkKWMimxCreateFEMeshFromBBGroup()
{
  this->ComponentFrame = NULL;
  this->SurfaceListComboBox = NULL;
  this->BBListComboBox = NULL;
  this->MeshListComboBox = NULL;
  this->OriginalPosition = NULL;
  this->SmoothFrame = NULL;
  this->SmoothingIterationsEntry = NULL;
  this->ElementSetNameEntry = NULL;
  this->RecalculationIterationsEntry = NULL;
  this->NodeElementNumbersGroup = NULL;
  this->MeshImprovementFrame = NULL;
  this->SmoothButton = NULL;
  this->ModeFrame = NULL;
  this->VolumetricMeshButton = NULL;
  this->SurfaceMeshButton = NULL;
  this->HexMeshButton = NULL;
  this->TetMeshButton = NULL;
  this->QuadMeshButton = NULL;
  this->TriMeshButton = NULL;
  this->TypeMenuButton = NULL;
  this->ComponentFrame = NULL;
  this->SelectFrame = NULL;
  this->SelectSurfaceButton = NULL;
  this->SelectFaceButton = NULL;
  this->SelectElementButton = NULL;
  this->SelectButton = NULL;
  this->DefineElementSetDialog = NULL;
  this->InterpolationMenuButton = NULL;
  this->InterpFrame = NULL;
  this->ExtractCount = 1;
  this->BBCount = 1;
  this->ExtrudeCount = 1;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshFromBBGroup::~vtkKWMimxCreateFEMeshFromBBGroup()
{
  if(this->ComponentFrame)
     this->ComponentFrame->Delete();
  if(this->SurfaceListComboBox)
     this->SurfaceListComboBox->Delete();
  if(this->BBListComboBox)
    this->BBListComboBox->Delete();
  if(this->OriginalPosition)
          this->OriginalPosition->Delete();
        if(this->SmoothingIterationsEntry)
          this->SmoothingIterationsEntry->Delete();
        if(this->ElementSetNameEntry)
          this->ElementSetNameEntry->Delete();
        if(this->RecalculationIterationsEntry)
          this->RecalculationIterationsEntry->Delete();
        if(this->NodeElementNumbersGroup)
                this->NodeElementNumbersGroup->Delete();
        if(this->MeshImprovementFrame)
                this->MeshImprovementFrame->Delete();
        if(this->SmoothButton)
                this->SmoothButton->Delete();
        if (this->ModeFrame )
                this->ModeFrame->Delete();
  if (this->VolumetricMeshButton )
                this->VolumetricMeshButton->Delete();
  if (this->SurfaceMeshButton )
                this->SurfaceMeshButton->Delete();
  if (this->HexMeshButton )
                this->HexMeshButton->Delete();
  if (this->TetMeshButton )
                this->TetMeshButton->Delete();
  if (this->QuadMeshButton )
                this->QuadMeshButton->Delete();
  if (this->TriMeshButton )
                this->TriMeshButton->Delete();
  if (this->TypeMenuButton )
                this->TypeMenuButton->Delete();
  if (this->ComponentFrame )
                this->ComponentFrame->Delete();
  if (this->SelectFrame )
                this->SelectFrame->Delete();
  if (this->SelectSurfaceButton )
                this->SelectSurfaceButton->Delete();
  if (this->SelectFaceButton )
                this->SelectFaceButton->Delete();
  if (this->SelectElementButton )
                this->SelectElementButton->Delete();
        if (this->SelectButton )
                this->SelectButton->Delete();
  if (this->DefineElementSetDialog)
    this->DefineElementSetDialog->Delete();
  if (this->InterpolationMenuButton)
    this->InterpolationMenuButton->Delete();
  if (this->InterpFrame)
    this->InterpFrame->Delete();  
 }
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Mesh From Building Block");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ModeFrame) 
     this->ModeFrame = vtkKWFrame::New();
  this->ModeFrame->SetParent(this->MainFrame);
  this->ModeFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x", 
    this->ModeFrame->GetWidgetName());
  
  
  if (!this->VolumetricMeshButton)      
     this->VolumetricMeshButton = vtkKWRadioButton::New();
  this->VolumetricMeshButton->SetParent( this->ModeFrame );
  this->VolumetricMeshButton->Create();
  this->VolumetricMeshButton->SetText("Volumetric Mesh");
  this->VolumetricMeshButton->SetCommand(this, "VolumetricMeshCallback");
  this->VolumetricMeshButton->SetValueAsInt(1);
  this->GetApplication()->Script(
    "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 2", 
    this->VolumetricMeshButton->GetWidgetName());
  
  if (!this->SurfaceMeshButton) 
     this->SurfaceMeshButton = vtkKWRadioButton::New();
  this->SurfaceMeshButton->SetParent( this->ModeFrame );
  this->SurfaceMeshButton->Create();
  this->SurfaceMeshButton->SetText("Surface Mesh");
  this->SurfaceMeshButton->SetCommand(this, "SurfaceMeshCallback");
  this->SurfaceMeshButton->SetValueAsInt(2);
  this->SurfaceMeshButton->SetVariableName(this->VolumetricMeshButton->GetVariableName()); 
  this->GetApplication()->Script(
    "grid %s -row 1 -column 0 -sticky nw -padx 2 -pady 2", 
    this->SurfaceMeshButton->GetWidgetName());    
  
  vtkKWIcon *hexIcon = vtkKWIcon::New();
  hexIcon->SetImage(    image_mimxHex, 
                        image_mimxHex_width, 
                        image_mimxHex_height, 
                        image_mimxHex_pixel_size); 
  if (!this->HexMeshButton)     
     this->HexMeshButton = vtkKWRadioButton::New();
  this->HexMeshButton->SetParent( this->ModeFrame );
  this->HexMeshButton->Create();
  this->HexMeshButton->SetValueAsInt(1);
  this->HexMeshButton->SetImageToIcon( hexIcon );
  this->HexMeshButton->SetSelectImageToIcon( hexIcon );
  this->HexMeshButton->SetBorderWidth(2);
  this->GetApplication()->Script(
    "grid %s -row 0 -column 1 -sticky nw -padx 2 -pady 2", 
    this->HexMeshButton->GetWidgetName()); 
  hexIcon->Delete();
  
  vtkKWIcon *tetIcon = vtkKWIcon::New();
  tetIcon->SetImage(    image_mimxTet, 
                        image_mimxTet_width, 
                        image_mimxTet_height, 
                        image_mimxTet_pixel_size); 
                          
  if (!this->TetMeshButton)     
     this->TetMeshButton = vtkKWRadioButton::New();
  this->TetMeshButton->SetParent( this->ModeFrame );
  this->TetMeshButton->Create();
  this->TetMeshButton->SetValueAsInt(2);
  this->TetMeshButton->SetImageToIcon( tetIcon );
  this->TetMeshButton->SetSelectImageToIcon( tetIcon );
  this->TetMeshButton->SetBorderWidth(2);
  this->TetMeshButton->SetVariableName(this->HexMeshButton->GetVariableName()); 
  this->GetApplication()->Script(
    "grid %s -row 0 -column 2 -sticky nw -padx 2 -pady 2", 
    this->TetMeshButton->GetWidgetName());
  tetIcon->Delete();
  
  vtkKWIcon *quadIcon = vtkKWIcon::New();
  quadIcon->SetImage(   image_mimxQuad, 
                        image_mimxQuad_width, 
                        image_mimxQuad_height, 
                        image_mimxQuad_pixel_size); 
                        
  if (!this->QuadMeshButton)    
     this->QuadMeshButton = vtkKWRadioButton::New();
  this->QuadMeshButton->SetParent( this->ModeFrame );
  this->QuadMeshButton->Create();
  this->QuadMeshButton->SetValueAsInt(1);
  this->QuadMeshButton->SetImageToIcon( quadIcon );
  this->QuadMeshButton->SetSelectImageToIcon( quadIcon );
  this->QuadMeshButton->SetBorderWidth(2);
  this->GetApplication()->Script(
    "grid %s -row 1 -column 1 -sticky nw -padx 2 -pady 2", 
    this->QuadMeshButton->GetWidgetName()); 
  quadIcon->Delete();
  
  vtkKWIcon *triIcon = vtkKWIcon::New();
  triIcon->SetImage(    image_mimxTri, 
                        image_mimxTri_width, 
                        image_mimxTri_height, 
                        image_mimxTri_pixel_size); 
                          
  if (!this->TriMeshButton)     
     this->TriMeshButton = vtkKWRadioButton::New();
  this->TriMeshButton->SetParent( this->ModeFrame );
  this->TriMeshButton->Create();
  this->TriMeshButton->SetValueAsInt(2);
  this->TriMeshButton->SetImageToIcon( triIcon );
  this->TriMeshButton->SetSelectImageToIcon( triIcon );
  this->TriMeshButton->SetBorderWidth(2);
  this->TriMeshButton->SetVariableName(this->QuadMeshButton->GetVariableName()); 
  this->GetApplication()->Script(
    "grid %s -row 1 -column 2 -sticky nw -padx 2 -pady 2", 
    this->TriMeshButton->GetWidgetName());
  triIcon->Delete();        
  
  if (!this->TypeMenuButton)    
                this->TypeMenuButton = vtkKWMenuButton::New();
        this->TypeMenuButton->SetParent(this->MainFrame);
        this->TypeMenuButton->Create();
        this->TypeMenuButton->SetBorderWidth(2);
        this->TypeMenuButton->SetReliefToGroove();
        this->TypeMenuButton->SetPadX(2);
        this->TypeMenuButton->SetPadY(2);
        this->TypeMenuButton->SetWidth(20);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 6", 
                this->TypeMenuButton->GetWidgetName());
        this->TypeMenuButton->GetMenu()->AddRadioButton(
    "Building Block",this, "VolumetricMeshBBCallback");
  this->TypeMenuButton->GetMenu()->AddRadioButton(
    "Extrude",this, "VolumetricMeshExtrudeCallback");
  this->TypeMenuButton->SetValue( "Building Block" );     
  
    
  if (!this->ComponentFrame)    
     this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent(this->MainFrame);
  this->ComponentFrame->Create();
  this->ComponentFrame->SetLabelText("Surface & Building Block");
  this->ComponentFrame->CollapseFrame( );
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x", 
    this->ComponentFrame->GetWidgetName());
  
  if(!this->SurfaceListComboBox)        
     this->SurfaceListComboBox = vtkKWComboBoxWithLabel::New();
  SurfaceListComboBox->SetParent(this->ComponentFrame->GetFrame());
  SurfaceListComboBox->Create();
  SurfaceListComboBox->SetLabelText("Surface: ");
  SurfaceListComboBox->SetLabelWidth(15);
  SurfaceListComboBox->GetWidget()->ReadOnlyOn();
  SurfaceListComboBox->GetWidget()->SetBalloonHelpString("Surface onto which the resulting mesh is projected");

  if(!this->BBListComboBox)     
    this->BBListComboBox = vtkKWComboBoxWithLabel::New();
  BBListComboBox->SetParent(this->ComponentFrame->GetFrame());
  BBListComboBox->Create();
  BBListComboBox->SetLabelText("Building Block : ");
  BBListComboBox->SetLabelWidth(15);
  BBListComboBox->GetWidget()->ReadOnlyOn();
  BBListComboBox->GetWidget()->SetBalloonHelpString("Building Block for mesh generation");
  
  if(!this->MeshListComboBox)   
    this->MeshListComboBox = vtkKWComboBoxWithLabel::New();
  this->MeshListComboBox->SetParent(this->ComponentFrame->GetFrame());
  this->MeshListComboBox->Create();
  this->MeshListComboBox->SetLabelText("Mesh : ");
  this->MeshListComboBox->SetLabelWidth(15);
  this->MeshListComboBox->GetWidget()->ReadOnlyOn();
  this->MeshListComboBox->GetWidget()->SetBalloonHelpString("Mesh for operation");  
    
  if (!this->NodeElementNumbersGroup)
    this->NodeElementNumbersGroup = vtkKWMimxNodeElementNumbersGroup::New();
  this->NodeElementNumbersGroup->SetParent(this->MainFrame);
  this->NodeElementNumbersGroup->SetMimxMainWindow(this->MimxMainWindow); //***
  this->NodeElementNumbersGroup->Create();
  //this->NodeElementNumbersGroup->GetMainFrame()->SetLabelText("Node and Element Numbers");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->NodeElementNumbersGroup->GetWidgetName());

  if (!this->InterpFrame)       
     this->InterpFrame = vtkKWFrame::New();
  this->InterpFrame->SetParent(this->MainFrame);
  this->InterpFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 10 -fill x", 
    this->InterpFrame->GetWidgetName());
    
  if (!this->InterpolationMenuButton)   
                this->InterpolationMenuButton = vtkKWMenuButtonWithLabel::New();
        this->InterpolationMenuButton->SetParent(this->InterpFrame);
        this->InterpolationMenuButton->Create();
        this->InterpolationMenuButton->SetLabelText("Interpolation: ");
        this->InterpolationMenuButton->GetWidget()->SetBorderWidth(2);
        this->InterpolationMenuButton->GetWidget()->SetReliefToGroove();
        this->InterpolationMenuButton->GetWidget()->SetPadX(2);
        this->InterpolationMenuButton->GetWidget()->SetPadY(2);
        this->InterpolationMenuButton->GetWidget()->SetWidth(12);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 6", 
                this->InterpolationMenuButton->GetWidgetName());
        this->InterpolationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Elliptical",this, "InterpolationModeCallback");
  this->InterpolationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Transfinite",this, "InterpolationModeCallback"); 
  this->InterpolationMenuButton->GetWidget()->SetValue( "Elliptical" );   
  
  
  if (!this->SmoothFrame)       
     this->SmoothFrame = vtkKWFrame::New();
  this->SmoothFrame->SetParent(this->MainFrame);
  this->SmoothFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->SmoothFrame->GetWidgetName());
          
  if (!this->SmoothButton)      
    this->SmoothButton = vtkKWCheckButtonWithLabel::New();
  this->SmoothButton->SetParent(this->SmoothFrame);
  this->SmoothButton->Create();
  this->SmoothButton->SetLabelText("Perform Smoothing");
  this->SmoothButton->GetWidget()->SetCommand(this, "SmoothFEMeshCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -padx 2 -pady 2", 
          this->SmoothButton->GetWidgetName());
  
  if(!this->MeshImprovementFrame)
          this->MeshImprovementFrame = vtkKWFrameWithLabel::New();
  this->MeshImprovementFrame->SetParent(this->MainFrame);
  this->MeshImprovementFrame->Create();
  this->MeshImprovementFrame->SetLabelText("Smoothing Parameters");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
          this->MeshImprovementFrame->GetWidgetName());
  this->MeshImprovementFrame->CollapseFrame();
  
  if(!this->SmoothingIterationsEntry)
   this->SmoothingIterationsEntry = vtkKWEntryWithLabel::New();
  this->SmoothingIterationsEntry->SetParent(this->MeshImprovementFrame->GetFrame());
  this->SmoothingIterationsEntry->Create();
  this->SmoothingIterationsEntry->SetWidth(5); 
  this->SmoothingIterationsEntry->SetLabelText("Smoothing Iterations : ");
  this->SmoothingIterationsEntry->SetLabelWidth(30);
  this->SmoothingIterationsEntry->GetWidget()->SetValueAsInt(1);
  this->SmoothingIterationsEntry->GetWidget()->SetRestrictValueToInteger();
  this->GetApplication()->Script(
   "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
   this->SmoothingIterationsEntry->GetWidgetName());

  vtkKWIcon *extrudeIcon = vtkKWIcon::New();
  extrudeIcon->SetImage(image_mimxExtrudeElements, 
                        image_mimxExtrudeElements_width, 
                        image_mimxExtrudeElements_height, 
                        image_mimxExtrudeElements_pixel_size); 
                        
  if (!this->SelectButton)
    this->SelectButton = vtkKWPushButton::New();
  this->SelectButton->SetParent( this->MainFrame );
  this->SelectButton->Create();
  //this->SelectButton->SetText( "Define Extraction Region" );
  this->SelectButton->SetReliefToFlat( );
  this->SelectButton->SetImageToIcon( extrudeIcon );
  this->SelectButton->SetCommand(this, "DefineRegionCallback" );
  this->GetApplication()->Script(
    "pack %s -side left -anchor n -padx 2 -pady 2", 
    this->SelectButton->GetWidgetName()); 
      
  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "CreateFEMeshFromBBApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());
  extrudeIcon->Delete();

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "CreateFEMeshFromBBCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

  this->SmoothButton->GetWidget()->SelectedStateOn();
  this->VolumetricMeshButton->SetSelectedState(1);
  this->HexMeshButton->SetSelectedState(1);
  this->VolumetricMeshBBCallback();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshFromBBGroup::CreateFEMeshFromBBApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

  vtkUnstructuredGrid *ugrid = NULL;
  vtkPolyData *polydata = NULL;
  vtkUnstructuredGrid *mesh = NULL;
  vtkMimxMeshActor *MeshActor = NULL;
  int bbIndex = -1;
  int surfaceIndex = -1;
  int meshIndex = -1;
  
  const char *surfacefoundationname = "";
  const char *meshfoundationname = "";
  if ( strcmp(this->TypeMenuButton->GetValue(), "Building Block" ) == 0)
  { 
    if(!strcmp(this->SurfaceListComboBox->GetWidget()->GetValue(),""))
    {
        callback->ErrorMessage("Projection surface not chosen");
        return 0;
    }
    if(!strcmp(this->BBListComboBox->GetWidget()->GetValue(),""))
    {
          callback->ErrorMessage("Building Block from which FE mesh to be generated not chosen");
          return 0;
        }
        
        vtkKWComboBox *combobox = this->SurfaceListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        surfaceIndex = combobox->GetValueIndex(name);
        if(surfaceIndex < 0 || surfaceIndex > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Surface");
                combobox->SetValue("");
                return 0;
        }

    polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
       ->GetItem(surfaceIndex))->GetDataSet();
    
        surfacefoundationname = this->SurfaceList->GetItem(surfaceIndex)->GetFoundationName();

    combobox = this->BBListComboBox->GetWidget();
    name = combobox->GetValue();

        bbIndex = combobox->GetValueIndex(name);
        if(bbIndex < 0 || bbIndex > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Building-block structure");
                combobox->SetValue("");
                return 0;
        }

    ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(bbIndex))->GetDataSet();
    if(!ugrid->GetCellData()->GetArray("Mesh_Seed"))
        {
                callback->ErrorMessage("Choose building-block structure with mesh seeds assigned");
        return 0;
        }
        }
        else
        {
          if(!strcmp(this->MeshListComboBox->GetWidget()->GetValue(),""))
    {
        callback->ErrorMessage("No mesh was selected");
        return 0;
    }
        
        vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        meshIndex = combobox->GetValueIndex(name);
        if(meshIndex < 0 || meshIndex > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Invalid mesh was selected");
                combobox->SetValue("");
                return 0;
        }
        MeshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(meshIndex));
    mesh = vtkMimxMeshActor::SafeDownCast(
        this->FEMeshList->GetItem(meshIndex))->GetDataSet();
        meshfoundationname = MeshActor->GetFoundationName();     
        }
  int nodeNumber = this->NodeElementNumbersGroup->GetNodeNumberEntry()->GetValueAsInt();
        if (nodeNumber < 1 )
  {
          callback->ErrorMessage("Node numbers must be greater than 0");
          return 0;
  }
  
  int elementNumber = this->NodeElementNumbersGroup->GetElementNumberEntry()->GetValueAsInt();
  if (elementNumber < 1 )
  {
          callback->ErrorMessage("Element numbers must be greater than 0");
          return 0;
  }
  
  const char *nodesetname = this->NodeElementNumbersGroup->GetNodeSetNameEntry()->GetValue();

  const char *elementsetname = this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue();

  if(!strcmp(nodesetname,"") || !strcmp(elementsetname,""))
  {
          callback->ErrorMessage("Please provide Node and Element Set Labels");
          return 0;
  }
  
  bool smoothFlag = false;
  int smoothingIterations = 0;
  int recalcIterations = 0;
  int interpolationMode = 0;
  if (strcmp(this->InterpolationMenuButton->GetWidget()->GetValue(), "Elliptical") == 0)
  {
    recalcIterations = 10;
    interpolationMode = 1;
  }
  
  if ( this->SmoothButton->GetWidget()->GetSelectedState() )
  {
    smoothFlag = true;
    smoothingIterations = this->SmoothingIterationsEntry->GetWidget()->GetValueAsInt();
    if (smoothingIterations < 1)
    {
      callback->ErrorMessage("Number of smoothing iterations should be greater than 0.");
      return 0;
    }
    
  }       
  
  int result = 0;
  double ExtrusionLength;
  int NumberOfDivisions;
  int meshingMode = this->GetMeshingMode();
  
  switch ( meshingMode )
  {
    case HEX_BB_SOLID_MESH:
      result = this->CreateMeshFromBuildingBlock(polydata, ugrid, smoothFlag, smoothingIterations,
                 nodesetname, nodeNumber, elementsetname, elementNumber, recalcIterations, false, false,
                                 surfacefoundationname);
      break;
    case HEX_EXTRUDE_SOLID_MESH:
                  ExtrusionLength = this->DefineElementSetDialog->GetExtrusionLength();
      NumberOfDivisions = this->DefineElementSetDialog->GetNumberOfDivisions();
                  result = this->ExtrudeHexSolidMesh(MeshActor, nodesetname,
                          elementsetname, nodeNumber, elementNumber, NumberOfDivisions, ExtrusionLength, meshfoundationname);
      break;
    case TET_BB_SOLID_MESH:    
      result = this->CreateMeshFromBuildingBlock(polydata, ugrid, smoothFlag, smoothingIterations,
                 nodesetname, nodeNumber, elementsetname, elementNumber, recalcIterations, false, true, surfacefoundationname);
      break;
    case TET_EXTRUDE_SOLID_MESH:
      //result = this->ExtrudeTetSolidMesh();
      break;
    case QUAD_BB_SURFACE_MESH:
      result = this->CreateMeshFromBuildingBlock(polydata, ugrid, smoothFlag, smoothingIterations,
                 nodesetname, nodeNumber, elementsetname, elementNumber, recalcIterations, true, false, surfacefoundationname);
      break;
    case QUAD_EXTRACT_SURFACE_MESH:
      result = this->ExtractSurfaceMesh(MeshActor, nodesetname, elementsetname, nodeNumber, elementNumber, false, meshfoundationname);
      break;
    case TRI_BB_SURFACE_MESH:
      result = this->CreateMeshFromBuildingBlock(polydata, ugrid, smoothFlag, smoothingIterations,
                 nodesetname, nodeNumber, elementsetname, elementNumber, recalcIterations, true, true, surfacefoundationname);
      break;
    case TRI_EXTRACT_SURFACE_MESH:
      result = this->ExtractSurfaceMesh(MeshActor, nodesetname, elementsetname, nodeNumber, elementNumber, true, meshfoundationname);
      break;
  }
  
  if(!result)   return 0;
  if ((this->DefineElementSetDialog != NULL) && (this->DefineElementSetDialog->IsMapped()))
  {
    this->DefineElementSetDialog->Withdraw();
  }
  
  /* Hide the selected Surface and Building Block */
  if ( bbIndex != -1 )
  {
    this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
                                this->BBoxList->GetItem(bbIndex)->GetActor());
        this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();             
        }
        if ( surfaceIndex != -1 )
        {
        this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
                                this->SurfaceList->GetItem(surfaceIndex)->GetActor());
        this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();     
        }
        if ( meshIndex != -1 )
        {
        vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(meshIndex))->HideMesh();
        this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();     
        }
        return result;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::CreateFEMeshFromBBCancelCallback()
{
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        this->MenuGroup->SetMenuButtonsEnabled(1);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
        this->SmoothButton->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::UpdateObjectLists()
{
  this->UpdateSurfaceComboBox( this->SurfaceListComboBox->GetWidget() );
  this->UpdateBuildingBlockComboBox( this->BBListComboBox->GetWidget() );
  int item = this->UpdateMeshComboBox( this->MeshListComboBox->GetWidget() );
  
  //this->SurfaceListComboBox->GetWidget()->DeleteAllValues();
   
  if(this->OriginalPosition)
          this->OriginalPosition->Initialize();
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
    defaultItem = this->SurfaceList->GetNumberOfItems()-1;
    
  if (defaultItem != -1)
  {
    SurfaceListComboBox->GetWidget()->SetValue(
          this->SurfaceList->GetItem(defaultItem)->GetFileName());
  }
*/
/*  
  this->BBListComboBox->GetWidget()->DeleteAllValues();
  
  defaultItem = -1;
        for (int i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
        {
                const char *bbName = this->BBoxList->GetItem(i)->GetFileName();
                BBListComboBox->GetWidget()->AddValue( bbName );
                  
          int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                this->BBoxList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
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
/*  
  this->MeshListComboBox->GetWidget()->DeleteAllValues();
        
        defaultItem = -1;
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
        {
                this->MeshListComboBox->GetWidget()->AddValue(
                        this->FEMeshList->GetItem(i)->GetFileName());
                        
          int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->FEMeshList->GetItem(i)->GetActor());
    if (  viewedItem )
                {
                  defaultItem = i;
                }
        }
        if ( (defaultItem == -1) && (this->FEMeshList->GetNumberOfItems() > 0) )
          defaultItem = this->FEMeshList->GetNumberOfItems() - 1;
*/  
        vtkUnstructuredGrid *grid = NULL;
        if (item != -1)
  {
    //this->MeshListComboBox->GetWidget()->SetValue(
    //      this->FEMeshList->GetItem(defaultItem)->GetFileName());
    this->SelectButton->SetStateToNormal();
    grid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(item))->GetDataSet();
  }
  else
  {
    this->SelectButton->SetStateToDisabled();
  }
 
        /* Set the Deafault Node and Element Numbers */
        if(!grid)       return;
        int maxNodeNumber = 0;
        int maxElementNumber = 0;
        
    vtkIntArray *nodeArray = vtkIntArray::SafeDownCast(grid->GetPointData()->GetArray("Node_Numbers"));
    vtkIntArray *elementArray = vtkIntArray::SafeDownCast(grid->GetCellData()->GetArray("Element_Numbers"));
        int i;
    if ( nodeArray )
    {
      for (i=0; i<nodeArray->GetNumberOfTuples(); i++)
      {
                  if(nodeArray->GetValue(i) > maxNodeNumber)    maxNodeNumber = nodeArray->GetValue(i);
      }
    }
    if ( elementArray )
    {
                for (i=0; i<elementArray->GetNumberOfTuples(); i++)
                {
                        if(elementArray->GetValue(i) > maxElementNumber)        maxElementNumber = elementArray->GetValue(i);
                }

    }
  //}
  maxNodeNumber++;
  maxElementNumber++;
  this->NodeElementNumbersGroup->GetNodeNumberEntry()->SetValueAsInt( maxNodeNumber );
        this->NodeElementNumbersGroup->GetElementNumberEntry()->SetValueAsInt( maxElementNumber );
        this->NodeElementNumbersGroup->GetNodeSetNameEntry()->SetValue( "" );
        this->NodeElementNumbersGroup->GetElementSetNameEntry()->SetValue( "" );
                
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::CreateFEMeshFromBBDoneCallback()
{
        if(this->CreateFEMeshFromBBApplyCallback())
                this->CreateFEMeshFromBBCancelCallback();
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::VolumetricMeshCallback()
{
        this->HexMeshButton->SetStateToNormal();
        /***VAM - Tet is currently disabled ***/
        //this->TetMeshButton->SetStateToNormal();
        this->TetMeshButton->SetStateToDisabled();
        this->QuadMeshButton->SetStateToDisabled();
        this->TriMeshButton->SetStateToDisabled();
        
        this->TypeMenuButton->GetMenu()->DeleteAllItems();
        this->TypeMenuButton->GetMenu()->AddRadioButton(
    "Building Block",this, "VolumetricMeshBBCallback");
  this->TypeMenuButton->GetMenu()->AddRadioButton(
    "Extrude",this, "VolumetricMeshExtrudeCallback");
  this->TypeMenuButton->SetValue("Building Block");   
  this->HexMeshButton->SetSelectedState(1);
  this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 6", 
                this->InterpolationMenuButton->GetWidgetName());
  this->VolumetricMeshBBCallback();
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::SurfaceMeshCallback()
{
        this->QuadMeshButton->SetStateToNormal();
        this->TriMeshButton->SetStateToNormal();
        this->HexMeshButton->SetStateToDisabled();
        this->TetMeshButton->SetStateToDisabled();
        
        this->TypeMenuButton->GetMenu()->DeleteAllItems();
        this->TypeMenuButton->GetMenu()->AddRadioButton(
    "Building Block",this, "SurfaceMeshBBCallback");
  this->TypeMenuButton->GetMenu()->AddRadioButton(
    "Extract",this, "SurfaceMeshExtractCallback");
  this->TypeMenuButton->SetValue("Building Block");  
  this->QuadMeshButton->SetSelectedState(1);
  this->InterpolationMenuButton->Unpack();
  this->SurfaceMeshBBCallback();
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::VolumetricMeshBBCallback()
{
  this->ComponentFrame->SetLabelText("Surface & Building Block");
  this->GetApplication()->Script("pack forget %s", this->MeshListComboBox->GetWidgetName());
  //this->GetApplication()->Script("pack forget %s", this->ExtrudeFrame->GetWidgetName());
  this->GetApplication()->Script("pack forget %s", this->SelectButton->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->SurfaceListComboBox->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->BBListComboBox->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x -after %s", 
    this->SmoothFrame->GetWidgetName(),this->InterpFrame->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x -after %s", 
    this->MeshImprovementFrame->GetWidgetName(),this->SmoothFrame->GetWidgetName());
    
  if ((this->DefineElementSetDialog != NULL) && (this->DefineElementSetDialog->IsMapped()))
  {
    this->DefineElementSetDialog->Withdraw();
  }
  
  this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 2", 
                this->InterpolationMenuButton->GetWidgetName());
                
  this->ApplyButton->SetStateToNormal();
  
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::VolumetricMeshExtrudeCallback()
{
  this->ComponentFrame->SetLabelText("Mesh");
  this->GetApplication()->Script("pack forget %s", this->SurfaceListComboBox->GetWidgetName());
  this->GetApplication()->Script("pack forget %s", this->BBListComboBox->GetWidgetName());
  this->GetApplication()->Script("pack forget %s", this->SmoothFrame->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->MeshListComboBox->GetWidgetName());
  
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -padx 2 -pady 2 -after %s", 
    this->SelectButton->GetWidgetName(), this->NodeElementNumbersGroup->GetWidgetName());
  vtkKWIcon *extrudeIcon = vtkKWIcon::New();
  extrudeIcon->SetImage(image_mimxExtrudeElements, 
                        image_mimxExtrudeElements_width, 
                        image_mimxExtrudeElements_height, 
                        image_mimxExtrudeElements_pixel_size); 
  this->SelectButton->SetImageToIcon( extrudeIcon );
  extrudeIcon->Delete();  
  this->InterpolationMenuButton->Unpack();
        this->MeshImprovementFrame->Unpack();   
  this->ApplyButton->SetStateToDisabled();
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::SurfaceMeshBBCallback()
{
  this->ComponentFrame->SetLabelText("Surface & Building Block");
  this->GetApplication()->Script("pack forget %s", this->MeshListComboBox->GetWidgetName());
  //this->GetApplication()->Script("pack forget %s", this->ExtrudeFrame->GetWidgetName());
  this->GetApplication()->Script("pack forget %s", this->SelectButton->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->SurfaceListComboBox->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->BBListComboBox->GetWidgetName());  
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x -after %s", 
    this->SmoothFrame->GetWidgetName(),this->InterpFrame->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x -after %s", 
    this->MeshImprovementFrame->GetWidgetName(),this->SmoothFrame->GetWidgetName());
  if ((this->DefineElementSetDialog != NULL) && (this->DefineElementSetDialog->IsMapped()))
  {
    this->DefineElementSetDialog->Withdraw();
  }
  
  this->ApplyButton->SetStateToNormal();  
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::SurfaceMeshExtractCallback()
{
  this->ComponentFrame->SetLabelText("Mesh");
  this->GetApplication()->Script("pack forget %s", this->SurfaceListComboBox->GetWidgetName());
  this->GetApplication()->Script("pack forget %s", this->BBListComboBox->GetWidgetName());
  //this->GetApplication()->Script("pack forget %s", this->ExtrudeFrame->GetWidgetName());
  this->GetApplication()->Script("pack forget %s", this->SmoothFrame->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->MeshListComboBox->GetWidgetName());
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -padx 2 -pady 2 -after %s", 
    this->SelectButton->GetWidgetName(), this->NodeElementNumbersGroup->GetWidgetName());
  vtkKWIcon *extractIcon = vtkKWIcon::New();
  extractIcon->SetImage(image_mimxExtractElements, 
                        image_mimxExtractElements_width, 
                        image_mimxExtractElements_height, 
                        image_mimxExtractElements_pixel_size); 
  this->SelectButton->SetImageToIcon( extractIcon );
  this->MeshImprovementFrame->Unpack(); 
  this->ApplyButton->SetStateToDisabled();
  extractIcon->Delete();
}      
//---------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::SmoothFEMeshCallback(int state)
{
        if (state)
        {
        this->SmoothingIterationsEntry->GetWidget()->SetStateToNormal();
        }
        else
        {
        this->SmoothingIterationsEntry->GetWidget()->SetStateToDisabled();
        }
        
}

//---------------------------------------------------------------------------------
/*
void vtkKWMimxCreateFEMeshFromBBGroup::AddMeshToDisplay(
        vtkUnstructuredGrid *mesh, const char *namePrefix, int index, const char *FoundationName)
{
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::New();
  meshActor->SetFoundationName(FoundationName);
  this->FEMeshList->AppendItem( meshActor );
        meshActor->SetDataSet( mesh );
        meshActor->SetRenderer( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer() );
        meshActor->SetInteractor( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor() );
  
  //this->Count++;
  vtkIdType idNumber = index;
  meshActor->SetObjectName( namePrefix, idNumber);
  // check if triangle or quad elements exist
  vtkCellTypes *cellTypes = vtkCellTypes::New();
  meshActor->GetDataSet()->GetCellTypes(cellTypes);
  int i;
  for (int i=0; i<cellTypes->GetNumberOfTypes(); i++)
  {
          if(cellTypes->GetCellType(i) == VTK_TRIANGLE ||
                  cellTypes->GetCellType(i) == VTK_QUAD)
          {
                meshActor->AddReferenceNode(this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue());
                break;
          }
  }

  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  
  int itemIndex = this->FEMeshList->GetNumberOfItems()-1;
        this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( this->FEMeshList->GetItem(itemIndex) );
                      
}
*/
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshFromBBGroup::GetMeshingMode( )
{
  if ( this->VolumetricMeshButton->GetSelectedState() )
  {
    if ( this->HexMeshButton->GetSelectedState() )
    {
      if (strcmp(this->TypeMenuButton->GetValue(), "Building Block") == 0 )
        return HEX_BB_SOLID_MESH;
      else
        return HEX_EXTRUDE_SOLID_MESH;
    }
    else
    {
      if (strcmp(this->TypeMenuButton->GetValue(), "Building Block") == 0 )
        return TET_BB_SOLID_MESH;
      else
        return TET_EXTRUDE_SOLID_MESH;
    }
  }
  else
  {
    if ( this->QuadMeshButton->GetSelectedState() )
    {
      if (strcmp(this->TypeMenuButton->GetValue(), "Building Block") == 0 )
        return QUAD_BB_SURFACE_MESH;
      else
        return QUAD_EXTRACT_SURFACE_MESH;
    }
    else
    {
      if (strcmp(this->TypeMenuButton->GetValue(), "Building Block") == 0 )
        return TRI_BB_SURFACE_MESH;
      else
        return TRI_EXTRACT_SURFACE_MESH;
    }
  }
                      
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshFromBBGroup::CreateMeshFromBuildingBlock(vtkPolyData *polyDaya, 
                     vtkUnstructuredGrid *buildingBlock, bool smoothMesh, int numSmooth,
                     const char *nodeSetName, int nodeNumber, const char *elementSetName, int elementNumber,
                     int numRecalc, bool generateSurfaceMesh, bool generateTriMesh, const char *FoundationName)
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  
  vtkMimxUnstructuredGridFromBoundingBox *ugridfrombbox = vtkMimxUnstructuredGridFromBoundingBox::New();
  ugridfrombbox->SetBoundingBox( buildingBlock );
  ugridfrombbox->SetSurface( polyDaya );
  callback->SetState(0);
  ugridfrombbox->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
  ugridfrombbox->Update();
  
  if (!callback->GetState())
  {
        vtkMimxEquivalancePoints *equivalance = vtkMimxEquivalancePoints::New();
        equivalance->SetInput(ugridfrombbox->GetOutput());
        equivalance->Update();
        
        vtkUnstructuredGrid *mesh = equivalance->GetOutput();
        
        
    /* Smooth the Mesh if Requested */
    vtkMimxSmoothUnstructuredGridFilter *smooth = vtkMimxSmoothUnstructuredGridFilter::New();
    if ( smoothMesh )
    {
        smooth->SetSource( polyDaya );
        smooth->SetInput(mesh);
        smooth->SetNumberOfIterations( numSmooth );
        smooth->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
        smooth->Update();
        if ( callback->GetState() )
        {
                smooth->RemoveObserver(callback);
                smooth->Delete();                       
                callback->ErrorMessage("Error while smoothing the mesh.");
                return 0;
        }

        smooth->RemoveObserver(callback);
        mesh = smooth->GetOutput();
        }
        
        /***VAM - This is currently a Hack
         *  vtkMimxRecalculateInteriorNodes runs Transfinite followed by Elliptical Interp.
         *  setting the number of iterations essentially turns off Elliptical Interpolation
         */
        vtkMimxRecalculateInteriorNodes *recalc = vtkMimxRecalculateInteriorNodes::New();
        recalc->SetBoundingBox( buildingBlock );
    recalc->SetRecalculateBBInteriorVertices(1);
    recalc->SetInput( mesh );
    recalc->SetNumberOfIterations(numRecalc);
    recalc->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    recalc->Update();
    if ( callback->GetState() )
    {
        recalc->RemoveObserver(callback);
        recalc->Delete();                       
        callback->ErrorMessage("Failed to calculate interior mesh nodes.");
        return 0;
    }
    mesh = recalc->GetOutput(); 
        
          std::string meshPrefix = "VMesh-";
        vtkMimxGeometryFilter *surfaceExtractFilter = vtkMimxGeometryFilter::New();
        vtkConnectivityFilter *ugridFilter = vtkConnectivityFilter::New();
        if ( generateSurfaceMesh )
        {
          surfaceExtractFilter->SetInput( mesh );
      surfaceExtractFilter->Update();
      vtkPolyData *surfaceMesh = surfaceExtractFilter->GetOutput();
      
      ugridFilter->SetInput( surfaceMesh );
      ugridFilter->ColorRegionsOff( );
      ugridFilter->SetExtractionModeToAllRegions( );
      ugridFilter->Update( );
      mesh = ugridFilter->GetOutput();
            meshPrefix = "SMesh-";
        }
        
    vtkDataSetTriangleFilter *triangleFilter = vtkDataSetTriangleFilter::New();
    if ( generateTriMesh )
    {
      triangleFilter->SetInput( mesh );
      if (! generateSurfaceMesh )
        triangleFilter->TetrahedraOnlyOn();
      triangleFilter->Update( );
      mesh = triangleFilter->GetOutput();
    }
    
    this->InitializeMeshFieldData(mesh, nodeSetName, elementSetName, nodeNumber, elementNumber);
    const char *elementSetName1 = this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue();
    this->AddMeshToDisplay(mesh, meshPrefix.c_str(), FoundationName, elementSetName1);
    this->BBCount++;
    smooth->Delete();           
    recalc->Delete();   
    surfaceExtractFilter->Delete();
    ugridFilter->Delete();
    triangleFilter->Delete(); 
    
    ugridfrombbox->RemoveObserver(callback);
    ugridfrombbox->Delete();
    equivalance->Delete();
    
    this->GetMimxMainWindow()->SetStatusText("Created Mesh");
    this->CreateFEMeshFromBBCancelCallback();
    
    return 1;
  }
  ugridfrombbox->RemoveObserver(callback);
  ugridfrombbox->Delete();
  return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::InitializeMeshFieldData(vtkUnstructuredGrid *mesh, 
                const char *nodeSetName, const char *elementSetName, int nodeNumber, int elementNumber)
{
  vtkMimxApplyNodeElementNumbers *apply = new vtkMimxApplyNodeElementNumbers;
        apply->SetUnstructuredGrid( mesh );
        apply->SetNodeSetName( nodeSetName );
        apply->SetStartingNodeNumber( nodeNumber );
        apply->ApplyNodeNumbers();
        apply->SetElementSetName( elementSetName );
        apply->SetStartingElementNumber( elementNumber );
        apply->ApplyElementNumbers();
        delete apply;
  
  // creation of a new field data for element set storage
  vtkStringArray *elsetarray = vtkStringArray::New();
  elsetarray->SetName("Element_Set_Names");
  elsetarray->InsertNextValue( elementSetName );

  vtkStringArray *nodesetarray = vtkStringArray::New();
  nodesetarray->SetName("Node_Set_Names");
  nodesetarray->InsertNextValue( nodeSetName );

  mesh->GetFieldData()->AddArray(elsetarray);
  elsetarray->Delete();

  mesh->GetFieldData()->AddArray(nodesetarray);
  nodesetarray->Delete();

  vtkIntArray *BoundCond = vtkIntArray::New();
  BoundCond->SetNumberOfValues(1);
  BoundCond->SetValue(0,1);
  BoundCond->SetName("Boundary_Condition_Number_Of_Steps");
  mesh->GetFieldData()->AddArray(BoundCond);
  BoundCond->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::DefineRegionCallback( )
{
  char elementSetLabel[64];
  char dialogTitle[64];
  const char *textLabel = this->TypeMenuButton->GetValue();
  int mode;
  if (strcmp(textLabel, "Extrude") == 0 )
  {
    //std::cout << "Extrude" << std::endl;
    strcpy(elementSetLabel, MESH_INTERNAL_EXTRUDE_SET);
    strcpy(dialogTitle, "Define Extrusion Region");
    mode = 1;
  }
  else
  {
    //std::cout << "Extract" << std::endl;
    strcpy(elementSetLabel, MESH_INTERNAL_EXTRACT_SET);
    strcpy(dialogTitle, "Define Extraction Region");
    mode = 2;
  }
  
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        
        if(!strcmp(this->MeshListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("Mesh must be selected");
                return;
        }

  vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
  const char *name = combobox->GetValue();
  
        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Invalid Mesh was selected");
                combobox->SetValue("");
                return;
        }
        
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
                
  if (this->DefineElementSetDialog == NULL)
  {
    this->DefineElementSetDialog = vtkKWMimxDefineElSetGroup::New();
    this->DefineElementSetDialog->SetApplication( this->GetApplication() );
          this->DefineElementSetDialog->SetMimxMainWindow( this->GetMimxMainWindow() );
          this->DefineElementSetDialog->SetDimensionState(1);
          this->DefineElementSetDialog->SetCreateFEMeshFromBBGroup(this);
          this->DefineElementSetDialog->Create();
        }
        this->DefineElementSetDialog->SetDialogTitle( dialogTitle );
        this->DefineElementSetDialog->SetMeshActor( meshActor );
        this->DefineElementSetDialog->SetModeSurfaceOnly( );
        this->DefineElementSetDialog->SetWithdrawOnApply(1);
        //this->DefineElementSetDialog->SetElementSetCombobox(this->ElementSetComboBox->GetWidget());
        
        this->DefineElementSetDialog->GetApplyButton()->SetCommand(this, "CreateFEMeshFromBBApplyCallback");
        this->DefineElementSetDialog->GetSetLabelEntry()->GetWidget()->SetValue(elementSetLabel);
        this->GetApplication()->Script("pack forget %s", 
           this->DefineElementSetDialog->GetSetLabelEntry()->GetWidgetName());
        if (mode == 1)
        {
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x -after %s", 
           this->DefineElementSetDialog->GetExtrudeFrame()->GetWidgetName(),
           this->DefineElementSetDialog->GetModeFrame()->GetWidgetName());
        }
        else
        {
        this->GetApplication()->Script("pack forget %s", 
           this->DefineElementSetDialog->GetExtrudeFrame()->GetWidgetName());
        }
        
        this->DefineElementSetDialog->Display();
}  

//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshFromBBGroup::ExtrudeHexSolidMesh(vtkMimxMeshActor *MeshActor, const char *NodeSetName,
                                                                                                                   const char *ElSetName, int StartNodeNum, int StartElemNum,
                                                                                                                   int NumberOfDiv, double ExtrusionLength, const char *FoundationName)
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        // check the node, element number and entry sets are valid.
        vtkUnstructuredGrid *ugrid = MeshActor->GetDataSet();

        //vtkIntArray *nodenumbers = vtkIntArray::SafeDownCast(
        //      ugrid->GetPointData()->GetArray("Node_Numbers"));
        //vtkIntArray *elementnumbers = vtkIntArray::SafeDownCast(
        //      ugrid->GetCellData()->GetArray("Element_Numbers"));

        // check if the node and element set name already exists
        if(ugrid->GetPointData()->GetArray(NodeSetName))
        {
                callback->ErrorMessage("This Label has already been assigned to a Node Set.  Please select another label.");
                return 0;
        }
        //
        if(ugrid->GetCellData()->GetArray(ElSetName))
        {
                callback->ErrorMessage("This Label has already been assigned to an Element Set.  Please select another label.");
                return 0;
        }
        // KHS
        if(!this->DefineElementSetDialog)
        {
                callback->ErrorMessage("No region was selected for extrusion");
                return 0;
        }
        if(!this->DefineElementSetDialog->GetSelectedMesh()->GetNumberOfCells())
        {
                callback->ErrorMessage("No region was selected for extrusion");
                return 0;
        }
        /* Find the New Element Set */
        
        vtkGeometryFilter *geofil = vtkGeometryFilter::New();
        geofil->SetInput(this->DefineElementSetDialog->GetSelectedMesh());
        geofil->Update();
        vtkMimxExtrudePolyData *extrude = vtkMimxExtrudePolyData::New();
        extrude->SetInput( geofil->GetOutput() );

        extrude->SetExtrusionLength(ExtrusionLength);
        extrude->SetNumberOfDivisions(NumberOfDiv);
        extrude->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
        callback->SetState(0);
        extrude->Update();
        extrude->RemoveObserver(callback);
        if (!callback->GetState())
        {
                // apply Node and element numbers to the extruded part of the mesh
                vtkMimxApplyNodeElementNumbers *apply = new vtkMimxApplyNodeElementNumbers;
                apply->SetUnstructuredGrid(extrude->GetOutput());
                apply->SetNodeSetName(NodeSetName);
                apply->SetStartingNodeNumber(StartNodeNum);
                apply->ApplyNodeNumbers();

                apply->SetElementSetName(ElSetName);
                apply->SetStartingElementNumber(StartElemNum);
                apply->ApplyElementNumbers();
                delete apply;
                //
                vtkStringArray *elsetarray = vtkStringArray::New();
                elsetarray->SetName("Element_Set_Names");
                elsetarray->InsertNextValue(ElSetName);

                vtkStringArray *nodesetarray = vtkStringArray::New();
                nodesetarray->SetName("Node_Set_Names");
                nodesetarray->InsertNextValue(NodeSetName);

                extrude->GetOutput()->GetFieldData()->AddArray(elsetarray);
                elsetarray->Delete();

                extrude->GetOutput()->GetFieldData()->AddArray(nodesetarray);
                nodesetarray->Delete();

                // merge the original and extruded mesh
                vtkDataSetCollection *collection = vtkDataSetCollection::New();
                collection->InitTraversal();
                collection->AddItem(extrude->GetOutput());
                collection->AddItem(ugrid);

                vtkMimxMergeNodesPointAndCellData *merge = new vtkMimxMergeNodesPointAndCellData;

                merge->SetDataSetCollection(collection);
                merge->SetTolerance(0.0);
                merge->SetNodesMerge(1);

                if(merge->MergeDataSets())
                {
                        const char *elementSetName = this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue();
                        this->AddMeshToDisplay(merge->GetMergedDataSet(), "Mesh_Extrude-", FoundationName, elementSetName);     
                        this->ExtrudeCount++;
                                this->GetMimxMainWindow()->SetStatusText("Created Extruded Mesh");
                        this->UpdateObjectLists();
                        if(this->DefineElementSetDialog)
                        {
                                this->DefineElementSetDialog->Withdraw();
                        }
                }
                extrude->Delete();
                delete merge;
                collection->Delete();
                geofil->Delete();
                return 1;
        }
        geofil->Delete();
        extrude->Delete();
        return 0;
}

//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshFromBBGroup::ExtractSurfaceMesh(vtkMimxMeshActor *MeshActor, const char *NodeSetName,
                                                                                                                   const char *ElSetName, int StartNodeNum, 
                                                                                                                   int StartElemNum, bool generateTriMesh, const char *FoundationName)
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        // check the node, element number and entry sets are valid.
        vtkUnstructuredGrid *ugrid = MeshActor->GetDataSet();

        //vtkIntArray *nodenumbers = vtkIntArray::SafeDownCast(
        //      ugrid->GetPointData()->GetArray("Node_Numbers"));
        //vtkIntArray *elementnumbers = vtkIntArray::SafeDownCast(
        //      ugrid->GetCellData()->GetArray("Element_Numbers"));

        if ((!this->DefineElementSetDialog) || (!this->DefineElementSetDialog->GetSelectedMesh()->GetNumberOfCells()))
        {
                callback->ErrorMessage("No region was selected for extrusion");
                return 0;
        }
        
        vtkGeometryFilter *geofil = vtkGeometryFilter::New();
        geofil->SetInput(this->DefineElementSetDialog->GetSelectedMesh());
        geofil->Update();
        
        vtkConnectivityFilter *ugridFilter = vtkConnectivityFilter::New();
  ugridFilter->SetInput( geofil->GetOutput() );
  ugridFilter->ColorRegionsOff( );
  ugridFilter->SetExtractionModeToAllRegions( );
  ugridFilter->Update( );
  vtkUnstructuredGrid *mesh = ugridFilter->GetOutput();
  
  vtkDataSetTriangleFilter *triangleFilter = vtkDataSetTriangleFilter::New();
  if ( generateTriMesh )
  {
    triangleFilter->SetInput( mesh );
    triangleFilter->Update( );
    mesh = triangleFilter->GetOutput();
  }
        
        // apply Node and element numbers to the extruded part of the mesh
        vtkMimxApplyNodeElementNumbers *apply = new vtkMimxApplyNodeElementNumbers;
        apply->SetUnstructuredGrid( mesh );
        apply->SetNodeSetName(NodeSetName);
        apply->SetStartingNodeNumber(StartNodeNum);
        apply->ApplyNodeNumbers();
        apply->SetElementSetName(ElSetName);
        apply->SetStartingElementNumber(StartElemNum);
        apply->ApplyElementNumbers();
        delete apply;
        
        vtkStringArray *elsetarray = vtkStringArray::New();
        elsetarray->SetName("Element_Set_Names");
        elsetarray->InsertNextValue(ElSetName);

        vtkStringArray *nodesetarray = vtkStringArray::New();
        nodesetarray->SetName("Node_Set_Names");
        nodesetarray->InsertNextValue(NodeSetName);

        mesh->GetFieldData()->AddArray(elsetarray);
        elsetarray->Delete();

        mesh->GetFieldData()->AddArray(nodesetarray);
        nodesetarray->Delete();

  const char *elementSetName = this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue();
        this->AddMeshToDisplay(mesh, "Extract-", FoundationName, elementSetName);
        vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1))->HideMesh();
        this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();     

        // merge the solid and surface datasets together
        vtkDataSetCollection *collection = vtkDataSetCollection::New();
        collection->InitTraversal();
        collection->AddItem(ugrid);
        collection->AddItem(mesh);
        vtkMimxMergeNodesPointAndCellData *merge = new vtkMimxMergeNodesPointAndCellData;

        merge->SetDataSetCollection(collection);
        merge->SetTolerance(0.0);
        merge->SetNodesMerge(1);

        if(merge->MergeDataSets())
        {
                vtkUnstructuredGrid *unsgrid = merge->GetMergedDataSet();
                if(ugrid->GetFieldData()->GetArray("Mesh_Seed"))
                {
                        vtkIntArray *intarray = vtkIntArray::New();
                        intarray->SetName("Mesh_Seed");
                        intarray->DeepCopy(vtkIntArray::SafeDownCast(
                                ugrid->GetFieldData()->GetArray("Mesh_Seed")));
                        unsgrid->GetFieldData()->AddArray(intarray);
                        intarray->Delete();
                }
                const char *elementSetName1 = this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue();
                this->AddMeshToDisplay(unsgrid, "Mesh_Extract-", FoundationName, elementSetName1);       
                this->ExtractCount++;
                this->GetMimxMainWindow()->SetStatusText("Created Extracted Mesh");
                this->UpdateObjectLists();
                if(this->DefineElementSetDialog)
                {
                        this->DefineElementSetDialog->Withdraw();
                }
        }
        delete merge;
        collection->Delete();
        this->GetMimxMainWindow()->SetStatusText("Created Extracted Mesh");
        this->UpdateObjectLists();
                
        geofil->Delete();
        ugridFilter->Delete();
        triangleFilter->Delete();
        return 1;       
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::InterpolationModeCallback( )
{

}
//---------------------------------------------------------------------------------
