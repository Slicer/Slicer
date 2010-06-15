/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImportModelSurfaceGroup.cxx,v $
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

#include "vtkKWMimxImportModelSurfaceGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"
#include "vtkPolyData.h"

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
#include "vtkMRMLModelNode.h"


#include "vtkSTLReader.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataReader.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxImportModelSurfaceGroup);
vtkCxxRevisionMacro(vtkKWMimxImportModelSurfaceGroup, "$Revision: 1.1.2.1 $");
//----------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::FillModelComboBox()
{
  // since slicer models can serve as surfaces to begin the meshing process, we want
  // to discover any new models  and add them to a list the user can
  // browse.

  std::vector<vtkMRMLNode *> hnodes;
  int nnodes;

  hnodes.clear();
  this->ModelListComboBox->GetWidget()->DeleteAllValues();
  vtkMRMLScene* scene = vtkMRMLScene::GetActiveScene();
  nnodes = scene->GetNodesByClass("vtkMRMLModelNode", hnodes);

  for (unsigned int i=0; i<hnodes.size(); i++)
  {

    vtkMRMLModelNode *hnode = vtkMRMLModelNode::SafeDownCast(hnodes[i]);
    //cout << "found model        : " << hnode->GetName() << endl;
    //cout << "      model has tag: " << hnode->GetNodeTagName() << endl;

    // defeat the addition of slice viewers.  They show up as Models in the MRML scene, but
    // we can ignore them by rejecting anything containing the substring "Volume Slice".  This
    // may have to be expanded in the future to reject other non-data Models in the MRML tree.

    if (!(strstr(hnode->GetName(),"Volume Slice")))
       this->ModelListComboBox->GetWidget()->AddValue( hnode->GetName());
  }
}

vtkKWMimxImportModelSurfaceGroup::vtkKWMimxImportModelSurfaceGroup()
{
    this->ModelListComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxImportModelSurfaceGroup::~vtkKWMimxImportModelSurfaceGroup()
{
    if(this->ModelListComboBox)
       this->ModelListComboBox->Delete();
  
}
//----------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::CreateWidget()
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
    if (!this->ModelListComboBox)
        this->ModelListComboBox = vtkKWComboBoxWithLabel::New();
    //ModelListComboBox->SetParent(this->MainFrame);
    ModelListComboBox->SetParent(this->MainFrame);
    ModelListComboBox->Create();
    ModelListComboBox->SetLabelText("Models : ");
    ModelListComboBox->SetLabelWidth( 15 );
    ModelListComboBox->GetWidget()->ReadOnlyOn();
     this->GetApplication()->Script(
       "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x",
       ModelListComboBox->GetWidgetName());

     this->ApplyButton->SetParent(this->MainFrame);
      this->ApplyButton->Create();
      this->ApplyButton->SetReliefToFlat();
      this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
      this->ApplyButton->SetCommand(this, "SelectModelToImportToSurfaceCallback");
      this->GetApplication()->Script(
              "pack %s -side left -anchor nw -expand y -padx 5 -pady 6",
              this->ApplyButton->GetWidgetName());

      this->CancelButton->SetParent(this->MainFrame);
      this->CancelButton->Create();
      this->CancelButton->SetReliefToFlat();
      this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
      this->CancelButton->SetCommand(this, "ImportToSurfaceCancelCallback");
      this->GetApplication()->Script(
        "pack %s -side right -anchor ne -expand y -padx 5 -pady 6",
        this->CancelButton->GetWidgetName());

      // fill the box with the models that are currently in the scene, so the user can select from them
      this->FillModelComboBox();
}

int vtkKWMimxImportModelSurfaceGroup::SelectModelToImportToSurfaceCallback()
{

    // if the user hasn't selected a model and there are available model, then issue an error prompt.  If they have selected, then
    // find the name from the combobox, then import the polys corresponding to the model into the SurfaceList. If there were no models
    // to select, then don't issue an error message, just return without doing anything.

    if(!strcmp(this->ModelListComboBox->GetWidget()->GetValue(),""))
    {
            if (ModelListComboBox->GetWidget()->GetNumberOfValues()>0)
                    this->GetMimxMainWindow()->SetStatusText("No Model Selected for Import");
            return 0;
    }
    vtkKWComboBox *combobox = this->ModelListComboBox->GetWidget();
    const char *name = combobox->GetValue();
//    int num = combobox->GetValueIndex(name);

    // get the model polys here by retrieving from the MRML scene. Then add the polydata as a new
    // entry in the SurfaceList so processing of the model can continue through the IA-FEMesh pipeline

    std::vector<vtkMRMLNode *> hnodes;
    int nnodes;

    hnodes.clear();
    vtkMRMLScene* scene = vtkMRMLScene::GetActiveScene();
    nnodes = scene->GetNodesByClass("vtkMRMLModelNode", hnodes);

      for (unsigned int i=0; i<hnodes.size(); i++)
      {

        vtkMRMLModelNode *hnode = vtkMRMLModelNode::SafeDownCast(hnodes[i]);

        // defeat the addition of slice viewers.  They show up as Models in the MRML scene, but
        // we can ignore them by rejecting anything containing the substring "Volume Slice".  This
        // may have to be expanded in the future to reject other non-data Models in the MRML tree.

        if (!(strstr(hnode->GetName(),"Volume Slice")))
            // now check that the name matches what we are looking for, as selected by the user
            if (!(strcmp(hnode->GetName(),name)))
            {
                // we know it is a model with the matching name, so get the polydata and create a new entry in the SurfaceList
                vtkPolyData* modelPolys = vtkMRMLModelNode::SafeDownCast(hnode)->GetPolyData();
                if (modelPolys)
                {
                    //cout << "Found model with " << numberOfPolys << " polygons" << endl;
                    this->AddSurfaceToDisplay(modelPolys, "", name );
                    this->UpdateObjectLists();
                    this->GetMimxMainWindow()->SetStatusText("Imported a MRML Model for meshing");
                    // *** we could disable the display of the original model here, but we are assuming the user will do that if desired
                }
                else
                    vtkErrorMacro("IA-FEMesh: couldn't import model polygons.  An error occured reading MRML scene")
            }
      }
      // restore interface now that the model has been added
      this->ImportToSurfaceCancelCallback();
      return 1;
}


//----------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::ImportToSurfaceCancelCallback()
{

  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName() );
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}

//----------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkKWMimxImportModelSurfaceGroup::UpdateObjectLists()
{
        
}
//---------------------------------------------------------------------------------
