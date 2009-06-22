#include "vtkEMSegmentIntensityDistributionsStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include <string>
#include <stdio.h>
#include <iostream>

#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnListWithScrollbarsWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWNotebook.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWHistogram.h"
#include "vtkKWColorTransferFunctionEditor.h"
//#include "vtkKWColorTransferFunctionEditorDerived.h"
#include "vtkColorTransferFunction.h"
#include "vtkKWParameterValueFunctionInterface.h"
#include "vtkKWParameterValueFunctionEditor.h"

#include "vtkKWPushButton.h"

#include "vtkKWEntryWithLabel.h"

#include "vtkKWMultiColumnList.h"

#include "vtkPointData.h"

#include "vtkEMSegmentAnatomicalStructureStep.h"

#include "vtkSlicerInteractorStyle.h"

#include "vtkMatrix4x4.h"
#include "vtkImageReslice.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkVolume.h"

#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"

#include "vtkImageInteractionCallback.h"

#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkImageData.h"
#include "vtkDataSetMapper.h"

#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityDistributionsStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityDistributionsStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::vtkEMSegmentIntensityDistributionsStep()
{
  this->SetName("6/9. Specify Intensity Distributions");
  this->SetDescription(
    "Define intensity distribution for each anatomical structure.");

  this->IntensityDistributionNotebook                = NULL;
  this->IntensityDistributionSpecificationMenuButton = NULL;
  this->IntensityDistributionMeanMatrix              = NULL;
  this->IntensityDistributionCovarianceMatrix        = NULL;
  this->IntensityDistributionManualSamplingList      = NULL;
  this->ContextMenu  = NULL;
  
  this->IntensityDistributionHistogramFrame          = NULL;
  this->IntensityDistributionHistogramButton         = NULL;
  this->IntensityDistributionHistogramHistogram      = NULL;
  this->IntensityDistributionHistogramHistogramVisu  = NULL;
  
  this->NbOfClassesEntryLabel                        = NULL;
  
  this->ClassAndNodeList                             = NULL;
  
  this->TestButton                                   = NULL;
  
}

//----------------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::~vtkEMSegmentIntensityDistributionsStep()
{

  if (this->TestButton)
    {
    this->TestButton->Delete();
    this->TestButton = NULL;
    }

  if (this->ClassAndNodeList)
    {
    this->ClassAndNodeList->Delete();
    this->ClassAndNodeList = NULL;
    }

  if (this->NbOfClassesEntryLabel)
    {
    this->NbOfClassesEntryLabel->Delete();
    this->NbOfClassesEntryLabel = NULL;
    }

  if (this->IntensityDistributionHistogramHistogramVisu)
    {
    this->IntensityDistributionHistogramHistogramVisu->Delete();
    this->IntensityDistributionHistogramHistogramVisu = NULL;
    }

  if (this->IntensityDistributionHistogramHistogram)
    {
    this->IntensityDistributionHistogramHistogram->Delete();
    this->IntensityDistributionHistogramHistogram = NULL;
    }


  if (this->IntensityDistributionHistogramButton)
    {
    this->IntensityDistributionHistogramButton->Delete();
    this->IntensityDistributionHistogramButton = NULL;
    }
    

  if (this->IntensityDistributionHistogramFrame)
    {
    this->IntensityDistributionHistogramFrame->Delete();
    this->IntensityDistributionHistogramFrame = NULL;
    }


  if (this->IntensityDistributionNotebook)
    {
    this->IntensityDistributionNotebook->Delete();
    this->IntensityDistributionNotebook = NULL;
    }

  if (this->IntensityDistributionSpecificationMenuButton)
    {
    this->IntensityDistributionSpecificationMenuButton->Delete();
    this->IntensityDistributionSpecificationMenuButton = NULL;
    }

  if (this->IntensityDistributionMeanMatrix)
    {
    this->IntensityDistributionMeanMatrix->Delete();
    this->IntensityDistributionMeanMatrix = NULL;
    }

  if (this->IntensityDistributionCovarianceMatrix)
    {
    this->IntensityDistributionCovarianceMatrix->Delete();
    this->IntensityDistributionCovarianceMatrix = NULL;
    }

  if (this->IntensityDistributionManualSamplingList)
    {
    this->IntensityDistributionManualSamplingList->Delete();
    this->IntensityDistributionManualSamplingList = NULL;
    }

  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  anat_step->ShowAnatomicalStructureTree();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkIdType vol_id = mrmlManager->GetTreeRootNodeID();
  const char *root_node = 
    anat_step->GetAnatomicalStructureTree()->GetWidget()->FindNodeWithUserDataAsInt(NULL, vol_id);
  if (root_node && *root_node)
    {
    anat_step->SetAnatomicalTreeParentNodeSelectableState(root_node, 0);
    }

  this->AddManualIntensitySamplingGUIObservers();
  
  // Override the tree callbacks for that specific step

  anat_step->GetAnatomicalStructureTree()->GetWidget()->SetSelectionChangedCommand(
      this, "DisplaySelectedNodeIntensityDistributionsCallback");

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  // Create the notebook

  if (!this->IntensityDistributionNotebook)
    {
    this->IntensityDistributionNotebook = vtkKWNotebook::New();
    }
  if (!this->IntensityDistributionNotebook->IsCreated())
    {
    this->IntensityDistributionNotebook->SetParent(parent);
    this->IntensityDistributionNotebook->Create();
    this->IntensityDistributionNotebook->AddPage(
      "Intensity Distribution");
    this->IntensityDistributionNotebook->AddPage(
      "Manual Sampling");
     this->IntensityDistributionNotebook->AddPage(
      "Intuitive");
    }
  vtkKWFrame *intensity_page = 
    this->IntensityDistributionNotebook->GetFrame("Intensity Distribution");
  vtkKWFrame *manual_sampling_page = 
    this->IntensityDistributionNotebook->GetFrame("Manual Sampling");
  vtkKWFrame *intuitive_page = 
    this->IntensityDistributionNotebook->GetFrame("Intuitive");

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2", 
    this->IntensityDistributionNotebook->GetWidgetName());

  // Create the distribution specification menu button

  if (!this->IntensityDistributionSpecificationMenuButton)
    {
    this->IntensityDistributionSpecificationMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->IntensityDistributionSpecificationMenuButton->IsCreated())
    {
    this->IntensityDistributionSpecificationMenuButton->SetParent(
      intensity_page);
    this->IntensityDistributionSpecificationMenuButton->Create();
    this->IntensityDistributionSpecificationMenuButton->
      SetLabelText("Specification:");
    this->IntensityDistributionSpecificationMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionSpecificationMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->IntensityDistributionSpecificationMenuButton->SetBalloonHelpString(
      "Select intensity distribution specification type.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->IntensityDistributionSpecificationMenuButton->GetWidgetName());

  // Create the distribution mean vector/matrix

  if (!this->IntensityDistributionMeanMatrix)
    {
    this->IntensityDistributionMeanMatrix = 
      vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->IntensityDistributionMeanMatrix->IsCreated())
    {
    this->IntensityDistributionMeanMatrix->SetParent(intensity_page);
    this->IntensityDistributionMeanMatrix->Create();
    this->IntensityDistributionMeanMatrix->SetLabelText("Log Mean:");
    this->IntensityDistributionMeanMatrix->ExpandWidgetOff();
    this->IntensityDistributionMeanMatrix->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionMeanMatrix->SetBalloonHelpString(
      "Set the intensity distribution mean.");
    
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionMeanMatrix->GetWidget();
    matrix->SetNumberOfColumns(0);
    matrix->SetNumberOfRows(0);
    matrix->SetElementWidth(6);
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  this->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2",
               this->IntensityDistributionMeanMatrix->GetWidgetName());

  // Create the distribution covariance vector/matrix

  if (!this->IntensityDistributionCovarianceMatrix)
    {
    this->IntensityDistributionCovarianceMatrix = 
      vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->IntensityDistributionCovarianceMatrix->IsCreated())
    {
    this->IntensityDistributionCovarianceMatrix->SetParent(intensity_page);
    this->IntensityDistributionCovarianceMatrix->Create();
    this->IntensityDistributionCovarianceMatrix->SetLabelText(
      "Log Covariance:");
    this->IntensityDistributionCovarianceMatrix->ExpandWidgetOff();
    this->IntensityDistributionCovarianceMatrix->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionCovarianceMatrix->SetBalloonHelpString(
      "Set the intensity distribution covariance.");
    
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionCovarianceMatrix->GetWidget();
    matrix->SetNumberOfColumns(0);
    matrix->SetNumberOfRows(0);
    matrix->SetElementWidth(6);
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  this->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2",
               this->IntensityDistributionCovarianceMatrix->GetWidgetName());
  
  // Create the manual sampling frame

  if (!this->IntensityDistributionManualSamplingList)
    {
    this->IntensityDistributionManualSamplingList = 
      vtkKWMultiColumnListWithScrollbarsWithLabel::New();
    }
  if (!this->IntensityDistributionManualSamplingList->IsCreated())
    {
    this->IntensityDistributionManualSamplingList->SetParent(
      manual_sampling_page);
    this->IntensityDistributionManualSamplingList->Create();
    this->IntensityDistributionManualSamplingList->SetLabelPositionToTop();
    this->IntensityDistributionManualSamplingList->SetLabelText(
      "Control+Click in a slice window to pick a sample.");
    this->IntensityDistributionManualSamplingList->GetWidget()->
      HorizontalScrollbarVisibilityOff();

    vtkKWMultiColumnList *list = 
      this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
    list->SetRightClickCommand(
      this, "PopupManualIntensitySampleContextMenuCallback");
    list->SetHeight(4);
    list->MovableColumnsOff();
    list->SetSelectionModeToSingle();
    list->ResizableColumnsOff();
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2", 
    this->IntensityDistributionManualSamplingList->GetWidgetName());

  // Update the UI with the proper value, if there is a selection

  this->DisplaySelectedNodeIntensityDistributionsCallback();
  
/*
  // Create the histogram frame

  if (!this->IntensityDistributionHistogramFrame)
    {
    this->IntensityDistributionHistogramFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->IntensityDistributionHistogramFrame->IsCreated())
    {
    this->IntensityDistributionHistogramFrame->SetParent(parent);
    this->IntensityDistributionHistogramFrame->Create();
    this->IntensityDistributionHistogramFrame->SetLabelText(
      "Histogram");
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->IntensityDistributionHistogramFrame->GetWidgetName());

    vtkKWFrame* histoFrame = this->IntensityDistributionHistogramFrame->GetFrame();
    */
    
  /*  
    
     // Number of classes to segment

  if (!this->NbOfClassesEntryLabel)
    {
    this->NbOfClassesEntryLabel = vtkKWEntryWithLabel::New();
    }
  if (!this->NbOfClassesEntryLabel->IsCreated())
    {
    this->NbOfClassesEntryLabel->SetParent(intuitive_page);
    this->NbOfClassesEntryLabel->Create();
    this->NbOfClassesEntryLabel->SetLabelText("Nb of class:");
    this->NbOfClassesEntryLabel->SetLabelWidth(EMSEG_WIDGETS_LABEL_WIDTH -8);
    vtkKWEntry *entry = this->NbOfClassesEntryLabel->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    
    }

  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2",
               this->NbOfClassesEntryLabel->GetWidgetName());
    
    
    
    */
    
    
  // GET NUMBER OF LEAF
  
  //vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  this->nbOfLeaf = 0;
  //this->leafID[0] = 0;
  
  //this->nameOfLeaf[2] = {"un","deux"};
   
  if (mrmlManager)
    {
    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetNumberOfLeaf(NULL, root_id);
      }
    }  
            
  // Create the histogram volume selector

  if (!this->IntensityDistributionHistogramButton)
    {
    this->IntensityDistributionHistogramButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->IntensityDistributionHistogramButton->IsCreated())
    {
    this->IntensityDistributionHistogramButton->SetParent(intuitive_page);
    this->IntensityDistributionHistogramButton->Create();
    this->IntensityDistributionHistogramButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->IntensityDistributionHistogramButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->IntensityDistributionHistogramButton->
      SetLabelText("Target Image:");
    this->IntensityDistributionHistogramButton->
      SetBalloonHelpString("Select a target image to adjust intensity distribution");

    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5", 
    this->IntensityDistributionHistogramButton->GetWidgetName());
  
  this->PopulateIntensityDistributionTargetVolumeSelector();
    
  
  if (!this->IntensityDistributionHistogramHistogram)
    {
    this->IntensityDistributionHistogramHistogram = vtkKWHistogram::New();
    this->IntensityDistributionHistogramHistogramVisu = vtkKWColorTransferFunctionEditor::New();
    this->IntensityDistributionHistogramHistogramFunc = vtkColorTransferFunction::New();
    }
    
   if (!this->IntensityDistributionHistogramHistogramVisu->IsCreated())
    {
    this->IntensityDistributionHistogramHistogramVisu->SetParent(intuitive_page);
    this->IntensityDistributionHistogramHistogramVisu->Create();
    //this->VisuHisto->GetTclName();
    this->IntensityDistributionHistogramHistogramVisu->SetBorderWidth(2);
    this->IntensityDistributionHistogramHistogramVisu->SetReliefToGroove();
    this->IntensityDistributionHistogramHistogramVisu->SetPadX(2);
    this->IntensityDistributionHistogramHistogramVisu->SetPadY(2);
    //this->IntensityDistributionHistogramHistogramVisu->ParameterTicksVisibilityOn();
    //this->IntensityDistributionHistogramHistogramVisu->ValueTicksVisibilityOn();
    //this->IntensityDistributionHistogramHistogramVisu->ComputeValueTicksFromHistogramOn();
    //this->IntensityDistributionHistogramHistogramVisu->SetParameterTicksFormat("%-#6.0f");
    //this->IntensityDistributionHistogramHistogramVisu->SetValueTicksFormat(
    //this->IntensityDistributionHistogramHistogramVisu->GetParameterTicksFormat()); 
    this->IntensityDistributionHistogramHistogramVisu->SetPointPositionInValueRangeToTop(); 
    this->IntensityDistributionHistogramHistogramVisu->PointGuidelineVisibilityOn(); 
    this->IntensityDistributionHistogramHistogramVisu->SetLabelPositionToTop();
    }
     
         
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2", 
    this->IntensityDistributionHistogramHistogramVisu->GetWidgetName());
    
    vtkEMSegmentMRMLManager *mrmlManager0 = this->GetGUI()->GetMRMLManager();
    this->IntensityDistributionHistogramButton->SetEnabled(
    mrmlManager0->GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if(this->IntensityDistributionHistogramButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int vol_id = mrmlManager->GetVolumeNthID(0);
    this->IntensityDistributionTargetSelectionChangedCallback(vol_id);
    }
    
  this->AddPointMovingGUIEvents();
  this->AddPointAddGUIEvents();
  
  
   // ADD VOLUME SELECTORS FOR CLASSES
  /*
    vtkKWMenuButtonWithLabel* class1 = vtkKWMenuButtonWithLabel::New();
    class1->SetParent(intuitive_page);
    class1->Create();
    class1->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    class1->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    class1->
      SetLabelText("Class1 Node:");
    class1->
      SetBalloonHelpString("Select the node corresponding to this class");
      
        this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5", 
    class1->GetWidgetName());*/
    
    if (!this->ClassAndNodeList)
    {
    this->ClassAndNodeList = vtkKWMultiColumnList::New();
    }
    
   if (!this->ClassAndNodeList->IsCreated())
    {
    this->ClassAndNodeList->SetParent(intuitive_page);
    this->ClassAndNodeList->Create();
    this->ClassAndNodeList->MovableColumnsOff();
    //this->ClassAndNodeList->SetWidth(0);
    
    int col_index;
    
    col_index = this->ClassAndNodeList->AddColumn("Class");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    this->ClassAndNodeList->SetColumnFormatCommandToEmptyOutput(col_index);
    col_index = this->ClassAndNodeList->AddColumn("Point");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    col_index = this->ClassAndNodeList->AddColumn("Color");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    this->ClassAndNodeList->SetColumnFormatCommandToEmptyOutput(col_index);
    
    }
      
  int i = 0;
  int j = 0;
  
  const char* maintin[200];// = {"Nicolas","je","suis"};
  //mrmlManager->GetTreeNodeName(vol_id)
  //vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget(); 
   
  for(j = 0;j < this->nbOfLeaf;j++){
   
   
   //ERREUR
  maintin[j] = mrmlManager->GetTreeNodeName(leafID[j]);
  //std::cout<<"TEST :"<< maintin[j] <<std::endl;
  
  }
  
 
  //this->nameOfLeaf[] = {"Nicolas","je","suis"};
  
  //maintin[3] = "coucou";
  
   
 // int sizz = sizeof(maintin)/sizeof(maintin[0]);
  
 // std::cout<<"SIZE :"<< sizz <<std::endl;
  
  for(i=0; i < this->nbOfLeaf; i++)
  {
  this->ClassAndNodeList->InsertCellText(i,0,maintin[i]);
  this->ClassAndNodeList->SetCellWindowCommandToComboBoxWithValues(i,0,this->nbOfLeaf,maintin);
  this->ClassAndNodeList->InsertCellTextAsInt(i,1,i+1);

  //this->ClassAndNodeList->SetCellWindowCommandToColorButton(i,2);
  //->SetCellBackgroundColor(thisRow, ColourColumn, colour);
  
  
  }
  
  //std::cout<<"TEST ICI:"<< maintin[j] <<std::endl;
  
  //this->PopulateClassAndNodeList();
                          
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2", 
    this->ClassAndNodeList->GetWidgetName());
    
    //this->PopulateClassAndNodeList();
    
 //std::cout<<"TEST LA:"<< maintin[j] <<std::endl;   
    
  if (!this->TestButton)
    {
    this->TestButton = vtkKWPushButton::New();
    }
  if (!this->TestButton->IsCreated())
    {
    this->TestButton->SetParent(intuitive_page);
    this->TestButton->Create();
    this->TestButton->SetText("Get Proportions");    
    }

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
               this->TestButton->GetWidgetName());
  
  this->AddTestButtonGUIEvents();
    
    
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  PopulateClassAndNodeList()
{  
  vtkIdType target_vol_id;
  char buffer[256];
  
  // CLASS COLUMN CREATION
  int i;
  //int nbOfClass[this->nbOfLeaf];
  
  for(i=1; i=this->nbOfLeaf; i++)
  {
  //nbOfClass[i-1] = i;
  this->ClassAndNodeList->InsertCellTextAsInt(i-1,1,i);
  }

/*
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  vtkKWMenu* menu = this->IntensityDistributionHistogramButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
    sprintf(buffer, "%s %d", 
            "IntensityDistributionTargetSelectionChangedCallback", 
            static_cast<int>(target_vol_id));
    const char *name = mrmlManager->GetVolumeName(target_vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }*/
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  PopulateIntensityDistributionTargetVolumeSelector()
{  
  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  vtkKWMenu* menu = this->IntensityDistributionHistogramButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
    sprintf(buffer, "%s %d", 
            "IntensityDistributionTargetSelectionChangedCallback", 
            static_cast<int>(target_vol_id));
    const char *name = mrmlManager->GetVolumeName(target_vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }
}
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  IntensityDistributionTargetSelectionChangedCallback(vtkIdType target_vol_id)
{
std::cout<<"INTENSITY DISTRIBUTION TARGET SELECTION CHANGED CALLBACK"<<std::endl;
std::cout<<"vol id: "<< target_vol_id <<std::endl;
  
vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(target_vol_id)->GetImageData()->GetPointData()->GetScalars();
this->IntensityDistributionHistogramHistogram->BuildHistogram(array,0);

this->IntensityDistributionHistogramHistogramVisu->SetHistogram(this->IntensityDistributionHistogramHistogram);

double* range = this->IntensityDistributionHistogramHistogram->GetRange();

int size;
size = this->IntensityDistributionHistogramHistogramVisu->GetFunctionSize();
this->IntensityDistributionHistogramHistogramVisu->SetDisableAddAndRemove(0);

//if(size == 0){

/*
vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

this->nbOfLeaf = 0;

if (mrmlManager)
    {
    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetNumberOfLeaf(NULL, root_id);
      }
    }

*/
if(size > 0){
this->IntensityDistributionHistogramHistogramFunc->RemoveAllPoints();
}

this->IntensityDistributionHistogramHistogramFunc->SetColorSpaceToHSV();
this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[0],0.66,1.0,1.0);
this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[1],0.0,1.0,1.0);

double i;
float color;

for(i = 1; i< this->nbOfLeaf - 1 ; i++){

color = 0.66*(1-i/(this->nbOfLeaf -1));

std::cout<<"color: "<<color << std::endl;

this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint((range[0]+range[1])*(i)/(this->nbOfLeaf-1),color,1.0,1.0);

}

this->IntensityDistributionHistogramHistogramVisu->SetDisableAddAndRemove(1);
//this->IntensityDistributionHistogramHistogramVisu->SetReadOnly(1);
//this->IntensityDistributionHistogramHistogramVisu->SetReadOnly(this->nbOfLeaf);

//}

this->IntensityDistributionHistogramHistogramVisu->SetColorTransferFunction(this->IntensityDistributionHistogramHistogramFunc);
this->IntensityDistributionHistogramHistogramVisu->SetWholeParameterRangeToFunctionRange();
this->IntensityDistributionHistogramHistogramVisu->SetVisibleParameterRangeToWholeParameterRange();
this->IntensityDistributionHistogramHistogramVisu->ParameterRangeVisibilityOn();

this->IntensityDistributionHistogramHistogramVisu->ExpandCanvasWidthOn();
this->IntensityDistributionHistogramHistogramVisu->SetCanvasHeight(180);


/*

  if (!this->IntensityDistributionHistogramHistogramVisu->HasFunction() || id < 0 || id >= this->IntensityDistributionHistogramHistogramVisu->GetFunctionSize() ||
      !values)
    {
    return 0;
    }
  
  int dim = this->IntensityDistributionHistogramHistogramVisu->GetFunctionPointDimensionality();

#if VTK_MAJOR_VERSION > 5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION > 0)
  double node_value[6];
  this->IntensityDistributionHistogramHistogramVisu->ColorTransferFunction->GetNodeValue(id, node_value);
  memcpy(values, node_value + 1, dim * sizeof(double));
#else
  memcpy(values, 
         (this->IntensityDistributionHistogramHistogramVisu->ColorTransferFunction->GetDataPointer() + id * (1 + dim) + 1), 
         dim * sizeof(double));
#endif


*/






//this->IntensityDistributionHistogramHistogramVisu->GetFunctionPointParameter(2,para);
//data = this->IntensityDistributionHistogramHistogramFunc->GetDataPointer();

//std::cout<<"para: "<< para[0] << para[1] << std::endl;

//std::cout<<"data: "<< data[0] << data[1] << std::endl;
/*
double x1,x2;
int n;
double* table;

this->IntensityDistributionHistogramHistogramFunc->GetTable (x1, x2, n, table);

std::cout<<"test2: "<< x1 << x2 << n << table[0] << std::endl;*/
//this->ExtractSlice(target_vol_id);

}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemoveManualIntensitySamplingGUIObservers();
}

//----------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::DisplaySelectedNodeIntensityDistributionsCallback()
{
  // Update the UI with the proper value, if there is a selection

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  if (!anat_step)
    {
    return;
    }
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id = 0;
  int manually_sample_mode = 0;
  int has_valid_selection = tree->HasSelection();
  if (has_valid_selection)
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    has_valid_selection = mrmlManager->GetTreeNodeIsLeaf(sel_vol_id);
    manually_sample_mode = 
      mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) ==
      vtkEMSegmentMRMLManager::
      DistributionSpecificationManuallySample;
    }

  int enabled = tree->GetEnabled();
  int row, col;
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  char buffer[256];

  // Update the distribution specification menu button

  if (this->IntensityDistributionSpecificationMenuButton)
    {
    vtkKWMenu *menu = this->IntensityDistributionSpecificationMenuButton->
      GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (has_valid_selection)
      {
      vtksys_stl::string value;
      this->IntensityDistributionSpecificationMenuButton->SetEnabled(enabled);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d", 
        static_cast<int>(sel_vol_id), vtkEMSegmentMRMLManager::
        DistributionSpecificationManual);
      menu->AddRadioButton("Manual", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d", 
        static_cast<int>(sel_vol_id),vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample);
      menu->AddRadioButton("Manual Sampling", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d", 
        static_cast<int>(sel_vol_id), vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample);
      menu->AddRadioButton("Auto Sampling", this, buffer);

      // temporarily disable auto sampling because it is not currently
      // implemented
      menu->SetItemStateToDisabled("Auto Sampling");

      switch (mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id))
        {
        case vtkEMSegmentMRMLManager::DistributionSpecificationManual:
          value = "Manual";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample:
          value = "Manual Sampling";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample:
          value = "Auto Sampling";
          break;
        }
      this->IntensityDistributionSpecificationMenuButton->GetWidget()->
        SetValue(value.c_str());
      }
    else
      {
      this->IntensityDistributionSpecificationMenuButton->SetEnabled(0);
      this->IntensityDistributionSpecificationMenuButton->GetWidget()->
        SetValue("");
      }
    }

  // Update the distribution mean vector/matrix

  if (this->IntensityDistributionMeanMatrix)
    {
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionMeanMatrix->GetWidget();
    if (has_valid_selection)
      {
      this->IntensityDistributionMeanMatrix->SetEnabled(
        nb_of_target_volumes ? enabled : 0);
      matrix->SetNumberOfColumns(nb_of_target_volumes);
      matrix->SetNumberOfRows(1);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer, "IntensityDistributionMeanChangedCallback %d", 
        static_cast<int>(sel_vol_id));
      matrix->SetElementChangedCommand(this, buffer);

      for(col = 0; col < nb_of_target_volumes; col++)
        {
        matrix->SetElementValueAsDouble(
          0, col, 
          mrmlManager->GetTreeNodeDistributionLogMean(sel_vol_id, col));
        }
      }
    else
      {
      this->IntensityDistributionMeanMatrix->SetEnabled(0);
      matrix->SetNumberOfColumns(0);
      matrix->SetElementChangedCommand(NULL, NULL);
      }
    }

  // Update the distribution covariance vector/matrix

  if (this->IntensityDistributionCovarianceMatrix)
    {
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionCovarianceMatrix->GetWidget();
    if (has_valid_selection)
      {
      this->IntensityDistributionCovarianceMatrix->SetEnabled(
        nb_of_target_volumes ? enabled : 0);
      matrix->SetNumberOfColumns(nb_of_target_volumes);
      matrix->SetNumberOfRows(nb_of_target_volumes);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer,"IntensityDistributionCovarianceChangedCallback %d",
        static_cast<int>(sel_vol_id));
      matrix->SetElementChangedCommand(this, buffer);

      for (row = 0; row < nb_of_target_volumes; row++)
        {
        for (col = 0; col < nb_of_target_volumes; col++)
          {
          matrix->SetElementValueAsDouble(
            row, col, 
            mrmlManager->GetTreeNodeDistributionLogCovariance(
              sel_vol_id, row, col));
          }
        }
      }
    else
      {
      this->IntensityDistributionCovarianceMatrix->SetEnabled(0);
      matrix->SetNumberOfColumns(0);
      matrix->SetElementChangedCommand(NULL, NULL);
      }
    }

  // Update the manual sampling list

  if (this->IntensityDistributionNotebook)
    {
    this->IntensityDistributionNotebook->SetPageEnabled(
      "Manual Sampling", manually_sample_mode);
    }

  if (this->IntensityDistributionManualSamplingList)
    {
    vtkKWMultiColumnList *list = 
      this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
    list->DeleteAllRows();
    if (has_valid_selection && manually_sample_mode)
      {
      this->IntensityDistributionManualSamplingList->SetEnabled(enabled);
      int nb_cols = list->GetNumberOfColumns();
      for (;nb_cols < nb_of_target_volumes; nb_cols++)
        {
        int col_id = list->AddColumn("");
        list->SetColumnWidth(col_id, 0);
        list->ColumnStretchableOff(col_id);
        list->SetColumnSortMode(col_id, vtkKWMultiColumnList::SortModeReal);
        }
      for (;nb_cols > nb_of_target_volumes; nb_cols--)
        {
        list->DeleteColumn(nb_cols - 1);
        }
      for (col = 0; col < nb_of_target_volumes; ++col)
        {
        vtkIdType volumeID = 
          mrmlManager->GetTargetSelectedVolumeNthID(col);
        const char* title = mrmlManager->GetVolumeName(volumeID);
        list->SetColumnTitle(col, title);
        }
      double intensity;
      int nb_samples = 
        mrmlManager->GetTreeNodeDistributionNumberOfSamples(sel_vol_id);
      for (row = 0; row < nb_samples; row++)
        {
        list->AddRow();
        for (col = 0; col < nb_of_target_volumes; col++)
          {
          int vol_id = mrmlManager->GetTargetSelectedVolumeNthID(col);
          intensity = mrmlManager->GetTreeNodeDistributionSampleIntensityValue(
            sel_vol_id, row, vol_id);
          list->SetCellTextAsDouble(row, col, intensity);
          }
        }
      }
    else
      {
      this->IntensityDistributionManualSamplingList->SetEnabled(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::IntensityDistributionSpecificationCallback(
  vtkIdType sel_vol_id, int type)
{
  // The distribution specification has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  if (type != mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id))
    {
    mrmlManager->SetTreeNodeDistributionSpecificationMethod(sel_vol_id, type);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
    }
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::IntensityDistributionMeanChangedCallback(
  vtkIdType sel_vol_id, int row, int col, const char *value)
{
  // The distribution mean vector has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeDistributionLogMean(sel_vol_id, col, atof(value));
}

//---------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::IntensityDistributionCovarianceChangedCallback(
  vtkIdType sel_vol_id, int row, int col, const char *value)
{
  // The distribution covariance matrix has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeDistributionLogCovariance(sel_vol_id, row,col,atof(value));
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddIntensityDistributionSamplePoint(
  double ras[3])
{
  // Since it is not a callback, make sure we are really allowed to add
  // a sample point now

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  if (!wizard_widget ||
      wizard_widget->GetWizardWorkflow()->GetCurrentStep() != 
      this)
    {
    return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  if (!anat_step)
    {
    return;
    }
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id;
  if (tree->HasSelection())
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    if (sel_node.size() &&
        mrmlManager->GetTreeNodeIsLeaf(sel_vol_id) &&
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) ==
        vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample)
      {
      mrmlManager->AddTreeNodeDistributionSamplePoint(sel_vol_id, ras);
      this->DisplaySelectedNodeIntensityDistributionsCallback();
      int nb_samples = 
        mrmlManager->GetTreeNodeDistributionNumberOfSamples(sel_vol_id);
      vtkKWMultiColumnList *list = 
       this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
      list->SeeRow(nb_samples - 1);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::PopupManualIntensitySampleContextMenuCallback(int row, int, int x, int y)
{
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  if (!tree->HasSelection())
    {
    return;
    }

  vtksys_stl::string sel_node(tree->GetSelection());
  vtkIdType sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());

  vtkKWMultiColumnList *list = 
    this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
  list->SelectSingleRow(row);

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(list);
    this->ContextMenu->Create();
    }

  char buffer[256];
  this->ContextMenu->DeleteAllItems();

  if (row >= 0)
    {
    sprintf(
      buffer, "DeleteManualIntensitySampleCallback %d %d", 
      static_cast<int>(sel_vol_id), row);
    this->ContextMenu->AddCommand("Delete sample", this, buffer);
    }

  if (list->GetNumberOfRows())
    {
    sprintf(buffer, "DeleteAllManualIntensitySampleCallback %d", 
            static_cast<int>(sel_vol_id));
    this->ContextMenu->AddCommand("Delete all samples", this, buffer);
    }

  this->ContextMenu->PopUp(x, y);
}

//---------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::DeleteManualIntensitySampleCallback(
  vtkIdType sel_vol_id, int sample_index)
{
  // A sample has been deleted because of user interaction

  if (sample_index >= 0)
    {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    if (!mrmlManager)
      {
      return;
      }
    vtkKWMultiColumnList *list = 
      this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
    if (!list)
      {
      return;
      }
    list->DeleteRow(sample_index);
    mrmlManager->RemoveTreeNodeDistributionSamplePoint(sel_vol_id, sample_index);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
    }
}

//---------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::DeleteAllManualIntensitySampleCallback(vtkIdType sel_vol_id)
{
  // All samples have been deleted because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWMultiColumnList *list = 
    this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
  if (!mrmlManager || !list)
    {
    return;
    }
  list->DeleteAllRows();
  mrmlManager->RemoveAllTreeNodeDistributionSamplePoints(sel_vol_id);
  this->DisplaySelectedNodeIntensityDistributionsCallback();
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddManualIntensitySamplingGUIObservers() 
{
  // Slice GUI 0

  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveManualIntensitySamplingGUIObservers()
{
  // Slice GUI 0

  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessManualIntensitySamplingGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
  vtkSlicerInteractorStyle *s = vtkSlicerInteractorStyle::SafeDownCast(caller);

  if (s && 
      event == vtkCommand::LeftButtonPressEvent &&
      s->GetInteractor()->GetControlKey())
    {
    // Slice GUI 0

    vtkSlicerSliceGUI *sliceGUI0 = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red");

    vtkRenderWindowInteractor *rwi0 = sliceGUI0->GetSliceViewer()->
      GetRenderWidget()->GetRenderWindowInteractor();

    // Slice GUI 1

    vtkSlicerSliceGUI *sliceGUI1 = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow");

    vtkRenderWindowInteractor *rwi1 = sliceGUI1->GetSliceViewer()->
      GetRenderWidget()->GetRenderWindowInteractor();

    // Slice GUI 2

    vtkSlicerSliceGUI *sliceGUI2 = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green");

    vtkRenderWindowInteractor *rwi2 =  sliceGUI2->GetSliceViewer()->
      GetRenderWidget()->GetRenderWindowInteractor();

    vtkSlicerSliceGUI *sliceGUI = NULL;
    vtkRenderWindowInteractor *rwi = NULL;
    if (s == rwi0->GetInteractorStyle())
      {
      sliceGUI = sliceGUI0;
      rwi = rwi0;
      }
    else if (s == rwi1->GetInteractorStyle())
      {
      sliceGUI = sliceGUI1;
      rwi = rwi1;
      }
    else if (s == rwi2->GetInteractorStyle())
      {
      sliceGUI = sliceGUI2;
      rwi = rwi2;
      }

    int point[2];
    rwi->GetLastEventPosition(point);
    double inPt[4] = {point[0], point[1], 0, 1};
    double outPt[4];
    vtkMatrix4x4 *matrix = sliceGUI->GetLogic()->GetSliceNode()->GetXYToRAS();
    matrix->MultiplyPoint(inPt, outPt); 
    double ras[3] = {outPt[0], outPt[1], outPt[2]};
    
    std::cout<<"test1: "<<ras[0]<<std::endl;
    std::cout<<"test2: "<<ras[1]<<std::endl;
    std::cout<<"test3: "<<ras[2]<<std::endl;
    
    this->AddIntensityDistributionSamplePoint(ras);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddPointMovingGUIEvents() 
{

    this->IntensityDistributionHistogramHistogramVisu
    ->AddObserver(vtkKWParameterValueFunctionEditor::PointChangingEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemovePointMovingGUIEvents()
{
//this->VisuHisto->RemoveObserver(vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessPointMovingGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
if(event == vtkKWParameterValueFunctionEditor::PointChangingEvent){
  double* para,data;
int* test;
double* values;
int id = 2;
int dim;
double node_value[6];
double node_value_next[6];
double node_value_prev[6];
//this->IntensityDistributionHistogramHistogramVisu->GetFunctionPointValues(id,values);

//dim = this->IntensityDistributionHistogramHistogramVisu->GetFunctionPointDimensionality();

this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(id, node_value);

//memcpy(values, node_value + 1, dim * sizeof(double));

std::cout<<"values: "<< node_value[0] <<" occurence: "<<this->IntensityDistributionHistogramHistogram->GetOccurenceAtValue(node_value[0])<< std::endl;
std::cout<<"next mid: "<< node_value[4] << std::endl;


//std::cout<<"nb of class: "<< this->NbOfClassesEntryLabel->GetWidget()->GetValue() << std::endl;

std::cout<<"Size move "<< this->size << std::endl;
/*
if(this->size == 6){
*/

// GET POSITION OF CLASSES

 
 this->test();
/* int i;
 while(i<6){
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(id, node_value);
 this->position[id] = node_value[0];
  i++;
 }*/
 
 /*for(int i = 0;i < 6;i++){
this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(id, node_value);
this->position[id] = node_value[0];
}*/
 /*
 std::cout<<"positions "<< this->position[0] << std::endl;
 std::cout<<"positions "<< this->position[1] << std::endl;
 std::cout<<"positions "<< this->position[2] << std::endl;
 std::cout<<"positions "<< this->position[3] << std::endl;
 std::cout<<"positions "<< this->position[4] << std::endl;
 std::cout<<"positions "<< this->position[5] << std::endl;
*/
// GET SIZE OF CLASSES
/*
 int j = 0;
 double midmin;
 double midmax;
 
 for(j = 1;j<5;j++){
 
 std::cout<<"rentre dans la boucle 3"<< std::endl;
 
 midmin = 0;
 midmax = 0;
 
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i-1, node_value_prev);
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i, node_value);
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i+1, node_value_next);
 
 midmin = node_value[0] - (this->position[i] - this->position[i-1])*(node_value_prev[4]);
 midmax = node_value_next[0] - (this->position[i+1] - this->position[i])*(node_value[4]);   
 
 class_size[j*2] = midmin;
 class_size[j*2+1] = midmax;
 
 }
 
 //std::cout<<"class_size "<< this->class_size[0] << std::endl;
 //std::cout<<"class_size "<< this->class_size[1] << std::endl;
 std::cout<<"class_size "<< class_size[2] << std::endl;
 std::cout<<"class_size "<< class_size[3] << std::endl;
 std::cout<<"class_size "<< class_size[4] << std::endl;
 std::cout<<"class_size "<< class_size[5] << std::endl;
 std::cout<<"class_size "<< class_size[6] << std::endl;
 std::cout<<"class_size "<< class_size[7] << std::endl;
 std::cout<<"class_size "<< class_size[8] << std::endl;
 std::cout<<"class_size "<< class_size[9] << std::endl;
 //std::cout<<"positions "<< this->class_size[4] << std::endl;
 //std::cout<<"positions "<< this->class_size[5] << std::endl;
*/

//}

  }
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddPointAddGUIEvents() 
{

    this->IntensityDistributionHistogramHistogramVisu
    ->AddObserver(vtkKWParameterValueFunctionEditor::PointAddedEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemovePointAddGUIEvents()
{
//this->VisuHisto->RemoveObserver(vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessPointAddGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
if(event == vtkKWParameterValueFunctionEditor::PointAddedEvent){
  double* para,data;
int* test;
double* values;
int id = 2;
int dim;
double node_value[6];

//this->IntensityDistributionHistogramHistogramVisu->GetFunctionPointValues(id,values);

//dim = this->IntensityDistributionHistogramHistogramVisu->GetFunctionPointDimensionality();
/*
this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(id, node_value);

//memcpy(values, node_value + 1, dim * sizeof(double));

std::cout<<"values: "<< node_value[0] <<" occurence: "<<this->IntensityDistributionHistogramHistogram->GetOccurenceAtValue(node_value[0])<< std::endl;
std::cout<<"next mid: "<< node_value[4] << std::endl;
*/

std::cout<<"Point added"<< std::endl;

this->size = this->IntensityDistributionHistogramHistogramVisu->GetFunctionSize();
std::cout<<"Size"<< this->size <<std::endl;
/*
if (this->size >= 6 ){
this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(5, node_value);
double parameter = node_value[0];
this->IntensityDistributionHistogramHistogramFunc->RemovePoint(parameter);
}*/

  }
}


void vtkEMSegmentIntensityDistributionsStep::test(){
int i = 0;
int j = 0;
int k = 0;
int m = 0;
double midmin;
double midmax;
double node_value_next[6];
double node_value_prev[6];
double node_value[6];
//double class_size[(this->nbOfLeaf)*2];
double* testt;
double position[this->nbOfLeaf]; //6
double sum[this->nbOfLeaf]; //6
double total_sum;
double percent[this->nbOfLeaf]; //6

std::cout<<"GET POSITIONS"<< std::endl;

 while(i<(this->nbOfLeaf)){
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i, node_value);
 position[i] = node_value[0];
 //std::cout<<"i: "<< i <<"value: "<< position[i] << std::endl;
 i++;
 }
 

std::cout<<"GET MID POSITIONS (NO BORDERS)"<< std::endl;
 
 for(j = 1;j<(this->nbOfLeaf)-1;j++){
 midmin = 0;
 midmax = 0;
 
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j-1, node_value_prev);
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j, node_value);
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j+1, node_value_next);
 
 midmin = node_value[0] - (position[j] - position[j-1])*(1-node_value_prev[4]);
 midmax = node_value_next[0] - (position[j+1] - position[j])*(1-node_value[4]);   
 
 this->class_size[j*2] = midmin;
 this->class_size[j*2+1] = midmax;
 }

// GET BORDERS POSITIONS


if( this->nbOfLeaf > 2 ){
 this->class_size[0] = position[0];
 this->class_size[1] = this->class_size[2];

 this->class_size[(this->nbOfLeaf)*2 - 2] = this->class_size[(this->nbOfLeaf)*2-3];
 this->class_size[(this->nbOfLeaf)*2 - 1] = position[(this->nbOfLeaf)-1];

 std::cout<<"size: "<< this->class_size[0] <<" to: "<< this->class_size[1] << std::endl;
 std::cout<<"size: "<< this->class_size[(this->nbOfLeaf)*2 - 2]<<" to: "<< this->class_size[(this->nbOfLeaf)*2 - 1] << std::endl;
}
else{

midmin = 0;

this->class_size[0] = position[0];
this->class_size[3] = position[1];

 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(0, node_value);
 std::cout<<"node: "<< node_value[4] << std::endl;

 midmin = position[0] + (position[1] - position[0])*(node_value[4]);

this->class_size[1] = midmin;
this->class_size[2] = midmin;


//std::cout<< "MID" << midmin << std::endl;

std::cout<<"size: "<< this->class_size[0] <<" to: "<< this->class_size[1] << std::endl;
std::cout<<"size: "<< this->class_size[2] <<" to: "<< this->class_size[3] << std::endl;

}

for(k=0;k<this->nbOfLeaf;k++){

this->classSize[2*k] = this->class_size[2*k];
this->classSize[2*k+1] = this->class_size[2*k+1];

}
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetNumberOfLeaf(
  const char *parent, vtkIdType vol_id)
{
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  
  if(nb_children == 0){
  
  this->nbOfLeaf = this->nbOfLeaf +1;
  this->leafID[this->nbOfLeaf-1] = static_cast<int>(vol_id);
  //std::cout <<" LEAF: "<< this->nbOfLeaf << std::endl;
  //std::cout <<" ID: "<< this->leafID[this->nbOfLeaf-1] << std::endl;
  std::cout <<"ROOT ID: "<< mrmlManager->GetTreeRootNodeID()<< std::endl;
  }
  
  for (int i = 0; i < nb_children; i++)
    {
    this->GetNumberOfLeaf(
      NULL, mrmlManager->GetTreeNodeChildNodeID(vol_id, i));
      }
}
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetParentPercent(
  int i, vtkIdType vol_id)
{
  this->depth = this->depth +1 ;
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  vtkIdType parent_ID = mrmlManager->GetTreeNodeParentNodeID(vol_id);
  
  
  if(parent_ID != mrmlManager->GetTreeRootNodeID()){
  
  this->classPercentOrder[this->depth][i] = parent_ID;
  this->classPercentOrderCP[this->depth][i] = parent_ID;
  this->GetParentPercent(i, parent_ID);
  
  }
  /*else{
  this->classPercentOrder[this->depth][i] = 0;
  
  }*/

}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessTestButtonGUIEvents(
    vtkObject *caller, unsigned long event, void *callData)
{

if(event == vtkKWPushButton::InvokedEvent){

std::cout<<"TEST PERCENT"<<std::endl;

int i = 0;

std::cout<<"nbOfLeaf "<<this->nbOfLeaf<<std::endl;

for(i = 0;i < this->nbOfLeaf;i++){

std::cout<<"iteration "<<i<<std::endl;
std::cout<<"leaf ID "<<this->leafID[i]<<std::endl;

this->depth = 0;
this->classPercentOrder[0][i] = this->leafID[i];
this->classPercentOrderCP[0][i] = this->leafID[i];

this->GetParentPercent(i,this->leafID[i]);

}

i = 0;
int j = 0;
int k = 0;
int l = 0;
int m = 0;

int control = 1;

int position = 0;
int positionF ;

vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

//const char* node_name[200];

//std::cout<<"READ WIDGET LIST " <<  std::endl;

//std::cout<<"MAIN LOOP " <<  std::endl;
for(i = 0;i<this->nbOfLeaf;i++)
{for(j = 0;j<200;j++)
{for(k = 0;k<this->nbOfLeaf;k++){
for(l = 0;l<200;l++){
if(this->classPercentOrder[j][i] == this->classPercentOrderCP[l][k] &&
this->classPercentOrder[j][i] != 0)
{
this->classPercentOrderCP[l][k] = 0;
// FIND IF POINT EXISTS AND WHERE " this->classPercentOrder[j][i]
for(m = 0; m < 200 ;m++){
if(this->correspondanceArray[0][m] == this->classPercentOrder[j][i]){
positionF = m;
control = 0;
std::cout<<"POINT FOUND: "<< positionF <<std::endl;
}
}
if(control != 0){
positionF = position;
std::cout<<"POINT NOT FOUND: "<< positionF <<std::endl;
position++;
}
this->correspondanceArray[0][positionF] = this->classPercentOrder[j][i];
std::cout<<"COMPARE WIDGET AND ARRAY TO GET POSITION " <<  std::endl;
for(int z = 0; z< this-> nbOfLeaf;z++ ){
std::cout<<"VALUE OF Z: "<< z << std::endl;
if(strcmp(this->ClassAndNodeList->GetCellText(z,0),
mrmlManager->GetTreeNodeName(this->classPercentOrder[0][k])) == 0)
{
//this->ClassAndNodeList->GetCellText(z,0) ==
//mrmlManager->GetTreeNodeName(this->correspondanceArray[0][positionF])){
//std::cout<<"POINT FOUND IN WIDGET: "<< std::endl;
//std::cout<<"COMPARAISON WORKED: "<<this->ClassAndNodeList->GetCellText(z,0) << std::endl;
//std::cout<<"node name: "<< node_name[z] <<std::endl;
//std::cout<<"node position: "<< z <<std::endl;
//std::cout<<"weight added: "<< this->GetWeight(z) <<std::endl;
this->class_weight[positionF] = this->class_weight[positionF] + this->GetWeight(z); //remplace 1 by NB_PIX_[Z]
std::cout<<"CLASS WEIGHT : "<< this->class_weight[positionF] <<std::endl;
}
//else{
//const char* test[200];
//test[0] = mrmlManager->GetTreeNodeName(this->correspondanceArray[0][positionF]);
//std::cout<<"POINT NOT FOUND IN WIDGET: "<< std::endl;
//std::cout<<"COMPARAISON FAILED: "<<this->ClassAndNodeList->GetCellText(z,0)<<" AND " << test[0] << std::endl;
//vtksys_stl::string sel_node;
//string one = this->ClassAndNodeList->GetCellText(z,0);
//string two = test[0];
//std::cout<<"COMPARE STRINGS: " << strcmp(this->ClassAndNodeList->GetCellText(z,0),test[0]) << std::endl;
//}
}
std::cout<<"END LOOP: "<< std::endl;
control = 1;
}
}
}
}
}

vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetPercent(3,root_id);
      }
}

}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddTestButtonGUIEvents()
{

    this->TestButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveTestButtonGUIEvents()
{
//this->UpdatePrior->RemoveObserver(vtkKWPushButton::InvokedEvent, this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetPercent(
  int j, vtkIdType vol_id)
{
  this->depth = this->depth +1 ;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  double weight = 0.0;
  
  if(nb_children > 0){
for (int i = 0; i < nb_children; i++)
{
for(int m = 0; m < 200 ;m++){
if(this->correspondanceArray[0][m] == mrmlManager->GetTreeNodeChildNodeID(vol_id, i)){
weight = weight + this->class_weight[m];
//std::cout<<"TEST WEIGHT:"<< weight << std::endl;
}
}
}
for (int i = 0; i < nb_children; i++)
{
for(int m = 0; m < 200 ;m++){
if(this->correspondanceArray[0][m] == mrmlManager->GetTreeNodeChildNodeID(vol_id, i)){
//std::cout<<"NODE:"<< this->correspondanceArray[0][m]<< std::endl;
//std::cout<<"TEST WEIGHT2:"<< weight << std::endl;
//test1 = weight;
//std::cout<<"TEST ARRAY VALUE:"<< this->class_weight[m] << std::endl;
//test2 = this->correspondanceArray[m];
//test = test2/test1;
this->class_weight[m] = (this->class_weight[m])/weight;
//test = (this->class_weight[m])/weight;
std::cout<<"ID:"<< mrmlManager->GetTreeNodeChildNodeID(vol_id, i) << std::endl;
std::cout<< "VALUE: " << this->class_weight[m] << std::endl;
}
}
}

    for (int i = 0; i < nb_children; i++)
    {
    this->GetPercent(3,mrmlManager->GetTreeNodeChildNodeID(vol_id, i));
    }
      
  /*else{
  this->classPercentOrder[this->depth][i] = 0;
  
  }*/
  
  
  
for (int i = 0; i < 200; i++)
{
if(this->correspondanceArray[0][i] != 0){
mrmlManager->SetTreeNodeClassProbability(this->correspondanceArray[0][i], this->class_weight[i]);
std::cout << "CLASS UPDATED: "<< this->correspondanceArray[0][i] << "WEIGHT: " << this->class_weight[i] << std::endl;
}
}

}

}

double vtkEMSegmentIntensityDistributionsStep::GetWeight(
int z)
{
double sum = 0.0;
//int l = this->class_size[z*2]

for(int l = round(this->class_size[z*2]) + 1; l < round(this->class_size[z*2 + 1]); l++){
//std::cout<<"l: "<< l << std::endl;
//std::cout<<"sum: "<< sum << std::endl;
sum = sum + this->IntensityDistributionHistogramHistogram->GetOccurenceAtValue(l);
}

return sum;
}
