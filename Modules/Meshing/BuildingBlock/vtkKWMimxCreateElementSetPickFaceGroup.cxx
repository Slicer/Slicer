/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateElementSetPickFaceGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.24.4.2 $

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

#include "vtkKWMimxCreateElementSetPickFaceGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"
#include "vtkMimxExtractElementNumbersOfaFaceFilter.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxUnstructuredGridFromBoundingBox.h"
#include "vtkKWRadioButton.h"
#include "vtkKWLoadSaveDialog.h"

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
#include "vtkKWRadioButtonSet.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkMimxExtractMultipleFaceWidget.h"
#include "vtkRenderer.h"
#include "vtkKWIcon.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkMimxMeshActor.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateElementSetPickFaceGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateElementSetPickFaceGroup, "$Revision: 1.24.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateElementSetPickFaceGroup::vtkKWMimxCreateElementSetPickFaceGroup()
{
  this->FEMeshListComboBox = NULL;
  this->BBListComboBox = NULL;
  this->OriginalPosition = NULL;
  this->RadioButtonSet = NULL;
  this->ExtractFaceWidget = NULL;
  this->AddButtonState = 0;
  this->ElementSetNameEntry = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateElementSetPickFaceGroup::~vtkKWMimxCreateElementSetPickFaceGroup()
{
  if(this->FEMeshListComboBox)
     this->FEMeshListComboBox->Delete();
  if(this->BBListComboBox)
    this->BBListComboBox->Delete();
  if(this->OriginalPosition)
          this->OriginalPosition->Delete();
  if(this->RadioButtonSet)
          this->RadioButtonSet->Delete();
  if(this->ExtractFaceWidget)
          this->ExtractFaceWidget->Delete();
  if(this->ElementSetNameEntry)
          this->ElementSetNameEntry->Delete();

  }
//----------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Mesh Element Set");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if(!this->FEMeshListComboBox) 
     this->FEMeshListComboBox = vtkKWComboBoxWithLabel::New();
  FEMeshListComboBox->SetParent(this->MainFrame);
  FEMeshListComboBox->Create();
  FEMeshListComboBox->SetLabelText("Mesh : ");
  FEMeshListComboBox->SetLabelWidth( 15 );
  FEMeshListComboBox->GetWidget()->ReadOnlyOn();
  FEMeshListComboBox->GetWidget()->SetBalloonHelpString("FE mesh whose elements needs to be extracted");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    FEMeshListComboBox->GetWidgetName());

  if(!this->BBListComboBox)     
    this->BBListComboBox = vtkKWComboBoxWithLabel::New();
  BBListComboBox->SetParent(this->MainFrame);
  BBListComboBox->Create();
  BBListComboBox->SetLabelText("Building Block : ");
  BBListComboBox->SetLabelWidth( 15 );
  BBListComboBox->GetWidget()->ReadOnlyOn();
  BBListComboBox->GetWidget()->SetBalloonHelpString("Building Block to which FE mesh corresponds to");
  BBListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    BBListComboBox->GetWidgetName());

  
  if(!this->ElementSetNameEntry)
          this->ElementSetNameEntry = vtkKWEntryWithLabel::New();
  this->ElementSetNameEntry->SetParent(this->MainFrame);
  this->ElementSetNameEntry->Create();
  this->ElementSetNameEntry->SetWidth(10);
  this->ElementSetNameEntry->SetLabelText("Element Set Name : ");
  this->ElementSetNameEntry->SetLabelWidth( 15 );
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
          this->ElementSetNameEntry->GetWidgetName());
  //
  this->RadioButtonSet = vtkKWRadioButtonSet::New();
  // radio button sets
  this->RadioButtonSet->SetParent(this->MainFrame);
  //this->RadioButtonSet->SetLabelText( "User Interaction" );
  this->RadioButtonSet->Create();
  this->RadioButtonSet->SetBorderWidth(2);
  this->RadioButtonSet->SetReliefToGroove();
  this->RadioButtonSet->SetMaximumNumberOfWidgetsInPackingDirection(1);

  for (int id = 0; id < 2; id++)          this->RadioButtonSet->AddWidget(id);

  this->RadioButtonSet->GetWidget(0)->SetCommand(this, "PickFaceCallback");
  this->RadioButtonSet->GetWidget(0)->SetText("Pick");
  this->RadioButtonSet->GetWidget(0)->SetImageToPredefinedIcon(vtkKWIcon::IconBoundingBox);
  this->RadioButtonSet->GetWidget(0)->IndicatorVisibilityOff();
  this->RadioButtonSet->GetWidget(0)->SetBalloonHelpString("Pick face of the Building Block");
  this->RadioButtonSet->GetWidget(0)->SetValue("Pick");
  this->RadioButtonSet->GetWidget(0)->SetCompoundModeToLeft();

  this->RadioButtonSet->GetWidget(1)->SetText("VTK");
  this->RadioButtonSet->GetWidget(1)->SetCommand(this, "VTKinteractionCallback");
  this->RadioButtonSet->GetWidget(1)->SetImageToPredefinedIcon(vtkKWIcon::IconRotate);
  this->RadioButtonSet->GetWidget(1)->IndicatorVisibilityOff();
  this->RadioButtonSet->GetWidget(1)->SetBalloonHelpString("VTK interaction");
  this->RadioButtonSet->GetWidget(1)->SetVariableName(this->RadioButtonSet->GetWidget(0)->GetVariableName());
  this->RadioButtonSet->GetWidget(1)->SetValue("VTK");
  this->RadioButtonSet->GetWidget(1)->SetCompoundModeToLeft();

  this->GetApplication()->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->RadioButtonSet->GetWidgetName());
  //
  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "CreateElementSetPickFaceApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateElementSetPickFaceCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

  this->RadioButtonSet->GetWidget(1)->SelectedStateOn();


}
//----------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateElementSetPickFaceGroup::CreateElementSetPickFaceApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

  if(!strcmp(this->FEMeshListComboBox->GetWidget()->GetValue(),""))
  {
        callback->ErrorMessage("Projection surface not chosen");
        return 0;
  }
    if(!strcmp(this->BBListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("Building Block from which FE mesh to be generated not chosen");
          return 0;
        }
    vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid FE Mesh");
                combobox->SetValue("");
                return 0;
        }

    vtkUnstructuredGrid *femesh = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

    combobox = this->BBListComboBox->GetWidget();
    name = combobox->GetValue();

        num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Building-block structure");
                combobox->SetValue("");
                return 0;
        }

    vtkUnstructuredGrid *bbox = vtkMimxUnstructuredGridActor::SafeDownCast(
      this->BBoxList->GetItem(this->OriginalPosition->GetValue(
          combobox->GetValueIndex(name))))->GetDataSet();

        if(!bbox->GetCellData()->GetVectors("Mesh_Seed"))
        {
                callback->ErrorMessage("Choose Building-block structure with mesh seeds assigned");
                combobox->SetValue("");
                return 0;
        }
    //if(!
//ugrid->GetCellData()->GetScalars("Mesh_Seed"))
    //  return;
        if(!this->ExtractFaceWidget)
        {
                callback->ErrorMessage("Face not picked");
                return 0;
        }
        if(this->ExtractFaceWidget->GetFacePoints()->GetNumberOfComponents() != 4)
        {
                callback->ErrorMessage("Faces picked should be quads");
                return 0;
        }

        if(this->ExtractFaceWidget->GetFacePoints()->GetNumberOfTuples() <= 0)
        {
                callback->ErrorMessage("Face selections invalid");
                return 0;
        }

        vtkMimxExtractElementNumbersOfaFaceFilter *extract = 
      vtkMimxExtractElementNumbersOfaFaceFilter::New();
    extract->SetBoundingBox(bbox);
    extract->SetFeMesh(femesh);
        extract->SetElementSetName(this->ElementSetNameEntry->GetWidget()->GetValue());
        callback->SetState(0);
        extract->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
        extract->SetVertexList(this->ExtractFaceWidget->GetFacePoints());
    extract->Update();
    if (!callback->GetState())
    {
                //vtkKWLoadSaveDialog *FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
                //FileBrowserDialog->SetApplication(this->GetApplication());
                //FileBrowserDialog->Create();
                //FileBrowserDialog->SetTitle ("Element Sets");
                //FileBrowserDialog->SetFileTypes ("{{TXT files} {.txt}}");
                //FileBrowserDialog->SetDefaultExtension (".txt");

                //FileBrowserDialog->Invoke();
                //if(FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
                //{
                //      if(FileBrowserDialog->GetFileName())
                //      {
                //              ofstream FileOutput;
                //              FileOutput.open(FileBrowserDialog->GetFileName(),std::ios::out);
                //              for(int i=0; i <extract->GetCellIdList()->GetNumberOfIds(); i++)
                //              {
                //                      FileOutput <<extract->GetCellIdList()->GetId(i)<<std::endl;
                //              }
                //      }
                //}
         // 
          femesh->GetCellData()->AddArray(extract->GetOutput()->GetCellData()
                  ->GetArray(this->ElementSetNameEntry->GetWidget()->GetValue())); 
          extract->RemoveObserver(callback);
          extract->Delete();
          if(this->ExtractFaceWidget)
          {
                  if(this->ExtractFaceWidget->GetEnabled())
                  {
                          this->ExtractFaceWidget->SetEnabled(0);
                  }
          }
          this->GetMimxMainWindow()->SetStatusText("Created Element Set");
          return 1;
     }
        extract->RemoveObserver(callback);
    extract->Delete();
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::CreateElementSetPickFaceCancelCallback()
{
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        this->MenuGroup->SetMenuButtonsEnabled(1);
        if(this->ExtractFaceWidget)
        {
                if(this->ExtractFaceWidget->GetEnabled())
                {
                        this->ExtractFaceWidget->SetEnabled(0);
                }
        }
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::UpdateObjectLists()
{
  this->UpdateMeshComboBox( this->FEMeshListComboBox->GetWidget() );
  this->UpdateBuildingBlockComboBox( this->BBListComboBox->GetWidget() );
  /*
  this->FEMeshListComboBox->GetWidget()->DeleteAllValues();
  
  if(this->OriginalPosition)
          this->OriginalPosition->Initialize();

  int defaultItem = -1;
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    FEMeshListComboBox->GetWidget()->AddValue(
      this->FEMeshList->GetItem(i)->GetFileName());
      
    int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->FEMeshList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
  }
  
  if (defaultItem != -1)
  {
    FEMeshListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }

  //for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  //{
         // vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
                //  SafeDownCast(this->BBoxList->GetItem(i));
         // vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
         // if(ugrid->GetCellData()->GetArray("Mesh_Seed"))
         // {
                //  BBListComboBox->GetWidget()->AddValue(
                //        this->BBoxList->GetItem(i)->GetFileName());
                //  if(!this->OriginalPosition)
                //        this->OriginalPosition = vtkIntArray::New();
                //  this->OriginalPosition->InsertNextValue(i);
         // }
  //}

  this->BBListComboBox->GetWidget()->DeleteAllValues();
  
  if (this->DoUndoTree != NULL )
  {
    defaultItem = -1;
    for (int i=0; i<this->DoUndoTree->GetNumberOfItems(); i++)
    {
          Node *currnode = this->DoUndoTree->GetItem(i);

          while(currnode->Child != NULL)
          {
                  currnode = currnode->Child;
          } 
          BBListComboBox->GetWidget()->AddValue(
                  currnode->Data->GetFileName());
                int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        currnode->Data->GetActor());
      if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
    }
    
    if (defaultItem != -1)
    {
      Node *currnode = this->DoUndoTree->GetItem(defaultItem);
      while(currnode->Child != NULL)
          {
                  currnode = currnode->Child;
          } 
          BBListComboBox->GetWidget()->SetValue( currnode->Data->GetFileName());
    }
  }
  */
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::CreateElementSetPickFaceDoneCallback()
{
        if(this->CreateElementSetPickFaceApplyCallback())
                this->CreateElementSetPickFaceCancelCallback();
}
//---------------------------------------------------------------------------------

void vtkKWMimxCreateElementSetPickFaceGroup::PickFaceCallback()
{
        if(!strcmp(this->BBListComboBox->GetWidget()->GetValue(),""))
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                callback->ErrorMessage("Building Block selection required");
        }
        else{
                if(this->AddButtonState)
                {
                        vtkKWComboBox *combobox = this->BBListComboBox->GetWidget();
                        const char *name = combobox->GetValue();
                        //int num = combobox->GetValueIndex(name);
                        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
                                ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                        if(this->ExtractFaceWidget)
                        {
                                if(this->ExtractFaceWidget->GetEnabled())
                                {
                                        this->ExtractFaceWidget->SetEnabled(0);
                                }
                                this->ExtractFaceWidget->Delete();
                                this->ExtractFaceWidget = NULL;
                        }
                        this->ExtractFaceWidget = vtkMimxExtractMultipleFaceWidget::New();
                        this->ExtractFaceWidget->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()
                                ->GetRenderWindowInteractor());
                        this->ExtractFaceWidget->SetInput(ugrid);
                        this->ExtractFaceWidget->SetInputActor(this->BBoxList
                                ->GetItem(combobox->GetValueIndex(name))->GetActor());
                        this->ExtractFaceWidget->SetEnabled(1);
                        this->AddButtonState = 0;
                }
                else
                {
                        if(!this->ExtractFaceWidget)
                        {
                                vtkKWComboBox *combobox = this->BBListComboBox->GetWidget();
                                const char *name = combobox->GetValue();
                                //int num = combobox->GetValueIndex(name);
                                vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
                                        this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                                this->ExtractFaceWidget = vtkMimxExtractMultipleFaceWidget::New();
                                this->ExtractFaceWidget->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()
                                        ->GetRenderWindowInteractor());
                                this->ExtractFaceWidget->SetInput(ugrid);
                                this->ExtractFaceWidget->SetInputActor(this->BBoxList
                                        ->GetItem(combobox->GetValueIndex(name))->GetActor());
                                this->ExtractFaceWidget->SetEnabled(1);
                                this->AddButtonState = 0;
                        }
                        else
                        {
                                if(!this->ExtractFaceWidget->GetEnabled())
                                {
                                        this->ExtractFaceWidget->SetEnabled(1);
                                }
                        }
                }
        }
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup:: VTKinteractionCallback()
{
        if(this->ExtractFaceWidget)
        {
                if(this->ExtractFaceWidget->GetEnabled())
                {
                        this->ExtractFaceWidget->SetEnabled(0);
                }
        }
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateElementSetPickFaceGroup::SelectionChangedCallback(const char *vtkNotUsed(dummy))
{
        this->AddButtonState = 1;
        if(this->RadioButtonSet->GetWidget(0)->GetSelectedState())
        {
                this->RadioButtonSet->GetWidget(0)->SetSelectedState(1);
                this->PickFaceCallback();
        }
}
//---------------------------------------------------------------------------------
