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
    this->LM_OptionTree=NULL;
    this->MapperRaycast=NULL;
    this->MapperRaycastHighDetail=NULL;
    this->VMPW_Shading=NULL;
}

vtkSlicerVRLabelmapHelper::~vtkSlicerVRLabelmapHelper(void)
{
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
        this->MapperRaycast->SetSampleDistance(.1);

        vtkVolumeRayCastCompositeFunction  *compositeFunction=vtkVolumeRayCastCompositeFunction::New();
        //compositeFunction->SetCompositeMethodToClassifyFirst();
        this->MapperRaycastHighDetail=vtkVolumeRayCastMapper::New();
        this->MapperRaycastHighDetail->SetVolumeRayCastFunction(compositeFunction);

        vtkImageShiftScale *converter=vtkImageShiftScale::New();
        converter->SetOutputScalarTypeToUnsignedChar();
        converter->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        this->MapperRaycastHighDetail->SetInput(converter->GetOutput());
        this->MapperRaycastHighDetail->SetSampleDistance(0.1);
        converter->Delete();
        compositeFunction->Delete();
    }

    //this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    //this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
   // this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    //Only needed if using performance enhancement
    //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);

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
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
}
void vtkSlicerVRLabelmapHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    Superclass::Init(gui);
    this->VMPW_Shading=vtkKWVolumeMaterialPropertyWidget::New();
    this->VMPW_Shading->SetParent(this->Gui->GetdetailsFrame()->GetFrame());
    this->VMPW_Shading->Create();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->VMPW_Shading->GetWidgetName());
    this->LM_OptionTree=vtkSlicerLabelMapWidget::New();
    this->LM_OptionTree->SetParent(this->Gui->GetdetailsFrame()->GetFrame());
    this->LM_OptionTree->Create();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->LM_OptionTree->GetWidgetName());

}
void vtkSlicerVRLabelmapHelper::UpdateRendering(void)
{
    //first check if REndering was already called
    if(this->Volume==NULL)
    {
        this->Rendering();
        return;
    }
    this->MapperRaycastHighDetail->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
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
    //TODO not the right place for this
    vtkRenderWindow *callerRen=vtkRenderWindow::SafeDownCast(caller);
    if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::AbortCheckEvent)
    {
        this->CheckAbort();
        return;
    }
    else if (eid==vtkCommand::ProgressEvent)
    {
        float *progress=(float*)callData;
        if(*progress==0)
        {
            return;
        }
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100**progress);
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
void vtkSlicerVRLabelmapHelper::UpdateGUIElements(void)
{
    this->LM_OptionTree->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),vtkLabelMapPiecewiseFunction::SafeDownCast(this->Gui->GetcurrentNode()->GetVolumeProperty()->GetScalarOpacity()));
}



