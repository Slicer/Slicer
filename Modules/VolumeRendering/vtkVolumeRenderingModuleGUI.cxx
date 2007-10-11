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
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"
#include "vtkLabelMapPiecewiseFunction.h"
#include "vtkLabelMapColorTransferFunction.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWTkUtilities.h"
#include "vtkTimerLog.h"


vtkVolumeRenderingModuleGUI::vtkVolumeRenderingModuleGUI(void)
{
    //Take care about own callback
    this->VolumeRenderingCallbackCommand=vtkCallbackCommand::New();
    this->VolumeRenderingCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
    this->VolumeRenderingCallbackCommand->SetCallback(vtkVolumeRenderingModuleGUI::VolumeRenderingCallback);

    this->Utilities=vtkKWTkUtilities::New();
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
    this->NS_VolumeRenderingDataSlicer=NULL;
    this->NS_VolumeRenderingDataScene=NULL;
    this->EWL_CreateNewVolumeRenderingNode=NULL;

    //Frame Details
    this->SVP_VolumeProperty=NULL;
        this->Histograms=NULL;
    this->detailsFrame=NULL;
    this->LM_OptionTree=NULL;

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

    //Not Delete?!
    //vtkVolumeRenderingModuleLogic *Logic;
    //vtkSlicerViewerWidget *ViewerWidget;
    //vtkSlicerViewerInteractorStyle *InteractorStyle;
    //vtkMRMLVolumeRenderingNode  *currentNode;//really delete this

    if(this->PB_Testing)
    {
        this->PB_Testing->SetParent(NULL);
        this->PB_Testing->Delete();
        this->PB_Testing=NULL;
    }

    if(this->PB_LoadVolumeRenderingDataSlicer)
    {
        this->PB_LoadVolumeRenderingDataSlicer->SetParent(NULL);
        this->PB_LoadVolumeRenderingDataSlicer->Delete();
        this->PB_LoadVolumeRenderingDataSlicer=NULL;
    }

    if (this->PB_CreateNewVolumeRenderingNode)
    {
        this->PB_CreateNewVolumeRenderingNode->SetParent(NULL);
        this->PB_CreateNewVolumeRenderingNode->Delete();
        this->PB_CreateNewVolumeRenderingNode=NULL;
    }

    if (this->NS_ImageData)
    {
        this->NS_ImageData->SetParent(NULL);
        this->NS_ImageData->Delete();
        this->NS_ImageData=NULL;
    }

    if(this->NS_VolumeRenderingDataScene)
    {
        this->NS_VolumeRenderingDataScene->SetParent(NULL);
        this->NS_VolumeRenderingDataScene->Delete();
        this->NS_VolumeRenderingDataScene=NULL;
    }

    if(this->NS_VolumeRenderingDataSlicer)
    {
        this->NS_VolumeRenderingDataSlicer->SetParent(NULL);
        this->NS_VolumeRenderingDataSlicer->Delete();
        this->NS_VolumeRenderingDataSlicer=NULL;
    }

    if(this->EWL_CreateNewVolumeRenderingNode)
    {
        this->EWL_CreateNewVolumeRenderingNode->SetParent(NULL);
        this->EWL_CreateNewVolumeRenderingNode->Delete();
        this->EWL_CreateNewVolumeRenderingNode=NULL;
    }

    if(this->SVP_VolumeProperty)
    {
        this->SVP_VolumeProperty->SetParent(NULL);
        this->SVP_VolumeProperty->Delete();
        this->SVP_VolumeProperty=NULL;
    }

    if(this->Histograms)
    {
        this->Histograms->Delete();
        this->Histograms=NULL;
    }

    if(this->presets)
    {
        this->presets->Delete();
        this->presets=NULL;
    }

    if(this->mapper)
    {
        this->mapper->Delete();
        this->mapper=NULL;
    }

    if(this->matrix)
    {
        this->matrix->Delete();
        this->matrix=NULL;
    }

    if(this->volume)
    {
        this->volume->Delete();
        this->volume=NULL;
    }

    if(this->VolumeRenderingCallbackCommand)
    {
        this->VolumeRenderingCallbackCommand->Delete();
        this->VolumeRenderingCallbackCommand=NULL;
    }

    if(this->Utilities)
    {
        this->Utilities->Delete();
        this->Utilities=NULL;
    }
    if(this->detailsFrame)
    {
        this->detailsFrame->Delete();
        this->detailsFrame=NULL;
    }
    if(this->LM_OptionTree)
    {
        this->LM_OptionTree->Delete();
        this->LM_OptionTree=NULL;
    }
    this->SetViewerWidget(NULL);
    this->SetInteractorStyle(NULL);
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
    this->NS_VolumeRenderingDataSlicer=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataSlicer->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_VolumeRenderingDataSlicer->Create();
    this->NS_VolumeRenderingDataSlicer->SetLabelText("Presets");
    this->NS_VolumeRenderingDataSlicer->EnabledOff();//By default off
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_VolumeRenderingDataSlicer->GetWidgetName());



    //NodeSelector for VolumeRenderingNode Scene
    this->NS_VolumeRenderingDataScene=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
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
    this->detailsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->detailsFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    this->detailsFrame->Create();
    this->detailsFrame->ExpandFrame();
    this->detailsFrame->SetLabelText("Details");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->detailsFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

    
    //set subnodes
    //Delete frames
    loadSaveDataFrame->Delete();
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
    this->NS_VolumeRenderingDataSlicer->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->PB_Testing->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );
    this->PB_CreateNewVolumeRenderingNode->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand*)this->GUICallbackCommand);

   
}
void vtkVolumeRenderingModuleGUI::RemoveGUIObservers(void)
{
    this->NS_ImageData->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->NS_VolumeRenderingDataScene->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->NS_VolumeRenderingDataSlicer->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->PB_Testing->RemoveObservers (vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
    this->PB_CreateNewVolumeRenderingNode->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
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
    // TODO Testing Button  Remove later
    if(callerObject==this->PB_Testing&&event==vtkKWPushButton::InvokedEvent)
    {
        int index=0;
        int count=this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
        for(int i=0;i<count;i++)
        {
            (vtkMRMLModelNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLModelNode")))->GetModelDisplayNode()->VisibilityOff();
        }
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
    }
    //
    // End Check PushButtons
    // 

    //
    //Check Node Selectors
    //
    vtkSlicerNodeSelectorWidget *callerObjectNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    //Load Volume
    if(callerObjectNS==this->NS_ImageData&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
        if(this->NS_ImageData->GetSelected()==NULL)
        {
            VolumeSelectedOff();
            this->ShutdownPipeline();
            this->PreviousNS_ImageData="";
        }
        //Only proceed event,if new Node
        else if(strcmp(this->NS_ImageData->GetSelected()->GetID(),this->PreviousNS_ImageData.c_str())!=0)
        {
            vtkMRMLScalarVolumeNode *selectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
            //This is a LabelMap
            if(selectedImageData->GetLabelMap()==1)
            {
                this->PackLabelMapGUI();

            }
            //This is NO LabelMap
            else
            {
                this->PackSvpGUI();
                //Take care about HistogramSet and ImageData
                this->SVP_VolumeProperty->SetDataSet(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData());

                //Remove all old Histograms
                this->Histograms->RemoveAllHistograms();

                //Add Histogram for image data
                this->Histograms->AddHistograms(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData()->GetPointData()->GetScalars());
                //Build the gradient histogram
                vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
                grad->SetDimensionality(3);
                grad->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData());
                grad->Update();
                vtkKWHistogram *gradHisto=vtkKWHistogram::New();
                gradHisto->BuildHistogram(grad->GetOutput()->GetPointData()->GetScalars(),0);
                this->Histograms->AddHistogram(gradHisto,"0gradient");

                this->InitializePipelineFromImageData();
                            //Delete
            grad->Delete();
            gradHisto->Delete();

            }
            this->VolumeSelectedOn();
            //update previous:
            this->PreviousNS_ImageData=this->NS_ImageData->GetSelected()->GetID();//only when not "None"


        }//else if
    }//if
    //Volume RenderingDataScene
    else if(callerObjectNS==this->NS_VolumeRenderingDataScene&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
        //Check for None selected //Just to be safe
        if(this->NS_VolumeRenderingDataScene->GetSelected()==NULL)
        {
            this->PreviousNS_VolumeRenderingDataScene="";
        }
        //Only proceed event,if new Node
        else if(strcmp(this->NS_VolumeRenderingDataScene->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingDataScene.c_str())!=0)
        {
            this->InitializePipelineFromMRMLScene();
            this->PreviousNS_VolumeRenderingDataScene=this->NS_VolumeRenderingDataScene->GetSelected()->GetID();
        }
    }
    //VolumeRenderingDataSlicer
    else if(callerObjectNS==this->NS_VolumeRenderingDataSlicer&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
        //Check for None selected
        if(this->NS_VolumeRenderingDataSlicer->GetSelected()==NULL)
        {
            this->PreviousNS_VolumeRenderingSlicer="";
        }
        //Only proceed event,if other Node
        else if(strcmp(this->NS_VolumeRenderingDataSlicer->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingSlicer.c_str())!=0)
        {
            //check if we have a preset or a "normal VolumeRenderingNode

            //We have a preset, we can find id in our presets
            if(this->presets->GetNodeByID(this->NS_VolumeRenderingDataSlicer->GetSelected()->GetID())!=NULL)
            {
                //Copy Preset Information in current Node
                this->currentNode->CopyParameterset(this->NS_VolumeRenderingDataSlicer->GetSelected());
                this->SVP_VolumeProperty->Modified();
                this->SVP_VolumeProperty->Update();
            }
            //It's not a preset so just update references, and select the new Added Node
            else 
            {
                this->currentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataSlicer->GetSelected());
                this->currentNode->AddReference(this->NS_ImageData->GetSelected()->GetID());
                this->NS_VolumeRenderingDataScene->UpdateMenu();
                this->NS_VolumeRenderingDataScene->SetSelected(this->NS_VolumeRenderingDataSlicer->GetSelected());
            }
        }
    }
    //
    //End Check NodeSelectors
    //
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

    //Update NodeSelector for VolumeRendering Node


    this->NS_VolumeRenderingDataScene->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataScene->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    //Set the new condition
    if(this->NS_ImageData->GetSelected()!=NULL)
    {
        this->NS_VolumeRenderingDataScene->SetCondition(this->NS_ImageData->GetSelected()->GetID(),true);
    }
    this->NS_VolumeRenderingDataScene->UpdateMenu();

    //Take care about Presets...
    //We need None for this
    this->NS_VolumeRenderingDataSlicer->NoneEnabledOn();
    this->NS_VolumeRenderingDataSlicer->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataSlicer->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    this->NS_VolumeRenderingDataSlicer->SetAdditionalMRMLScene(this->presets);
    this->NS_VolumeRenderingDataSlicer->UpdateMenu();

    if(this->NS_ImageData->GetSelected()!=NULL)
    {
        this->NS_VolumeRenderingDataSlicer->SetCondition(this->NS_ImageData->GetSelected()->GetID(),false);
    }

    //Disable/Enable after Volume is selected
    if(this->VolumeSelected==1)
    {
        this->PB_CreateNewVolumeRenderingNode->EnabledOn();
        this->PB_Testing->EnabledOn();
        this->NS_VolumeRenderingDataScene->EnabledOn();
        this->NS_VolumeRenderingDataScene->NoneEnabledOff();
        this->EWL_CreateNewVolumeRenderingNode->EnabledOn();
        this->NS_VolumeRenderingDataSlicer->EnabledOn();
    }
    else
    {
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
    vtkTimerLog *timer=vtkTimerLog::New();
    timer->StartTimer();
    this->currentNode=vtkMRMLVolumeRenderingNode::New();
    //Add Node to Scene
    this->currentNode->HideFromEditorsOff();
    this->GetLogic()->GetMRMLScene()->AddNode(this->currentNode);
    this->NS_VolumeRenderingDataScene->UpdateMenu();
    //take care about references
    this->currentNode->AddReference(this->NS_ImageData->GetSelected()->GetID());

    //Takes care about Histograms and VolumeProperty
    this->UpdateSVP();

    //automatic Mode
    if(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap()==1)
    {
        vtkTimerLog *timer1=vtkTimerLog::New();
        timer1->StartTimer();
        vtkLabelMapPiecewiseFunction *opacityNew=vtkLabelMapPiecewiseFunction::New();
        opacityNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected()),.3,0);
        this->currentNode->GetVolumeProperty()->SetScalarOpacity(opacityNew);
        this->currentNode->SetIsLabelMap(1);
        vtkLabelMapColorTransferFunction *colorNew=vtkLabelMapColorTransferFunction::New();
        colorNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected()));
        this->currentNode->GetVolumeProperty()->SetColor(colorNew);
        timer1->StopTimer();
        vtkErrorMacro("New current Node Labelmap withoutUpdateSVP calculated  in "<<timer1->GetElapsedTime()<<"seconds");
        this->UpdateSVP();

        //Delete
        opacityNew->Delete();
        colorNew->Delete();
        
    
    timer1->Delete();

    }
    //automatic Mode non LabelMaps
    else
    {
        vtkKWHistogram *histogram=this->Histograms->GetHistogramWithName("0");
        if(histogram==NULL)
        {
            vtkErrorMacro("Problems with HistogramSet");
            return;
        }
        double totalOccurance=histogram->GetTotalOccurence();
        double tresholdLow=totalOccurance*0.2;
        double tresholdHigh=totalOccurance*0.8;
        double range[2];
        
        histogram->GetRange(range);
        double tresholdLowIndex=range[0];
        double sumLowIndex=0;
        double tresholdHighIndex=range[0];
        double sumHighIndex=0;
        //calculate distance
        double bin_width = (range[1] == range[0] ? 1 :(double)histogram->GetNumberOfBins() / (range[1] - range[0]));
        while (sumLowIndex<tresholdLow)
        {
            sumLowIndex+=histogram->GetOccurenceAtValue(tresholdLowIndex);
            tresholdLowIndex+=bin_width;
        }
        while(sumHighIndex<tresholdHigh)
        {
            sumHighIndex+=histogram->GetOccurenceAtValue(tresholdHighIndex);
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
        this->SVP_VolumeProperty->Update();
        //and select Node as new Node
       
    }//else
            this->PipelineInitializedOn();
    timer->StopTimer();
    vtkErrorMacro("New current Node calculated in "<<timer->GetElapsedTime()<<"seconds");
    timer->Delete();
}//method
void vtkVolumeRenderingModuleGUI::InitializePipelineFromMRMLScene()
{
    //Change currentNode to selected Node
    //Delete old node if there is one
    if(this->currentNode!=NULL)
    {
        this->currentNode->Delete();
        this->currentNode=NULL;
    }
    this->currentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataScene->GetSelected());
    vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();

    this->UpdateSVP();
    this->VolumeRenderingNodeSelectedOn();
    this->UpdateRendering();
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
            //So everyting will be treated when InitializeFromMRMLScene
            this->NS_VolumeRenderingDataScene->SetSelected(tmp);
            this->InitializePipelineFromMRMLScene();
            firstNodeFound=true;
        }

    }//for

    //If not initialize a new auto generated Volume Rendering Node
    if(!firstNodeFound)
    {
        this->InitializePipelineNewCurrentNode();
    }
    //Render it
    this->PipelineInitializedOn();
    this->VolumeSelectedOn();
    this->UpdateRendering();
}
void vtkVolumeRenderingModuleGUI::Rendering()
{
    if(this->volume!=NULL)
    {
        vtkErrorMacro("Rendering already called, use update Rendering instead");
        return;
    }

    this->volume=vtkVolume::New();
    //TODO Dirty fix as Mapper
    if(this->currentNode->GetMapper()==vtkMRMLVolumeRenderingNode::Texture)
    {
        this->mapper=vtkVolumeTextureMapper3D::New();
        vtkVolumeTextureMapper3D::SafeDownCast(this->mapper)->SetSampleDistance(.1);
        this->mapper->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData());
        this->volume->SetMapper(mapper);
    }
    this->mapper->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->mapper->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->mapper->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    //TODO This is not the right place for this
   this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->volume->SetProperty(this->currentNode->GetVolumeProperty());
    this->matrix=vtkMatrix4x4::New();
    vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetIJKToRASMatrix(matrix);
    this->volume->PokeMatrix(matrix);
    vtkKWRenderWidget *renderWidget=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
    renderWidget->AddViewProp(volume);
    renderWidget->Render();
    //Deletes in destructor!
}
void vtkVolumeRenderingModuleGUI::UpdateRendering()
{
    //first check if REndering was already called
    if(this->volume==NULL)
    {
        this->Rendering();
    }
    //Update mapper
    this->mapper->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData());
    //Update Property
    this->volume->SetProperty(this->currentNode->GetVolumeProperty());
    //Update matrix
    vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetIJKToRASMatrix(this->matrix);
    this->volume->PokeMatrix(matrix);
    this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkVolumeRenderingModuleGUI::CheckAbort ()
{
    int pending=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    if(pending!=0)
    {
        this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        return;
    }
    int pendingGUI=this->CheckForPendingEvents();
    if(pendingGUI!=0)
    {
        this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
    }
}

void vtkVolumeRenderingModuleGUI::ShutdownPipeline()
{
    vtkKWRenderWidget *renderWidget=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
    //Remove Observers
    this->mapper->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->mapper->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->mapper->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    if(this->Utilities)
    {
        this->Utilities->Delete();
        this->Utilities=NULL;
    }
    //Remove Volume
    if(this->volume!=NULL)
    {
        renderWidget->RemoveViewProp(this->volume);
        this->volume->Delete();
        this->volume=NULL;
        renderWidget->Render();
    }
    if(this->mapper!=NULL)
    {
        this->mapper->Delete();
        this->mapper=NULL;
    }
    if(this->matrix!=NULL)
    {
        this->matrix->Delete();
        this->matrix=NULL;
    }
    //Take care about GUI
    this->Histograms->RemoveAllHistograms();
    this->SVP_VolumeProperty->SetVolumeProperty(NULL);
    this->SVP_VolumeProperty->SetDataSet(NULL);
}

void vtkVolumeRenderingModuleGUI::UpdateSVP()
{
    vtkTimerLog *timer=vtkTimerLog::New();
    timer->StartTimer();
    //Take care about histogram
    //Update Histogramm Scalar
    //First of all set New Property, Otherwise all Histograms will be overwritten
    //First check if we really need to update
    if(this->SVP_VolumeProperty->GetVolumeProperty()==this->currentNode->GetVolumeProperty())
    {
        this->SVP_VolumeProperty->Update();
        timer->StopTimer();
        vtkErrorMacro("Update SVP calculated in "<<timer->GetElapsedTime()<<"seconds");
        timer->Delete();
        return;
    }
    this->SVP_VolumeProperty->SetVolumeProperty(this->currentNode->GetVolumeProperty());
    this->SVP_VolumeProperty->SetHSVColorSelectorVisibility(1);

    this->SVP_VolumeProperty->Update();


    //Adjust mapping
    this->AdjustMapping();
    timer->StopTimer();
    vtkErrorMacro("Update SVP calculated in "<<timer->GetElapsedTime()<<"seconds");
    timer->Delete();

}


void vtkVolumeRenderingModuleGUI::AdjustMapping(){

    //Update Color    
    //vtkColorTransferFunction *functionColor=this->currentNode->GetVolumeProperty()->GetRGBTransferFunction();
    //double rangeNew[2];
    //this->HIST_Opacity->GetRange(rangeNew);
    //functionColor->AdjustRange(rangeNew);

    ////Update Opacity
    //vtkPiecewiseFunction *function=this->currentNode->GetVolumeProperty()->GetScalarOpacity();
    //this->HIST_Opacity->GetRange(rangeNew);
    //function->AdjustRange(rangeNew);
    ////Update
    //function=this->currentNode->GetVolumeProperty()->GetGradientOpacity();
    //this->HIST_Gradient->GetRange(rangeNew);
    //function->AdjustRange(rangeNew);
}


void vtkVolumeRenderingModuleGUI::VolumeRenderingCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{
    vtkVolumeRenderingModuleGUI *self = reinterpret_cast<vtkVolumeRenderingModuleGUI *>(clientData);


    if (self->GetInVolumeRenderingCallbackFlag())
    {
#ifdef _DEBUG
        vtkDebugWithObjectMacro(self, "In vtkVOlumeRendering *********GUICallback called recursively?");
#endif
        //return;
    }

    vtkDebugWithObjectMacro(self, "In vtkVolumeREndering GUICallback");

    self->SetInVolumeRenderingCallbackFlag(1);
    self->ProcessVolumeRenderingEvents(caller, eid, callData);
    self->SetInVolumeRenderingCallbackFlag(0);
}

void vtkVolumeRenderingModuleGUI::ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData){

    //TODO not the right place for this
    vtkRenderWindow *callerRen=vtkRenderWindow::SafeDownCast(caller);
    if(caller==this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::AbortCheckEvent)
    {
        this->CheckAbort();
        return;


    }
    //Check if caller equals mapper
    vtkAbstractMapper *callerMapper=vtkAbstractMapper::SafeDownCast(caller);
    if(callerMapper!=this->mapper)
    {
        return;
    }
    if(eid==vtkCommand::VolumeMapperComputeGradientsStartEvent)
    {
        this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Calculate Gradients");
    }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsEndEvent)
    {
        this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Calculate Gradients finished");
    }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsProgressEvent)
    {
        float *progress=(float*)callData;
        this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100**progress);
    }
}
int vtkVolumeRenderingModuleGUI_EventDeleteProc (Tcl_Event *evPtr, ClientData clientData)
{
  vtkVolumeRenderingModuleGUI *self = (  vtkVolumeRenderingModuleGUI *) (clientData);
  self->SetEventsPending(1);
  return 0;
}

//----------------------------------------------------------------------------
int vtkVolumeRenderingModuleGUI::CheckForPendingEvents()
{
  this->SetEventsPending(0);
  Tcl_DeleteEvents(vtkVolumeRenderingModuleGUI_EventDeleteProc, static_cast<ClientData> (this));
  return (this->GetEventsPending());
} 

void vtkVolumeRenderingModuleGUI::PackLabelMapGUI()
{
    if(this->LM_OptionTree!=NULL)
    {
        vtkErrorMacro("LM_OptionTree: Already packed");
        return;
    }
    this->UnpackSvpGUI();
    this->LM_OptionTree=vtkSlicerLabelMapWidget::New();
    this->LM_OptionTree->SetParent(this->detailsFrame->GetFrame());
    this->LM_OptionTree->Create();
    ((vtkSlicerApplication *)this->GetApplication())->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",this->LM_OptionTree->GetWidgetName(),this->detailsFrame->GetFrame()->GetWidgetName());
}

void vtkVolumeRenderingModuleGUI::UnpackLabelMapGUI()
{
    if(this->LM_OptionTree==NULL)
    {
        vtkErrorMacro("LM_OptionTree: Already unpacked");
        return;
    }
     ((vtkSlicerApplication *)this->GetApplication())->Script("pack forget %s",this->LM_OptionTree->GetWidgetName());
     this->LM_OptionTree->SetParent(NULL);
     this->LM_OptionTree->Delete();
     this->LM_OptionTree=NULL;
     //this->
}

void vtkVolumeRenderingModuleGUI::PackSvpGUI()
{
    //Histogram
     if(this->SVP_VolumeProperty!=NULL)
    {
        vtkErrorMacro("SVP_VolumeProperty: Already packed");
        return;
    }
    this->UnpackLabelMapGUI();
    this->Histograms=vtkKWHistogramSet::New();
    //Volume Property frame
    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(this->detailsFrame->GetFrame());
    this->SVP_VolumeProperty->Create();
    this->SVP_VolumeProperty->SetHistogramSet(this->Histograms);
    ((vtkSlicerApplication *)this->GetApplication())->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());

}
void vtkVolumeRenderingModuleGUI::UnpackSvpGUI()
{
    if(this->SVP_VolumeProperty==NULL)
    {
        vtkErrorMacro("SVP_VolumeProperty: Nothing to unpack");
        return;
    }

     ((vtkSlicerApplication *)this->GetApplication())->Script("pack forget %s",this->SVP_VolumeProperty->GetWidgetName());
     this->SVP_VolumeProperty->SetHistogramSet(NULL);
     this->SVP_VolumeProperty->SetVolumeProperty(NULL);
     this->SVP_VolumeProperty->SetDataSet(NULL);
     this->SVP_VolumeProperty->SetParent(NULL);

     if(this->Histograms)
     {
         this->Histograms->RemoveAllHistograms();
         this->Histograms->Delete();
         this->Histograms=NULL;
     }
     this->SVP_VolumeProperty->Delete();
     this->SVP_VolumeProperty=NULL;

}

