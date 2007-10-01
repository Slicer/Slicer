#include "vtkVolumeRenderingModuleGUI.h"

#include <ostream>
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkIndent.h"
#include "vtkPointData.h"
#include "vtkPiecewisefunction.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkKWEntry.h"
#include "vtkVolume.h"


vtkVolumeRenderingModuleGUI::vtkVolumeRenderingModuleGUI(void)
{
    //In Debug Mode
    this->DebugOff();
    this->presets=NULL;
    this->PreviousNS_ImageData="";
    this->PreviousNS_VolumeRenderingDataScene="";
    this->PreviousNS_VolumeRenderingSlicer="";
    this->volume=NULL;
    this->PB_Testing=NULL;
    this->PB_LoadVolumeRenderingDataSlicer=NULL;
    this->PB_CreateNewVolumeRenderingNode=NULL;
    this->NS_ImageData=NULL;
  this->NS_VolumeRenderingDataSlicer;
  this->NS_VolumeRenderingDataScene;
  this->EWL_CreateNewVolumeRenderingNode;

  //Frame Details
  this->SVP_VolumeProperty=NULL;
  this->HIST_Opacity=NULL;
  this->HIST_Gradient=NULL;


  //Other members
  this->currentNode=NULL;
  this->presets=NULL;

  //Rendering pipeline
  this->volume=NULL;
  this->mapper=NULL;
  this->matrix=NULL;
   
}

vtkVolumeRenderingModuleGUI::~vtkVolumeRenderingModuleGUI(void)
{
}
vtkVolumeRenderingModuleGUI* vtkVolumeRenderingModuleGUI::New() {
    // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingModuleGUI");
  if(ret)
    {
      return (vtkVolumeRenderingModuleGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingModuleGUI;


}
void vtkVolumeRenderingModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
    os<<indent<<"vtkVolumeRenderingModuleGUI"<<endl;
    os<<indent<<"vtkVolumeRenderingModuleLogic"<<endl;
    if(this->GetLogic())
    {
        this->GetLogic()->PrintSelf(os,indent.GetNextIndent());
    }
}
void vtkVolumeRenderingModuleGUI::BuildGUI(void)
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    this->GetUIPanel()->AddPage("VolumeRendering","VolumeRendering",NULL);

    // Define your help text and build the help frame here.
    const char *help = "VolumeRendering. 3D Segmentation This module is currently a prototype and will be under active development throughout 3DSlicer's Beta release.";
    const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRendering" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    //
    //Load and save
    //
    vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    loadSaveDataFrame->Create();
    loadSaveDataFrame->ExpandFrame();
    loadSaveDataFrame->SetLabelText("Load and save");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());
    
    //Testing Pushbutton
    this->PB_Testing= vtkKWPushButton::New();
    this->PB_Testing->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_Testing->Create();
    this->PB_Testing->SetText("Testing");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_Testing->GetWidgetName());
    
    //NodeSelector  for Node from MRML Scene
    this->NS_ImageData=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageData->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_ImageData->Create();
    this->NS_ImageData->NoneEnabledOn();
    this->NS_ImageData->SetLabelText("Source Volume");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_ImageData->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Preset
    this->NS_VolumeRenderingDataSlicer=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataSlicer->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_VolumeRenderingDataSlicer->Create();
    this->NS_VolumeRenderingDataSlicer->SetLabelText("Presets");
    this->NS_VolumeRenderingDataSlicer->EnabledOff();//By default off
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_VolumeRenderingDataSlicer->GetWidgetName());



    //NodeSelector for VolumeRenderingNode Scene
    this->NS_VolumeRenderingDataScene=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataScene->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_VolumeRenderingDataScene->Create();
    this->NS_VolumeRenderingDataScene->NoneEnabledOn();
    this->NS_VolumeRenderingDataScene->SetLabelText("VolumeRenderingNode from Scene");
    this->NS_VolumeRenderingDataScene->EnabledOff();//By default off
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_VolumeRenderingDataScene->GetWidgetName());
    //Missing: Load from file
    
    //Create New Volume Rendering Node
    //Entry With Label
    this->EWL_CreateNewVolumeRenderingNode=vtkKWEntryWithLabel::New();
    this->EWL_CreateNewVolumeRenderingNode->SetParent(loadSaveDataFrame->GetFrame());
    this->EWL_CreateNewVolumeRenderingNode->Create();
    this->EWL_CreateNewVolumeRenderingNode->SetLabelText("Name for new Volume Rendering Node");
    this->EWL_CreateNewVolumeRenderingNode->EnabledOff();
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2", this->EWL_CreateNewVolumeRenderingNode->GetWidgetName());
    

    this->PB_CreateNewVolumeRenderingNode=vtkKWPushButton::New();
    this->PB_CreateNewVolumeRenderingNode->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_CreateNewVolumeRenderingNode->Create();
    this->PB_CreateNewVolumeRenderingNode->SetText("Create new VolumeRenderingNode");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_CreateNewVolumeRenderingNode->GetWidgetName());

    //Details frame
     vtkSlicerModuleCollapsibleFrame *detailsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    detailsFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    detailsFrame->Create();
    detailsFrame->ExpandFrame();
    detailsFrame->SetLabelText("Details");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        detailsFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());
    
    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(detailsFrame->GetFrame());
    this->SVP_VolumeProperty->Create();
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());

    //Histogram
    this->HIST_Opacity=vtkKWHistogram::New();
    this->HIST_Gradient=vtkKWHistogram::New();
      
    //Delete frames
    loadSaveDataFrame->Delete();
    detailsFrame->Delete();
    this->Built=true;
}

void vtkVolumeRenderingModuleGUI::TearDownGUI(void)
{
    this->Exit();
    if ( this->Built )
    {
        this->RemoveGUIObservers();
    }
}

void vtkVolumeRenderingModuleGUI::CreateModuleEventBindings(void)
{
    vtkDebugMacro("VolumeRenderingModule: CreateModuleEventBindings: No ModuleEventBindings yet");
}

void vtkVolumeRenderingModuleGUI::ReleaseModuleEventBindings(void)
{
    vtkDebugMacro("VolumeRenderingModule: ReleaseModuleEventBindings: No ModuleEventBindings to remove yet");
}

void vtkVolumeRenderingModuleGUI::AddGUIObservers(void)
{
    this->NS_ImageData->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->NS_VolumeRenderingDataScene->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->PB_Testing->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );
    this->PB_CreateNewVolumeRenderingNode->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand*)this->GUICallbackCommand);

    //TODO is this the right place for this
//this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->CheckAbort);
}
void vtkVolumeRenderingModuleGUI::RemoveGUIObservers(void)
{
    this->PB_Testing->RemoveObservers (vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
}
void vtkVolumeRenderingModuleGUI::RemoveMRMLNodeObservers(void)
{

}
void vtkVolumeRenderingModuleGUI::RemoveLogicObservers(void)
{
}

void vtkVolumeRenderingModuleGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData)
{
     vtkDebugMacro("vtkVolumeRenderingModuleGUI::ProcessGUIEvents: event = " << event);
     //
     //Check PushButtons
     //
     vtkKWPushButton *callerObject=vtkKWPushButton::SafeDownCast(caller);
     //Testing Button ?
     if(callerObject==this->PB_Testing&&event==vtkKWPushButton::InvokedEvent)
     {
         int index=0;
         int count=this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
         for(int i=0;i<count;i++)
         {
             (vtkMRMLModelNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLModelNode")))->GetModelDisplayNode()->VisibilityOff();
         }
       this->Rendering();
     }
     //Create New VolumeRenderingNode
     else if (callerObject==this->PB_CreateNewVolumeRenderingNode&&event==vtkKWPushButton::InvokedEvent)
     {
         //Get a new auto currentNode
         this->InitializePipelineNewCurrentNode();
         this->currentNode->HideFromEditorsOff();
         //Set the right name
         const char *name=this->EWL_CreateNewVolumeRenderingNode->GetWidget()->GetValue();
         if(!name)
         {
             vtkErrorMacro("No Text for VolumeRenderingNode");
         }
         else 
         {
             this->currentNode->SetName(name);
         }
         //Now a Node is Selected
         this->VolumeRenderingNodeSelectedOn();
         //Remove Text from Entry
         this->EWL_CreateNewVolumeRenderingNode->GetWidget()->SetValue("");
         //Load the Node (because of Copy)
         //this->InitializePipelineFromMRMLScene();

     }
    
     //All PushButtons Processed
     //Node Selectors 
     vtkSlicerNodeSelectorWidget *callerObjectNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
     //Load Volume
     if(callerObjectNS==this->NS_ImageData&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
     {
         if(this->NS_ImageData->GetSelected()==NULL)
         {
             VolumeSelectedOff();
             this->ShutdownPipeline();
             //TODO Shutdown pipeline
         }
         //Only proceed event,if new Node
         else if(strcmp(this->NS_ImageData->GetSelected()->GetID(),this->PreviousNS_ImageData.c_str())!=0)
         {
             this->InitializePipelineFromImageData();
             this->VolumeSelectedOn();
         //update previous:
         this->PreviousNS_ImageData=this->NS_ImageData->GetSelected()->GetID();//only when not "None"
         }//else if
     }//if
     else if(callerObjectNS==this->NS_VolumeRenderingDataScene&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
     {
         //Only proceed event,if new Node
         if(strcmp(this->NS_VolumeRenderingDataScene->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingDataScene.c_str())!=0)
         {
             this->InitializePipelineFromMRMLScene();
             this->PreviousNS_VolumeRenderingDataScene=this->NS_VolumeRenderingDataScene->GetSelected()->GetID();
         }
     }
     //NodeSelectors End

     //Update GUI
     this->UpdateGUI();

}
void vtkVolumeRenderingModuleGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
}

void vtkVolumeRenderingModuleGUI::Enter(void)
{

    vtkDebugMacro("Enter Volume Rendering Module");
    //Load Presets
    vtkMRMLVolumeRenderingNode *vrNode=vtkMRMLVolumeRenderingNode::New();

    if(!this->presets)
    {
        //Instance internal MRMLScene for Presets
        this->presets=vtkMRMLScene::New();
        //Register node class
        this->presets->RegisterNodeClass(vrNode);
        vrNode->Delete();


        //GetPath
        vtksys_stl::string slicerHome;
        if (vtksys::SystemTools::GetEnv("SLICER_HOME") == NULL)
        {
            if (vtksys::SystemTools::GetEnv("PWD") != NULL)
            {
                slicerHome =  vtksys_stl::string(vtksys::SystemTools::GetEnv("PWD"));
            }
            else
            {
                slicerHome =  vtksys_stl::string("");
            }
        }
        else
        {
            slicerHome = vtksys_stl::string(vtksys::SystemTools::GetEnv("SLICER_HOME"));
        }
        // check to see if slicer home was set
        vtksys_stl::vector<vtksys_stl::string> filesVector;
        filesVector.push_back(""); // for relative path
        filesVector.push_back(slicerHome);
        filesVector.push_back(vtksys_stl::string("Modules/VolumeRendering/presets.xml"));
        vtksys_stl::string presetFileName = vtksys::SystemTools::JoinPath(filesVector);
        this->presets->SetURL(presetFileName.c_str());
        this->presets->Connect();
        
        //TODO Deal with difference between ID and name
        //add None
        this->NS_VolumeRenderingDataSlicer->GetWidget()->GetWidget()->GetMenu()->AddRadioButton("None");
        for(int i=0;i<this->presets->GetNumberOfNodes();i++)
        {
            vtkMRMLNode *node=this->presets->GetNthNode(i);
            this->NS_VolumeRenderingDataSlicer->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(node->GetID());
        }
    }
    //End Load presets

    if ( this->Built == false )
    {
        this->BuildGUI();
        this->AddGUIObservers();
    }
    this->CreateModuleEventBindings();
    this->UpdateGUI();
}

void vtkVolumeRenderingModuleGUI::Exit(void)
{
    vtkDebugMacro("Exit: removeObservers for VolumeRenderingModule");
  this->ReleaseModuleEventBindings();
}

void vtkVolumeRenderingModuleGUI::UpdateGUI(void)
{
   
    //First of all check if we have a MRML Scene
    if (!this->GetLogic()->GetMRMLScene())
    {
        //if not return
        return;
    }
    //Update the NodeSelector for Volumes
    this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageData->UpdateMenu();

    //Update NodeSelector for VolumeRendering Nodes

    this->NS_VolumeRenderingDataScene->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataScene->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    this->NS_VolumeRenderingDataScene->UpdateMenu();


    //Disable/Enable after Volume is selected
    if(this->VolumeSelected==1)
    {
       // this->PB_LoadVolumeRenderingDataSlicer->EnabledOn();
        this->PB_CreateNewVolumeRenderingNode->EnabledOn();
        this->PB_Testing->EnabledOn();
        this->NS_VolumeRenderingDataScene->EnabledOn();
        this->EWL_CreateNewVolumeRenderingNode->EnabledOn();
        //There ist always a VolumeRenderingNodeSelected that belongs to the MRML Scene
       // this->NS_VolumeRenderingDataScene->NewNodeEnabledOff();
        this->NS_VolumeRenderingDataSlicer->EnabledOn();
    }
    else
    {
       // this->PB_LoadVolumeRenderingDataSlicer->EnabledOff();
        this->EWL_CreateNewVolumeRenderingNode->EnabledOff();
        this->PB_CreateNewVolumeRenderingNode->EnabledOff();
        this->PB_Testing->EnabledOff();
        this->NS_VolumeRenderingDataScene->NoneEnabledOn();
        this->NS_VolumeRenderingDataScene->EnabledOff();
        this->NS_VolumeRenderingDataSlicer->EnabledOff();
    }
    this->Modified();
    
}
void vtkVolumeRenderingModuleGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingModuleGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}

void vtkVolumeRenderingModuleGUI::InitializePipelineNewCurrentNode()
{
     this->currentNode=vtkMRMLVolumeRenderingNode::New();
     //Add Node to Scene
     this->currentNode->HideFromEditorsOff();
     this->GetLogic()->GetMRMLScene()->AddNode(this->currentNode);
     this->NS_VolumeRenderingDataScene->UpdateMenu();
     //take care about references
     this->currentNode->AddReference(this->NS_ImageData->GetSelected()->GetID());
     vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();

    //Update Input of Mapper

    //Take care about histogram and automatic mapping
    //Add Histogram to GUI
    this->HIST_Opacity->BuildHistogram(imageData->GetPointData()->GetScalars(),0);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetHistogram(this->HIST_Opacity);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetHistogram(this->HIST_Opacity);
    //automatic Mode
    if(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap()==1)
    {
        vtkWarningMacro("Not yet implemented");
    }
    //automatic Mode for LabelMaps
    else
    {
        double totalOccurance=this->HIST_Opacity->GetTotalOccurence();
        double tresholdLow=totalOccurance*0.2;
        double tresholdHigh=totalOccurance*0.8;
        double *range=new double[2];
        this->HIST_Opacity->GetRange(range);
        double tresholdLowIndex=range[0];
        double sumLowIndex=0;
        double tresholdHighIndex=range[0];
        double sumHighIndex=0;
        //calculate distance
        double bin_width = (range[1] == range[0] ? 1 :(double)this->HIST_Opacity->GetNumberOfBins() / (range[1] - range[0]));
        while (sumLowIndex<tresholdLow)
        {
            sumLowIndex+=this->HIST_Opacity->GetOccurenceAtValue(tresholdLowIndex);
            tresholdLowIndex+=bin_width;
        }
        while(sumHighIndex<tresholdHigh)
        {
            sumHighIndex+=this->HIST_Opacity->GetOccurenceAtValue(tresholdHighIndex);
            tresholdHighIndex+=bin_width;

        }
        vtkPiecewiseFunction *opacity=this->currentNode->GetVolumeProperty()->GetScalarOpacity();
        opacity->RemoveAllPoints();
        opacity->AddPoint(range[0],0.);
        opacity->AddPoint(tresholdLowIndex,0.0);
        opacity->AddPoint(tresholdHighIndex,0.2);
        opacity->AddPoint(range[1],0.2);
        vtkColorTransferFunction *colorTransfer=this->currentNode->GetVolumeProperty()->GetRGBTransferFunction();
        colorTransfer->RemoveAllPoints();
        colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
        colorTransfer->AddRGBPoint(tresholdLowIndex,.3,.3,1.);
        colorTransfer->AddRGBPoint(tresholdLowIndex+.5*(tresholdHighIndex-tresholdLowIndex),.3,1.,.3);
        colorTransfer->AddRGBPoint(tresholdHighIndex,1.,.3,.3);
        colorTransfer->AddRGBPoint(range[1],1,.3,.3);
        vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
        grad->SetDimensionality(3);
        grad->SetInput(imageData);
        grad->Update();

        vtkImageData *test=grad->GetOutput();
        this->HIST_Gradient->BuildHistogram(test->GetPointData()->GetScalars(),0);
        this->SVP_VolumeProperty->GetGradientOpacityFunctionEditor()->SetHistogram(this->HIST_Gradient);
        vtkPiecewiseFunction *gradFunction=this->currentNode->GetVolumeProperty()->GetGradientOpacity();
        gradFunction->AdjustRange(test->GetPointData()->GetScalars()->GetRange());


        //Add VolumeProperty To Gui
        this->SVP_VolumeProperty->SetVolumeProperty(this->currentNode->GetVolumeProperty());

        //and select Node as new Node
        this->NS_VolumeRenderingDataScene->EnabledOn();
        this->NS_VolumeRenderingDataScene->NoneEnabledOff();
        this->Modified();
        this->NS_VolumeRenderingDataScene->SetSelected(this->currentNode);
        this->Modified();
    
    this->PipelineInitializedOn();
    }//else
}//method
void vtkVolumeRenderingModuleGUI::InitializePipelineFromMRMLScene()
{
    //Change currentNode to selected Node
    this->currentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataScene->GetSelected());
     vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();

    //Take care about histogram
    //Update Histogramm Scalar
    this->HIST_Opacity->BuildHistogram(imageData->GetPointData()->GetScalars(),0);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetHistogram(this->HIST_Opacity);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetHistogram(this->HIST_Opacity);

    //Update Histogram Gradient
    vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
    grad->SetDimensionality(3);
    grad->SetInput(imageData);
    grad->Update();
    vtkImageData *test=grad->GetOutput();
    this->HIST_Gradient->BuildHistogram(test->GetPointData()->GetScalars(),0);
    this->SVP_VolumeProperty->GetGradientOpacityFunctionEditor()->SetHistogram(this->HIST_Gradient);
    vtkPiecewiseFunction *gradFunction=this->currentNode->GetVolumeProperty()->GetGradientOpacity();
    gradFunction->AdjustRange(test->GetPointData()->GetScalars()->GetRange());

     this->SVP_VolumeProperty->SetVolumeProperty(this->currentNode->GetVolumeProperty());
     this->SVP_VolumeProperty->Modified();
     //Update Rendering

     this->VolumeRenderingNodeSelectedOn();
}
void vtkVolumeRenderingModuleGUI::InitializePipelineFromSlicer()
{
}
void vtkVolumeRenderingModuleGUI::InitializePipelineFromImageData()
{
    //First check if we already have a reference
    const char *id=this->NS_ImageData->GetSelected()->GetID();
    //loop over existing Nodes in scene
    bool firstNodeFound=false;
    bool separatorAdded=false;
    for( int i=0;i<this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeRenderingNode");i++)
    {
        vtkMRMLVolumeRenderingNode *tmp=vtkMRMLVolumeRenderingNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLVolumeRenderingNode"));
        if(tmp->HasReference(id)&&!firstNodeFound)
        {
            //Select first found Node
            this->NS_VolumeRenderingDataScene->SetSelected(tmp);
            firstNodeFound=true;
        }
        else if(!tmp->HasReference(id))
        {
            //Is there already a separator
            if (!separatorAdded)
            {
                this->NS_VolumeRenderingDataSlicer->GetWidget()->GetWidget()->GetMenu()->AddSeparator();
                separatorAdded=true;
            }
            else
            {
                this->NS_VolumeRenderingDataSlicer->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(tmp->GetID());
            }//else separator
        }//else if
    }//for

    //If not initialize a new auto generated Volume Rendering Node
    if(!firstNodeFound)
    {
        this->InitializePipelineNewCurrentNode();
    }
}
void vtkVolumeRenderingModuleGUI::Rendering()
{
    this->volume=vtkVolume::New();
    //TODO Dirty fix as Mapper
    if(this->currentNode->GetMapper()==vtkMRMLVolumeRenderingNode::Texture)
    {
        this->mapper=vtkVolumeTextureMapper3D::New();
        vtkVolumeTextureMapper3D::SafeDownCast(this->mapper)->SetSampleDistance(.1);
        mapper->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData());
        volume->SetMapper(mapper);
    }
    volume->SetProperty(this->currentNode->GetVolumeProperty());
    this->matrix=vtkMatrix4x4::New();
    vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetIJKToRASMatrix(matrix);
    volume->PokeMatrix(matrix);
    vtkKWRenderWidget *renderWidget=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
    renderWidget->AddViewProp(volume);
    renderWidget->Create();
}


void vtkVolumeRenderingModuleGUI::CheckAbort ()
{
    int pending=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    if(pending!=0)
    {
        this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
    }
}

void vtkVolumeRenderingModuleGUI::ShutdownPipeline()
{
vtkKWRenderWidget *renderWidget=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
//Remove Volume
if(this->volume!=NULL)
{
    renderWidget->RemoveViewProp(this->volume);
    this->volume->Delete();
    this->volume=NULL;
    renderWidget->Render();
}
 //   this->SVP_VolumeProperty->SetVolumeProperty(NULL);
}




