//Slicer
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerVRGrayscaleHelper.h"
#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColorDisplayWidget.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerBoxWidget.h"
#include "vtkSlicerVisibilityIcons.h"

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
#include "vtkRenderWindowInteractor.h"
#include "vtkPlanes.h"
#include "vtkProperty.h"

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
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWPushButtonWithLabel.h"
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
    this->RenViewport=NULL;
    this->RenPlane=NULL;
    this->MapperTexture=NULL;
    this->MapperRaycast=NULL;
    this->Timer=vtkTimerLog::New();
    this->RenderPlane=0;
    this->CurrentStage=0;
    this->Scheduled=0;
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
    this->CurrentTransformNodeCropping=NULL;

    //ThresholdGUI
    this->MB_ThresholdMode=NULL;
    this->MB_ColorMode=NULL;
    this->RA_RampRectangleHorizontal=NULL;
    this->RA_RampRectangleVertical=NULL;
    this->ColorMode=0;
    this->ThresholdMode=0;
    this->PB_Reset=NULL;
    this->PB_ThresholdZoomIn=NULL;

    //Clipping
    this->BW_Clipping=NULL;
    this->AdditionalClippingTransform=NULL;
    this->InverseAdditionalClippingTransform=NULL;

    //Set Standard Clipping Colors
    ColorsClippingHandles[0][0]=1;
    ColorsClippingHandles[0][1]=0;
    ColorsClippingHandles[0][2]=1;

    ColorsClippingHandles[1][0]=1;
    ColorsClippingHandles[1][1]=0;
    ColorsClippingHandles[1][2]=0;

    ColorsClippingHandles[2][0]=1;
    ColorsClippingHandles[2][1]=1;
    ColorsClippingHandles[2][2]=1;

    ColorsClippingHandles[3][0]=0;
    ColorsClippingHandles[3][1]=1;
    ColorsClippingHandles[3][2]=0;

    ColorsClippingHandles[4][0]=0;
    ColorsClippingHandles[4][1]=0;
    ColorsClippingHandles[4][2]=1;

    ColorsClippingHandles[5][0]=0;
    ColorsClippingHandles[5][1]=0;
    ColorsClippingHandles[5][2]=0;

    //PauseResume
    this->PB_PauseResume=NULL;
    this->RenderingPaused=0;
    this->VI_PauseResume=NULL;



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
    this->MapperRaycast->RemoveObservers(vtkCommand::ProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperRenderProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);


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
    //Don't delete the RenViewport(allocated in Outside)
    
    //Delete the rendering pipeline
    //this->volumeWillBeDeletedInSuperClass
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

    this->ResetRenderingAlgorithm();

    if(this->RenPlane!=NULL)
    {
        this->RenPlane->Delete();
        this->RenPlane=NULL;
    }
    if(this->Timer!=NULL)
    {
        this->Timer->StopTimer();
        this->Timer->Delete();
        this->Timer=NULL;
    }
    this->DestroyTreshold();
    this->DestroyPerformance();
    this->DestroyCropping();

    if(this->NB_Details)
    {
        this->Script("pack forget %s",this->NB_Details->GetWidgetName());
        this->NB_Details->SetParent(NULL);
        this->NB_Details->Delete();
        this->NB_Details=NULL;
    }



    if(this->PB_PauseResume)
    {
        this->PB_PauseResume->SetParent(NULL);
        this->PB_PauseResume->Delete();
        this->PB_PauseResume=NULL;
    }
    if(this->VI_PauseResume)
    {
        this->VI_PauseResume->Delete();
        this->VI_PauseResume=NULL;
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

    //TODO: Move Pause Resume Button to another Place
    //Pause Resume Button

    this->VI_PauseResume=vtkSlicerVisibilityIcons::New();
    this->PB_PauseResume=vtkKWPushButtonWithLabel::New();
    this->PB_PauseResume->SetParent(this->Gui->GetDetailsFrame()->GetFrame());
    this->PB_PauseResume->Create();
    this->PB_PauseResume->SetBalloonHelpString("Toggle the visibility of volume rendering.");
    this->PB_PauseResume->SetLabelText("Visiblity of Volume Rendering: ");
    this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());
    this->Script("pack %s -side top -anchor nw -padx 10 -pady 10",
        this->PB_PauseResume->GetWidgetName());
    this->PB_PauseResume->GetWidget()->SetCommand(this,"ProcessPauseResume");


    //Create a notebook
    this->NB_Details=vtkKWNotebook::New();
    this->NB_Details->SetParent(this->Gui->GetDetailsFrame()->GetFrame());
    this->NB_Details->Create();
    this->NB_Details->AddPage("Threshold","Edit volume rendering mapping options by using a threshold mechanism.");
    this->NB_Details->AddPage("Performance","Influence the performance and quality of the rendering. Settings will still be available after starting Slicer3 again.");
    this->NB_Details->AddPage("Cropping","Crop the volume.Advantages: Volume rendering is much faster. You can blank out unnecessary parts of the volume.");
    this->NB_Details->AddPage("Advanced","Change mapping functions, shading, interpolation etc.");
    this->Script("pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2", 
        this->NB_Details->GetWidgetName());

    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(this->NB_Details->GetFrame("Advanced"));
    this->SVP_VolumeProperty->Create();
    this->SVP_VolumeProperty->ScalarOpacityUnitDistanceVisibilityOff ();
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
    this->CreateThreshold();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());
    this->CreatePerformance();
    this->CreateCropping();
}
void vtkSlicerVRGrayscaleHelper::InitializePipelineNewCurrentNode()
{
    std::stringstream autoname;
    autoname<<"autoVisualization";
    autoname<<this->Gui->GetNS_ImageData()->GetSelected()->GetName();
    this->Gui->GetCurrentNode()->SetName(autoname.str().c_str());
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
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetInterpolationTypeToLinear();
    vtkPiecewiseFunction *opacity=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    opacity->RemoveAllPoints();
    opacity->AddPoint(range[0],0.);
    opacity->AddPoint(thresholdLowIndex,0.0);
    opacity->AddPoint(thresholdHighIndex,0.2);
    opacity->AddPoint(range[1],0.2);
    vtkColorTransferFunction *colorTransfer=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
    colorTransfer->RemoveAllPoints();
    colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
    colorTransfer->AddRGBPoint(thresholdLowIndex,.3,.3,1.);
    colorTransfer->AddRGBPoint(thresholdLowIndex+.5*(thresholdHighIndex-thresholdLowIndex),.3,1.,.3);
    colorTransfer->AddRGBPoint(thresholdHighIndex,1.,.3,.3);
    colorTransfer->AddRGBPoint(range[1],1,.3,.3);


    //Enable shading as default

    this->Gui->GetCurrentNode()->GetVolumeProperty()->ShadeOn();
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetAmbient(.30);
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetDiffuse(.60);
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetSpecular(.50);
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetSpecularPower(40);//this is really weird

    //Set cropping
    this->Gui->GetCurrentNode()->CroppingEnabledOff();

    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    vtkMRMLScalarVolumeNode *volumeNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected());
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    double pointA[4];
    double pointB[4];
    for(int i=0;i<3;i++)
    {
        pointA[i]=iData->GetOrigin()[i];
        pointB[i]=iData->GetDimensions()[i];
    }
    pointA[3]=1;
    pointB[3]=1;
    this->CalculateMatrix(matrix);
    matrix->MultiplyPoint(pointA,pointA);
    matrix->MultiplyPoint(pointB,pointB);
    
    this->Gui->GetCurrentNode()->SetCroppingRegionPlanes(pointA[0],pointB[0],
        pointA[1],pointB[1],
        pointA[2],pointB[2]);

    //Disable Gradient Opacity

    this->UpdateGUIElements();
    matrix->Delete();
    //this->UpdateSVP();
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
    //First of all set our sample distances right
    this->CalculateAndSetSampleDistances();
    
    this->Volume=vtkVolume::New();

    //Init the texture mapper
    this->MapperTexture=vtkSlicerVolumeTextureMapper3D::New();
    this->MapperTexture->SetSampleDistance(this->SampleDistanceLowRes);
    this->MapperTexture->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    this->Volume->SetMapper(this->MapperTexture);

    //Also take care about Ray Cast
    this->MapperRaycast=vtkSlicerFixedPointVolumeRayCastMapper::New();
    this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    this->MapperRaycast->SetAutoAdjustSampleDistances(0);
    this->MapperRaycast->SetSampleDistance(this->SampleDistanceHighRes);
    this->MapperRaycast->SetImageSampleDistance(this->SampleDistanceHighResImage);

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

    //check if texture mapping is supported important to do it after registry
    //otherwise show textmessage
    int supportTexture=this->MapperTexture->IsRenderSupported(this->Gui->GetCurrentNode()->GetVolumeProperty());

    if(!supportTexture)
    {

        vtkKWLabel *errorText=vtkKWLabel::New();
        errorText->SetParent(this->MappersFrame->GetFrame());
        errorText->Create();
        errorText->SetText("OpenGL Texture Mapping is not supported");
        this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
            errorText->GetWidgetName());
        this->CB_RayCast->GetWidget()->SetSelectedState(1);            
        this->CB_TextureLow->GetWidget()->SetSelectedState(0);
        this->CB_TextureHigh->GetWidget()->SetSelectedState(0);
        this->CB_TextureLow->EnabledOff();
        this->CB_TextureHigh->EnabledOff();
        errorText->Delete();

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

    this->Volume->SetProperty(this->Gui->GetCurrentNode()->GetVolumeProperty());
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    //For Performance
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->AddViewProp(this->Volume);
    this->RenViewport=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetNthRenderer(0);
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
    this->Volume->SetProperty(this->Gui->GetCurrentNode()->GetVolumeProperty());
    //Update matrix
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    matrix->Delete();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData)
{
    vtkSlicerColorDisplayWidget *callerColor=vtkSlicerColorDisplayWidget::SafeDownCast(caller);
    if(caller==this->ColorDisplay&&eid==vtkSlicerColorDisplayWidget::ColorIDModifiedEvent)
    {

        //Trigger the update of
        vtkLookupTable *lookup=this->ColorDisplay->GetColorNode()->GetLookupTable();
        if(lookup==NULL)
        {
            vtkErrorMacro("No LookupTable");
            return;
        }
        vtkColorTransferFunction *colorTransfer=this->SVP_VolumeProperty->GetVolumeProperty()->GetRGBTransferFunction();
        colorTransfer->RemoveAllPoints();
        //this->AdjustRange(lookup->GetTableRange());
        double color[3];

        for (int i=(int)lookup->GetTableRange()[0];i<lookup->GetTableRange()[1];i++)
        {
            lookup->GetColor(i,color);
            colorTransfer->AddRGBPoint(i,color[0],color[1],color[2]);
            colorTransfer->AddRGBPoint(i+.9999,color[0],color[1],color[2]);

        }
        this->UpdateGUIElements();
        return;
    }
    vtkSlicerBoxWidget *callerBox=vtkSlicerBoxWidget::SafeDownCast(caller);
    if(caller==this->BW_Clipping&&eid==vtkCommand::InteractionEvent)
    {
        vtkPlanes *planes=vtkPlanes::New();
        callerBox->GetPlanes(planes);
        this->MapperTexture->SetClippingPlanes(planes);
        this->MapperRaycast->SetClippingPlanes(planes);

        //Decide if this event is triggered by the vtkBoxWidget or by the sliders
        //if sliders don't trigger setRange->this would lead to an endless loop
        if(!this->NoSetRangeNeeded)
        {

            vtkPolyData *vertices=vtkPolyData::New();

            callerBox->GetPolyData(vertices);
            double pointA[3];
            double pointB[3];
            vertices->GetPoint(0,pointA);
            vertices->GetPoint(6,pointB);
            //Include a possible transform in this calculation
            this->InverseAdditionalClippingTransform->TransformPoint(pointA,pointA);
            this->InverseAdditionalClippingTransform->TransformPoint(pointB,pointB);

            vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
            for(int i=0; i<3;i++)
            {
                double rangeFromImagedata=iData->GetDimensions()[i]/2.; 
                if((-rangeFromImagedata)<pointA[i])
                {
                    this->RA_Cropping[i]->SetWholeRange((-rangeFromImagedata),this->RA_Cropping[i]->GetWholeRange()[1]);
                }
                else
                {
                    this->RA_Cropping[i]->SetWholeRange(pointA[i],this->RA_Cropping[i]->GetWholeRange()[1]);
                }
                if(rangeFromImagedata>pointB[i])
                {
                    this->RA_Cropping[i]->SetWholeRange(this->RA_Cropping[i]->GetWholeRange()[0],rangeFromImagedata);
                }
                else
                {
                    this->RA_Cropping[i]->SetWholeRange(this->RA_Cropping[i]->GetWholeRange()[0],pointB[i]);
                }
                this->RA_Cropping[i]->SetRange(pointA[i],pointB[i]);

            }
            

            vertices->Delete();
            //transform->Delete();
        }
        planes->Delete();
        return;
    }
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
            this->MapperRaycast->SetSampleDistance(this->SampleDistanceHighRes);
            this->MapperRaycast->SetImageSampleDistance(this->SampleDistanceHighResImage);
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
        vtkSlicerVRHelperDebug("startevent Scheduled %d",this->Scheduled);
        vtkSlicerVRHelperDebug("startevent currentstage %d",this->CurrentStage);
        vtkSlicerVRHelperDebug("startevent id %s",this->EventHandlerID.c_str());

        //it is not a Scheduled event so we use stage the quality stage and abort every existings scheduling
        if(this->Scheduled==0||this->ButtonDown==1)
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
            this->CurrentStage=this->Quality;
        }//if
        if(this->CurrentStage==0)
        {

            this->Volume->SetMapper(this->MapperTexture);
            //always got to less sample distance
            this->MapperTexture->SetSampleDistance(this->SampleDistanceLowRes);
            this->CurrentStage=0;
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
            this->Timer->StartTimer();
            vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();

            //We don't like to see, what we are doing here
            renWin->SwapBuffersOff();
            //get the viewport renderer up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->RenPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->RenViewport);
            //Change viewport(simulation of "sample distance for "rays"" if using texture mapping)
            this->RenViewport->SetViewport(0,0,this->FactorLastLowRes,this->FactorLastLowRes);
            return;


        }
        //Stage 1
        if(this->CurrentStage==1)
        {
            this->Volume->SetMapper(MapperTexture);
            this->MapperTexture->SetSampleDistance(this->SampleDistanceHighRes);

            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using Middle Resolution");
            vtkSlicerVRHelperDebug("Stage 1 started","");
            //Remove plane Renderer and get viewport Renderer Up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->RenPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->RenViewport);
            //Go to Fullsize
            this->RenViewport->SetViewport(0,0,1,1);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SwapBuffersOn();
            this->Scheduled=0;
            return;
        }

        //Stage 2
        if(this->CurrentStage==2)
        {
            vtkRenderWindowInteractor *interactor=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetInteractor();
            if(this->ButtonDown&&this->CB_InteractiveFrameRate->GetWidget()->GetSelectedState())
            {

                this->MapperRaycast->SetManualInteractive(1);
            }
            //Do the following code just in case we skipped stage 1
            //Remove plane Renderer and get viewport Renderer Up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->RenPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->RenViewport);
            //Go to Fullsize
            this->RenViewport->SetViewport(0,0,1,1);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SwapBuffersOn();
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using Highest Resolution");
            vtkSlicerVRHelperDebug("Stage 2 started","");
            this->Volume->SetMapper(this->MapperRaycast);

            this->EventHandlerID="";
            this->Scheduled=0;
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
        vtkSlicerVRHelperDebug("endevent Scheduled %d",this->Scheduled);
        vtkSlicerVRHelperDebug("endevent currentstage %d",this->CurrentStage);
        if(this->CurrentStage==1)
        {
            this->EventHandlerID=this->Gui->Script("after 100 %s ScheduleRender",this->GetTclName());
            vtkSlicerVRHelperDebug("Stage 1 ended","");
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,1);
            return;
        }
        if(this->CurrentStage==2)
        {
            vtkSlicerVRHelperDebug("Stage 2 ended","");
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,100);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,100);
            if(this->MapperRaycast->GetManualInteractive())
            {
                this->MapperRaycast->SetManualInteractive(0);
                this->Scheduled=1;
                this->EventHandlerID=this->Gui->Script("after idle [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow] Render");
            }
            //We reached the highest Resolution, no scheduling

            return;
        }
        if(this->RenderPlane==1)
        {
            this->RenderPlane=0;
            this->Timer->StopTimer();
            this->LastTimeLowRes=this->Timer->GetElapsedTime();
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
        if(this->RenPlane==NULL)
        {
            this->RenPlane=vtkRenderer::New();
        }

        this->RenPlane->SetBackground(this->RenViewport->GetBackground());
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

        this->RenPlane->SetActiveCamera(this->RenViewport->GetActiveCamera());

        //Get Our Renderer Up
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->RenViewport);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->RenPlane);

        this->RenPlane->SetDisplayPoint(0,0,0.5);
        this->RenPlane->DisplayToWorld();
        double coordinatesA[4];
        this->RenPlane->GetWorldPoint(coordinatesA);

        this->RenPlane->SetDisplayPoint(size[0],0,0.5);
        this->RenPlane->DisplayToWorld();
        double coordinatesB[4];
        this->RenPlane->GetWorldPoint(coordinatesB);

        this->RenPlane->SetDisplayPoint(size[0],size[1],0.5);
        this->RenPlane->DisplayToWorld();
        double coordinatesC[4];
        this->RenPlane->GetWorldPoint(coordinatesC);

        this->RenPlane->SetDisplayPoint(0,size[1],0.5);
        this->RenPlane->DisplayToWorld();
        double coordinatesD[4];
        this->RenPlane->GetWorldPoint(coordinatesD);

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
        this->RenPlane->RemoveAllViewProps();

        this->RenPlane->AddActor(actor);
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
        vtkSlicerNodeSelectorWidget *callerNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    if (eid==vtkSlicerNodeSelectorWidget::NodeSelectedEvent&&callerNS==this->NS_TransformNode)
    {
        //Remove a potential observer from the old node
        if(this->CurrentTransformNodeCropping!=NULL)
        {
            this->CurrentTransformNodeCropping->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,this->VolumeRenderingCallbackCommand);
        }
        //Add an observer to the new node
        this->CurrentTransformNodeCropping=vtkMRMLLinearTransformNode::SafeDownCast(this->NS_TransformNode->GetSelected());
        if(this->CurrentTransformNodeCropping!=NULL)
        {
            this->CurrentTransformNodeCropping->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,this->VolumeRenderingCallbackCommand);
        }
                    this->ProcessClippingModified();
        return;
    }
    if(eid==vtkMRMLTransformableNode::TransformModifiedEvent)
    {
        this->ProcessClippingModified();
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
    else if (eid==vtkCommand::VolumeMapperRenderProgressEvent&&this->CurrentStage==1)
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
    this->Scheduled=1;
    this->CurrentStage++;

    //Security if we ran out of range or an event occured in between
    if(this->CurrentStage>2||vtkKWTkUtilities::CheckForPendingInteractionEvents(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow())!=0)
    {
        this->CurrentStage=0;
        this->Scheduled=0;
        //return;
    }
    else 
    {
        while(!ScheduleMask[this->CurrentStage])
        {

            this->CurrentStage++;
            //Nothing to schedule
            if(this->CurrentStage==3)
            {
                this->CurrentStage=0;
                this->Scheduled=0;
                return;
            }
        }
    }
    this->EventHandlerID=this->Gui->Script("after idle [[[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer] GetRenderWindow] Render");
    //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::UpdateSVP(void)
{
    //TODO really dirty here
    //First check if we have a SVP
    if(this->SVP_VolumeProperty==NULL)
    {
        vtkErrorMacro("SVP does not exist");
        return;
    }
    //First of all set New Property, Otherwise all Histograms will be overwritten
    //First check if we really need to update
    if(this->SVP_VolumeProperty->GetVolumeProperty()==this->Gui->GetCurrentNode()->GetVolumeProperty())
    {
        this->AdjustMapping();
        this->SVP_VolumeProperty->Update();
        //Set Treshold to none
        this->MB_ThresholdMode->GetWidget()->GetMenu()->SelectItem("None");
        this->ProcessThresholdModeEvents(0);
        //Reset cropping
        //Get Cropping from node

    }
    else
    {
    
        //for(int i=0;i<3;i++)
        //{
        //    this->RA_Cropping[i]->SetRange(this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[2*i],this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[2*i+1]);
        //}
        this->CB_Cropping->GetWidget()->SetSelectedState(this->Gui->GetCurrentNode()->GetCroppingEnabled());
        this->ProcessEnableDisableCropping(this->Gui->GetCurrentNode()->GetCroppingEnabled());
    }
    this->SVP_VolumeProperty->SetVolumeProperty(this->Gui->GetCurrentNode()->GetVolumeProperty());
    this->SVP_VolumeProperty->SetHSVColorSelectorVisibility(1);
    this->SVP_VolumeProperty->Update();
}

void vtkSlicerVRGrayscaleHelper::UpdateGUIElements(void)
{
    Superclass::UpdateGUIElements();
    this->UpdateSVP();
    for(int i=0;i<3;i++)
    {
        this->RA_Cropping[i]->SetRange(this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[2*i],this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[2*i+1]);    
    }
    this->CB_Cropping->GetWidget()->SetSelectedState(this->Gui->GetCurrentNode()->GetCroppingEnabled());
    //this->ColorDisplay->SetMRMLScene(this->Gui->GetMRMLScene());

    //Add update of gui here
}
void vtkSlicerVRGrayscaleHelper::CheckAbort(void)
{
    int pending=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    if(pending!=0)
    {
        vtkSlicerVRHelperDebug("got an abort","");
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        this->Scheduled=0;
        return;
    }
    int pendingGUI=vtkKWTkUtilities::CheckForPendingInteractionEvents(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow());
    if(pendingGUI!=0)
    {
        vtkSlicerVRHelperDebug("got an abort from gui","");
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        this->Scheduled=0;
        return;

    }
}
void vtkSlicerVRGrayscaleHelper::AdjustMapping(){

    //Update Color    
    vtkColorTransferFunction *functionColor=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
    double rangeNew[2];
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetPointData()->GetScalars()->GetRange(rangeNew);
    functionColor->AdjustRange(rangeNew);

    //Update Opacity
    vtkPiecewiseFunction *function=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    function->AdjustRange(rangeNew);
    //Update
    rangeNew[1]=(rangeNew[1]-rangeNew[0])/4;
    rangeNew[0]=0;
    function=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetGradientOpacity();
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
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::Cropping(int index, double min,double max)
{

    if(this->MapperTexture==NULL||this->MapperRaycast==NULL)
    {
        return;
    }
    this->BW_Clipping->PlaceWidget(
        this->RA_Cropping[0]->GetRange()[0],
        this->RA_Cropping[0]->GetRange()[1],
        this->RA_Cropping[1]->GetRange()[0],
        this->RA_Cropping[1]->GetRange()[1],
        this->RA_Cropping[2]->GetRange()[0],
        this->RA_Cropping[2]->GetRange()[1]);
    this->BW_Clipping->SetTransform(this->AdditionalClippingTransform);
    this->NoSetRangeNeeded=1;
    this->ProcessVolumeRenderingEvents(this->BW_Clipping,vtkCommand::InteractionEvent,0);
    this->NoSetRangeNeeded=0;
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::CreateCropping()
{

        //Create our additional transforms
    this->AdditionalClippingTransform=vtkTransform::New();
    this->AdditionalClippingTransform->Identity();
    this->InverseAdditionalClippingTransform=vtkTransform::New();
    this->InverseAdditionalClippingTransform->Identity();

    vtkKWFrameWithLabel *croppingFrame=vtkKWFrameWithLabel::New();
    croppingFrame->SetParent(this->NB_Details->GetFrame("Cropping"));
    croppingFrame->Create();
    croppingFrame->AllowFrameToCollapseOff();
    croppingFrame->SetLabelText("Cropping (IJK coordinates)");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        croppingFrame->GetWidgetName() );

    int labelwidth=20;
    

    this->CB_Cropping=vtkKWCheckButtonWithLabel::New();
    this->CB_Cropping->SetParent(croppingFrame->GetFrame());
    this->CB_Cropping->Create();
    this->CB_Cropping->GetWidget()->SetSelectedState(0);
    this->CB_Cropping->SetBalloonHelpString("Enable/ Disable the configured clipping planes.");
    this->CB_Cropping->SetLabelText("Clipping in general");
    this->CB_Cropping->SetLabelWidth(labelwidth);
    this->CB_Cropping->GetWidget()->SetCommand(this, "ProcessEnableDisableCropping");
    this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10",
        this->CB_Cropping->GetWidgetName());


    this->CB_Clipping=vtkKWCheckButtonWithLabel::New();
    this->CB_Clipping->SetParent(croppingFrame->GetFrame());
    this->CB_Clipping->Create();
    this->CB_Clipping->SetBalloonHelpString("Display or suppress the clipping box. The configured clipping planes will still be enabled");
    this->CB_Clipping->GetWidget()->SetSelectedState(0);
    this->CB_Clipping->SetLabelText("Display Clipping Box");
    this->CB_Clipping->SetLabelWidth(labelwidth);
    this->CB_Clipping->GetWidget()->SetCommand(this, "ProcessEnableDisableClippingPlanes");
    this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10",
        this->CB_Clipping->GetWidgetName());

    for(int i=0;i<3;i++)
    {
        this->RA_Cropping[i]=vtkKWRange::New();
        this->RA_Cropping[i]->SetParent(croppingFrame->GetFrame());
        this->RA_Cropping[i]->Create();
        this->RA_Cropping[i]->SetBalloonHelpString("Configure the clipping planes relative to the center of the volume. You can also use the clipping box to do this.");
        this->RA_Cropping[i]->SymmetricalInteractionOff();
        std::stringstream str;
        str<<"Cropping ";
        str<<i;
        this->RA_Cropping[i]->SetCommand(this,str.str().c_str());
        this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10",this->RA_Cropping[i]->GetWidgetName());
    }
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    this->RA_Cropping[0]->SetLabelText("I");
    this->RA_Cropping[0]->SetWholeRange(-500,500);
    this->RA_Cropping[0]->SetSlider1Color(this->ColorsClippingHandles[0]);
    this->RA_Cropping[0]->SetSlider2Color(this->ColorsClippingHandles[1]);
    this->RA_Cropping[0]->SetRange(iData->GetOrigin()[0],iData->GetDimensions()[0]);
    this->RA_Cropping[1]->SetLabelText("J");
    this->RA_Cropping[1]->SetWholeRange(-500,500);
    this->RA_Cropping[1]->SetSlider1Color(this->ColorsClippingHandles[2]);
    this->RA_Cropping[1]->SetSlider2Color(this->ColorsClippingHandles[3]);
    this->RA_Cropping[1]->SetRange(iData->GetOrigin()[1],iData->GetDimensions()[1]);
    this->RA_Cropping[2]->SetLabelText("K");
    this->RA_Cropping[2]->SetWholeRange(-500,500);
    this->RA_Cropping[2]->SetSlider1Color(this->ColorsClippingHandles[4]);
    this->RA_Cropping[2]->SetSlider2Color(this->ColorsClippingHandles[5]);
    this->RA_Cropping[2]->SetRange(iData->GetOrigin()[2],iData->GetDimensions()[2]);

    //Now we have the cropping ranges
    //Build GUI
    this->NS_TransformNode=vtkSlicerNodeSelectorWidget::New();
    this->NS_TransformNode->SetParent(croppingFrame->GetFrame());
    this->NS_TransformNode->Create();
    this->NS_TransformNode->SetLabelText("Transform Node for Clipping");
    std::stringstream ss;
    ss<<"Use a transform in addition to the clipping planes configured with the clipping box or the sliders above. ";
    ss<<"Changes made to the sliders or to the clipping box will not change the transform node.";
    this->NS_TransformNode->SetBalloonHelpString(ss.str().c_str());
    this->NS_TransformNode->SetNodeClass("vtkMRMLTransformNode",NULL,NULL,NULL);
    this->NS_TransformNode->SetMRMLScene(this->Gui->GetLogic()->GetMRMLScene());
    this->NS_TransformNode->NoneEnabledOn();
    this->NS_TransformNode->SetSelected(NULL);
    this->NS_TransformNode->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10",this->NS_TransformNode->GetWidgetName());
    this->ProcessEnableDisableCropping(0);
    croppingFrame->Delete();
}

void vtkSlicerVRGrayscaleHelper::CreateThreshold()
{



    vtkKWFrameWithLabel *thresholdFrame=vtkKWFrameWithLabel::New();
    thresholdFrame->SetParent(this->NB_Details->GetFrame("Threshold"));
    thresholdFrame->Create();
    thresholdFrame->SetLabelText("Threshold");
    this->Script("pack %s -side top -anchor nw -fill both -expand yes -padx 0 -pady 2", 
        thresholdFrame->GetWidgetName());

    this->MB_ThresholdMode=vtkKWMenuButtonWithLabel::New();
    this->MB_ThresholdMode->SetParent(thresholdFrame->GetFrame());
    this->MB_ThresholdMode->Create();
    std::stringstream ss;
    ss<<"Select which kind of threshoding to use for transfer functions. ";
    ss<<"\"Rectangle\" will result in sharp surface, while \"ramp\" creates a smoother result.";
    this->MB_ThresholdMode->SetBalloonHelpString(ss.str().c_str());
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
    ss.str("");
    ss<<"Which color should the volume have, ";
    ss<<"all \"static\" colors result in a single color for every gray value while \"dynamic\" produces a color gradient.";
    this->MB_ColorMode->SetBalloonHelpString(ss.str().c_str());
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
    this->RA_RampRectangleHorizontal->SetBalloonHelpString("Apply thresholds to the gray values of volume.");
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
    ss.str("");
    ss<<"Zoom into the threshold sliders. ";
    ss<<"Use this if you need a higher resolution for adjusting the sliders. ";
    ss<<"Use \"Reset\" to zoom out completely."; 
    this->PB_ThresholdZoomIn->SetBalloonHelpString(ss.str().c_str());
    this->PB_ThresholdZoomIn->SetText("Zoom In");
    this->PB_ThresholdZoomIn->EnabledOff();
    this->PB_ThresholdZoomIn->SetCommand(this,"ProcessThresholdZoomIn");
    this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
        this->PB_ThresholdZoomIn->GetWidgetName());

    this->PB_Reset=vtkKWPushButton::New();
    this->PB_Reset->SetParent(thresholdFrame->GetFrame());
    this->PB_Reset->Create();
    this->PB_Reset->SetBalloonHelpString("Cancel \"Zoom In\".");
    this->PB_Reset->SetText("Reset");
    this->PB_Reset->EnabledOff();
    this->PB_Reset->SetCommand(this,"ProcessThresholdReset");
    this->Script("pack %s -side top -anchor nw -fill x -expand n -padx 2 -pady 2",
        this->PB_Reset->GetWidgetName());

    this->RA_RampRectangleVertical=vtkKWRange::New();
    this->RA_RampRectangleVertical->SetParent(thresholdFrame->GetFrame());
    this->RA_RampRectangleVertical->Create();
    ss.str("");
    ss<<"Set the lower and upper opacity levels for the ramp and the rectangle. ";
    ss<<"Lower values make the volume more translucent.";
    this->RA_RampRectangleVertical->SetBalloonHelpString(ss.str().c_str());
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
    vtkPiecewiseFunction *opacity=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    opacity->RemoveAllPoints();
    //opacity->AdjustRange(iData->GetScalarRange());

    vtkColorTransferFunction *colorTransfer=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
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
    if(this->MapperTexture==NULL||this->MapperRaycast==NULL)
    {
        return;
    }
    for(int i=0;i<3;i++)
    {
        this->RA_Cropping[i]->SetEnabled(cbSelectedState);
    }
    this->NS_TransformNode->SetEnabled(cbSelectedState);
    //There is not automatical enabling when
    //this->ProcessEnableDisableClippingPlanes(cbSelectedState);
    this->CB_Clipping->SetEnabled(cbSelectedState);
    if(this->Gui->GetCurrentNode()!=NULL)
    {
        this->Gui->GetCurrentNode()->SetCroppingEnabled(cbSelectedState);
    }
    if(cbSelectedState)
    {
        //If we enable clipping we choose the current state of CB_Clipping
        this->ProcessEnableDisableClippingPlanes(this->CB_Clipping->GetWidget()->GetSelectedState());
        this->Cropping(0,0,0);
        //PlaceWidget

    }
    else
    {
        this->ProcessEnableDisableClippingPlanes(0);
        this->MapperTexture->RemoveAllClippingPlanes();
        this->MapperRaycast->RemoveAllClippingPlanes();
    }

    //Trigger a Render
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();

}

void vtkSlicerVRGrayscaleHelper::ProcessEnableDisableClippingPlanes(int clippingEnabled)
{
    if(this->BW_Clipping==NULL)
    {
        this->BW_Clipping=vtkSlicerBoxWidget::New();
        vtkRenderWindowInteractor *interactor=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetInteractor();
        this->BW_Clipping->SetInteractor(interactor);
        this->BW_Clipping->SetPlaceFactor(1);
        this->BW_Clipping->SetProp3D(this->Volume);
        //data is saved in IJK->Convert to ras

        double pointA[3];
        pointA[0]=this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[0];
            pointA[1]=this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[2];
            pointA[2]=this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[4];

        double pointB[3];
        pointB[0]=this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[1];
            pointB[1]=this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[3];
            pointB[2]=this->Gui->GetCurrentNode()->GetCroppingRegionPlanes()[5];
            this->NoSetRangeNeeded=1;
        for(int i=0;i<3;i++)
        {
            if(pointA[i]<pointB[i])
            {
            this->RA_Cropping[i]->SetRange(pointA[i],pointB[i]);
            }
            else
            {
                this->RA_Cropping[i]->SetRange(pointB[i],pointA[i]);
            }
        }
        this->BW_Clipping->PlaceWidget(pointA[0],pointB[0],pointA[0],pointB[1],pointA[2],pointB[2]);
        this->BW_Clipping->InsideOutOn();
        this->BW_Clipping->RotationEnabledOff();
        this->BW_Clipping->TranslationEnabledOn();
        this->BW_Clipping->GetSelectedHandleProperty()->SetColor(0.2,0.6,0.15);
        this->NoSetRangeNeeded=0;
        //interactor->UpdateSize(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetSize()[0],
        //    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetSize()[1]);

        this->BW_Clipping->AddObserver(vtkCommand::InteractionEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetInteractor()->ReInitialize();
    }
    if(clippingEnabled)
    {
        this->BW_Clipping->On();
    }
    else
    {
        this->BW_Clipping->Off();
    }
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::ProcessSelection(void)
{
    int selectedCount=this->ColorDisplay->GetMultiColumnList()->GetWidget()->GetNumberOfSelectedRows();
    int *indices=new int[selectedCount];
    this->ColorDisplay->GetMultiColumnList()->GetWidget()->GetSelectedRows(indices);
    vtkPiecewiseFunction *piecewise=this->SVP_VolumeProperty->GetVolumeProperty()->GetScalarOpacity();
    piecewise->RemoveAllPoints();
    for(int i=0;i<selectedCount;i++)
    {
        int entry = this->ColorDisplay->GetMultiColumnList()->GetWidget()->GetCellTextAsInt(indices[i],0);
        piecewise->AddPoint(entry-.5,0);
        piecewise->AddPoint(entry-.49,1);
        piecewise->AddPoint(entry+.5,0);
        piecewise->AddPoint(entry+.49,1);
    }
    this->UpdateGUIElements();
    this->ColorDisplay->GetMultiColumnList()->GetWidget()->SetSelectionType(0);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();

    delete indices;
}

void vtkSlicerVRGrayscaleHelper::ProcessPauseResume(void)
{
    //Resume Rendering
    if(this->RenderingPaused)
    {
        this->RenderingPaused=0;
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->Volume->VisibilityOn();
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
        this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());;

    }
    //Pause Rendering
    else if (!this->RenderingPaused)
    {
        this->RenderingPaused=1;
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        //Clear ProgressGauge
        this->ResetRenderingAlgorithm();

        this->Volume->VisibilityOff();
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
        this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetInvisibleIcon());
    }
    else
    {
        vtkErrorMacro("RenderingPaused is not a valid number");
    }
    this->Script("put \"ProcessPauseResume\"");
}

void vtkSlicerVRGrayscaleHelper::ResetRenderingAlgorithm(void)
{
    //Ensure that Render is back to normal
    vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();
    //get the viewport renderer up
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->RenPlane);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->RenViewport);
    //this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("");
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,0);
    //Change viewport(simulation of "sample distance for "rays"" if using texture mapping)
    this->RenViewport->SetViewport(0,0,1,1);
    renWin->SwapBuffersOn();
}

void vtkSlicerVRGrayscaleHelper::DestroyCropping(void)
{
    if(this->BW_Clipping)
    {
        this->BW_Clipping->RemoveObservers(vtkCommand::InteractionEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);
        this->BW_Clipping->Off();
        this->BW_Clipping->Delete();
        this->BW_Clipping=NULL;
    }
    if(this->CB_Clipping)
    {
        this->CB_Clipping->SetParent(NULL);
        this->CB_Clipping->Delete();
        this->CB_Clipping=NULL;
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
    if(this->NS_TransformNode!=NULL)
    {
        this->NS_TransformNode->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->NS_TransformNode->SetParent(NULL);
        this->NS_TransformNode->Delete();
        this->NS_TransformNode=NULL;
    }
    if(this->AdditionalClippingTransform!=NULL)
    {
        this->AdditionalClippingTransform->Delete();
        this->AdditionalClippingTransform=NULL;
    }
    if(this->InverseAdditionalClippingTransform!=NULL)
    {
        this->InverseAdditionalClippingTransform->Delete();
        this->InverseAdditionalClippingTransform=NULL;
    }
}
void vtkSlicerVRGrayscaleHelper::DestroyTreshold(void)
{
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

void vtkSlicerVRGrayscaleHelper::CreatePerformance(void)
{
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
    this->CB_TextureLow->SetBalloonHelpString("Enable very fast, lower quality GPU based rendering.");
    this->CB_TextureLow->SetLabelText("Use Texture Low");
    this->CB_TextureLow->SetLabelWidth(labelWidth);
    this->CB_TextureLow->GetWidget()->SetSelectedState(1);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->CB_TextureLow->GetWidgetName() );
    this->CB_TextureLow->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);


    this->CB_TextureHigh=vtkKWCheckButtonWithLabel::New();
    this->CB_TextureHigh->SetParent(this->MappersFrame->GetFrame());
    this->CB_TextureHigh->Create();
    this->CB_TextureHigh->SetBalloonHelpString("Enable slow, high quality GPU based rendering.");
    this->CB_TextureHigh->SetLabelText("Use Texture High");
    this->CB_TextureHigh->SetLabelWidth(labelWidth);
    this->CB_TextureHigh->GetWidget()->SetSelectedState(1);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->CB_TextureHigh->GetWidgetName() );
    this->CB_TextureHigh->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);

    this->CB_RayCast=vtkKWCheckButtonWithLabel::New();
    this->CB_RayCast->SetParent(this->MappersFrame->GetFrame());
    this->CB_RayCast->Create();
    this->CB_RayCast->SetBalloonHelpString("Enable highest quality software rendering. If you encouter problems with other method, this method shoul still work.");
    this->CB_RayCast->SetLabelText("Use Raycast      ");
    this->CB_RayCast->SetLabelWidth(labelWidth);
    this->CB_RayCast->GetWidget()->SetSelectedState(1);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->CB_RayCast->GetWidgetName() );
    this->CB_RayCast->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);

    this->CB_InteractiveFrameRate=vtkKWCheckButtonWithLabel::New();
    this->CB_InteractiveFrameRate->SetParent(this->MappersFrame->GetFrame());
    this->CB_InteractiveFrameRate->Create();
    this->CB_InteractiveFrameRate->SetBalloonHelpString("Enable low quality software rendering. Use together with check box above");
    this->CB_InteractiveFrameRate->SetLabelText("Raycast interactive?!");
    this->CB_InteractiveFrameRate->SetLabelWidth(labelWidth);
    this->CB_InteractiveFrameRate->EnabledOff();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->CB_InteractiveFrameRate->GetWidgetName() );
    this->CB_InteractiveFrameRate->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);

    //Framerate
    this->SC_Framerate=vtkKWScaleWithLabel::New();
    this->SC_Framerate->SetParent(this->MappersFrame->GetFrame());
    this->SC_Framerate->Create();
    this->SC_Framerate->SetBalloonHelpString("Influence the speed of interactive rendering methods. 20 very fast, 1 slow but higher quality.");
    this->SC_Framerate->SetLabelText("FPS (Interactive):");
    this->SC_Framerate->SetLabelWidth(labelWidth);
    this->SC_Framerate->GetWidget()->SetRange(1,20);
    this->SC_Framerate->GetWidget()->SetResolution(1);
    this->SC_Framerate->GetWidget()->SetValue(1./this->GoalLowResTime);
    this->SC_Framerate->SetBalloonHelpString("set frames per sec for lowest resolution rendering");
    this->SC_Framerate->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->SC_Framerate->GetWidgetName() );

}
void vtkSlicerVRGrayscaleHelper::DestroyPerformance(void)
{
    //Save old values to registry
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
        this->CB_RayCast->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_RayCast->SetParent(NULL);
        this->CB_RayCast->Delete();
        this->CB_RayCast=NULL;
    }
    if(this->CB_TextureLow!=NULL)
    {
        this->CB_TextureLow->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_TextureLow->SetParent(NULL);
        this->CB_TextureLow->Delete();
        this->CB_TextureLow=NULL;
    }
    if(this->CB_TextureHigh!=NULL)
    {
        this->CB_TextureHigh->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_TextureHigh->SetParent(NULL);
        this->CB_TextureHigh->Delete();
        this->CB_TextureHigh=NULL;
    }
    if(this->SC_Framerate!=NULL)
    {
        this->SC_Framerate->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->SC_Framerate->SetParent(NULL);
        this->SC_Framerate->Delete();
        this->SC_Framerate=NULL;
    }
    if(this->CB_InteractiveFrameRate!=NULL)
    {
        this->CB_InteractiveFrameRate->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->CB_InteractiveFrameRate->SetParent(NULL);
        this->CB_InteractiveFrameRate->Delete();
        this->CB_InteractiveFrameRate=NULL;

    }
}


void vtkSlicerVRGrayscaleHelper::ProcessClippingModified(void)
{
    //If we have an additional Transform Node use it
    if(this->CurrentTransformNodeCropping!=NULL)
    {
        vtkMatrix4x4 *matrix=this->CurrentTransformNodeCropping->GetMatrixTransformToParent();
        this->AdditionalClippingTransform->SetMatrix(matrix);
        this->InverseAdditionalClippingTransform->SetMatrix(matrix);
        this->InverseAdditionalClippingTransform->Inverse();
        matrix->Delete();
    }
    //Otherwise go back to Identity;
    else
    {
        this->AdditionalClippingTransform->Identity();
        this->InverseAdditionalClippingTransform->Identity();
    }
    this->BW_Clipping->SetTransform(this->AdditionalClippingTransform);
    this->BW_Clipping->InvokeEvent(vtkCommand::InteractionEvent);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();

}
//TODO: Scaling,translation with transform (scaling part of transform or part or placewidget)->preferred: part of transform
//TODO: Adjust initial range to size of volume, reduce until old volume is reached 

//Note: we save clipping planes in ijk space, show it in ras and

void vtkSlicerVRGrayscaleHelper::CalculateAndSetSampleDistances(void)
{
    
    double *spacing=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetSpacing();
    double minSpacing=10000;
    for(int i=0;i<3;i++)
    {
        if(spacing[i]<minSpacing)
        {
            minSpacing=spacing[i];
        }
    }

    this->SampleDistanceHighRes=minSpacing/2.;
    this->SampleDistanceHighResImage=this->SampleDistanceHighRes;
    this->SampleDistanceLowRes=this->SampleDistanceHighRes*2;

}

void vtkSlicerVRGrayscaleHelper::ConvertWorldToBoxCoordinates(double *inputOutput)
{
}
void vtkSlicerVRGrayscaleHelper::ConvertBoxCoordinatesToWorld(double* inputOutput)
{
}
