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
#include "vtkKWVolumeMaterialPropertyWidget.h"
#include "vtkKWProgressGauge.h"
vtkCxxRevisionMacro(vtkSlicerVRLabelmapHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRLabelmapHelper);
vtkSlicerVRLabelmapHelper::vtkSlicerVRLabelmapHelper(void)
{
    this->ButtonDown=0;
    this->ScheduledRenderID="";
    this->LM_OptionTree=NULL;
    this->MapperRaycast=NULL;
    this->MapperRaycastHighDetail=NULL;
    this->VMPW_Shading=NULL;
    this->ProgressLock=0;
    this->CurrentStage=0;
    this->OldSampleDistance=0;
}

vtkSlicerVRLabelmapHelper::~vtkSlicerVRLabelmapHelper(void)
{
    this->Gui->Script("bind all <Any-ButtonPress> {%s {}}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {%s {}}",this->GetTclName());
    //Remove all the Observers we added
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
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
    if(this->VMPW_Shading!=NULL)
    {
        this->Gui->Script("pack forget %s",this->VMPW_Shading->GetWidgetName());
        this->VMPW_Shading->SetParent(NULL);
        this->VMPW_Shading->Delete();
        this->VMPW_Shading=NULL;

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

        //vtkVolumeRayCastCompositeFunction  *compositeFunction=vtkVolumeRayCastCompositeFunction::New();
        ////compositeFunction->SetCompositeMethodToClassifyFirst();
        //this->MapperRaycastHighDetail=vtkVolumeRayCastMapper::New();
        //this->MapperRaycastHighDetail->SetVolumeRayCastFunction(compositeFunction);

        //vtkImageShiftScale *converter=vtkImageShiftScale::New();
        //converter->SetOutputScalarTypeToUnsignedChar();
        //converter->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        //this->MapperRaycastHighDetail->SetInput(converter->GetOutput());
        //this->MapperRaycastHighDetail->SetSampleDistance(0.1);
        //converter->Delete();
        //compositeFunction->Delete();
    }

    this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    //this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    // this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    //Only needed if using performance enhancement
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);

    //TODO This is not the right place for this
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->Gui->GetcurrentNode()->GetVolumeProperty()->ShadeOff();
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetInterpolationTypeToNearest();
    this->Volume->SetProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());

    //this->Volume->SetMapper(this->MapperRaycastHighDetail);
    this->Volume->SetMapper(this->MapperRaycast);
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
    this->Volume->PokeMatrix(matrix);
    this->VMPW_Shading->SetVolumeProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    //For Performance
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->AddViewProp(this->Volume);
    matrix->Delete();
    //Render
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}
void vtkSlicerVRLabelmapHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    Superclass::Init(gui);
    this->Gui->Script("bind all <Any-ButtonPress> {%s SetButtonDown 1}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {%s SetButtonDown 0}",this->GetTclName());
    this->VMPW_Shading=vtkKWVolumeMaterialPropertyWidget::New();
    this->VMPW_Shading->SetParent(this->Gui->GetdetailsFrame()->GetFrame());
    this->VMPW_Shading->Create();
    this->VMPW_Shading->SetPropertyChangingCommand(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer(),"Render");
    this->VMPW_Shading->SetPropertyChangedCommand(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer(),"Render");
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->VMPW_Shading->GetWidgetName());
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
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    matrix->Delete();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}
void vtkSlicerVRLabelmapHelper::InitializePipelineNewCurrentNode()
{
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
        this->LM_OptionTree->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),opacityNew);
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
    this->SetTCLDebug(1);
    
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
            this->MapperRaycast->ManualInteractiveOff();
        }
        else
        {
            if(strcmp(this->ScheduledRenderID.c_str(),"")!=0)
            {
                this->Script("after cancel %s", this->ScheduledRenderID.c_str());
                this->ScheduledRenderID="";
            }

            this->MapperRaycast->ManualInteractiveOn();
        }

        return;
        //if(this->Volume->GetAllocatedRenderTime()>=1.0)
        //{
        //            this->Volume->SetAllocatedRenderTime(0.01,NULL);
        //}

        //return;
        ////it is a interactiv rendering->everything is alright
        //if(this->MapperRaycast->GetSampleDistance()==this->MapperRaycast->GetInteractiveSampleDistance())
        //{
        //    vtkSlicerVRHelperDebug("interactiv started","");
        //   return;
        //}
        ////it is a scheduled renderer
        //if(this->CurrentStage==1)
        //{        
        //    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->SetStillUpdateRate(0.00001);
        //    vtkSlicerVRHelperDebug("scheduled started","");
        //    return;
        //}
        ////It is a still rendering, but we want it interactive
        ////Abort potential high resolution rendering
        //if(strcmp(this->ScheduledRenderID.c_str(),"")!=0)
        //{
        //    this->Script("after cancel %s",this->ScheduledRenderID.c_str());
        //    this->ScheduledRenderID="";
        //}

        //this->MapperRaycast->SetAutoAdjustSampleDistances(1);
        //this->OldSampleDistance=this->MapperRaycast->GetSampleDistance();
        //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetDesiredUpdateRate(0.0001);//->SetDesiredUpdateRate(20);
        //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->SetStillUpdateRate(20);
        ////this->MapperRaycast->SetSampleDistance(this->MapperRaycast->GetInteractiveSampleDistance());
        //vtkSlicerVRHelperDebug("faked interactiv started","");


    }
    else if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::EndEvent)
    {
        if(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetAbortRender()==1)
        {
            this->CurrentStage=0;
            //Take care about potential Rendering
            if(strcmp(this->ScheduledRenderID.c_str(),"")!=0)
            {
                this->Script("after cancel %s", this->ScheduledRenderID.c_str());
                this->ScheduledRenderID="";
            }
            //Start a new low resolution renderer
            this->ScheduledRenderID=this->Script("after 100 %s ScheduleRender 0",this->GetTclName());
        }
        if(this->CurrentStage==0)
        {
            this->ScheduledRenderID=this->Script("after 100 %s ScheduleRender 1",this->GetTclName());
        }
        else
        {
            this->CurrentStage=0;
        }
        //return;
        // //if we have an interactive sample distance and we don't pretend to be interactiv
        //if(this->MapperRaycast->GetSampleDistance()==this->MapperRaycast->GetInteractiveSampleDistance()&&this->OldSampleDistance==0)
        //{
        //    vtkSlicerVRHelperDebug("interactiv ended","");
        //    return;
        //}
        ////The next Rendering has to be interactiv
        //if(this->CurrentStage==1)
        //{
        //    vtkSlicerVRHelperDebug("scheduled ended","");
        //    this->CurrentStage=0;
        //    return;

        //}
        //vtkSlicerVRHelperDebug("faked interactiv ended","");
        ////We pretend to be interactiv
        ////this->MapperRaycast->SetSampleDistance(this->OldSampleDistance);
        ////this->MapperRaycast->SetAutoAdjustSampleDistances(1);

        //this->OldSampleDistance=0;

    }
    vtkSlicerFixedPointVolumeRayCastMapper *callerMapper=vtkSlicerFixedPointVolumeRayCastMapper::SafeDownCast(caller);
    if (callerMapper==this->MapperRaycast&&eid==vtkCommand::ProgressEvent)
    {

        if(this->MapperRaycast->GetSampleDistance()==this->MapperRaycast->GetInteractiveSampleDistance())
        {
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(0);
            this->Script("puts \"Interactive no progress \"");
            return;
        }
        float *progress=(float*)callData;
        if(*progress==0)
        {
            return;
        }
        int progressInt =*progress*100;
        this->Script("puts \"progress %d\"",progressInt);
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100**progress);
        //Debug stuff
        const char *result=this->Script("after info");
        this->Script("puts \"[after info]\"");
        this->Script("puts \"[after info [lindex [after info] 0]]\"");
        this->Script("puts \"[after info]\"");
        return;
    }
    vtkSlicerLabelMapWidget *callerLabelmapWidget=vtkSlicerLabelMapWidget::SafeDownCast(caller);
    if(callerLabelmapWidget==this->LM_OptionTree&&eid==vtkSlicerLabelMapWidget::NeedForRenderEvent)
    {

        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
    }

}

void vtkSlicerVRLabelmapHelper::CheckAbort(void)
{
    int pending=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    if(pending!=0)
    {
        this->Gui->Script("puts \"got an abort\"");

        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        return;
    }
    int pendingGUI=vtkKWTkUtilities::CheckForPendingInteractionEvents(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow());
    if(pendingGUI!=0)
    {
        this->Gui->Script("puts \"got an abort from gui\"");
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
    this->LM_OptionTree->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),vtkLabelMapPiecewiseFunction::SafeDownCast(this->Gui->GetcurrentNode()->GetVolumeProperty()->GetScalarOpacity()));
}



