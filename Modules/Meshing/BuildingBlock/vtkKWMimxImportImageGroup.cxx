/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImportImageGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.1.2.1 $

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

#include "vtkKWMimxImportImageGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"
#include "vtkImageData.h"

#include "vtkKWApplication.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEvent.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"

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
#include "vtkRenderer.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkCommand.h"
#include "vtkKWEntry.h"
#include "vtkCollection.h"


#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxImportImageGroup);
vtkCxxRevisionMacro(vtkKWMimxImportImageGroup, "$Revision: 1.1.2.1 $");
//----------------------------------------------------------------------------
vtkKWMimxImportImageGroup::vtkKWMimxImportImageGroup()
{
    this->ImageListComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxImportImageGroup::~vtkKWMimxImportImageGroup()
{
    if(this->ImageListComboBox)
       this->ImageListComboBox->Delete();
  
}
//----------------------------------------------------------------------------
void vtkKWMimxImportImageGroup::CreateWidget()
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

    // display a list of models for the user to choose from; register a callback
    // upon selection to set and import the selected model
    if (!this->ImageListComboBox)
        this->ImageListComboBox = vtkKWComboBoxWithLabel::New();
    //ImageListComboBox->SetParent(this->MainFrame);
    ImageListComboBox->SetParent(this->MainFrame);
    ImageListComboBox->Create();
    ImageListComboBox->SetLabelText("MRML Volumes : ");
    ImageListComboBox->SetLabelWidth( 15 );
    ImageListComboBox->GetWidget()->ReadOnlyOn();
     this->GetApplication()->Script(
       "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x",
       ImageListComboBox->GetWidgetName());

     this->ApplyButton->SetParent(this->MainFrame);
      this->ApplyButton->Create();
      this->ApplyButton->SetReliefToFlat();
      this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
      this->ApplyButton->SetCommand(this, "SelectVolumeToImportToImageCallback");
      this->GetApplication()->Script(
              "pack %s -side left -anchor nw -expand y -padx 5 -pady 6",
              this->ApplyButton->GetWidgetName());

      this->CancelButton->SetParent(this->MainFrame);
      this->CancelButton->Create();
      this->CancelButton->SetReliefToFlat();
      this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
      this->CancelButton->SetCommand(this, "ImportToImageCancelCallback");
      this->GetApplication()->Script(
        "pack %s -side right -anchor ne -expand y -padx 5 -pady 6",
        this->CancelButton->GetWidgetName());

      // fill the box with the models that are currently in the scene, so the user can select from them
      this->FillModelComboBox();
}

void vtkKWMimxImportImageGroup::FillModelComboBox()
{
  // since slicer models can serve as surfaces to begin the meshing process, we want
  // to discover any new models  and add them to a list the user can
  // browse.

  std::vector<vtkMRMLNode *> hnodes;
  int nnodes;

  hnodes.clear();
  this->ImageListComboBox->GetWidget()->DeleteAllValues();
  vtkMRMLScene* scene = vtkMRMLScene::GetActiveScene();
  nnodes = scene->GetNodesByClass("vtkMRMLScalarVolumeNode", hnodes);

  for (unsigned int i=0; i<hnodes.size(); i++)
  {

    vtkMRMLScalarVolumeNode *hnode = vtkMRMLScalarVolumeNode::SafeDownCast(hnodes[i]);
//    cout << "found volume        : " << hnode->GetName() << endl;
//    cout << "      volume has tag: " << hnode->GetNodeTagName() << endl;

    // don't allow the user to pick previously-imported image datasets.  If a user has previously imported an image,
    // it would show up on the list for import again, unless we exclude items of the FiniteElementImage class
    if (!(strstr(hnode->GetNodeTagName(),"FiniteElementImage")))
    this->ImageListComboBox->GetWidget()->AddValue( hnode->GetName());
  }
}

int vtkKWMimxImportImageGroup::SelectVolumeToImportToImageCallback()
{

    // if the user hasn't selected a model and there are available model, then issue an error prompt.  If they have selected, then
    // find the name from the combobox, then import the polys corresponding to the model into the SurfaceList. If there were no models
    // to select, then don't issue an error message, just return without doing anything.

    if(!strcmp(this->ImageListComboBox->GetWidget()->GetValue(),""))
    {
            if (ImageListComboBox->GetWidget()->GetNumberOfValues()>0)
                    this->GetMimxMainWindow()->SetStatusText("No Model Selected for Import");
            return 0;
    }
    vtkKWComboBox *combobox = this->ImageListComboBox->GetWidget();
    const char *name = combobox->GetValue();
//    int num = combobox->GetValueIndex(name);

    // get the model polys here by retrieving from the MRML scene. Then add the polydata as a new
    // entry in the SurfaceList so processing of the model can continue through the IA-FEMesh pipeline

    std::vector<vtkMRMLNode *> hnodes;
    int nnodes;

    hnodes.clear();
    vtkMRMLScene* scene = vtkMRMLScene::GetActiveScene();
    nnodes = scene->GetNodesByClass("vtkMRMLScalarVolumeNode", hnodes);

      for (unsigned int i=0; i<hnodes.size(); i++)
      {
          double imageOrigin[3], imageSpacing[3];
          vtkMRMLScalarVolumeNode *hnode = vtkMRMLScalarVolumeNode::SafeDownCast(hnodes[i]);
          hnode->GetOrigin(imageOrigin);

          // now check that the name matches what we are looking for, as selected by the user.
          // this may have to be ammended later to filter out a previously imported image of the same name.
          //if (!(strstr(hnode->GetNodeTagName(),"FiniteElementImage")))

          if (!(strcmp(hnode->GetName(),name)))
          {
               // we now it is a volume with the matching name, so get the image data and create a new entry in the ImageList.
               vtkImageData* newimage = vtkMRMLScalarVolumeNode::SafeDownCast(hnode)->GetImageData();
               vtkMatrix4x4* matrix = vtkMatrix4x4::New();
               vtkMRMLScalarVolumeNode::SafeDownCast(hnode)->GetRASToIJKMatrix(matrix);

               hnode->GetOrigin(imageOrigin);
               hnode->GetSpacing(imageSpacing);

//               cout << "ImportImageGroup matrix:" << matrix << endl;
//               matrix->Print(std::cout);
//               cout << "origin: (" << imageOrigin[0] << imageOrigin[1] << imageOrigin[2] << ")" << endl;
//               cout << "spacing: (" << imageSpacing[0] << imageSpacing[1] << imageSpacing[2] << ")" << endl;

               this->AddImageToDisplay(newimage, "", name, matrix, imageOrigin,imageSpacing );
               //this->AddImageToDisplay(newimage, "", name );
              this->UpdateObjectLists();
              this->GetMimxMainWindow()->SetStatusText("Imported a MRML Volume for display");
              matrix->Delete();
              // *** we could disable the display of the original object here, but we are assuming the user will do that if desired
            }
      }
      // restore interface now that the model has been added
      this->ImportToImageCancelCallback();

      return VTK_OK;
}


//----------------------------------------------------------------------------
void vtkKWMimxImportImageGroup::ImportToImageCancelCallback()
{

  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName() );
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}

//----------------------------------------------------------------------------
void vtkKWMimxImportImageGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxImportImageGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxImportImageGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkKWMimxImportImageGroup::UpdateObjectLists()
{
        
}
//---------------------------------------------------------------------------------
