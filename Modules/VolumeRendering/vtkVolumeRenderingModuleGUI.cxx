#include "vtkVolumeRenderingModuleGUI.h"

#include <ostream>
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkIndent.h"
#include "vtkPointData.h"
#include "vtkPiecewiseFunction.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkKWEntry.h"
#include "vtkVolume.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWTkUtilities.h"
#include "vtkTimerLog.h"
#include "vtkSlicerMRMLTreeWidget.h"
#include "vtkRendererCollection.h"
#include "vtkImageData.h"
#include "vtkImageMapper.h"
#include "vtkCylinderSource.h"
#include "vtkBMPWriter.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkTexture.h"
#include "vtkPlaneSource.h"
#include "vtkBMPReader.h"
#include "vtkFloatArray.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkKWEvent.h"
#include "vtkSlicerVRHelper.h"
#include "vtkSlicerVRGrayscaleHelper.h"
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWCheckButton.h"
#include "vtkSlicerVolumePropertyWidget.h"

vtkVolumeRenderingModuleGUI::vtkVolumeRenderingModuleGUI(void)
{
    //In Debug Mode
    this->DebugOff();
    this->Presets=NULL;
    this->PreviousNS_ImageData="";
    this->PreviousNS_VolumeRenderingDataScene="";
    this->PreviousNS_VolumeRenderingSlicer="";
    this->PB_Testing=NULL;
    this->PB_CreateNewVolumeRenderingNode=NULL;
    this->NS_ImageData=NULL;
    this->NS_VolumeRenderingDataSlicer=NULL;
    this->NS_VolumeRenderingDataScene=NULL;
    this->EWL_CreateNewVolumeRenderingNode=NULL;

    //Frame Details
    this->DetailsFrame=NULL;

    //Other members
    this->CurrentNode=NULL;
    this->Presets=NULL;
    this->Helper=NULL;
}

vtkVolumeRenderingModuleGUI::~vtkVolumeRenderingModuleGUI(void)
{

    //Not Delete?!
    //vtkVolumeRenderingModuleLogic *Logic;
    //vtkSlicerViewerWidget *ViewerWidget;
    //vtkSlicerViewerInteractorStyle *InteractorStyle;
    //vtkMRMLVolumeRenderingNode  *CurrentNode;//really delete this

    if(this->PB_Testing)
    {
        this->PB_Testing->SetParent(NULL);
        this->PB_Testing->Delete();
        this->PB_Testing=NULL;
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


    if(this->Presets)
    {
        this->Presets->Delete();
        this->Presets=NULL;
    }

    if(this->DetailsFrame)
    {
        this->DetailsFrame->Delete();
        this->DetailsFrame=NULL;
    }
    if(this->Helper)
    {
        this->Helper->Delete();
        this->Helper=NULL;
    }
    if(this->CurrentNode)
    {
        this->CurrentNode->Delete();
        this->CurrentNode=NULL;
    }

    //Remove the MRML observer
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand);
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

    int labelWidth=20;
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    this->GetUIPanel()->AddPage("VolumeRendering","VolumeRendering",NULL);

    // Define your help text and build the help frame here.
    const char *help = "Volume Rendering allows the rendering of volumes in 3D space and not only as 2D surfaces defined in 3D space. Tutorials are available at <a>http://www.na-mic.org/Wiki/index.php/Slicer3:Volume_Rendering_Tutorials</a>";
    const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRendering" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    //
    //Load and save
    //
    vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    loadSaveDataFrame->Create();
    loadSaveDataFrame->ExpandFrame();
    loadSaveDataFrame->SetLabelText("Load and Save");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

    //Testing Pushbutton
    this->PB_Testing= vtkKWPushButton::New();
    this->PB_Testing->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_Testing->Create();
    this->PB_Testing->SetText("Make All Models Invisible");
    this->PB_Testing->SetBalloonHelpString("Make all surface models invisible. Go to models module to enable, disable only some of them.");
    this->PB_Testing->SetWidth(labelWidth);
    app->Script("pack %s -side top -anchor ne -padx 2 -pady 2",this->PB_Testing->GetWidgetName());

    //NodeSelector  for Node from MRML Scene
    this->NS_ImageData=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageData->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_ImageData->Create();
    this->NS_ImageData->NoneEnabledOn();
    this->NS_ImageData->SetLabelText("Source Volume: ");
    this->NS_ImageData->SetBalloonHelpString("Select volume to render. Only one volume at the some time is possible.");
    this->NS_ImageData->SetLabelWidth(labelWidth);
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageData->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Preset
    this->NS_VolumeRenderingDataSlicer=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataSlicer->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_VolumeRenderingDataSlicer->Create();
    this->NS_VolumeRenderingDataSlicer->SetLabelText("Existing Parametersets: ");
        this->NS_VolumeRenderingDataSlicer->SetBalloonHelpString("Select one of the existing parametersets or presets.");
    this->NS_VolumeRenderingDataSlicer->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataSlicer->EnabledOff();//By default off
    this->NS_VolumeRenderingDataSlicer->NoneEnabledOn();
    this->NS_VolumeRenderingDataSlicer->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataSlicer->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Scene
    this->NS_VolumeRenderingDataScene=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataScene->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_VolumeRenderingDataScene->Create();
    this->NS_VolumeRenderingDataScene->NoneEnabledOn();
    this->NS_VolumeRenderingDataScene->SetLabelText("Current Parametersets: ");
    this->NS_VolumeRenderingDataScene->SetBalloonHelpString("Select how the volume should be displayed. Several parameter sets per volume are possible");
    this->NS_VolumeRenderingDataScene->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataScene->EnabledOff();//By default off
    this->NS_VolumeRenderingDataScene->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataScene->GetWidgetName());
    //Missing: Load from file

    //Create New Volume Rendering Node
    //Entry With Label
    this->EWL_CreateNewVolumeRenderingNode=vtkKWEntryWithLabel::New();
    this->EWL_CreateNewVolumeRenderingNode->SetParent(loadSaveDataFrame->GetFrame());
    this->EWL_CreateNewVolumeRenderingNode->Create();
    this->EWL_CreateNewVolumeRenderingNode->SetBalloonHelpString("Specify a name for a new parameter set.");
    this->EWL_CreateNewVolumeRenderingNode->SetLabelText("Name New Parameterset: ");
    this->EWL_CreateNewVolumeRenderingNode->SetLabelWidth(labelWidth);
    this->EWL_CreateNewVolumeRenderingNode->EnabledOff();
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->EWL_CreateNewVolumeRenderingNode->GetWidgetName());


    this->PB_CreateNewVolumeRenderingNode=vtkKWPushButton::New();
    this->PB_CreateNewVolumeRenderingNode->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_CreateNewVolumeRenderingNode->Create();
    this->PB_CreateNewVolumeRenderingNode->SetBalloonHelpString("Create a new parameter set for the current volume. This way you can switch between different visualization settings for the same volume.");
    this->PB_CreateNewVolumeRenderingNode->SetText("Create Visualization Parameterset");
    app->Script("pack %s -side top -anchor ne -padx 2 -pady 2",this->PB_CreateNewVolumeRenderingNode->GetWidgetName());

    //Details frame
    this->DetailsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->DetailsFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    this->DetailsFrame->Create();
    this->DetailsFrame->ExpandFrame();
    this->DetailsFrame->SetLabelText("Details");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->DetailsFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());


    //set subnodes
    //Delete frames
    if ( this->GetApplicationGUI() &&  this->GetApplicationGUI()->GetMRMLScene())
    {
        this->GetApplicationGUI()->GetMRMLScene()->AddObserver( vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand );
    }
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
    this->PB_CreateNewVolumeRenderingNode->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
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
        //Get a new auto CurrentNode
        this->InitializePipelineNewCurrentNode();
        this->CurrentNode->HideFromEditorsOff();
        //Set the right name
        const char *name=this->EWL_CreateNewVolumeRenderingNode->GetWidget()->GetValue();
        if(!name)
        {
            vtkErrorMacro("No Text for VolumeRenderingNode");
        }
        else 
        {
            this->CurrentNode->SetName(name);
        }
        //Remove Text from Entry
        this->EWL_CreateNewVolumeRenderingNode->GetWidget()->SetValue("");
        this->NS_VolumeRenderingDataScene->UpdateMenu();
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
            //Remove the old observer for the imagedata
            if(strcmp("",this->PreviousNS_ImageData.c_str())==0)
            {
                vtkMRMLNode *oldNode=this->GetLogic()->GetMRMLScene()->GetNodeByID(this->PreviousNS_ImageData.c_str());
                if(oldNode!=NULL)
                {
                    oldNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
                }
            
            }
            
            //Unpack the details frame
            this->UnpackSvpGUI();
            //Try to keep navigation render online
            this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOn();
            this->PreviousNS_ImageData="";
        }
        //Only proceed event,if new Node
        else if(strcmp(this->NS_ImageData->GetSelected()->GetID(),this->PreviousNS_ImageData.c_str())!=0)
        {
            //Try to keep navigation Render online
            //this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOff();
            this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

            vtkMRMLScalarVolumeNode *selectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
            //Add observer to trigger update of transform
            selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
            this->UnpackSvpGUI();
            this->PackSvpGUI();

            //Initialize the Pipeline
            this->InitializePipelineFromImageData();

            //update previous:
            this->PreviousNS_ImageData=this->NS_ImageData->GetSelected()->GetID();//only when not "None"
            this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Enable();
            this->Helper->WithdrawProgressDialog();


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
            this->CurrentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataScene->GetSelected());
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

            //We have a preset, we can find id in our Presets
            if(this->Presets->GetNodeByID(this->NS_VolumeRenderingDataSlicer->GetSelected()->GetID())!=NULL)
            {
                //Copy Preset Information in current Node
                this->CurrentNode->CopyParameterset(this->NS_VolumeRenderingDataSlicer->GetSelected());
                this->Helper->UpdateGUIElements();
            }
            //It's not a preset so just update references, and select the new Added Node
            else 
            {
                this->CurrentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataSlicer->GetSelected());
                this->CurrentNode->AddReference(this->NS_ImageData->GetSelected()->GetID());
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
    if (event == vtkMRMLScene::SceneCloseEvent)
    {
        if(this->Helper!=NULL)
        {
            this->Helper->Delete();
            this->Helper=NULL;
        }
        //Reset every Node related stuff
        this->PreviousNS_ImageData="";
        this->PreviousNS_VolumeRenderingDataScene="";
        this->PreviousNS_VolumeRenderingSlicer="";
        this->CurrentNode=NULL;
        this->UpdateGUI();

    }
    if(event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
        if (this->Helper!=NULL)
        {
            this->Helper->UpdateRendering();
        }
        //TODO when can we remove the op
    }
}

void vtkVolumeRenderingModuleGUI::Enter(void)
{


    vtkDebugMacro("Enter Volume Rendering Module");
    //Load Presets


    if(!this->Presets)
    {
            vtkMRMLVolumeRenderingNode *vrNode=vtkMRMLVolumeRenderingNode::New();
        //Instance internal MRMLScene for Presets
        this->Presets=vtkMRMLScene::New();
        //Register node class
        this->Presets->RegisterNodeClass(vrNode);
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
        this->Presets->SetURL(presetFileName.c_str());
        this->Presets->Connect();
        this->NS_VolumeRenderingDataSlicer->SetAdditionalMRMLScene(this->Presets);
    }
    //End Load Presets

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
    if(this->NS_ImageData->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
        //Update the NodeSelector for Volumes
        this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
        this->NS_ImageData->UpdateMenu();
    }
    if(this->NS_VolumeRenderingDataScene->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
        //Update NodeSelector for VolumeRendering Node
        this->NS_VolumeRenderingDataScene->SetMRMLScene(this->GetLogic()->GetMRMLScene());
        this->NS_VolumeRenderingDataScene->UpdateMenu();
    }



    //Set the new condition
    if(this->NS_ImageData->GetSelected()!=NULL&&(this->NS_VolumeRenderingDataScene->GetCondition()!=this->NS_ImageData->GetSelected()->GetID()))
    {
        this->NS_VolumeRenderingDataScene->SetCondition(this->NS_ImageData->GetSelected()->GetID(),vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap(),true);
    }
            this->NS_VolumeRenderingDataScene->UpdateMenu();


    //Take care about Presets...
    //We need None for this
    if(this->NS_VolumeRenderingDataSlicer->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
        this->NS_VolumeRenderingDataSlicer->SetMRMLScene(this->GetLogic()->GetMRMLScene());
        this->NS_VolumeRenderingDataSlicer->UpdateMenu();
    }  

    if(this->NS_ImageData->GetSelected()!=NULL)
    {
        this->NS_VolumeRenderingDataSlicer->SetCondition(this->NS_ImageData->GetSelected()->GetID(),vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap(),false);
        this->NS_VolumeRenderingDataSlicer->UpdateMenu();
    }

    //Disable/Enable after Volume is selected
    if(this->NS_ImageData->GetSelected()!=NULL)
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
        this->NS_VolumeRenderingDataScene->SetSelected(NULL);
        this->NS_VolumeRenderingDataScene->EnabledOff();
        this->NS_VolumeRenderingDataSlicer->EnabledOff();
    }
    //In Presets always "None" is selected
    this->NS_VolumeRenderingDataSlicer->SetSelected(NULL);
}
void vtkVolumeRenderingModuleGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingModuleGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}


void vtkVolumeRenderingModuleGUI::InitializePipelineFromMRMLScene()
{
    
    vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();
    this->Helper->UpdateGUIElements();
    this->Helper->UpdateRendering();
}

void vtkVolumeRenderingModuleGUI::PackSvpGUI()
{
    this->Helper=vtkSlicerVRGrayscaleHelper::New();
    this->Helper->Init(this);
}
void vtkVolumeRenderingModuleGUI::UnpackSvpGUI()
{
    if(this->Helper!=NULL)
    {
        this->Helper->Delete();
        this->Helper=NULL;
    }
}

void vtkVolumeRenderingModuleGUI::InitializePipelineNewCurrentNode()
{
    //TODO move this part
    this->CurrentNode=vtkMRMLVolumeRenderingNode::New();
    this->CurrentNode->HideFromEditorsOff();
    //Add Node to Scene
    this->GetLogic()->GetMRMLScene()->AddNode(this->CurrentNode);
    this->CurrentNode->AddReference(this->NS_ImageData->GetSelected()->GetID());

    //Update the menu
    this->PreviousNS_VolumeRenderingDataScene=this->CurrentNode->GetID();
    this->NS_VolumeRenderingDataScene->SetSelected(this->CurrentNode);
    this->NS_VolumeRenderingDataScene->UpdateMenu(); 
    //this->PreviousNS_VolumeRenderingDataScene=this->Gui->GetCurrentNode()->GetID();
    //The Helper has to do something too
    this->Helper->InitializePipelineNewCurrentNode();
    this->Helper->UpdateGUIElements();
    this->Helper->UpdateRendering();


    //take care about references
}

void vtkVolumeRenderingModuleGUI::InitializePipelineFromImageData()
{


    //First check if we already have a reference
    const char *id=this->NS_ImageData->GetSelected()->GetID();
    //loop over existing Nodes in scene
    bool firstNodeFound=false;

    for( int i=0;i<this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeRenderingNode");i++)
    {
        vtkMRMLVolumeRenderingNode *tmp=vtkMRMLVolumeRenderingNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLVolumeRenderingNode"));
        if(tmp->HasReference(id)&&!firstNodeFound)
        {
            //Select first found Node
            //So everyting will be treated when InitializeFromMRMLScene
            this->PreviousNS_VolumeRenderingDataScene=tmp->GetID();
            this->CurrentNode=tmp;
            this->NS_VolumeRenderingDataScene->SetSelected(tmp);
            //We will call the initialize on our own.
            this->InitializePipelineFromMRMLScene();
            firstNodeFound=true;
        }//if
    }//for

    //If not initialize a new auto generated Volume Rendering Node
    if(!firstNodeFound)
    {
        this->InitializePipelineNewCurrentNode();
    }
    //Ensure that none is not visible
    this->NS_VolumeRenderingDataScene->NoneEnabledOff();
    this->NS_VolumeRenderingDataScene->UpdateMenu();
    //Render it
    this->PipelineInitializedOn();
    this->Helper->UpdateRendering();
}

