#include "vtkSlicerVRLabelmapHelper.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkLabelMapPiecewiseFunction.h"
#include "vtkLabelMapColorTransferFunction.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkVolume.h"
#include "vtkSlicerApplication.h"
#include "vtkVolumeRayCastCompositeFunction.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkImageShiftScale.h"

#include "vtkKWProgressGauge.h"
#include "vtkKWProgressDialog.h"
vtkCxxRevisionMacro(vtkSlicerVRLabelmapHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRLabelmapHelper);
vtkSlicerVRLabelmapHelper::vtkSlicerVRLabelmapHelper(void)
{
    this->ButtonDown=0;
    this->ScheduledRenderID="";
    this->LM_OptionTree=NULL;
    this->MapperRaycast=NULL;
    this->MapperRaycastHighDetail=NULL;
    this->ProgressLock=0;
    this->CurrentStage=0;
    this->OldSampleDistance=0;
}

vtkSlicerVRLabelmapHelper::~vtkSlicerVRLabelmapHelper(void)
{
    this->Gui->Script("bind all <Any-ButtonPress> {}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {}",this->GetTclName());
    //Stop potential Rendering
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
    if(strcmp(this->ScheduledRenderID.c_str(),"")!=0)
    {
        this->Script("after cancel %s", this->ScheduledRenderID.c_str());
        this->ScheduledRenderID="";
    }

    //Set ProgressBar to 0
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(0);

    //Remove all the Observers we added
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::ProgressEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->LM_OptionTree->GetNode()->RemoveObservers(vtkCommand::ModifiedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);
   
    //Delete everything
    if(this->LM_OptionTree)
    {
        this->Gui->Script("pack forget %s",this->LM_OptionTree->GetWidgetName());
        this->LM_OptionTree->SetParent(NULL);
        this->LM_OptionTree->Delete();
        this->LM_OptionTree=NULL;
    }

    if(this->MapperRaycast!=NULL)
    {
        this->MapperRaycast->Delete();
        this->MapperRaycast=NULL;
    }
    if(this->MapperRaycastHighDetail!=NULL)
    {
        this->MapperRaycastHighDetail->Delete();
        this->MapperRaycastHighDetail=NULL;
    }

}
void vtkSlicerVRLabelmapHelper::Rendering(void)
{
    if(this->Volume!=NULL)
    {
        vtkErrorMacro("Rendering already called, use update Rendering instead");
        return;
    }

    this->Volume=vtkVolume::New();
    //TODO Dirty fix as Mapper
    if(this->Gui->GetcurrentNode()->GetMapper()==vtkMRMLVolumeRenderingNode::Texture)
    {
        this->MapperRaycast=vtkSlicerFixedPointVolumeRayCastMapper::New();
        this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        this->MapperRaycast->SetBlendModeToComposite();
        this->MapperRaycast->SetAutoAdjustSampleDistances(1);
        this->MapperRaycast->SetSampleDistance(.1);
    }
    //Test for colornode
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetVolumeDisplayNode()->AddObserver(vtkCommand::ModifiedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);   
    this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);

    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    //Only needed if using performance enhancement
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);

    //TODO This is not the right place for this
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->Gui->GetcurrentNode()->GetVolumeProperty()->ShadeOff();
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetInterpolationTypeToNearest();
    this->Volume->SetProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    this->Volume->SetMapper(this->MapperRaycast);
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->AddViewProp(this->Volume);
    matrix->Delete();
    //Render
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
    this->UpdateGUIElements();
}
void vtkSlicerVRLabelmapHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    Superclass::Init(gui);
    this->Gui->Script("bind all <Any-ButtonPress> {%s SetButtonDown 1}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {%s SetButtonDown 0}",this->GetTclName());

    this->LM_OptionTree=vtkSlicerLabelMapWidget::New();
    this->LM_OptionTree->SetParent(this->Gui->GetdetailsFrame()->GetFrame());
    this->LM_OptionTree->Create();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->LM_OptionTree->GetWidgetName());
    this->LM_OptionTree->AddObserver(vtkSlicerLabelMapWidget::NeedForRenderEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

}
void vtkSlicerVRLabelmapHelper::UpdateRendering(void)
{
    //first check if REndering was already called
    if(this->Volume==NULL)
    {
        this->Rendering();
        return;
    }
    // this->MapperRaycastHighDetail->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    //Update Property
    this->Volume->SetProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    //Update matrix
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    matrix->Delete();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}
void vtkSlicerVRLabelmapHelper::InitializePipelineNewCurrentNode()
{
    std::stringstream autoname;
    autoname<<"autoVisualization";
    autoname<<this->Gui->GetNS_ImageData()->GetSelected()->GetName();
    this->Gui->GetcurrentNode()->SetName(autoname.str().c_str());
     this->Gui->GetLogic()->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent);
    //Labelmap
    if(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetLabelMap()==1)
    {
        vtkLabelMapPiecewiseFunction *opacityNew=vtkLabelMapPiecewiseFunction::New();
        opacityNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),.1,0);
        
        this->Gui->GetcurrentNode()->GetVolumeProperty()->SetScalarOpacity(opacityNew);
        this->Gui->GetcurrentNode()->SetIsLabelMap(1);
        vtkLabelMapColorTransferFunction *colorNew=vtkLabelMapColorTransferFunction::New();
        colorNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()));
        this->Gui->GetcurrentNode()->GetVolumeProperty()->SetColor(colorNew);

        this->RecentColorNodeID=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetVolumeDisplayNode()->GetColorNode()->GetID();
        //Check if we already made an init only update (performance)
        if(this->LM_OptionTree->GetNode()==vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()))
        {
            this->LM_OptionTree->UpdateVolumeRenderingNode(this->Gui->GetcurrentNode());
            this->LM_OptionTree->UpdateGuiElements();
        }
        this->LM_OptionTree->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),this->Gui->GetcurrentNode());
        this->UpdateLM();

        //Delete
        opacityNew->Delete();
        colorNew->Delete();

    }
}
void vtkSlicerVRLabelmapHelper::UpdateLM()
{
    if(this->LM_OptionTree==NULL)
    {
        vtkErrorMacro("LM Option Tree does not exist");
        return;
    }
}

void vtkSlicerVRLabelmapHelper::ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData)
{
    
    //TODO not the right place for this
    vtkRenderWindow *callerRen=vtkRenderWindow::SafeDownCast(caller);
    if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::AbortCheckEvent)
    {
        this->CheckAbort();
        return;
    }
    else if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::StartEvent)
    {            
        
        if(this->CurrentStage==1&&this->ButtonDown==0)
        {
            vtkSlicerVRHelperDebug("StartHIghRes","");
            this->MapperRaycast->ManualInteractiveOff();
        }
        else
        {
            vtkSlicerVRHelperDebug("StartLowRes","");
            if(strcmp(this->ScheduledRenderID.c_str(),"")!=0)
            {
                this->Script("after cancel %s", this->ScheduledRenderID.c_str());
                this->ScheduledRenderID="";
            }

            this->MapperRaycast->ManualInteractiveOn();
        }

        return;
    }
    else if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::EndEvent)
    {
        if(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetAbortRender()==1)
        {
            vtkSlicerVRHelperDebug("scheduleafterabort","");
            this->CurrentStage=0;
            //Take care about potential Rendering
            if(strcmp(this->ScheduledRenderID.c_str(),"")!=0)
            {
                this->Script("after cancel %s", this->ScheduledRenderID.c_str());
                this->ScheduledRenderID="";
            }
            //Start a new low resolution renderer
            this->ScheduledRenderID=this->Script("after 100 %s ScheduleRender 0",this->GetTclName());
            return;
        }
        if(this->CurrentStage==0)
        {
            vtkSlicerVRHelperDebug("scheduleHighresolution","");
            this->ScheduledRenderID=this->Script("after 100 %s ScheduleRender 1",this->GetTclName());
        }
        else
        {
            this->CurrentStage=0;
        }
        return;
    }
    vtkSlicerFixedPointVolumeRayCastMapper *callerMapper=vtkSlicerFixedPointVolumeRayCastMapper::SafeDownCast(caller);
    if (callerMapper==this->MapperRaycast&&eid==vtkCommand::ProgressEvent)
    {

        if(this->MapperRaycast->GetSampleDistance()==this->MapperRaycast->GetInteractiveSampleDistance())
        {
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(0);
            return;
        }
        float *progress=(float*)callData;
        if(*progress==0)
        {
            return;
        }
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100**progress);
        return;
    }
    if(callerMapper==this->MapperRaycast&&eid==vtkCommand::VolumeMapperComputeGradientsStartEvent)
    {
        this->GradientDialog = vtkKWProgressDialog::New();
        this->GradientDialog->SetParent (  this->Gui->GetApplicationGUI()->GetMainSlicerWindow());
        this->GradientDialog->SetDisplayPositionToMasterWindowCenter();
        this->GradientDialog->Create ( );
        this->GradientDialog->SetMessageText("Standby: Gradients are calculated");
        this->GradientDialog->Display();
        return;
    }
    if(callerMapper==this->MapperRaycast&&eid==vtkCommand::VolumeMapperComputeGradientsEndEvent)
    {
        this->GradientDialog->Withdraw();
        this->GradientDialog->SetParent(NULL);
        this->GradientDialog->Delete();
        this->GradientDialog=NULL;
        return;
       
    }
    if(eid==vtkCommand::VolumeMapperComputeGradientsProgressEvent)
    {
        float *progress=(float*)callData;
        this->GradientDialog->UpdateProgress(*progress);
        return;
    }
    vtkSlicerLabelMapWidget *callerLabelmapWidget=vtkSlicerLabelMapWidget::SafeDownCast(caller);
    if(callerLabelmapWidget==this->LM_OptionTree&&eid==vtkSlicerLabelMapWidget::NeedForRenderEvent)
    {

        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
        return;
    }
    vtkMRMLVolumeDisplayNode *callerVolumeDisplay=vtkMRMLVolumeDisplayNode::SafeDownCast(caller);
    if(callerVolumeDisplay==vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetVolumeDisplayNode()&&eid==vtkCommand::ModifiedEvent)
    {
        this->UpdateGUIElements();
        return;
    }
    vtkDebugMacro("GotEventButDontUseIT");

}

void vtkSlicerVRLabelmapHelper::CheckAbort(void)
{
    int pending=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    if(pending!=0)
    {
        vtkSlicerVRHelperDebug("got an abort","");

        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        return;
    }
    int pendingGUI=vtkKWTkUtilities::CheckForPendingInteractionEvents(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow());
    if(pendingGUI!=0)
    {
        vtkSlicerVRHelperDebug("got an abort from GUI","");
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        return;
    }
}

void vtkSlicerVRLabelmapHelper::ScheduleRender(int stage)
{
    this->CurrentStage=stage;
    if(stage==1)
    {
    this->ScheduledRenderID=this->Gui->Script("after idle [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] Render");
    }
    else
    {
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
    }
}
void vtkSlicerVRLabelmapHelper::UpdateGUIElements(void)
{

    //Check if the colorTable has changed:+
     if(strcmp(this->RecentColorNodeID.c_str(),vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetVolumeDisplayNode()->GetColorNode()->GetID())!=0)
     {
        vtkLabelMapColorTransferFunction *colorNew=vtkLabelMapColorTransferFunction::New();
        colorNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()));
        this->Gui->GetcurrentNode()->GetVolumeProperty()->SetColor(colorNew);
        colorNew->Delete();
        this->RecentColorNodeID=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetVolumeDisplayNode()->GetColorNode()->GetID();
     }
  
    //We have to init
    if(vtkLabelMapColorTransferFunction::SafeDownCast(this->Gui->GetcurrentNode()->GetVolumeProperty()->GetRGBTransferFunction())==NULL)
    {
         vtkLabelMapColorTransferFunction *colorNew=vtkLabelMapColorTransferFunction::New();
        colorNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()));
        this->Gui->GetcurrentNode()->GetVolumeProperty()->SetColor(colorNew);
        colorNew->Delete();

    }
    //We have to init
    if(this->LM_OptionTree->GetVolumeRenderingNode()==NULL)
    {
        this->LM_OptionTree->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),this->Gui->GetcurrentNode());      
    }
    else
    {
    this->LM_OptionTree->UpdateVolumeRenderingNode(this->Gui->GetcurrentNode());
    this->LM_OptionTree->UpdateGuiElements();
    }
    
}



