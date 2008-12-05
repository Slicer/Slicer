/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDeleteObjectGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.27.4.2 $

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

#include "vtkKWMimxDeleteObjectGroup.h"

#include "vtkActor.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxImageActor.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkKWMimxMainNotebook.h"
//#include "vtkKWMimxMainNoteBook.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainUserInterfacePanel.h"

#include "vtkLinkedListWrapper.h"
#include "vtkLinkedListWrapperTree.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDeleteObjectGroup);
vtkCxxRevisionMacro(vtkKWMimxDeleteObjectGroup, "$Revision: 1.27.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxDeleteObjectGroup::vtkKWMimxDeleteObjectGroup()
{
  this->ObjectListComboBox = NULL;
  this->BBoxList = NULL;
  this->SurfaceList = NULL;
  this->ImageList = NULL;
  this->FEMeshList = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxDeleteObjectGroup::~vtkKWMimxDeleteObjectGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Delete Objects");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Object : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  this->UpdateObjectLists();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  this->ApplyButton->SetParent( this->MainFrame );
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "DeleteObjectApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent( this->MainFrame );
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "DeleteObjectCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxDeleteObjectGroup::DeleteObjectApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
          if (this->BBoxList) callback->ErrorMessage("Select building block to be deleted");
          if (this->FEMeshList) callback->ErrorMessage("Select mesh to be deleted");
          if (this->ImageList) callback->ErrorMessage("Select image to be deleted");
          if (this->SurfaceList) callback->ErrorMessage("Select surface to be deleted");
          return 0;
  }
    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        if(this->SurfaceList)
        {
                //vtkMimxActorBase *actorbase = this->SurfaceList->GetItem(
                //      combobox->GetValueIndex(name));
                vtkMimxSurfacePolyDataActor *polydataactor = vtkMimxSurfacePolyDataActor::
                        SafeDownCast(this->SurfaceList->GetItem(combobox->GetValueIndex(name)));
                if(polydataactor)
                {
                        this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(2, num);
                        polydataactor->Delete();
                        this->SurfaceList->RemoveItem(num);
                        this->UpdateObjectLists();
                        this->ObjectListComboBox->GetWidget()->SetValue("");
                        this->GetMimxMainWindow()->SetStatusText("Deleted Surface");
                        return 1;
                }
        }
        if(this->BBoxList)
        {
                // bbox list needs to be treated differently
                Node *childnode = this->DoUndoTree->GetItem(name)->Child;
                Node *parentnode = this->DoUndoTree->GetItem(name)->Parent;
                Node *currnode = this->DoUndoTree->GetItem(name);
                int deleteStatus = false;
                if(childnode || parentnode)
                {
                        vtkKWMessageDialog *Dialog = vtkKWMessageDialog::New();
                        Dialog->SetStyleToOkOtherCancel();
                        Dialog->SetOKButtonText("All");
                        Dialog->SetOtherButtonText("Current");
                        Dialog->SetApplication(this->GetApplication());
                        Dialog->Create();
                        Dialog->SetTitle("Your Attention Please!");
                        Dialog->SetText("Several building block structures contributed to this structure. Which block(s) would you like to remove?");
                        Dialog->Invoke();
                        if(Dialog->GetStatus() == vtkKWMessageDialog::StatusCanceled)
                        {
                                Dialog->Delete();
                                return 1;
                        }
                        if(Dialog->GetStatus() == vtkKWMessageDialog::StatusOK)
                        {
                                deleteStatus = true;
                        }
                        if(childnode)
                                childnode->Parent = parentnode;
                        if(parentnode)
                                parentnode->Child = childnode;
                        //              this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(name);
                        this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(5,num);
                        this->BBoxList->RemoveItem(num);
                        if(childnode)
                        {
                                if(!deleteStatus)
                                {
                                        this->BBoxList->AppendItem(childnode->Data);
                                        this->GetMimxMainWindow()->GetViewProperties()->
                                                AddObjectList(childnode->Data);
                                }
                        }
                        else
                                if(parentnode)
                                {
                                        if(!deleteStatus)
                                        {
                                                this->BBoxList->AppendItem(parentnode->Data);
                                                this->GetMimxMainWindow()->GetViewProperties()->
                                                        AddObjectList(parentnode->Data);
                                        }
                                }
                                this->DoUndoTree->RemoveItem(this->DoUndoTree->GetItemNumber(name));
                                currnode->Data->Delete();
                                delete currnode;
                                Dialog->Delete();
                }
                else{
                        if(childnode)
                                childnode->Parent = parentnode;
                        if(parentnode)
                                parentnode->Child = childnode;
                        //              this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(name);
                        this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(5,num);
                        this->BBoxList->RemoveItem(num);
                        if(childnode)
                        {
                                this->BBoxList->AppendItem(childnode->Data);
                                this->GetMimxMainWindow()->GetViewProperties()->
                                        AddObjectList(childnode->Data);
                        }
                        else
                                if(parentnode)
                                {
                                        this->BBoxList->AppendItem(parentnode->Data);
                                        this->GetMimxMainWindow()->GetViewProperties()->
                                                AddObjectList(parentnode->Data);
                                }
                                this->DoUndoTree->RemoveItem(this->DoUndoTree->GetItemNumber(name));
                                currnode->Data->Delete();
                                delete currnode;
                                this->ObjectListComboBox->GetWidget()->SetValue("");
                                this->UpdateObjectLists();
                                return 1;
                }
                if(!deleteStatus)
                {
                        this->ObjectListComboBox->GetWidget()->SetValue("");
                        this->UpdateObjectLists();
                        return 1;
                }
                //
                if(parentnode)  currnode = parentnode;
                if(childnode)   currnode = childnode;
                // else go to the last building-block
                while (currnode->Child)
                {
                        currnode = currnode->Child;
                }
                //  and delete bottom up
                while (currnode)
                {
                        parentnode = currnode->Parent;
                        this->DoUndoTree->RemoveItem(
                                this->DoUndoTree->GetItemNumber(currnode->Data->GetFileName()));
                        currnode->Data->Delete();
                        delete currnode;
                        currnode = parentnode;
//                      if(!currnode)   return 1;
                }
                this->UpdateObjectLists();
                this->ObjectListComboBox->GetWidget()->SetValue("");
                return 1;
        }
        if(this->FEMeshList)
        {
                //vtkMimxActorBase *actorbase = this->FEMeshList->GetItem(
                //      combobox->GetValueIndex(name));
                vtkMimxMeshActor *ugridactor = vtkMimxMeshActor::SafeDownCast(
                        this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
                if(ugridactor)
                {
                        this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(6, num);
                        ugridactor->Delete();
                        this->FEMeshList->RemoveItem(num);
                        this->UpdateObjectLists();
                        this->ObjectListComboBox->GetWidget()->SetValue("");
                        this->GetMimxMainWindow()->SetStatusText("Deleted Mesh");
                        return 1;
                }
        }
        if(this->ImageList)
        {
                //vtkMimxActorBase *actorbase = this->ImageList->GetItem(
                //      combobox->GetValueIndex(name));
                vtkMimxImageActor *actor = vtkMimxImageActor::SafeDownCast(
                        this->ImageList->GetItem(combobox->GetValueIndex(name)));
                if (actor )
                {
                        this->GetMimxMainWindow()->GetViewProperties()->DeleteObjectList(ACTOR_IMAGE, num);
                        actor->DisplayActor(0);
                        //actor->Delete();
                        this->ImageList->RemoveItem(num);
                        this->UpdateObjectLists();
                        this->ObjectListComboBox->GetWidget()->SetValue("");
                        this->GetMimxMainWindow()->SetStatusText("Deleted Image");
                        return 1;
                }
        }
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::DeleteObjectCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::UpdateObjectLists()
{
  this->ObjectListComboBox->GetWidget()->DeleteAllValues();
  
  int defaultItem = -1;
  if (this->SurfaceList)
  {
          for (int i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
          {
                ObjectListComboBox->GetWidget()->AddValue(
                  this->SurfaceList->GetItem(i)->GetFileName());
                int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                                         this->SurfaceList->GetItem(i)->GetActor());
      /* Logic Here is opposite of others - Choose the first unviewed Item */
      if ( (defaultItem == -1) && ( ! viewedItem ) )
                {
                  defaultItem = i;
                }
          }
          if (defaultItem != -1)
    {
      ObjectListComboBox->GetWidget()->SetValue(
            this->SurfaceList->GetItem(defaultItem)->GetFileName());
    }
          return;
  }
  if (this->BBoxList)
  {
          for (int i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
          {
                  ObjectListComboBox->GetWidget()->AddValue(
                          this->BBoxList->GetItem(i)->GetFileName());
                        int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                                         this->BBoxList->GetItem(i)->GetActor());
      /* Logic Here is opposite of others - Choose the first unviewed Item */
      if ( (defaultItem == -1) && ( ! viewedItem ) )
                {
                  defaultItem = i;
                }
          }
          if (defaultItem != -1)
    {
      ObjectListComboBox->GetWidget()->SetValue(
            this->BBoxList->GetItem(defaultItem)->GetFileName());
    }
          return;
  }
  if (this->FEMeshList)
  {
          for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
          {
                  ObjectListComboBox->GetWidget()->AddValue(
                          this->FEMeshList->GetItem(i)->GetFileName());
                        int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                                         this->FEMeshList->GetItem(i)->GetActor());
      /* Logic Here is opposite of others - Choose the first unviewed Item */
      if ( (defaultItem == -1) && ( ! viewedItem ) )
                {
                  defaultItem = i;
                }
          }
          if (defaultItem != -1)
    {
      ObjectListComboBox->GetWidget()->SetValue(
            this->FEMeshList->GetItem(defaultItem)->GetFileName());
    }
          return;
  }
  if (this->ImageList)
  {
          for (int i = 0; i < this->ImageList->GetNumberOfItems(); i++)
          {
                  ObjectListComboBox->GetWidget()->AddValue(
                          this->ImageList->GetItem(i)->GetFileName());
                }       
          return;
  }
}
//------------------------------------------------------------------------------
