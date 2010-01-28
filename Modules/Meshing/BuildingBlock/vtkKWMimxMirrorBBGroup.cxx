/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMirrorBBGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.19.4.2 $

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

#include "vtkKWMimxMirrorBBGroup.h"

#include "vtkActor.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneWidget.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWPushButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRenderWidget.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxMirrorUnstructuredHexahedronGridCell.h"

#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxMainWindow.h"

#include "vtkLinkedListWrapper.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMirrorBBGroup);
vtkCxxRevisionMacro(vtkKWMimxMirrorBBGroup, "$Revision: 1.19.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxMirrorBBGroup::vtkKWMimxMirrorBBGroup()
{
        this->EditBBGroup = NULL;
        this->VTKRadioButton = NULL;
        this->TypeOfMirroring = vtkKWCheckButtonWithLabel::New();
        this->AxisSelection = vtkKWRadioButtonSet::New();
        this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
        this->MirrorPlaneWidget = NULL;
        this->Count = 0;
}

//----------------------------------------------------------------------------
vtkKWMimxMirrorBBGroup::~vtkKWMimxMirrorBBGroup()
{
        this->TypeOfMirroring->Delete();
        this->AxisSelection->Delete();
        this->ObjectListComboBox->Delete();
        if(this->MirrorPlaneWidget)
                this->MirrorPlaneWidget->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::CreateWidget()
{
  if(this->IsCreated())
  {
  vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Mirror Building Blocks");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  this->ObjectListComboBox->SetParent(this->MainFrame);
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetEnabled(0);
  this->ObjectListComboBox->SetWidth(20);
  this->ObjectListComboBox->SetLabelText("Building Block : ");

  int i;
  for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
          ObjectListComboBox->GetWidget()->AddValue(
                  this->BBoxList->GetItem(i)->GetFileName());
  }
  this->ObjectListComboBox->GetWidget()->SetValue(
          this->EditBBGroup->GetObjectListComboBox()->GetWidget()->GetValue());
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6", 
          this->ObjectListComboBox->GetWidgetName());

  this->TypeOfMirroring->SetParent(this->MainFrame);
  this->TypeOfMirroring->Create();
  this->TypeOfMirroring->SetLabelText("About arbitrary plane");

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
          this->TypeOfMirroring->GetWidgetName());
  this->TypeOfMirroring->GetWidget()->SetEnabled(0);

  this->AxisSelection->SetParent(this->MainFrame);
  this->AxisSelection->Create();
  this->AxisSelection->SetBorderWidth(2);
  this->AxisSelection->SetReliefToGroove();

  vtkKWRadioButton *rb;
  rb = this->AxisSelection->AddWidget(0);
  rb->SetText("About XY Plane");
  rb = this->AxisSelection->AddWidget(1);
  rb->SetText("About XZ Plane");
  rb = this->AxisSelection->AddWidget(2);
  rb->SetText("About YZ Plane");

  this->AxisSelection->GetWidget(0)->SetCommand(this, "PlaceMirroringPlaneAboutZ");
  this->AxisSelection->GetWidget(1)->SetCommand(this, "PlaceMirroringPlaneAboutY");
  this->AxisSelection->GetWidget(2)->SetCommand(this, "PlaceMirroringPlaneAboutX");

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->AxisSelection->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "MirrorBBApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "MirrorBBCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());
 // this->AxisSelection->GetWidget(0)->SetSelectedState(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxMirrorBBGroup::MirrorBBApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        callback->SetState(0);

                if(this->MirrorPlaneWidget)
                {
                        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                        const char *name = combobox->GetValue();

                        int num = combobox->GetValueIndex(name);
                        if(num < 0 || num > combobox->GetNumberOfValues()-1)
                        {
                                callback->ErrorMessage("Choose valid Building-block structure");
                                combobox->SetValue("");
                                return 0;
                        }

                        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
                                this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                        const char *foundationname = this->BBoxList->GetItem(combobox->GetValueIndex(name))->GetFoundationName();
                        vtkMimxMirrorUnstructuredHexahedronGridCell *mirror = 
                                vtkMimxMirrorUnstructuredHexahedronGridCell::New();
                        mirror->SetInput(ugrid);
                        vtkPlane *Plane = vtkPlane::New();
                        this->MirrorPlaneWidget->GetPlane(Plane);
                        mirror->SetMirrorPlane(Plane);
                        mirror->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                        mirror->Update();
                        //vtkIdType objnum = combobox->GetValueIndex(name);
                        if (!callback->GetState())
                        {
                                this->MirrorPlaneWidget->SetEnabled(0);
                                this->EditBBGroup->AddEditedBB(num, mirror->GetOutput(), "Mirror_", this->Count, foundationname);
                                mirror->RemoveObserver(callback);
                                mirror->Delete();
                                this->UpdateObjectLists();
                                return 1;
                        }
                        mirror->RemoveObserver(callback);
                        mirror->Delete();
                        Plane->Delete();
                        this->GetMimxMainWindow()->SetStatusText("Mirrored Building Block");
                        return 0;
                }
                return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::MirrorBBCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
//  this->MenuGroup->SetMenuButtonsEnabled(1);
  if(this->EditBBGroup)
        this->GetApplication()->Script(
                 "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
                this->EditBBGroup->GetMainFrame()->GetWidgetName());
  else
            this->MenuGroup->SetMenuButtonsEnabled(1);
  if(this->VTKRadioButton)
          this->VTKRadioButton->SetSelectedState(1);

  if(this->MirrorPlaneWidget)
  {
          if(this->MirrorPlaneWidget->GetEnabled())
          {
                  this->MirrorPlaneWidget->SetEnabled(0);
          }
          this->MirrorPlaneWidget->Delete();
          this->MirrorPlaneWidget = NULL;
  }
  this->EditBBGroup->DeselectAllButtons();
  this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
          this->BBoxList->GetItem(this->ObjectListComboBox->GetWidget()->GetValueIndex(
          this->ObjectListComboBox->GetWidget()->GetValue()))->GetActor());
  
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//--------------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::PlaceMirroringPlaneAboutX()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("Building Block selection required");
        }
        else
        {
                vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                const char *name = combobox->GetValue();
                int num = combobox->GetValueIndex(name);
                if(num < 0 || num > combobox->GetNumberOfValues()-1)
                {
                        callback->ErrorMessage("Choose valid Building-block structure");
                        combobox->SetValue("");
                        return;
                }
                vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
                        ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                vtkActor *actor = this->BBoxList->GetItem(combobox->GetValueIndex(name))->GetActor();
                if(!this->MirrorPlaneWidget)
                {
                        this->MirrorPlaneWidget = vtkPlaneWidget::New();
                        double bounds[6];
                        ugrid->GetBounds(bounds);
                        this->MirrorPlaneWidget->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()
                                ->GetRenderWindowInteractor());
                        this->MirrorPlaneWidget->SetProp3D(actor);
                        this->MirrorPlaneWidget->PlaceWidget(bounds);
                }
                else
                {
                        double center[3];
                        this->MirrorPlaneWidget->SetEnabled(0);
                        this->MirrorPlaneWidget->GetOrigin(center);
                        this->MirrorPlaneWidget->SetProp3D(actor);
                        this->MirrorPlaneWidget->SetOrigin(center);
                }
                this->MirrorPlaneWidget->SetNormal(1.0,0.0,0.0);
                //this->MirrorPlaneWidget->NormalToXAxisOn();
                this->MirrorPlaneWidget->SetEnabled(1);
        }
}
//----------------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::PlaceMirroringPlaneAboutY()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("Building Block selection required");
        }
        else
        {
                vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                const char *name = combobox->GetValue();
                int num = combobox->GetValueIndex(name);
                if(num < 0 || num > combobox->GetNumberOfValues()-1)
                {
                        callback->ErrorMessage("Choose valid Building-block structure");
                        combobox->SetValue("");
                        return;
                }
                vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
                        ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                vtkActor *actor = this->BBoxList->GetItem(combobox->GetValueIndex(name))->GetActor();
                if(!this->MirrorPlaneWidget)
                {
                        this->MirrorPlaneWidget = vtkPlaneWidget::New();
                        double bounds[6];
                        ugrid->GetBounds(bounds);
                        this->MirrorPlaneWidget->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()
                                ->GetRenderWindowInteractor());
                        this->MirrorPlaneWidget->SetProp3D(actor);
                        this->MirrorPlaneWidget->PlaceWidget(bounds);
                }
                else
                {
                        double center[3];
                        this->MirrorPlaneWidget->SetEnabled(0);
                        this->MirrorPlaneWidget->GetOrigin(center);
                        this->MirrorPlaneWidget->SetProp3D(actor);
                        this->MirrorPlaneWidget->SetOrigin(center);
                }
                this->MirrorPlaneWidget->SetNormal(0.0,1.0,0.0);
                this->MirrorPlaneWidget->SetEnabled(1);
        }

}
//----------------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::PlaceMirroringPlaneAboutZ()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("Building Block selection required");
        }
        else
        {
                vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                const char *name = combobox->GetValue();
                int num = combobox->GetValueIndex(name);
                if(num < 0 || num > combobox->GetNumberOfValues()-1)
                {
                        callback->ErrorMessage("Choose valid Building-block structure");
                        combobox->SetValue("");
                        return;
                }
                vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
                        ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                vtkActor *actor = this->BBoxList->GetItem(combobox->GetValueIndex(name))->GetActor();
                if(!this->MirrorPlaneWidget)
                {
                        this->MirrorPlaneWidget = vtkPlaneWidget::New();
                        double bounds[6];
                        ugrid->GetBounds(bounds);
                        this->MirrorPlaneWidget->SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()
                                ->GetRenderWindowInteractor());
                        this->MirrorPlaneWidget->SetProp3D(actor);
                        this->MirrorPlaneWidget->PlaceWidget(bounds);
                }
                else
                {
                        double center[3];
                        this->MirrorPlaneWidget->SetEnabled(0);
                        this->MirrorPlaneWidget->GetOrigin(center);
                        this->MirrorPlaneWidget->SetProp3D(actor);
                        this->MirrorPlaneWidget->SetOrigin(center);
                }
                this->MirrorPlaneWidget->SetNormal(0.0,0.0,1.0);
                this->MirrorPlaneWidget->SetEnabled(1);
        }
}
//----------------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::UpdateObjectLists()
{
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        int i;
        for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->BBoxList->GetItem(i)->GetFileName());
        }
        this->ObjectListComboBox->GetWidget()->SetValue(
                this->EditBBGroup->GetObjectListComboBox()->GetWidget()->GetValue());
}
//-----------------------------------------------------------------------------------
void vtkKWMimxMirrorBBGroup::SelectionChangedCallback(const char* vtkNotUsed(dummy))
{
        if(this->AxisSelection->GetWidget(0)->GetSelectedState())
        {
                this->PlaceMirroringPlaneAboutX();
                return;
        }
        if(this->AxisSelection->GetWidget(1)->GetSelectedState())
        {
                this->PlaceMirroringPlaneAboutY();
                return;
        }
        if(this->AxisSelection->GetWidget(2)->GetSelectedState())
        {
                this->PlaceMirroringPlaneAboutZ();
                return;
        }
}
//----------------------------------------------------------------------------------------
