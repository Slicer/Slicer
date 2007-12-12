//Slicer
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerVRGrayscaleHelper.h"
#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkSlicerApplication.h"

//VTK
#include "vtkObjectFactory.h"
#include "vtkVolume.h"
#include "vtkTimerLog.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkTexture.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkInteractorStyle.h"

//KWWidgets
#include "vtkKWHistogram.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWProgressGauge.h" 
#include "vtkKWEvent.h"
#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWProgressDialog.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWPiecewiseFunctionEditor.h"

//Compiler
#include <math.h>



vtkCxxRevisionMacro(vtkSlicerVRGrayscaleHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRGrayscaleHelper);
vtkSlicerVRGrayscaleHelper::vtkSlicerVRGrayscaleHelper(void)
{

    //Debug
    this->DebugOff();
    this->SetTCLDebug(0);
    this->Histograms=NULL;

    this->ScheduleMask[0]=1;
    this->ScheduleMask[1]=1;
    this->ScheduleMask[2]=1;
    //this->MB_Quality=NULL;
    this->renViewport=NULL;
    this->renPlane=NULL;
    this->MapperTexture=NULL;
    this->MapperRaycast=NULL;
    this->timer=vtkTimerLog::New();
    this->RenderPlane=0;
    this->currentStage=0;
    this->scheduled=0;
    this->EventHandlerID="";
    this->InitialDropLowRes=0.2;
    this->FactorLastLowRes=0;
    this->LastTimeLowRes=0;
    this->LastTimeHighRes=0;
    this->GoalLowResTime=0.05;
    //.6 seems to be best
    this->PercentageNoChange=0.6;
    this->TimeToWaitForHigherStage=0.1;
    this->NextRenderHighResolution=0;
    this->IgnoreStepZero=0;
    this->Quality=0;
    this->StageZeroEventHandlerID="";
    this->ButtonDown=0;

    //GUI:
    this->CB_TextureLow=NULL;
    this->CB_TextureHigh=NULL;
    this->CB_RayCast=NULL;
    this->CB_InteractiveFrameRate=NULL;
    this->SC_Framerate=NULL;
    this->SVP_VolumeProperty=NULL;
    this->MappersFrame=NULL;
    this->SavedStillRate=0;

    //Cropping:
    this->CB_Cropping=NULL;
    for(int i=0;i<3;i++)
    {
        this->RA_Cropping[i]=NULL;
    }

    //ThresholdGUI
    this->MB_ThresholdMode=NULL;
    this->MB_ColorMode=NULL;
    this->RA_RampRectangleHorizontal=NULL;
    this->RA_RampRectangleVertical=NULL;
    this->ColorMode=0;
    this->ThresholdMode=0;
    this->PB_Reset=NULL;
    this->PB_ThresholdZoomIn=NULL;


}

vtkSlicerVRGrayscaleHelper::~vtkSlicerVRGrayscaleHelper(void)
{
    //Remove Bindings
    this->Gui->Script("bind all <Any-ButtonPress> {}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {}",this->GetTclName());
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,0);
    //cancel potential rendering
    if(strcmp(this->EventHandlerID.c_str(),"")!=0)
    {
        this->Script("after cancel %s", this->EventHandlerID.c_str());
        this->EventHandlerID="";
    }
    if(strcmp(this->StageZeroEventHandlerID.c_str(),"")!=0)
    {
        this->Script("after cancel %s", this->StageZeroEventHandlerID.c_str());
        this->StageZeroEventHandlerID="";
    }
    //Remove Obersvers
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    //Only needed if using performance enhancement
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    vtkKWRenderWidget *renderWidget=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
    //Remove Observers


    //Remove Volume


    if(this->SVP_VolumeProperty!=NULL)
    {
        this->Gui->Script("pack forget %s",this->SVP_VolumeProperty->GetWidgetName());
        this->SVP_VolumeProperty->SetHistogramSet(NULL);
        this->SVP_VolumeProperty->SetVolumeProperty(NULL);
        this->SVP_VolumeProperty->SetDataSet(NULL);
        this->SVP_VolumeProperty->SetParent(NULL);
        this->SVP_VolumeProperty->RemoveObservers(vtkKWEvent::VolumePropertyChangingEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->SVP_VolumeProperty->Delete();
        this->SVP_VolumeProperty=NULL;
    }
    if(this->Histograms!=NULL)
    {
        this->Histograms->RemoveAllHistograms();
        this->Histograms->Delete();
        this->Histograms=NULL;
    }
    //Don't delete the renViewport(allocated in Outside)

    if(this->MapperTexture!=NULL)
    {
        this->MapperTexture->Delete();
        this->MapperTexture=NULL;
    }
    if(this->MapperRaycast!=NULL)
    {
        this->MapperRaycast->Delete();
        this->MapperRaycast=NULL;
    }

    //Ensure that Render is back to normal
    vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();
    //get the viewport renderer up
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renPlane);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renViewport);
    //this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("");
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,0);
    //Change viewport(simulation of "sample distance for "rays"" if using texture mapping)
    this->renViewport->SetViewport(0,0,1,1);
    renWin->SwapBuffersOn();

    if(this->renPlane!=NULL)
    {
        this->renPlane->Delete();
        this->renPlane=NULL;
    }
    if(this->timer!=NULL)
    {
        this->timer->StopTimer();
        this->timer->Delete();
        this->timer=NULL;
    }
    //Save State of MappersFrame
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","CB_RayCast","%d",this->CB_RayCast->GetWidget()->GetSelectedState());
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","CB_TextureLow","%d",this->CB_TextureLow->GetWidget()->GetSelectedState());
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","CB_TextureHigh","%d",this->CB_TextureHigh->GetWidget()->GetSelectedState());
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","CB_InteractiveFrameRate","%d",this->CB_InteractiveFrameRate->GetWidget()->GetSelectedState());
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","SC_FrameRate","%e",this->SC_Framerate->GetWidget()->GetValue());
    if(this->MappersFrame!=NULL)
    {
        this->Gui->Script("pack forget %s",this->MappersFrame->GetWidgetName());
        this->MappersFrame->SetParent(NULL);
        this->MappersFrame->Delete();
        this->MappersFrame=NULL;
    }
    if(this->CB_RayCast!=NULL)
    {
        this->CB_RayCast->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_RayCast->SetParent(NULL);
        this->CB_RayCast->Delete();
        this->CB_RayCast=NULL;
    }
    if(this->CB_TextureLow!=NULL)
    {
        this->CB_TextureLow->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_TextureLow->SetParent(NULL);
        this->CB_TextureLow->Delete();
        this->CB_TextureLow=NULL;
    }
    if(this->CB_TextureHigh!=NULL)
    {
        this->CB_TextureHigh->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_TextureHigh->SetParent(NULL);
        this->CB_TextureHigh->Delete();
        this->CB_TextureHigh=NULL;
    }
    if(this->SC_Framerate!=NULL)
    {
        this->SC_Framerate->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->SC_Framerate->SetParent(NULL);
        this->SC_Framerate->Delete();
        this->SC_Framerate=NULL;
    }
    if(this->CB_InteractiveFrameRate!=NULL)
    {
        this->CB_InteractiveFrameRate->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->CB_InteractiveFrameRate->SetParent(NULL);
        this->CB_InteractiveFrameRate->Delete();
        this->CB_InteractiveFrameRate=NULL;

    }
    if(this->CB_Cropping)
    {
        this->CB_Cropping->SetParent(NULL);
        this->CB_Cropping->Delete();
        this->CB_Cropping=NULL;
    }
    for(int i=0;i<3;i++)
    {
        if(this->RA_Cropping[i])
        {
            this->RA_Cropping[i]->SetParent(NULL);
            this->RA_Cropping[i]->Delete();
            this->RA_Cropping[i]=NULL;
        }
    }
    if(this->NB_Details)
    {
        this->Script("pack forget %s",this->NB_Details->GetWidgetName());
        this->NB_Details->SetParent(NULL);
        this->NB_Details->Delete();
        this->NB_Details=NULL;
    }

    //ThresholdGUI
    if(this->MB_ThresholdMode)
    {
        this->MB_ThresholdMode->SetParent(NULL);
        this->MB_ThresholdMode->Delete();
        this->MB_ThresholdMode=NULL;

    }
    if(this->MB_ColorMode)
    {
        this->MB_ColorMode->SetParent(NULL);
        this->MB_ColorMode->Delete();
        this->MB_ColorMode=NULL;
    }
    if(this->RA_RampRectangleHorizontal)
    {
        this->RA_RampRectangleHorizontal->SetParent(NULL);
        this->RA_RampRectangleHorizontal->Delete();
        this->RA_RampRectangleHorizontal=NULL;
    }
    if(this->RA_RampRectangleVertical)
    {
        this->RA_RampRectangleVertical->SetParent(NULL);
        this->RA_RampRectangleVertical->Delete();
        this->RA_RampRectangleVertical=NULL;
    }
    if(this->PB_Reset)
    {
        this->PB_Reset->SetParent(NULL);
        this->PB_Reset->Delete();
        this->PB_Reset=NULL;
    }
    if(this->PB_ThresholdZoomIn)
    {
        this->PB_ThresholdZoomIn->SetParent(NULL);
        this->PB_ThresholdZoomIn->Delete();
        this->PB_ThresholdZoomIn=NULL;
    }
}
void vtkSlicerVRGrayscaleHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    if(this->SVP_VolumeProperty!=NULL)
    {
        vtkErrorMacro("Init already called!");
        this->UpdateGUIElements();
        return;
    }
    //Start dialog right here
    Superclass::Init(gui);
    this->Gui->Script("bind all <Any-ButtonPress> {%s SetButtonDown 1}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {%s SetButtonDown 0}",this->GetTclName());

    //Create a notebook
    this->NB_Details=vtkKWNotebook::New();
    this->NB_Details->SetParent(this->Gui->GetdetailsFrame()->GetFrame());
    this->NB_Details->Create();
    this->NB_Details->AddPage("Mapping");
    this->NB_Details->AddPage("Performance");
    this->NB_Details->AddPage("Cropping");
    this->Script("pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2", 
        this->NB_Details->GetWidgetName());

    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(this->NB_Details->GetFrame("Mapping"));
    this->SVP_VolumeProperty->Create();
    this->SVP_VolumeProperty->SetDataSet(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    this->Histograms=vtkKWHistogramSet::New();

    //Add Histogram for image data
    this->Histograms->AddHistograms(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetPointData()->GetScalars());
    //Build the gradient histogram
    vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
    grad->SetDimensionality(3);
    grad->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    grad->Update();
    vtkKWHistogram *gradHisto=vtkKWHistogram::New();
    gradHisto->BuildHistogram(grad->GetOutput()->GetPointData()->GetScalars(),0);
    this->Histograms->AddHistogram(gradHisto,"0gradient");

    //Delete      
    this->SVP_VolumeProperty->SetHistogramSet(this->Histograms);

    //AddEvent for Interactive apply 
    this->SVP_VolumeProperty->AddObserver(vtkKWEvent::VolumePropertyChangingEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    grad->Delete();
    gradHisto->Delete(); 
    int labelWidth=15;
    this->MappersFrame=vtkKWFrameWithLabel::New();
    this->MappersFrame->SetParent(this->NB_Details->GetFrame("Performance"));
    this->MappersFrame->Create();
    this->MappersFrame->AllowFrameToCollapseOff();
    this->MappersFrame->SetLabelText("Quality / Performance");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        MappersFrame->GetWidgetName() );

    this->CB_TextureLow=vtkKWCheckButtonWithLabel::New();
    this->CB_TextureLow->SetParent(this->MappersFrame->GetFrame());
    this->CB_TextureLow->Create();
    this->CB_TextureLow->SetLabelText("Use Texture Low");
    this->CB_TextureLow->SetLabelWidth(labelWidth);
    this->CB_TextureLow->GetWidget()->SetSelectedState(1);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->CB_TextureLow->GetWidgetName() );
    this->CB_TextureLow->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);


    this->CB_TextureHigh=vtkKWCheckButtonWithLabel::New();
    this->CB_TextureHigh->SetParent(this->MappersFrame->GetFrame());
    this->CB_TextureHigh->Create();
    this->CB_TextureHigh->SetLabelText("Use Texture High");
    this->CB_TextureHigh->SetLabelWidth(labelWidth);
    this->CB_TextureHigh->GetWidget()->SetSelectedState(1);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->CB_TextureHigh->GetWidgetName() );
    this->CB_TextureHigh->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);

    this->CB_RayCast=vtkKWCheckButtonWithLabel::New();
    this->CB_RayCast->SetParent(this->MappersFrame->GetFrame());
    this->CB_RayCast->Create();
    this->CB_RayCast->SetLabelText("Use Raycast      ");
    this->CB_RayCast->SetLabelWidth(labelWidth);
    this->CB_RayCast->GetWidget()->SetSelectedState(1);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->CB_RayCast->GetWidgetName() );
    this->CB_RayCast->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);

    this->CB_InteractiveFrameRate=vtkKWCheckButtonWithLabel::New();
    this->CB_InteractiveFrameRate->SetParent(this->MappersFrame->GetFrame());
    this->CB_InteractiveFrameRate->Create();
    this->CB_InteractiveFrameRate->SetLabelText("Raycast interactive?!");
    this->CB_InteractiveFrameRate->SetLabelWidth(labelWidth);
    this->CB_InteractiveFrameRate->EnabledOff();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->CB_InteractiveFrameRate->GetWidgetName() );
    this->CB_InteractiveFrameRate->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);



    //Framerate
    this->SC_Framerate=vtkKWScaleWithLabel::New();
    this->SC_Framerate->SetParent(this->MappersFrame->GetFrame());
    this->SC_Framerate->Create();
    this->SC_Framerate->SetLabelText("FPS (Interactive):");
    this->SC_Framerate->SetLabelWidth(labelWidth);
    this->SC_Framerate->GetWidget()->SetRange(1,20);
    this->SC_Framerate->GetWidget()->SetResolution(1);
    this->SC_Framerate->GetWidget()->SetValue(1./this->GoalLowResTime);
    this->SC_Framerate->SetBalloonHelpString("set frames per sec for lowest resolution rendering");
    this->SC_Framerate->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->SC_Framerate->GetWidgetName() );

    this->CreateThreshold();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());

    this->CreateCropping();

}
void vtkSlicerVRGrayscaleHelper::InitializePipelineNewCurrentNode()
{
    std::stringstream autoname;
    autoname<<"autoVisualization";
    autoname<<this->Gui->GetNS_ImageData()->GetSelected()->GetName();
    this->Gui->GetcurrentNode()->SetName(autoname.str().c_str());
    this->Gui->GetLogic()->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent);
    vtkKWHistogram *histogram=this->Histograms->GetHistogramWithName("0");
    if(histogram==NULL)
    {
        vtkErrorMacro("Problems with HistogramSet");
        return;
    }
    double totalOccurance=histogram->GetTotalOccurence();
    double thresholdLow=totalOccurance*0.2;
    double thresholdHigh=totalOccurance*0.8;
    double range[2];

    histogram->GetRange(range);
    double thresholdLowIndex=range[0];
    double sumLowIndex=0;
    double thresholdHighIndex=range[0];
    double sumHighIndex=0;
    //calculate distance
    double bin_width = (range[1] == range[0] ? 1 : (range[1] - range[0])/(double)histogram->GetNumberOfBins());
    while (sumLowIndex<thresholdLow)
    {
        sumLowIndex+=histogram->GetOccurenceAtValue(thresholdLowIndex);
        thresholdLowIndex+=bin_width;
    }
    while(sumHighIndex<thresholdHigh)
    {
        sumHighIndex+=histogram->GetOccurenceAtValue(thresholdHighIndex);
        thresholdHighIndex+=bin_width;

    }
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetInterpolationTypeToLinear();
    vtkPiecewiseFunction *opacity=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    opacity->RemoveAllPoints();
    opacity->AddPoint(range[0],0.);
    opacity->AddPoint(thresholdLowIndex,0.0);
    opacity->AddPoint(thresholdHighIndex,0.2);
    opacity->AddPoint(range[1],0.2);
    vtkColorTransferFunction *colorTransfer=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
    colorTransfer->RemoveAllPoints();
    colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
    colorTransfer->AddRGBPoint(thresholdLowIndex,.3,.3,1.);
    colorTransfer->AddRGBPoint(thresholdLowIndex+.5*(thresholdHighIndex-thresholdLowIndex),.3,1.,.3);
    colorTransfer->AddRGBPoint(thresholdHighIndex,1.,.3,.3);
    colorTransfer->AddRGBPoint(range[1],1,.3,.3);


    //Enable shading as default

    this->Gui->GetcurrentNode()->GetVolumeProperty()->ShadeOn();
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetAmbient(.30);
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetDiffuse(.60);
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetSpecular(.50);
    this->Gui->GetcurrentNode()->GetVolumeProperty()->SetSpecularPower(40);//this is really weird

    //Disable Gradient Opacity

    this->UpdateSVP();
}

void vtkSlicerVRGrayscaleHelper::Rendering(void)
{
    if(this->Volume!=NULL)
    {
        vtkErrorMacro("Rendering already called, use update Rendering instead");
        return;
    }
    if(this->Gui==NULL)
    {
        vtkErrorMacro("Call init before calling rendering");
        return;
    }

    this->Volume=vtkVolume::New();
    //TODO Dirty fix as Mapper
    if(this->Gui->GetcurrentNode()->GetMapper()==vtkMRMLVolumeRenderingNode::Texture)
    {
        this->MapperTexture=vtkSlicerVolumeTextureMapper3D::New();
        this->MapperTexture->SetSampleDistance(2);
        this->MapperTexture->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        this->Volume->SetMapper(this->MapperTexture);
        //Also take care about Ray Cast
        this->MapperRaycast=vtkSlicerFixedPointVolumeRayCastMapper::New();
        this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        this->MapperRaycast->SetAutoAdjustSampleDistances(0);
        this->MapperRaycast->SetSampleDistance(0.1);
    }

    //Try to load from the registry; do it here to ensure all objects are there
    if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","CB_RayCast"))
    {
        this->CB_RayCast->GetWidget()->SetSelectedState(this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","CB_RayCast"));
    }
    if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","CB_TextureLow"))
    {
        this->CB_TextureLow->GetWidget()->SetSelectedState(this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","CB_TextureLow"));
    }
    if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","CB_TextureHigh"))
    {
        this->CB_TextureHigh->GetWidget()->SetSelectedState(this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","CB_TextureHigh"));
    }
    if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","SC_FrameRate"))
    {
        this->SC_Framerate->GetWidget()->SetValue(this->Gui->GetApplication()->GetFloatRegistryValue(2,"VolumeRendering","SC_FrameRate"));
    }
    if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","CB_InteractiveFrameRate"))
    {
        this->CB_InteractiveFrameRate->GetWidget()->SetSelectedState(this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","CB_InteractiveFrameRate"));
        this->GoalLowResTime=1./this->SC_Framerate->GetWidget()->GetValue();
        this->MapperRaycast->SetManualInteractiveRate(this->GoalLowResTime);
    }


    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperRenderProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    //Only needed if using performance enhancement
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);

    //TODO This is not the right place for this

    this->Volume->SetProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    //For Performance
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->AddViewProp(this->Volume);
    this->renViewport=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetNthRenderer(0);
    matrix->Delete();
    //Render
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
}

void vtkSlicerVRGrayscaleHelper::UpdateRendering()
{
    //first check if REndering was already called
    if(this->Volume==NULL)
    {
        this->Rendering();
        return;
    }
    //Update mapper
    this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    this->MapperTexture->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    //Update Property
    this->Volume->SetProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    //Update matrix
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    matrix->Delete();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}
void vtkSlicerVRGrayscaleHelper::ShutdownPipeline(void)
{

    if(this->MapperRaycast!=NULL)
    {
        this->MapperRaycast->Delete();
        this->MapperRaycast=NULL;
    }
    if(this->MapperTexture!=NULL)
    {
        this->MapperTexture->Delete();
        this->MapperTexture=NULL;
    }
}

void vtkSlicerVRGrayscaleHelper::ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData)
{

    //Check the checkbuttons
    vtkKWCheckButton *callerObjectCheckButton=vtkKWCheckButton::SafeDownCast(caller);
    if(callerObjectCheckButton==this->CB_TextureLow->GetWidget()&&eid==vtkKWCheckButton::SelectedStateChangedEvent)
    {
        this->ScheduleMask[0]=callerObjectCheckButton->GetSelectedState();
        this->UpdateQualityCheckBoxes();

        return;
    }
    if(callerObjectCheckButton==this->CB_TextureHigh->GetWidget()&&eid==vtkKWCheckButton::SelectedStateChangedEvent)
    {
        this->ScheduleMask[1]=callerObjectCheckButton->GetSelectedState();
        this->UpdateQualityCheckBoxes();
        return;
    }   
    if(callerObjectCheckButton==this->CB_RayCast->GetWidget()&&eid==vtkKWCheckButton::SelectedStateChangedEvent)
    {
        this->ScheduleMask[2]=callerObjectCheckButton->GetSelectedState();
        this->UpdateQualityCheckBoxes();
        return;
    }
    if(callerObjectCheckButton==this->CB_InteractiveFrameRate->GetWidget()&&eid==vtkKWCheckButton::SelectedStateChangedEvent)
    {

        this->MapperRaycast->SetAutoAdjustSampleDistances(callerObjectCheckButton->GetSelectedState());
        if(!callerObjectCheckButton->GetSelectedState())
        {
            this->MapperRaycast->SetSampleDistance(.1);
            this->MapperRaycast->SetImageSampleDistance(1);
        }

        return;
    }
    vtkKWScale *callerObjectSC=vtkKWScale::SafeDownCast(caller);
    if(callerObjectSC==this->SC_Framerate->GetWidget()&&eid==vtkKWScale::ScaleValueChangedEvent)
    {
        this->GoalLowResTime=1./callerObjectSC->GetValue();
        this->MapperRaycast->SetManualInteractiveRate(this->GoalLowResTime);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
        return;
    }
    vtkSlicerVolumePropertyWidget *callerObjectSVP=vtkSlicerVolumePropertyWidget::SafeDownCast(caller);
    if(callerObjectSVP==this->SVP_VolumeProperty&&eid==vtkKWEvent::VolumePropertyChangingEvent)
    {
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
        return;
    }

    //TODO not the right place for this
    vtkRenderWindow *callerRen=vtkRenderWindow::SafeDownCast(caller);
    if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::AbortCheckEvent)
    {
        this->CheckAbort();
        return;
    }
    else if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::StartEvent)
    {

        //First check if we have to abort the ZeroStageRender
        if(strcmp(this->StageZeroEventHandlerID.c_str(),"")!=0)
        {
            const char* result=this->Gui->Script("after cancel %s",this->StageZeroEventHandlerID.c_str());
            vtkSlicerVRHelperDebug("Result cancel startEvent %s",result);
            this->StageZeroEventHandlerID="";
        }
        //It's the first time we render
        if(this->FactorLastLowRes==0)
        {
            this->FactorLastLowRes=this->InitialDropLowRes;

        }
        vtkSlicerVRHelperDebug("startevent scheduled %d",this->scheduled);
        vtkSlicerVRHelperDebug("startevent currentstage %d",this->currentStage);
        vtkSlicerVRHelperDebug("startevent id %s",this->EventHandlerID.c_str());

        //it is not a scheduled event so we use stage the quality stage and abort every existings scheduling
        if(this->scheduled==0||this->ButtonDown==1)
        {
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using LowestResolution");
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);


            //Check if we have an Event Scheduled, if this is the case abort it
            if(strcmp(this->EventHandlerID.c_str(),"")!=0)
            {
                const char* result=this->Gui->Script("after cancel %s",this->EventHandlerID.c_str());
                const char* resulta=this->Gui->Script("after info");
                vtkSlicerVRHelperDebug("ResultCancel: %s",result);
                vtkSlicerVRHelperDebug("Result info: %s",resulta);
                this->EventHandlerID="";
            }
            this->currentStage=this->Quality;
        }//if
        if(this->currentStage==0)
        {

            this->Volume->SetMapper(this->MapperTexture);
            //always got to less sample distance
            this->MapperTexture->SetSampleDistance(2);
            this->currentStage=0;
            vtkSlicerVRHelperDebug("Stage 0 started","");
            //Decide if we REnder plane or not
            if(this->RenderPlane==1)
            {
              return;
            }
            //Time to adjust our Factor
            if(this->LastTimeLowRes<(1-this->PercentageNoChange)*this->GoalLowResTime)
            {
                this->FactorLastLowRes=sqrt((this->FactorLastLowRes*this->FactorLastLowRes)+0.2);

            }
            else if(this->LastTimeLowRes>(1+this->PercentageNoChange)*this->GoalLowResTime)
            {
                double tmp=(this->FactorLastLowRes*this->FactorLastLowRes-0.2);
                if(tmp>0)
                {
                    this->FactorLastLowRes=sqrt(tmp);
                }

            }
            if(this->FactorLastLowRes<this->InitialDropLowRes)
            {
                this->FactorLastLowRes=this->InitialDropLowRes;
            }
            if(this->FactorLastLowRes>1)
            {
                this->FactorLastLowRes=1;
                // Maxbe later
                //this->IgnoreStepZero;
            }
            vtkSlicerVRHelperDebug("NEWFACTORLASTLOWRES %f",this->FactorLastLowRes);
            timer->StartTimer();
            vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();

            //We don't like to see, what we are doing here
            renWin->SwapBuffersOff();
            //get the viewport renderer up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renViewport);
            //Change viewport(simulation of "sample distance for "rays"" if using texture mapping)
            this->renViewport->SetViewport(0,0,this->FactorLastLowRes,this->FactorLastLowRes);
            return;


        }
        //Stage 1
        if(this->currentStage==1)
        {
            this->Volume->SetMapper(MapperTexture);
            this->MapperTexture->SetSampleDistance(.1);

            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using Middle Resolution");
            vtkSlicerVRHelperDebug("Stage 1 started","");
            //Remove plane Renderer and get viewport Renderer Up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renViewport);
            //Go to Fullsize
            this->renViewport->SetViewport(0,0,1,1);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SwapBuffersOn();
            this->scheduled=0;
            return;
        }

        //Stage 2
        if(this->currentStage==2)
        {
            vtkRenderWindowInteractor *interactor=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetInteractor();
            if(this->ButtonDown&&this->CB_InteractiveFrameRate->GetWidget()->GetSelectedState())
            {

                this->MapperRaycast->SetManualInteractive(1);
            }
            //Do the following code just in case we skipped stage 1
            //Remove plane Renderer and get viewport Renderer Up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renViewport);
            //Go to Fullsize
            this->renViewport->SetViewport(0,0,1,1);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SwapBuffersOn();
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using Highest Resolution");
            vtkSlicerVRHelperDebug("Stage 2 started","");
            this->Volume->SetMapper(this->MapperRaycast);

            this->EventHandlerID="";
            this->scheduled=0;
            return;
        }   
    }
    else if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::EndEvent)
    {
        //The Rendering has been aborted
        if(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetAbortRender()==1)
        {
            if(strcmp(this->StageZeroEventHandlerID.c_str(),"")!=0)
            {
                const char* result=this->Gui->Script("after cancel %s",this->StageZeroEventHandlerID.c_str());
                vtkSlicerVRHelperDebug("Result cancel endEvent %s",result);
                this->StageZeroEventHandlerID="";
            }
            this->StageZeroEventHandlerID=this->Gui->Script("after 500 %s ScheduleStageZero", this->GetTclName());
            return;
        }
        vtkSlicerVRHelperDebug("endevent scheduled %d",this->scheduled);
        vtkSlicerVRHelperDebug("endevent currentstage %d",this->currentStage);
        if(this->currentStage==1)
        {
            this->EventHandlerID=this->Gui->Script("after 100 %s ScheduleRender",this->GetTclName());
            vtkSlicerVRHelperDebug("Stage 1 ended","");
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            return;
        }
        if(this->currentStage==2)
        {
            vtkSlicerVRHelperDebug("Stage 2 ended","");
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,100);
            if(this->MapperRaycast->GetManualInteractive())
            {
                this->MapperRaycast->SetManualInteractive(0);
                this->scheduled=1;
                this->EventHandlerID=this->Gui->Script("after idle [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow] Render");
            }
            //We reached the highest Resolution, no scheduling

            return;
        }
        if(this->RenderPlane==1)
        {
            this->RenderPlane=0;
            this->timer->StopTimer();
            this->LastTimeLowRes=this->timer->GetElapsedTime();
            vtkSlicerVRHelperDebug("Last time low %f",this->LastTimeLowRes);
            //It's time to start for the Scheduled Rendering
            this->EventHandlerID=this->Gui->Script("after 100 %s ScheduleRender",this->GetTclName());
            vtkSlicerVRHelperDebug("Stage 0 ended","");
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Enable();
            return;
        }


        vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();
        //Get current size of window
        int *size=renWin->GetSize();

        //RGB Data from the smaller viewport
        vtkUnsignedCharArray *image=vtkUnsignedCharArray::New();
        renWin->GetRGBACharPixelData(0,0,(int)(this->FactorLastLowRes*size[0]),(int)(this->FactorLastLowRes*size[1]),0,image);

        vtkImageData *imageData=vtkImageData::New();
        imageData->GetPointData()->SetScalars(image);
        imageData->SetDimensions((int)(this->FactorLastLowRes*size[0])+1,(int)(this->FactorLastLowRes*size[1])+1,1);
        imageData->SetNumberOfScalarComponents(4);
        imageData->SetScalarTypeToUnsignedChar();
        imageData->SetOrigin(.0,.0,.0);
        imageData->SetSpacing(1.,1.,1.);
        //imageData->Update();
        vtkImageExtractComponents *components=vtkImageExtractComponents::New();
        components->SetInput(imageData);
        components->SetComponents(0,1,2);
        if(this->renPlane==NULL)
        {
            this->renPlane=vtkRenderer::New();
        }

        this->renPlane->SetBackground(this->renViewport->GetBackground());
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

        this->renPlane->SetActiveCamera(this->renViewport->GetActiveCamera());

        //Get Our Renderer Up
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renViewport);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renPlane);

        this->renPlane->SetDisplayPoint(0,0,0.5);
        this->renPlane->DisplayToWorld();
        double coordinatesA[4];
        this->renPlane->GetWorldPoint(coordinatesA);

        this->renPlane->SetDisplayPoint(size[0],0,0.5);
        this->renPlane->DisplayToWorld();
        double coordinatesB[4];
        this->renPlane->GetWorldPoint(coordinatesB);

        this->renPlane->SetDisplayPoint(size[0],size[1],0.5);
        this->renPlane->DisplayToWorld();
        double coordinatesC[4];
        this->renPlane->GetWorldPoint(coordinatesC);

        this->renPlane->SetDisplayPoint(0,size[1],0.5);
        this->renPlane->DisplayToWorld();
        double coordinatesD[4];
        this->renPlane->GetWorldPoint(coordinatesD);

        //Create the Polydata
        vtkPoints *points=vtkPoints::New();
        points->InsertPoint(0,coordinatesA);
        points->InsertPoint(1,coordinatesB);
        points->InsertPoint(2,coordinatesC);
        points->InsertPoint(3,coordinatesD);

        vtkCellArray *polygon=vtkCellArray::New();
        polygon->InsertNextCell(4);
        polygon->InsertCellPoint(0);
        polygon->InsertCellPoint(1);
        polygon->InsertCellPoint(2);
        polygon->InsertCellPoint(3);
        //Take care about Texture coordinates
        vtkFloatArray *textCoords=vtkFloatArray::New();
        textCoords->SetNumberOfComponents(2);
        textCoords->Allocate(8);
        float tc[2];
        tc[0]=0;
        tc[1]=0;
        textCoords->InsertNextTuple(tc);
        tc[0]=1;
        tc[1]=0;
        textCoords->InsertNextTuple(tc);
        tc[0]=1;
        tc[1]=1;
        textCoords->InsertNextTuple(tc);
        tc[0]=0;
        tc[1]=1;
        textCoords->InsertNextTuple(tc);

        vtkPolyData *polydata=vtkPolyData::New();
        polydata->SetPoints(points);
        polydata->SetPolys(polygon);
        polydata->GetPointData()->SetTCoords(textCoords);

        vtkPolyDataMapper *polyMapper=vtkPolyDataMapper::New();
        polyMapper->SetInput(polydata);

        vtkActor *actor=vtkActor::New(); 
        actor->SetMapper(polyMapper);

        //Take care about the texture
        vtkTexture *atext=vtkTexture::New();
        atext->SetInput(components->GetOutput());
        atext->SetInterpolate(1);
        actor->SetTexture(atext);

        //Remove all old Actors
        this->renPlane->RemoveAllViewProps();

        this->renPlane->AddActor(actor);
        //Remove the old Renderer

        renWin->SwapBuffersOn();
        //Do we do that
        this->RenderPlane=1;



        //Delete everything we have done
        image->Delete();
        imageData->Delete();
        components->Delete();
        points->Delete();
        polygon->Delete();
        textCoords->Delete();
        polydata->Delete();
        polyMapper->Delete();
        actor->Delete();
        atext->Delete();
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
        return;

    }
    //Check if caller equals mapper
    vtkAbstractMapper *callerMapper=vtkAbstractMapper::SafeDownCast(caller);
    if((this->Volume!=0)&&(callerMapper!=this->Volume->GetMapper()))
    {
        return;
    }
    if(eid==vtkCommand::VolumeMapperComputeGradientsStartEvent)
    {
        this->DisplayProgressDialog("Please standby: Gradients are calculated");
        return;
    }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsEndEvent)
    {
        this->WithdrawProgressDialog();
        return;

    }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsProgressEvent)
    {
        float *progress=(float*)callData;
        if(this->GradientDialog!=NULL)
        {
        this->GradientDialog->UpdateProgress(*progress);
        }
        return;
    }
    else if (eid==vtkCommand::VolumeMapperRenderProgressEvent&&this->currentStage==1)
    {
        double *progress=(double*)callData;
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100**progress);
        return;
    }
    else if (eid==vtkCommand::ProgressEvent)
    {
        float *progress=(float*)callData;
        if(*progress==0)
        {
            return;
        }
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,100**progress);
        return;
    }
    vtkDebugMacro("observed event but didn't process it");
}

void vtkSlicerVRGrayscaleHelper::ScheduleRender(void)
{
    this->scheduled=1;
    this->currentStage++;

    //Security if we ran out of range or an event occured in between
    if(this->currentStage>2||vtkKWTkUtilities::CheckForPendingInteractionEvents(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow())!=0)
    {
        this->currentStage=0;
        this->scheduled=0;
        //return;
    }
    else 
    {
        while(!ScheduleMask[this->currentStage])
        {

            this->currentStage++;
            //Nothing to schedule
            if(this->currentStage==3)
            {
                this->currentStage=0;
                this->scheduled=0;
                return;
            }
        }
    }
    this->EventHandlerID=this->Gui->Script("after idle [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow] Render");
    //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::UpdateSVP(void)
{
    //First check if we have a SVP
    if(this->SVP_VolumeProperty==NULL)
    {
        vtkErrorMacro("SVP does not exist");
        return;
    }
    //First of all set New Property, Otherwise all Histograms will be overwritten
    //First check if we really need to update
    if(this->SVP_VolumeProperty->GetVolumeProperty()==this->Gui->GetcurrentNode()->GetVolumeProperty())
    {
        this->AdjustMapping();
        this->SVP_VolumeProperty->Update();
        //Set Treshold to none
        this->MB_ThresholdMode->GetWidget()->GetMenu()->SelectItem("None");
        //Reset cropping
        this->CB_Cropping->GetWidget()->SetSelectedState(0);
        this->ProcessThresholdModeEvents(0);
    }
    this->SVP_VolumeProperty->SetVolumeProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    this->SVP_VolumeProperty->SetHSVColorSelectorVisibility(1);
    this->SVP_VolumeProperty->Update();
}

void vtkSlicerVRGrayscaleHelper::UpdateGUIElements(void)
{
    Superclass::UpdateGUIElements();
    this->UpdateSVP();
}
void vtkSlicerVRGrayscaleHelper::CheckAbort(void)
{
    int pending=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    if(pending!=0)
    {
        vtkSlicerVRHelperDebug("got an abort","");
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        this->scheduled=0;
        return;
    }
    int pendingGUI=vtkKWTkUtilities::CheckForPendingInteractionEvents(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow());
    if(pendingGUI!=0)
    {
        vtkSlicerVRHelperDebug("got an abort from gui","");
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        this->scheduled=0;
        return;

    }
}
void vtkSlicerVRGrayscaleHelper::AdjustMapping(){

    //Update Color    
    vtkColorTransferFunction *functionColor=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
    double rangeNew[2];
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetPointData()->GetScalars()->GetRange(rangeNew);
    functionColor->AdjustRange(rangeNew);

    //Update Opacity
    vtkPiecewiseFunction *function=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    function->AdjustRange(rangeNew);
    //Update
    rangeNew[1]=(rangeNew[1]-rangeNew[0])/4;
    rangeNew[0]=0;
    function=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetGradientOpacity();
    function->RemovePoint(255);//Remove the standard value
    //this->Histograms->GetHistogramWithName("0gradient")->GetRange(rangeNew);
    function->AdjustRange(rangeNew);
}

void vtkSlicerVRGrayscaleHelper::ScheduleStageZero()
{
    this->StageZeroEventHandlerID=this->Gui->Script("after idle [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow] Render");

}

void vtkSlicerVRGrayscaleHelper::UpdateQualityCheckBoxes(void)
{
    //Update the quality
    int i=0;
    this->Quality=0;
    while(!this->ScheduleMask[i]&&i<2)
    {
        i++;
        this->Quality=i;
    }

    //Check if we have to enable or  disable FPS
    //this->SC_Framerate->SetEnabled(this->ScheduleMask[0]);

    //Check if we have to enable the last Checkbutton
    int count=0;
    int indexMayBeDisabled=-1;
    for(int i=0;i<3;i++)
    {
        if(this->ScheduleMask[i])
        {
            count++;
            indexMayBeDisabled=i;
        }
    }
    if(count>1)
    {
        this->CB_RayCast->EnabledOn();
        this->CB_TextureHigh->EnabledOn();
        this->CB_TextureLow->EnabledOn();
    }
    else if(indexMayBeDisabled==0)
    {
        this->CB_TextureLow->EnabledOff();
    }
    else if(indexMayBeDisabled==1)
    {
        this->CB_TextureHigh->EnabledOff();
    }
    else if(indexMayBeDisabled==2)
    {
        this->CB_RayCast->EnabledOff();
    }
    if(!this->ScheduleMask[0]&&!this->ScheduleMask[1])
    {
        this->CB_InteractiveFrameRate->EnabledOn();
    }
    else
    {
        this->CB_InteractiveFrameRate->GetWidget()->SetSelectedState(0);
        this->CB_InteractiveFrameRate->EnabledOff();
    }
    //Check if we can activate the interactive checkbox

}

void vtkSlicerVRGrayscaleHelper::Cropping(int index, double min,double max)
{
    if(this->MapperTexture==NULL||this->MapperRaycast==NULL)
    {
        return;
    }

    //double *oldCropping=this->MapperTexture->GetCroppingRegionPlanes();
    //oldCropping[2*index]=min;
    //oldCropping[2*index+1]=max;
    double oldCropping[6];
    for(int i=0;i<3;i++)
    {
        oldCropping[2*i]=this->RA_Cropping[i]->GetRange()[0];
        oldCropping[2*i+1]=this->RA_Cropping[i]->GetRange()[1];
    }
    this->MapperRaycast->SetCroppingRegionPlanes(oldCropping);
    this->MapperTexture->SetCroppingRegionPlanes(oldCropping);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::CreateCropping()
{
    vtkKWFrameWithLabel *croppingFrame=vtkKWFrameWithLabel::New();
    croppingFrame->SetParent(this->NB_Details->GetFrame("Cropping"));
    croppingFrame->Create();
    croppingFrame->AllowFrameToCollapseOff();
    croppingFrame->SetLabelText("Cropping (IJK coordinates)");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        croppingFrame->GetWidgetName() );

    //Build GUI

    this->CB_Cropping=vtkKWCheckButtonWithLabel::New();
    this->CB_Cropping->SetParent(croppingFrame->GetFrame());
    this->CB_Cropping->Create();
    this->CB_Cropping->GetWidget()->SetSelectedState(0);
    this->CB_Cropping->SetLabelText("Enable /Disable Cropping");
    this->CB_Cropping->GetWidget()->SetCommand(this, "ProcessEnableDisableCropping");
    this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10",
        this->CB_Cropping->GetWidgetName());
    for(int i=0;i<3;i++)
    {
        this->RA_Cropping[i]=vtkKWRange::New();
        this->RA_Cropping[i]->SetParent(croppingFrame->GetFrame());
        this->RA_Cropping[i]->Create();
        this->RA_Cropping[i]->SymmetricalInteractionOff();
        std::stringstream str;
        str<<"Cropping ";
        str<<i;
        this->RA_Cropping[i]->SetCommand(this,str.str().c_str());
        this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10",this->RA_Cropping[i]->GetWidgetName());
    }
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    this->RA_Cropping[0]->SetLabelText("I");
    this->RA_Cropping[0]->SetWholeRange(iData->GetOrigin()[0],iData->GetDimensions()[0]);
    this->RA_Cropping[0]->SetRange(iData->GetOrigin()[0],iData->GetDimensions()[0]);
    this->RA_Cropping[1]->SetLabelText("J");
    this->RA_Cropping[1]->SetWholeRange(iData->GetOrigin()[1],iData->GetDimensions()[1]);
    this->RA_Cropping[1]->SetRange(iData->GetOrigin()[1],iData->GetDimensions()[1]);
    this->RA_Cropping[2]->SetLabelText("K");
    this->RA_Cropping[2]->SetWholeRange(iData->GetOrigin()[2],iData->GetDimensions()[2]);
    this->RA_Cropping[2]->SetRange(iData->GetOrigin()[2],iData->GetDimensions()[2]);

    //Now we have the cropping ranges
    this->ProcessEnableDisableCropping(0);
}

void vtkSlicerVRGrayscaleHelper::CreateThreshold()
{
    vtkKWFrameWithLabel *thresholdFrame=vtkKWFrameWithLabel::New();
    thresholdFrame->SetParent(this->NB_Details->GetFrame("Mapping"));
    thresholdFrame->Create();
    thresholdFrame->SetLabelText("Threshold");
    this->Script("pack %s -side top -anchor nw -fill both -expand yes -padx 0 -pady 2", 
        thresholdFrame->GetWidgetName());

    this->MB_ThresholdMode=vtkKWMenuButtonWithLabel::New();
    this->MB_ThresholdMode->SetParent(thresholdFrame->GetFrame());
    this->MB_ThresholdMode->Create();
    this->MB_ThresholdMode->SetLabelText("Threshold:");
    this->MB_ThresholdMode->SetLabelWidth(10);
    this->MB_ThresholdMode->GetWidget()->GetMenu()->AddRadioButton("None");
    this->MB_ThresholdMode->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessThresholdModeEvents 0");
    this->MB_ThresholdMode->GetWidget()->GetMenu()->AddRadioButton("Ramp");
    this->MB_ThresholdMode->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessThresholdModeEvents 1");
    this->MB_ThresholdMode->GetWidget()->GetMenu()->AddRadioButton("Rectangle");
    this->MB_ThresholdMode->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessThresholdModeEvents 2");
    this->MB_ThresholdMode->GetWidget()->GetMenu()->SelectItem("None");
    this->Script("pack %s -side top -anchor nw -fill both -expand y -padx 2 -pady 2", 
        this->MB_ThresholdMode->GetWidgetName());

    this->MB_ColorMode=vtkKWMenuButtonWithLabel::New();
    this->MB_ColorMode->SetParent(thresholdFrame->GetFrame());
    this->MB_ColorMode->Create();
    this->MB_ColorMode->SetLabelText("Color Mode:"); 
    this->MB_ColorMode->SetLabelWidth(10);
    this->MB_ColorMode->GetWidget()->GetMenu()->AddRadioButton("Grayscale dynamic");
    this->MB_ColorMode->GetWidget()->GetMenu()->SetItemCommand(0,this,"ProcessColorModeEvents 0");
    this->MB_ColorMode->GetWidget()->GetMenu()->AddRadioButton("Grayscale static");
    this->MB_ColorMode->GetWidget()->GetMenu()->SetItemCommand(1,this,"ProcessColorModeEvents 1");
    this->MB_ColorMode->GetWidget()->GetMenu()->AddRadioButton("Rainbow");
    this->MB_ColorMode->GetWidget()->GetMenu()->SetItemCommand(2,this,"ProcessColorModeEvents 2");
    this->MB_ColorMode->GetWidget()->GetMenu()->SelectItem("Grayscale static");
    this->ProcessColorModeEvents(1);
    this->MB_ColorMode->EnabledOff();
    this->Script("pack %s -side top -anchor nw -fill both -expand y -padx 2 -pady 2", 
        this->MB_ColorMode->GetWidgetName());

    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    this->RA_RampRectangleHorizontal=vtkKWRange::New();
    this->RA_RampRectangleHorizontal->SetParent(thresholdFrame->GetFrame());
    this->RA_RampRectangleHorizontal->Create();
    this->RA_RampRectangleHorizontal->SetLabelText("Threshold");
    this->RA_RampRectangleHorizontal->SetWholeRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->RA_RampRectangleHorizontal->SetRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->RA_RampRectangleHorizontal->SetCommand(this, "ProcessThresholdRange");
    this->RA_RampRectangleHorizontal->EnabledOff();
    this->Script("pack %s -side left -anchor nw -expand no -fill x -padx 2 -pady 2", 
        this->RA_RampRectangleHorizontal->GetWidgetName());

    this->PB_ThresholdZoomIn=vtkKWPushButton::New();
    this->PB_ThresholdZoomIn->SetParent(thresholdFrame->GetFrame());
    this->PB_ThresholdZoomIn->Create();
    this->PB_ThresholdZoomIn->SetText("Zoom In");
    this->PB_ThresholdZoomIn->EnabledOff();
    this->PB_ThresholdZoomIn->SetCommand(this,"ProcessThresholdZoomIn");
    this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
        this->PB_ThresholdZoomIn->GetWidgetName());

    this->PB_Reset=vtkKWPushButton::New();
    this->PB_Reset->SetParent(thresholdFrame->GetFrame());
    this->PB_Reset->Create();
    this->PB_Reset->SetText("Reset");
    this->PB_Reset->EnabledOff();
    this->PB_Reset->SetCommand(this,"ProcessThresholdReset");
    this->Script("pack %s -side top -anchor nw -fill x -expand n -padx 2 -pady 2",
        this->PB_Reset->GetWidgetName());

    this->RA_RampRectangleVertical=vtkKWRange::New();
    this->RA_RampRectangleVertical->SetParent(thresholdFrame->GetFrame());
    this->RA_RampRectangleVertical->Create();
    this->RA_RampRectangleVertical->SetLabelText("Opacity");
    this->RA_RampRectangleVertical->SetOrientationToVertical();
    this->RA_RampRectangleVertical->SetWholeRange(1,0);
    this->RA_RampRectangleVertical->SetRange(1,0);
    this->RA_RampRectangleVertical->EnabledOff();
    this->RA_RampRectangleVertical->SetCommand(this, "ProcessThresholdRange");
    this->Script("pack %s -side left -anchor w -expand n -padx 2 -pady 2", 
        this->RA_RampRectangleVertical->GetWidgetName());


    thresholdFrame->Delete();
}

void vtkSlicerVRGrayscaleHelper::ProcessThresholdModeEvents(int id)
{

    this->ThresholdMode=id;
    //Disable Everything and go back to standard
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    if(id==0)
    {
        this->MB_ColorMode->EnabledOff();

        this->RA_RampRectangleHorizontal->SetRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
        this->RA_RampRectangleHorizontal->EnabledOff();
        this->RA_RampRectangleVertical->SetRange(1,0);
        this->RA_RampRectangleVertical->EnabledOff();
        this->PB_Reset->EnabledOff();
        this->PB_ThresholdZoomIn->EnabledOff();
        return;
    }
    //Before we continue enabled everything
    this->MB_ColorMode->EnabledOn();
    this->RA_RampRectangleHorizontal->EnabledOn();
    this->RA_RampRectangleVertical->EnabledOn();
    this->PB_Reset->EnabledOn();
    this->PB_ThresholdZoomIn->EnabledOn();
    this->ProcessThresholdRange(.0,.0);







}

void vtkSlicerVRGrayscaleHelper::ProcessColorModeEvents(int id)
{
    this->ColorMode=id;
    this->ProcessThresholdRange(0.,0.);
}

void vtkSlicerVRGrayscaleHelper::ProcessThresholdRange(double notUsed,double notUsedA)
{
    if(this->ThresholdMode==0)
    {
        return;
    }
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    //Delete all old Mapping Points
    vtkPiecewiseFunction *opacity=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    opacity->RemoveAllPoints();
    //opacity->AdjustRange(iData->GetScalarRange());

    vtkColorTransferFunction *colorTransfer=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
    colorTransfer->RemoveAllPoints();
    //colorTransfer->AdjustRange(iData->GetScalarRange());

    opacity->AddPoint(iData->GetScalarRange()[0],this->RA_RampRectangleVertical->GetRange()[1]);
    opacity->AddPoint(iData->GetScalarRange()[1],this->RA_RampRectangleVertical->GetRange()[1]);
    if(this->ThresholdMode==1)
    {
        opacity->AddPoint(iData->GetScalarRange()[0],this->RA_RampRectangleVertical->GetRange()[1]);
        opacity->AddPoint(iData->GetScalarRange()[1],this->RA_RampRectangleVertical->GetRange()[0]);
        opacity->AddPoint(this->RA_RampRectangleHorizontal->GetRange()[0],this->RA_RampRectangleVertical->GetRange()[1]);
        opacity->AddPoint(this->RA_RampRectangleHorizontal->GetRange()[1],this->RA_RampRectangleVertical->GetRange()[0]);

    }
    else if(this->ThresholdMode==2)
    {

        opacity->AddPoint(iData->GetScalarRange()[0],this->RA_RampRectangleVertical->GetRange()[1]);
        opacity->AddPoint(iData->GetScalarRange()[1],this->RA_RampRectangleVertical->GetRange()[1]);

        opacity->AddPoint(this->RA_RampRectangleHorizontal->GetRange()[0],this->RA_RampRectangleVertical->GetRange()[1]);
        opacity->AddPoint(this->RA_RampRectangleHorizontal->GetRange()[0]+0.1,this->RA_RampRectangleVertical->GetRange()[0]);
        opacity->AddPoint(this->RA_RampRectangleHorizontal->GetRange()[1]-0.1,this->RA_RampRectangleVertical->GetRange()[0]);
        opacity->AddPoint(this->RA_RampRectangleHorizontal->GetRange()[1],this->RA_RampRectangleVertical->GetRange()[1]);

    }

    if(!this->ColorMode)
    {
        colorTransfer->AddRGBPoint(iData->GetScalarRange()[0],.5,.5,.5);
        colorTransfer->AddRGBPoint(iData->GetScalarRange()[1],1,1,1);

        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[0],.5,.5,.5);
        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[1],1,1,1);
    }
    else if(this->ColorMode==1)
    {
        colorTransfer->AddRGBPoint(iData->GetScalarRange()[0],.5,.5,.5);
        colorTransfer->AddRGBPoint(iData->GetScalarRange()[1],.5,.5,.5);

        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[0],.5,.5,.5);
        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[1],.5,.5,.5);

    }
    else
    {
        colorTransfer->AddRGBPoint(iData->GetScalarRange()[0],.3,.3,1.);
        colorTransfer->AddRGBPoint(iData->GetScalarRange()[1],1.,.3,.3);

        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[0],.3,.3,1.);
        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[0]+.5*(this->RA_RampRectangleHorizontal->GetRange()[1]-this->RA_RampRectangleHorizontal->GetRange()[0]),.3,1.,.3);
        colorTransfer->AddRGBPoint(this->RA_RampRectangleHorizontal->GetRange()[1],1.,.3,.3);
    }

    this->SVP_VolumeProperty->Update();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
}
void vtkSlicerVRGrayscaleHelper::ProcessThresholdZoomIn(void)
{
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    double leftBorder=this->RA_RampRectangleHorizontal->GetRange()[0];
    double rightBorder=this->RA_RampRectangleHorizontal->GetRange()[1];
    double expansion=.1*(rightBorder-leftBorder);
    double leftBorderWholeRange=leftBorder-expansion;
    double rightBorderWholeRange=rightBorder+expansion;

    if(leftBorderWholeRange<iData->GetScalarRange()[0])
    {
        leftBorderWholeRange=iData->GetScalarRange()[0];
    }
    if(rightBorderWholeRange>iData->GetScalarRange()[1])
    {
        rightBorderWholeRange=iData->GetScalarRange()[1];
    }
    this->RA_RampRectangleHorizontal->SetWholeRange(leftBorderWholeRange,rightBorderWholeRange);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetVisibleParameterRange(leftBorderWholeRange,rightBorderWholeRange);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetVisibleParameterRange(leftBorderWholeRange,rightBorderWholeRange);

}
void vtkSlicerVRGrayscaleHelper::ProcessThresholdReset(void)
{
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();

    this->RA_RampRectangleHorizontal->SetWholeRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetVisibleParameterRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetVisibleParameterRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
}


void vtkSlicerVRGrayscaleHelper::ProcessEnableDisableCropping(int cbSelectedState)
{
    if(this->MapperTexture!=NULL&&this->MapperRaycast!=NULL)
    {
        this->MapperTexture->SetCropping(cbSelectedState);
        this->MapperRaycast->SetCropping(cbSelectedState);
        this->MapperTexture->SetCroppingRegionFlagsToSubVolume();
        this->MapperRaycast->SetCroppingRegionFlagsToSubVolume();
    }
    //Reset range
    if(!cbSelectedState)
    {
            vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
            this->RA_Cropping[0]->SetRange(iData->GetOrigin()[0],iData->GetDimensions()[0]);
            this->RA_Cropping[1]->SetRange(iData->GetOrigin()[1],iData->GetDimensions()[1]);
            this->RA_Cropping[2]->SetRange(iData->GetOrigin()[2],iData->GetDimensions()[2]);
    }
    for(int i=0;i<3;i++)
    {
        this->RA_Cropping[i]->SetEnabled(cbSelectedState);
    }
    this->Cropping(0,0,0);
}
