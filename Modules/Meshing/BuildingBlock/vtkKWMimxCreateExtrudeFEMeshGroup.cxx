/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateExtrudeFEMeshGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.37.4.1 $

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

#include "vtkKWMimxCreateExtrudeFEMeshGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxTraceContourWidget.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxExtrudePolyData.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkMimxMeshActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkProperty.h"
#include "vtkMimxMeshActor.h"
#include "vtkGeometryFilter.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkLinkedListWrapper.h"
#include "vtkMath.h"
#include "vtkIntArray.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkStringArray.h"
#include "vtkKWEntry.h"

#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkPolyDataNormals.h"
#include "vtkHedgeHog.h"
#include "vtkRenderer.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxNodeElementNumbersGroup.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"

#include "vtkMimxSelectCellsWidgetFEMesh.h"
#include "vtkMimxApplyNodeElementNumbers.h"
#include "vtkDataSetCollection.h"
#include "vtkMimxMergeNodesPointAndCellData.h"
#include "vtkKWMimxDefineElSetGroup.h"

#include "Resources/mimxElSetDfns.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateExtrudeFEMeshGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateExtrudeFEMeshGroup, "$Revision: 1.37.4.1 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateExtrudeFEMeshGroup::vtkKWMimxCreateExtrudeFEMeshGroup()
{
  this->MimxMainWindow = NULL;
  this->ObjectListComboBox = NULL;
  this->ExtrusionLength = NULL;
  this->NumberOfDivisions = NULL;
  this->ShowNormalsCheckButton = NULL;
  this->NormalsActor = NULL;
  this->NodeElementNumbersGroup = NULL;
  this->SelectSubsetRadiobuttonSet = NULL;
  this->SelectCellsWidget = NULL;
  this->DefineElSetButton = NULL;
  this->DefineElementSetDialog = NULL;
  this->MaxEleNum = 0;
  this->MaxNodeNum = 0;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateExtrudeFEMeshGroup::~vtkKWMimxCreateExtrudeFEMeshGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
  if(this->ExtrusionLength)
    this->ExtrusionLength->Delete();
  if(this->ShowNormalsCheckButton)
    this->ShowNormalsCheckButton->Delete();
  if(this->NormalsActor)
    this->NormalsActor->Delete();
  if(this->NumberOfDivisions)
    this->NumberOfDivisions->Delete();
  if (this->NodeElementNumbersGroup)
    this->NodeElementNumbersGroup->Delete();
  if (this->SelectSubsetRadiobuttonSet)
    this->SelectSubsetRadiobuttonSet->Delete();
  if(this->SelectCellsWidget)
    this->SelectCellsWidget->Delete();
  if(this->DefineElSetButton)
    this->DefineElSetButton->Delete();
  if(this->DefineElementSetDialog)
    this->DefineElementSetDialog->Delete();
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::CreateWidget()
{
  if(this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  this->Superclass::CreateWidget();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Mesh by Surface extrusion");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if(!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  this->ObjectListComboBox->SetParent(this->MainFrame);
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  this->ObjectListComboBox->SetLabelText("Mesh : ");
  this->ObjectListComboBox->SetLabelWidth(20);
  this->ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->ObjectListComboBox->GetWidget()->SetCommandTrigger(0);
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6 -fill x", 
    this->ObjectListComboBox->GetWidgetName());
  //
  this->ExtrusionLength = vtkKWEntryWithLabel::New();
  this->ExtrusionLength->SetParent(this->MainFrame);
  this->ExtrusionLength->Create();
  this->ExtrusionLength->SetLabelText("Extrusion Length : ");
  this->ExtrusionLength->SetLabelWidth(20);
  this->ExtrusionLength->GetWidget()->SetRestrictValueToDouble();
  // this->ExtrusionLength->SetEnabled(0);
  this->ExtrusionLength->GetWidget()->SetWidth(5);
  this->ExtrusionLength->GetWidget()->SetValueAsDouble(1.0);

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->ExtrusionLength->GetWidgetName());

  this->NumberOfDivisions = vtkKWEntryWithLabel::New();
  this->NumberOfDivisions->SetParent(this->MainFrame);
  this->NumberOfDivisions->Create();
  this->NumberOfDivisions->SetLabelText("Number Of Divisions : ");
  this->NumberOfDivisions->SetLabelWidth(20);
  this->NumberOfDivisions->GetWidget()->SetRestrictValueToInteger();
  // this->ExtrusionLength->SetEnabled(0);
  this->NumberOfDivisions->GetWidget()->SetWidth(5);
  this->NumberOfDivisions->GetWidget()->SetValueAsInt(1);

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->NumberOfDivisions->GetWidgetName());

  vtkKWIcon *defineElSetIcon = vtkKWIcon::New();
  defineElSetIcon->SetImage(    image_mimxElSetDfns, 
                                image_mimxElSetDfns_width, 
                                image_mimxElSetDfns_height, 
                                image_mimxElSetDfns_pixel_size); 

  if ( !this->DefineElSetButton )
    this->DefineElSetButton = vtkKWPushButton::New();
  this->DefineElSetButton->SetParent(this->MainFrame);
  this->DefineElSetButton->Create();
  this->DefineElSetButton->SetCommand(this, "DefineElementSetCallback");
  this->DefineElSetButton->SetImageToIcon( defineElSetIcon );
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -padx 2 -pady 6", 
    this->DefineElSetButton->GetWidgetName());
  defineElSetIcon->Delete();

  // check button for showing normals
  //this->ShowNormalsCheckButton = vtkKWCheckButtonWithLabel::New();
  //this->ShowNormalsCheckButton->SetParent(this->MainFrame->GetFrame());
  //this->ShowNormalsCheckButton->Create();
  //this->ShowNormalsCheckButton->SetLabelText("Show Normals ");
  //this->ShowNormalsCheckButton->SetLabelWidth(20);
  //this->ShowNormalsCheckButton->SetLabelPositionToLeft();
  //this->ShowNormalsCheckButton->GetWidget()->SetCommand(this, "ShowNormalsCallback");
  //this->GetApplication()->Script(
  // "pack %s -side top -anchor nw -expand n -padx 6 -pady 6", 
  // this->ShowNormalsCheckButton->GetWidgetName());

  this->NodeElementNumbersGroup = vtkKWMimxNodeElementNumbersGroup::New();
  this->NodeElementNumbersGroup->SetParent(this->MainFrame);
  this->NodeElementNumbersGroup->Create();
  //this->NodeElementNumbersGroup->GetMainFrame()->SetLabelText("Node and Element Numbers");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 20 -pady 6", 
    this->NodeElementNumbersGroup->GetWidgetName());

// buttons for selection of surface elements
  //if(!this->SelectSubsetRadiobuttonSet)
  // this->SelectSubsetRadiobuttonSet = vtkKWRadioButtonSet::New();
  //this->SelectSubsetRadiobuttonSet->SetParent(this->MainFrame);
  //this->SelectSubsetRadiobuttonSet->Create();
  //this->SelectSubsetRadiobuttonSet->SetBorderWidth(2);
  //this->SelectSubsetRadiobuttonSet->SetReliefToGroove();
  //this->SelectSubsetRadiobuttonSet->SetMaximumNumberOfWidgetsInPackingDirection(1);
  //for (int id = 0; id < 3; id++)       
  // this->SelectSubsetRadiobuttonSet->AddWidget(id);

  //this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetCommand(this, "SelectElementsThroughCallback");
  //this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetText("ET");
  //this->SelectSubsetRadiobuttonSet->GetWidget(0)->IndicatorVisibilityOff();
  //this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetBalloonHelpString(
  // "Select Elements Through");
  //this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetValue("ET");
  //this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetCompoundModeToLeft();

  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetText("ES");
  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetCommand(this, "SelectElementsSurfaceCallback");
  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->IndicatorVisibilityOff();
  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetBalloonHelpString(
  // "Select Surface Elements");
  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetVariableName(
  // this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetValue("ES");
  //this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetCompoundModeToLeft();

  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetText("VTK");
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetCommand(this, "VTKInteractionCallback");
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->IndicatorVisibilityOff();
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetBalloonHelpString(
  // "VTK Interaction");
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetVariableName(
  // this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetValue("VTK");
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetCompoundModeToLeft();
  //this->SelectSubsetRadiobuttonSet->GetWidget(2)->SelectedStateOn();

  //this->GetApplication()->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
  // this->SelectSubsetRadiobuttonSet->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "CreateExtrudeFEMeshApplyCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 5 -pady 6", 
    this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateExtrudeFEMeshCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand n -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::UpdateEnableState()
{
  this->UpdateObjectLists();
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateExtrudeFEMeshGroup::CreateExtrudeFEMeshApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Surface to be extruded not selected");
    return 0;
    }
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Choose valid Mesh");
    combobox->SetValue("");
    return 0;
    }

  // check the node, element number and entry sets are valid.
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::
    SafeDownCast(this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

  vtkIntArray *nodenumbers = vtkIntArray::SafeDownCast(
    ugrid->GetPointData()->GetArray("Node_Numbers"));
  vtkIntArray *elementnumbers = vtkIntArray::SafeDownCast(
    ugrid->GetCellData()->GetArray("Element_Numbers"));
  if(nodenumbers)
    {
    if(this->NodeElementNumbersGroup->GetNodeNumberEntry()->
       GetValueAsInt() <= this->MaxNodeNum)
      {
      callback->ErrorMessage("Starting Node Number should always be > ");
      this->NodeElementNumbersGroup->GetNodeNumberEntry()->
        SetValueAsInt(this->MaxNodeNum);
      return 0;
      }
    }

  if(elementnumbers)
    {
    if(this->NodeElementNumbersGroup->GetElementNumberEntry()->
       GetValueAsInt() <= this->MaxEleNum)
      {
      callback->ErrorMessage("Starting Element Number should always be > " );
      this->NodeElementNumbersGroup->GetElementNumberEntry()->
        SetValueAsInt(this->MaxEleNum);
      return 0;
      }
    }
        
  const char *nodesetname = this->NodeElementNumbersGroup->GetNodeSetNameEntry()->
    GetValue();

  const char *elementsetname = this->NodeElementNumbersGroup->GetElementSetNameEntry()->
    GetValue();

  if(!strcmp(nodesetname,""))
    {
    callback->ErrorMessage("Node set name cannot be empty");
    return 0;
    }

  if(!strcmp(elementsetname,""))
    {
    callback->ErrorMessage("Element set name cannot be empty");
    return 0;
    }
  // check if the node and element set name already exists
  if(ugrid->GetPointData()->GetArray(nodesetname))
    {
    callback->ErrorMessage("Node set with the chosen name already exists. Change the node set name");
    return 0;
    }
  //
  if(ugrid->GetCellData()->GetArray(elementsetname))
    {
    callback->ErrorMessage("Element set with the chosen name already exists. Change the element set name");
    return 0;
    }
  //if(!this->SelectCellsWidget)
  //{
  //      callback->ErrorMessage("Make a selection ");
  //      return 0;
  //}
  //if(!this->SelectCellsWidget->GetEnabled())
  //{
  //      callback->ErrorMessage("Enable selection first ");
  //      return 0;
  //}
        
  //vtkActor *extractedactor = this->SelectCellsWidget->GetExtractedSurfaceActor();
  //if(!extractedactor)
  //{
  //      callback->ErrorMessage("No selection made ");
  //      return 0;
  //}
  vtkUnstructuredGrid *selectedgrid = this->DefineElementSetDialog->GetSelectedMesh();
  if(!selectedgrid->GetNumberOfCells())
    {
    callback->ErrorMessage("Empty selection ");
    return 0;
    }

  if(this->ExtrusionLength->GetWidget()->GetValueAsDouble() <= 0.0)
    {
    callback->ErrorMessage("Extrusion length cannot be <= 0.0");
    return 0;
    }

  if(this->NumberOfDivisions->GetWidget()->GetValueAsInt() <= 0)
    {
    callback->ErrorMessage("Number of Divisions along extrusion direction cannot be < 1");
    return 0;
    }

  vtkGeometryFilter *geofil = vtkGeometryFilter::New();
  geofil->SetInput(selectedgrid);
  geofil->Update();
  vtkMimxExtrudePolyData *extrude = vtkMimxExtrudePolyData::New();
  extrude->SetInput(geofil->GetOutput());
                
  extrude->SetExtrusionLength(this->ExtrusionLength->GetWidget()->GetValueAsDouble());
  extrude->SetNumberOfDivisions(this->NumberOfDivisions->GetWidget()->GetValueAsInt());
  extrude->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
  callback->SetState(0);
  extrude->Update();
  extrude->RemoveObserver(callback);
  if (!callback->GetState())
    {
    // apply Node and element numbers to the extruded part of the mesh
    vtkMimxApplyNodeElementNumbers *apply = new vtkMimxApplyNodeElementNumbers;
    apply->SetUnstructuredGrid(extrude->GetOutput());
    apply->SetNodeSetName(
      this->NodeElementNumbersGroup->GetNodeSetNameEntry()->GetValue());
    apply->SetStartingNodeNumber(this->NodeElementNumbersGroup->GetNodeNumberEntry()->
                                 GetValueAsInt());
    apply->ApplyNodeNumbers();

    apply->SetElementSetName(
      this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue());
    apply->SetStartingElementNumber(this->NodeElementNumbersGroup->
                                    GetElementNumberEntry()->GetValueAsInt());
    apply->ApplyElementNumbers();
    delete apply;
    //
    vtkStringArray *elsetarray = vtkStringArray::New();
    elsetarray->SetName("Element_Set_Names");
    elsetarray->InsertNextValue(
      this->NodeElementNumbersGroup->GetElementSetNameEntry()->GetValue());

    vtkStringArray *nodesetarray = vtkStringArray::New();
    nodesetarray->SetName("Node_Set_Names");
    nodesetarray->InsertNextValue(
      this->NodeElementNumbersGroup->GetNodeSetNameEntry()->GetValue());

    extrude->GetOutput()->GetFieldData()->AddArray(elsetarray);
    elsetarray->Delete();

    extrude->GetOutput()->GetFieldData()->AddArray(nodesetarray);
    nodesetarray->Delete();

    // merge the original and extruded mesh
    vtkDataSetCollection *collection = vtkDataSetCollection::New();
    collection->InitTraversal();
    collection->AddItem(ugrid);
    collection->AddItem(extrude->GetOutput());
    vtkMimxMergeNodesPointAndCellData *merge = new vtkMimxMergeNodesPointAndCellData;
        
    merge->SetDataSetCollection(collection);
    merge->SetTolerance(0.0);
    merge->SetNodesMerge(1);

    if(merge->MergeDataSets())
      {
      vtkMimxMeshActor *meshActor = vtkMimxMeshActor::New();
      this->FEMeshList->AppendItem(meshActor);
      meshActor->SetDataSet( merge->GetMergedDataSet() );
      meshActor->SetRenderer( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer() );
      meshActor->SetInteractor( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor() );
                          
      this->Count++;
      vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                       this->FEMeshList->GetNumberOfItems()-1))->SetObjectName("Extrude_",Count);
      //vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
      //      this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->Modified();
      //this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(
      //      this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->GetActor());
      this->GetMimxMainWindow()->GetRenderWidget()->Render();
      this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
      this->GetMimxMainWindow()->GetViewProperties()->AddObjectList(
        this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1));
                                        
      this->GetMimxMainWindow()->SetStatusText("Created Extruded Mesh");
      this->UpdateObjectLists();
      meshActor->Delete();
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
//----------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::CreateExtrudeFEMeshCancelCallback()
{
  this->CancelStatus = 1;
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  if(this->NormalsActor)
    {
    this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
      this->NormalsActor);
    vtkPolyData::SafeDownCast(this->NormalsActor->GetMapper()
                              ->GetInput())->GetPoints()->Initialize();
    vtkPolyData::SafeDownCast(
      this->NormalsActor->GetMapper()->GetInput())->Initialize();
    //vtkPolyDataMapper::SafeDownCast(
    //      this->NormalsActor->GetMapper())->Delete();
    this->NormalsActor->Delete();
    this->NormalsActor = NULL;
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
    this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
    }
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
  this->CancelStatus = 0;
}
//----------------------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::UpdateObjectLists()
{
  this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
  int defaultItem = -1;
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
    {
    ObjectListComboBox->GetWidget()->AddValue(
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
    ObjectListComboBox->GetWidget()->SetValue(
      this->FEMeshList->GetItem(defaultItem)->GetFileName());
    }
  if(this->FEMeshList->GetNumberOfItems() > 0)
    {
    this->SelectionChangedCallback(NULL);
    }
        
  /* Set the Deafault Node and Element Numbers */
  int maxNodeNumber = 0;
  int maxElementNumber = 0;
        
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
    {
    vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
                                                                ->GetItem(i))->GetDataSet();

    vtkIntArray *nodeArray = vtkIntArray::SafeDownCast(ugrid->GetPointData()->GetArray("Node_Numbers"));
    vtkIntArray *elementArray = vtkIntArray::SafeDownCast(ugrid->GetCellData()->GetArray("Element_Numbers"));
    if ( nodeArray )
      {
      int maxIndex = nodeArray->GetNumberOfTuples();
      int currentValue = 0;
      if (maxIndex > 0) currentValue = nodeArray->GetValue( maxIndex -1 );
      if (currentValue > maxNodeNumber ) maxNodeNumber = currentValue;
      }
    if ( elementArray )
      {
      int maxIndex = elementArray->GetNumberOfTuples();
      int currentValue = 0;
      if (maxIndex > 0) currentValue = elementArray->GetValue( maxIndex -1 );
      if (currentValue > maxElementNumber ) maxElementNumber = currentValue;
      }
    }
  maxNodeNumber++;
  maxElementNumber++;
  //std::cout << "Node Number: " << maxNodeNumber << std::endl;
  //std::cout << "Elemenmt Number: " << maxElementNumber << std::endl;
  this->NodeElementNumbersGroup->GetNodeNumberEntry()->SetValueAsInt( maxNodeNumber );
  this->NodeElementNumbersGroup->GetElementNumberEntry()->SetValueAsInt( maxElementNumber );
        
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::ShowNormalsCallback(int State)
{
  if(State)
    {
    if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
      {
      if(this->NormalsActor)
        {
        vtkPolyData::SafeDownCast(this->NormalsActor->GetMapper()
                                  ->GetInput())->GetPoints()->Initialize();
        vtkPolyData::SafeDownCast(
          this->NormalsActor->GetMapper()->GetInput())->Initialize();
        //vtkPolyDataMapper::SafeDownCast(
        //      this->NormalsActor->GetMapper())->Delete();
        this->NormalsActor->Delete();
        this->NormalsActor = NULL;
        }
      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(
        this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
      vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
      normals->SetInput(polydata);
      normals->SplittingOff();
      normals->ComputeCellNormalsOn();
      normals->Update();
      vtkHedgeHog *hedge = vtkHedgeHog::New();
      hedge->SetInput(normals->GetOutput());
      hedge->SetScaleFactor(1.0);
      hedge->SetVectorModeToUseNormal();
      vtkPolyDataMapper* hedgeMapper = vtkPolyDataMapper::New();
      hedgeMapper->SetInputConnection(hedge->GetOutputPort());
      this->NormalsActor = vtkActor::New();
      this->NormalsActor->SetMapper(hedgeMapper);
      this->NormalsActor->GetProperty()->SetRepresentationToWireframe();
      this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(this->NormalsActor);
      this->GetMimxMainWindow()->GetRenderWidget()->Render();
      this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
      hedge->Delete();
      normals->Delete();
      hedgeMapper->Delete();
      }
    }
  else{
  if(this->NormalsActor)
    {
    this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
      this->NormalsActor);
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
    this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
    }

  }
}
//------------------------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::SelectionChangedCallback(const char *vtkNotUsed(dummy))
{
  //if(this->ShowNormalsCheckButton->GetState())
  //{
  //      if(this->NormalsActor)
  //      {
  //              this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
  //                      this->NormalsActor);
  //              vtkPolyData::SafeDownCast(this->NormalsActor->GetMapper()
  //                      ->GetInput())->GetPoints()->Initialize();
  //              vtkPolyData::SafeDownCast(
  //                      this->NormalsActor->GetMapper()->GetInput())->Initialize();
  //              //vtkPolyDataMapper::SafeDownCast(
  //              //      this->NormalsActor->GetMapper())->Delete();
  //              this->NormalsActor->Delete();
  //              this->NormalsActor = NULL;
  //              this->GetMimxMainWindow()->GetRenderWidget()->Render();
  //              this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  //      }
  //}
  //this->ShowNormalsCheckButton->SetState(0);
        

  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    //callback->ErrorMessage("Select Mesh");
    return;
    }
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Choose valid Surface");
    combobox->SetValue("");
    return;
    }
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

  vtkIntArray *nodenumbers = vtkIntArray::SafeDownCast(
    ugrid->GetPointData()->GetArray("Node_Numbers"));
  vtkIntArray *elementnumbers = vtkIntArray::SafeDownCast(
    ugrid->GetCellData()->GetArray("Element_Numbers"));
  int i;
  int nodemax = 0;
  if(nodenumbers)
    {
    for (i = 0; i<nodenumbers->GetNumberOfTuples(); i++)
      {
      if(nodenumbers->GetValue(i) > nodemax)
        {
        nodemax = nodenumbers->GetValue(i);
        }
      }
    }
  this->MaxNodeNum = nodemax;
  this->NodeElementNumbersGroup->GetNodeNumberEntry()->SetValueAsInt(nodemax +1);

  int elementmax = 0;
  if(elementnumbers)
    {
    for (i = 0; i<elementnumbers->GetNumberOfTuples(); i++)
      {
      if(elementnumbers->GetValue(i) > elementmax)
        {
        elementmax = elementnumbers->GetValue(i);
        }
      }
    }
  this->MaxEleNum = elementmax;
  this->NodeElementNumbersGroup->GetElementNumberEntry()->SetValueAsInt(elementmax +1);
}
//------------------------------------------------------------------------------------------
int vtkKWMimxCreateExtrudeFEMeshGroup::SelectElementsThroughCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("FEMesh selection required");
    return 0;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Choose valid FE Mesh");
    combobox->SetValue("");
    return 0;
    }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  if(!this->SelectCellsWidget)
    {
    this->SelectCellsWidget = vtkMimxSelectCellsWidgetFEMesh::New();
    this->SelectCellsWidget->SetInteractor(this->GetMimxMainWindow()->
                                           GetRenderWidget()->GetRenderWindowInteractor());
    }
  if(this->SelectCellsWidget->GetEnabled())
    {
    this->SelectCellsWidget->SetEnabled(0);
    }
  this->SelectCellsWidget->SetInput(ugrid);
  this->SelectCellsWidget->SetCellSelectionState(4);
  this->SelectCellsWidget->SetEnabled(1);
  if(this->GetSelectedObject())
    {
    this->GetSelectedObject()->HideMesh();
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
    }
  return 1;
}
//------------------------------------------------------------------------------------------
int vtkKWMimxCreateExtrudeFEMeshGroup::SelectElementsSurfaceCallback()
{
  if(this->SelectElementsThroughCallback())
    {
    this->SelectCellsWidget->SetCellSelectionState(5);
    return 1;
    }
  return 0;
}
//------------------------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::CreateExtrudeFEMeshDoneCallback()
{
  if(this->CreateExtrudeFEMeshApplyCallback())
    this->CreateExtrudeFEMeshCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::VTKInteractionCallback()
{
  if(this->SelectCellsWidget)
    {
    if(this->SelectCellsWidget->GetEnabled())
      {
      this->SelectCellsWidget->SetEnabled(0);
      }
    }
  if(this->GetSelectedObject())
    {
    this->GetSelectedObject()->ShowMesh();
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
    }
}
//---------------------------------------------------------------------------------
vtkMimxMeshActor* vtkKWMimxCreateExtrudeFEMeshGroup::GetSelectedObject()
{
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    return NULL;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    return NULL;
    }

  return vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

}
//-----------------------------------------------------------------------------------
void vtkKWMimxCreateExtrudeFEMeshGroup::DefineElementSetCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Mesh must be selected");
    return;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
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

  if (!this->DefineElementSetDialog)
    {
    this->DefineElementSetDialog = vtkKWMimxDefineElSetGroup::New();
    this->DefineElementSetDialog->SetApplication( this->GetApplication() );
    this->DefineElementSetDialog->SetMimxMainWindow( this->GetMimxMainWindow() );
    this->DefineElementSetDialog->SetDimensionState(1);
    this->DefineElementSetDialog->Create();
    }
  this->DefineElementSetDialog->SetMeshActor( meshActor );
  this->DefineElementSetDialog->SetElementSetCombobox(NULL);
  this->DefineElementSetDialog->Display();
}
//-------------------------------------------------------------------------------------------------
