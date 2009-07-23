//Slicer
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerGPUVolumeTextureMapper3D.h"
#include "vtkSlicerGPURayCastVolumeTextureMapper3D.h"
#include "vtkSlicerGPURayCastVolumeMapper.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerVRGrayscaleHelper.h"
#include "vtkVolumeRenderingGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColorDisplayWidget.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerBoxWidget2.h"
#include "vtkSlicerBoxRepresentation.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerVRMenuButtonColorMode.h"
 
//CUDA
#include "vtkCudaVolumeMapper.h"

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
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWEntry.h"

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

    this->MapperTexture=NULL;
    this->MapperCUDARaycast=NULL;
    this->MapperGPURaycast=NULL;
    this->MapperGPURaycastII=NULL;
    this->MapperRaycast=NULL;
    this->RenderingMethod = -1;//users are required to select a mapper
    
    //GUI:
    this->CB_CUDARayCastShading=NULL;
    
    this->MB_GPURayCastTechnique=NULL;
    this->MB_GPURayCastTechniqueII=NULL;
    this->SC_GPURayCastDepthPeelingThreshold=NULL;
    this->MB_GPUMemorySize=NULL;
    this->MB_GPURayCastColorOpacityFusion=NULL;

    this->SC_ExpectedFPS=NULL;
    this->MB_Mapper= NULL;
    this->SVP_VolumeProperty=NULL;
    
    this->FrameFPS = NULL;
    this->FrameGPURayCasting = NULL;
    this->FramePolygonBlending = NULL;
    this->FrameCPURayCasting = NULL;
    this->FramePerformance = NULL;
    
    //Cropping:
    this->CB_Cropping=NULL;
    this->RA_Cropping[0]=NULL;
    this->RA_Cropping[1]=NULL;
    this->RA_Cropping[2]=NULL;
    this->CurrentTransformNodeCropping=NULL;

    //ThresholdGUI
    this->MB_ThresholdMode=NULL;
    this->VRMB_ColorMode=NULL;
    this->RA_RampRectangleScalar=NULL;
    this->RA_RampRectangleOpacity=NULL;
    this->ThresholdMode=0;
    this->PB_Reset=NULL;
    this->PB_ThresholdZoomIn=NULL;

    //Clipping
    this->BW_Clipping_Widget=NULL;
    this->BW_Clipping_Representation = NULL;
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

    ColorsClippingHandles[3][0]=.89;
    ColorsClippingHandles[3][1]=.6;
    ColorsClippingHandles[3][2]=.07;

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
    this->BW_Clipping_Widget = NULL;

    this->UpdateingGUI = 0;
    
    this->IsTextureMappingSupported = 0;
    this->IsGPURayCastingSupported = 0;
    this->IsCUDARayCastingSupported = 0;
    
    this->ButtonDown = 0;
    
    this->CPURayCastingInteractionFlag = 0;
}

vtkSlicerVRGrayscaleHelper::~vtkSlicerVRGrayscaleHelper(void)
{   
    //Remove Bindings
    this->Gui->Script("bind all <Any-ButtonPress> {}",this->GetTclName());
    this->Gui->Script("bind all <Any-ButtonRelease> {}",this->GetTclName());
     
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(0,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1,0);
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(2,0);
    
    //Remove Obersvers
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->RemoveObservers(vtkCommand::ProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
     
    this->MapperCUDARaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperCUDARaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperCUDARaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    
    this->MapperGPURaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperGPURaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperGPURaycast->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->RemoveObservers(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::EndEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    
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

    if(this->MapperCUDARaycast!=NULL)
    {
        this->MapperCUDARaycast->Delete();
        this->MapperCUDARaycast=NULL;
    }

    if(this->MapperGPURaycast!=NULL)
    {
        this->MapperGPURaycast->Delete();
        this->MapperGPURaycast=NULL;
    }
    
    if(this->MapperGPURaycastII!=NULL)
    {
        this->MapperGPURaycastII->Delete();
        this->MapperGPURaycastII=NULL;
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

void vtkSlicerVRGrayscaleHelper::Init(vtkVolumeRenderingGUI *gui)
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

    if (this->Gui->GetDetailsFrame())
    {
        this->VI_PauseResume=vtkSlicerVisibilityIcons::New();
        this->PB_PauseResume=vtkKWPushButtonWithLabel::New();
        this->PB_PauseResume->SetParent(this->Gui->GetDetailsFrame()->GetFrame());
        this->PB_PauseResume->Create();
        this->PB_PauseResume->SetBalloonHelpString("Toggle the visibility of volume rendering.");
        this->PB_PauseResume->SetLabelText("Visiblity of Volume Rendering: ");
        this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());
        this->Script("pack %s -side top -anchor n -padx 2 -pady 2", this->PB_PauseResume->GetWidgetName());
        this->PB_PauseResume->GetWidget()->SetCommand(this, "ProcessPauseResume");
    }

    //Create a notebook
    this->NB_Details=vtkKWNotebook::New();
    this->NB_Details->SetParent(this->Gui->GetDetailsFrame()->GetFrame());
    this->NB_Details->UseFrameWithScrollbarsOn();
    this->NB_Details->Create();
    this->NB_Details->AddPage("Performance","Influence the performance and quality of the rendering. Settings will still be available after starting Slicer3 again.");
    this->NB_Details->AddPage("Threshold","Edit volume rendering mapping options by using a threshold mechanism.");
    this->NB_Details->AddPage("Cropping","Crop the volume.Advantages: Volume rendering is much faster. You can blank out unnecessary parts of the volume.");
    this->NB_Details->AddPage("Advanced","Change mapping functions, shading, interpolation etc.");
    this->Script("pack %s -side top -anchor nw -fill both -expand y -padx 2 -pady 2", this->NB_Details->GetWidgetName());
    
    vtkImageData *imageData = NULL;
    vtkMRMLScalarVolumeNode *volume = NULL;
    
    if (this->Gui && this->Gui->GetNS_ImageData() && this->Gui->GetNS_ImageData()->GetSelected())
    {
        volume = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected());
    }
      
    if (volume)
    {
        imageData = volume->GetImageData();
    }
      
    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(this->NB_Details->GetFrame("Advanced"));
    this->SVP_VolumeProperty->Create();
    this->SVP_VolumeProperty->ScalarOpacityUnitDistanceVisibilityOff ();
    this->SVP_VolumeProperty->SetDataSet(imageData);
    
    this->SVP_VolumeProperty->InteractiveApplyModeOn ();
    
    this->Histograms=vtkKWHistogramSet::New();

    //Add Histogram for image data
    if (imageData && imageData->GetPointData())
    {
        this->Histograms->AddHistograms(imageData->GetPointData()->GetScalars());
    }
    
    //Build the gradient histogram
    vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
    grad->SetDimensionality(3);
    grad->SetInput(imageData);
    grad->Update();
    vtkKWHistogram *gradHisto=vtkKWHistogram::New();
    gradHisto->BuildHistogram(grad->GetOutput()->GetPointData()->GetScalars(),0);
    this->Histograms->AddHistogram(gradHisto, "0gradient");

    //Delete      
    this->SVP_VolumeProperty->SetHistogramSet(this->Histograms);

    //AddEvent for Interactive apply 
    this->SVP_VolumeProperty->AddObserver(vtkKWEvent::VolumePropertyChangingEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    
    grad->Delete();
    gradHisto->Delete(); 
    
    this->CreatePerformance();
    this->CreateCropping();
    this->CreateThreshold();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());
    
    this->WithdrawProgressDialog();
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::InitializePipelineNewCurrentNode()
{
    std::stringstream autoname;
    autoname<<"autoVisualization";
    autoname<<this->Gui->GetNS_ImageData()->GetSelected()->GetName();
    this->Gui->GetCurrentNode()->SetName(autoname.str().c_str());
    //this->Gui->GetLogic()->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent);
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
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetAmbient(.20);
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetDiffuse(.80);
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetSpecular(.50);
    this->Gui->GetCurrentNode()->GetVolumeProperty()->SetSpecularPower(40);//this is really weird

    //Set cropping
    this->Gui->GetCurrentNode()->CroppingEnabledOff();

    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    //vtkMRMLScalarVolumeNode *volumeNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected());
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
    
    this->Gui->GetCurrentNode()->SetCroppingRegionPlanes(pointA[0],pointB[0], pointA[1],pointB[1], pointA[2],pointB[2]);

    //Disable Gradient Opacity

    this->UpdateGUIElements();
    
    matrix->Delete();
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
    
    this->EstimateSampleDistances();
    
    this->Volume = vtkVolume::New();

    //init mappers
    {
        //Init the texture mapper
        this->MapperTexture = vtkSlicerVolumeTextureMapper3D::New();
        this->MapperTexture->SetSampleDistance(this->EstimatedSampleDistance);
        this->MapperTexture->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
 
        //create the CUDA raycast mapper

        this->MapperCUDARaycast = vtkCudaVolumeMapper::New();
        this->MapperCUDARaycast->SetIntendedFrameRate(this->SC_ExpectedFPS->GetValue());
        this->MapperCUDARaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());

        //create the raycast mapper
        this->MapperGPURaycast = vtkSlicerGPURayCastVolumeTextureMapper3D::New();
        this->MapperGPURaycast->SetFramerate(this->SC_ExpectedFPS->GetValue());
        this->MapperGPURaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        
        //create the raycast mapper II
        this->MapperGPURaycastII = vtkSlicerGPURayCastVolumeMapper::New();
        this->MapperGPURaycastII->SetFramerate(this->SC_ExpectedFPS->GetValue());
        this->MapperGPURaycastII->SetNthInput(0, vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        
        //we must have source input, but may or maybe not have background and labelmap inputs
        if (this->Gui->GetNS_ImageDataBg()->GetSelected())
            this->MapperGPURaycastII->SetNthInput(1, vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageDataBg()->GetSelected())->GetImageData());
        if (this->Gui->GetNS_ImageDataLabelmap()->GetSelected())
            this->MapperGPURaycastII->SetNthInput(2, vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageDataLabelmap()->GetSelected())->GetImageData());
        
        double scalarRange[2];
        vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
        this->MapperGPURaycast->SetDepthPeelingThreshold(scalarRange[0]);
        
        //Also take care about Ray Cast
        this->MapperRaycast=vtkSlicerFixedPointVolumeRayCastMapper::New();
        this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        this->MapperRaycast->SetSampleDistance(this->EstimatedSampleDistance);
        this->MapperRaycast->ManualInteractiveOff();      
        this->MapperRaycast->SetImageSampleDistance(1.0f);
        this->MapperRaycast->SetMinimumImageSampleDistance(1.0f);
        this->MapperRaycast->SetMaximumImageSampleDistance(20.0f);
    }
    
    // create default(safe) GUI in case user messed up parameters
    // do not save/load registration table
    
    //check if mappers are supported
    {
        this->IsTextureMappingSupported = this->MapperTexture->IsRenderSupported(this->Gui->GetCurrentNode()->GetVolumeProperty());//opengl Polygon Texture 3D
        this->IsCUDARayCastingSupported = this->MapperCUDARaycast->GetCUDAEnabled();//cuda ray cast
        this->IsGPURayCastingSupported = this->MapperGPURaycast->IsRenderSupported(this->Gui->GetCurrentNode()->GetVolumeProperty());//gpu ray cast (glsl)
        
        if (!IsCUDARayCastingSupported)
        {
            vtkErrorMacro("CUDA is not supported by your computer.");
            this->CB_CUDARayCastShading->EnabledOff();
        }

        if (!IsGPURayCastingSupported)
        {
            vtkErrorMacro("GPU ray casting (GLSL) is not supported by your computer.");
            this->MB_GPURayCastTechnique->EnabledOff();
            this->SC_GPURayCastDepthPeelingThreshold->EnabledOff();
            
            //GPU ray cast II
            this->MB_GPURayCastTechniqueII->EnabledOff();
            this->MB_GPURayCastColorOpacityFusion->EnabledOff();
        }
    
        if(!IsTextureMappingSupported)
        {
            vtkErrorMacro("OpenGL Texture Mapping is not supported by your computer.");
        }

    }
    
    //setup observers for mappers
    {
        //Polygon Texture 3D
        this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        
        //cpu ray casting
        this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        
        //hook up the cuda mapper
        this->MapperCUDARaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->MapperCUDARaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->MapperCUDARaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

        //hook up the gpu mapper
        this->MapperGPURaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
        this->MapperGPURaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->MapperGPURaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        
        // GPU ray cast II mapper calculate gradients in fragement shader
        
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    }

    //choose default mapper
    {
        //Try to load from the registry
        if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","MB_GPUMemorySize"))
        {
            int id = this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","MB_GPUMemorySize");
            this->MB_GPUMemorySize->GetWidget()->GetMenu()->SelectItem(id);
            
            switch(id)
            {
            case 0://128M
                this->MapperGPURaycast->SetInternalVolumeSize(256);//256^3
                this->MapperGPURaycastII->SetInternalVolumeSize(256);//256^3
                this->MapperTexture->SetInternalVolumeSize(256);
                break;
            case 1://256M
                this->MapperGPURaycast->SetInternalVolumeSize(384);//384^3
                this->MapperGPURaycastII->SetInternalVolumeSize(384);//384^3
                this->MapperTexture->SetInternalVolumeSize(256);
                break;
            case 2://512M
                this->MapperGPURaycast->SetInternalVolumeSize(512);//512^3
                this->MapperGPURaycastII->SetInternalVolumeSize(512);//512^3
                this->MapperTexture->SetInternalVolumeSize(512);
                break;
            case 3://1024M
                this->MapperGPURaycast->SetInternalVolumeSize(640);//640^3
                this->MapperGPURaycast->SetInternalVolumeSize(640);//640^3
                this->MapperTexture->SetInternalVolumeSize(512);
                break;
            }
        }
        
        this->FrameCPURayCasting->CollapseFrame();
        this->FrameCUDARayCasting->CollapseFrame();
        this->FrameGPURayCasting->CollapseFrame();
        this->FrameGPURayCastingII->CollapseFrame();
        this->FramePolygonBlending->CollapseFrame();
            
        if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","MB_Mapper"))
        {
            int id = this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","MB_Mapper");
            this->MB_Mapper->GetWidget()->GetMenu()->SelectItem(id);
            
            switch(id)
            {
            case 0:
                this->Volume->SetMapper(this->MapperRaycast);
                this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CPU Raycasting: High Quality");
                this->FrameCPURayCasting->ExpandFrame();
                break;
            case 1:
                this->Volume->SetMapper(this->MapperGPURaycast);
                if (this->IsGPURayCastingSupported)
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using GPU Raycasting");
                else
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("GPU Raycasting not supported on your computer!");
                this->FrameGPURayCasting->ExpandFrame();
                break;
            case 2:
                this->Volume->SetMapper(this->MapperGPURaycastII);
                if (this->IsGPURayCastingSupported)
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using GPU Raycasting II");
                else
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("GPU Raycasting not supported on your computer!");
                this->FrameGPURayCastingII->ExpandFrame();
                break;
            case 3:
                this->Volume->SetMapper(this->MapperTexture);
                if (this->IsTextureMappingSupported)
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using OpenGL Polygon Texture 3D");
                else
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("OpenGL Polygon Texture 3D not supported on your computer!");
                this->FramePolygonBlending->ExpandFrame();
                break;
            case 4:
                this->Volume->SetMapper(this->MapperCUDARaycast);
                if (this->IsCUDARayCastingSupported)
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CUDA");
                else
                    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("CUDA not supported on your computer!");
                this->FrameCUDARayCasting->ExpandFrame();
                break;
            }
        }
        else//first time enable CPU ray casting
        {
            this->MB_Mapper->GetWidget()->GetMenu()->SelectItem(0);
            this->Volume->SetMapper(this->MapperRaycast);
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CPU Raycasting: High Quality");
            this->FrameCPURayCasting->ExpandFrame();
        }
    }
    
    //TODO This is not the right place for this
    this->Volume->SetProperty(this->Gui->GetCurrentNode()->GetVolumeProperty());
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    //For Performance
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->AddViewProp(this->Volume);
    matrix->Delete();
    
    //Render
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::UpdateRendering()
{
    //first check if REndering was already called
    if(this->Volume == NULL)
    {
        this->Rendering();
        return;
    }
    
    //Update mapper
    vtkImageData *input= vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
        
    if (this->MapperCUDARaycast->GetInput() != input)
      this->MapperCUDARaycast->SetInput(input);
    
    if (this->MapperGPURaycast->GetInput() != input)
      this->MapperGPURaycast->SetInput(input);
    
    if (this->MapperRaycast->GetInput() != input)
      this->MapperRaycast->SetInput(input);
    
    if (this->MapperTexture->GetInput() != input)
      this->MapperTexture->SetInput(input);
      
    if (this->MapperGPURaycastII->GetNthInput(0) != input)
        this->MapperGPURaycastII->SetNthInput(0, input);
        
    {
        if (this->Gui->GetNS_ImageDataBg()->GetSelected())
        {
            input = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageDataBg()->GetSelected())->GetImageData();
            if (input != this->MapperGPURaycastII->GetNthInput(1))
                this->MapperGPURaycastII->SetNthInput(1, input);
        }
        if (this->Gui->GetNS_ImageDataLabelmap()->GetSelected())
        {
            input = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageDataLabelmap()->GetSelected())->GetImageData();
            if (input != this->MapperGPURaycastII->GetNthInput(2))
                this->MapperGPURaycastII->SetNthInput(2, input);
        }
    }
    
    //Update Property
    this->Volume->SetProperty(this->Gui->GetCurrentNode()->GetVolumeProperty());
    //Update matrix
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    this->Volume->PokeMatrix(matrix);
    matrix->Delete();
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData)
{
    //clipping box widget
    {
        vtkSlicerBoxWidget2 *callerBox=vtkSlicerBoxWidget2::SafeDownCast(caller);
        if(callerBox && caller==this->BW_Clipping_Widget && eid==vtkCommand::InteractionEvent)
        {
            vtkPlanes *planes=vtkPlanes::New();
            this->BW_Clipping_Representation->GetPlanes(planes);
            this->MapperTexture->SetClippingPlanes(planes);
            this->MapperRaycast->SetClippingPlanes(planes);

            this->MapperCUDARaycast->SetClippingPlanes(planes);
            this->MapperCUDARaycast->ClippingOn();

            this->MapperGPURaycast->SetClippingPlanes(planes);
            this->MapperGPURaycast->ClippingOn();
        
            planes->Delete();
            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            return;
        }
        
        if(callerBox && caller==this->BW_Clipping_Widget && eid==vtkCommand::EndInteractionEvent)
        {
            vtkPlanes *planes=vtkPlanes::New();
            this->BW_Clipping_Representation->GetPlanes(planes);
            this->MapperTexture->SetClippingPlanes(planes);
            this->MapperRaycast->SetClippingPlanes(planes);

            this->MapperCUDARaycast->SetClippingPlanes(planes);
            this->MapperCUDARaycast->ClippingOn();
    
            this->MapperGPURaycast->SetClippingPlanes(planes);
            this->MapperGPURaycast->ClippingOn();
            
            //Decide if this event is triggered by the vtkBoxWidget or by the sliders
            //if sliders don't trigger setRange->this would lead to an endless loop
            if(!this->NoSetRangeNeeded)
            {
                vtkPolyData *vertices=vtkPolyData::New();

                this->BW_Clipping_Representation->GetPolyData(vertices);
                double pointA[3];
                double pointB[3];
                vertices->GetPoint(0,pointA);
                vertices->GetPoint(6,pointB);
                
                //Include a possible transform in this calculation
                this->InverseAdditionalClippingTransform->TransformPoint(pointA,pointA);
                this->InverseAdditionalClippingTransform->TransformPoint(pointB,pointB);
                
                //Convert this stuff into the box coordinate system
                this->ConvertWorldToBoxCoordinates(pointA);
                this->ConvertWorldToBoxCoordinates(pointB);

                double pointSmallestValue[3];
                double pointHighestValue[3];

                for(int i = 0; i < 3; i++)
                {
                    if(pointA[i] > pointB[i])
                    {
                        pointSmallestValue[i]=pointB[i];
                        pointHighestValue[i]=pointA[i];
                    }
                    else
                    {
                        pointSmallestValue[i]=pointA[i];
                        pointHighestValue[i]=pointB[i];
                    }
    
                    double valueLeft=this->VolumeBoundariesBoxCoordinates[0][i];
                    if(pointSmallestValue[i] < this->VolumeBoundariesBoxCoordinates[0][i])
                        valueLeft=pointSmallestValue[i];
    
                    double valueRight=this->VolumeBoundariesBoxCoordinates[1][i];
                    if(pointHighestValue[i] > this->VolumeBoundariesBoxCoordinates[1][i])
                    {
                        valueRight=pointHighestValue[i];
                    }
                    
                    this->RA_Cropping[i]->SetWholeRange(valueLeft,valueRight);          
                    this->RA_Cropping[i]->SetRange(pointA[i],pointB[i]);
                }

                vertices->Delete();
            }
            
            planes->Delete();
            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            return;
        }
    }
        
    //Check the checkbuttons
    {
        vtkKWCheckButton *callerObjectCheckButton = vtkKWCheckButton::SafeDownCast(caller);
    
        if(callerObjectCheckButton == this->CB_CPURayCastMIP->GetWidget())
        {
            if (this->CB_CPURayCastMIP->GetWidget()->GetSelectedState())
                this->MapperRaycast->SetBlendModeToMaximumIntensity();
            else
                this->MapperRaycast->SetBlendModeToComposite();

            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            return;
        }
    
        if(callerObjectCheckButton == this->CB_CUDARayCastShading->GetWidget())
        {
            if (this->CB_CUDARayCastShading->GetWidget()->GetSelectedState())
                this->MapperCUDARaycast->ShadingOn();
            else
                this->MapperCUDARaycast->ShadingOff();

            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            return;
        }
    }
    
    //framerate
    {
        vtkKWScale *callerObjectSC=vtkKWScale::SafeDownCast(caller);
        if(callerObjectSC == this->SC_ExpectedFPS)
        {
            ProcessExpectedFPS();
            return;
        }
        
        if(callerObjectSC == this->SC_GPURayCastDepthPeelingThreshold)
        {
            this->MapperGPURaycast->SetDepthPeelingThreshold(this->SC_GPURayCastDepthPeelingThreshold->GetValue());
            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            return;
        }
    }
    
    //SVP
    {
        vtkSlicerVolumePropertyWidget *callerObjectSVP=vtkSlicerVolumePropertyWidget::SafeDownCast(caller);
        if(callerObjectSVP == this->SVP_VolumeProperty && eid == vtkKWEvent::VolumePropertyChangingEvent)
        {
            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            return;
        }
    }
    
    //check abort
    if(caller == this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow() && eid == vtkCommand::AbortCheckEvent)
    {
        this->CheckAbort();
        return;
    }
    
    {//clipping
        vtkSlicerNodeSelectorWidget *callerNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
        if (eid==vtkSlicerNodeSelectorWidget::NodeSelectedEvent&&callerNS==this->NS_TransformNode)
        {
            //Remove a potential observer from the old node
            if(this->CurrentTransformNodeCropping!=NULL)
                this->CurrentTransformNodeCropping->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,this->VolumeRenderingCallbackCommand);
    
            //Add an observer to the new node
            this->CurrentTransformNodeCropping=vtkMRMLLinearTransformNode::SafeDownCast(this->NS_TransformNode->GetSelected());
            if(this->CurrentTransformNodeCropping!=NULL)
                this->CurrentTransformNodeCropping->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,this->VolumeRenderingCallbackCommand);
    
            this->ProcessClippingModified();
            return;
        }
        if(eid==vtkMRMLTransformableNode::TransformModifiedEvent)
        {
            this->ProcessClippingModified();
            return;        
        }
    }
        
    vtkSlicerVRMenuButtonColorMode *callerVRMB=vtkSlicerVRMenuButtonColorMode::SafeDownCast(caller);
    if(callerVRMB==this->VRMB_ColorMode&&eid==vtkSlicerVRMenuButtonColorMode::ColorModeChangedEvent)
    {
        this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    }

    vtkRenderWindow *renderWindow = vtkRenderWindow::SafeDownCast(caller);
    if (renderWindow && eid==vtkCommand::EndEvent)
      {
      char buf[32] = "Done";
      this->Gui->GetApplicationGUI()->SetExternalProgress(buf, 0.);
      return;
      }     
    
    //Check if caller equals mapper
    //All other event catchers before
    
    vtkAbstractMapper *callerMapper=vtkAbstractMapper::SafeDownCast(caller);

    if((this->Volume != NULL) && (callerMapper != this->Volume->GetMapper()))
      {
      return;
      }

    if(eid==vtkCommand::VolumeMapperComputeGradientsStartEvent)
      {
//      this->DisplayProgressDialog("Please standby: Gradients are being calculated");
      return;
      }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsEndEvent)
      {
//      this->WithdrawProgressDialog();
      return;
      }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsProgressEvent)
      {
      float *progress = (float*)callData;
      char buf[32] = "Gradient...";
      this->Gui->GetApplicationGUI()->SetExternalProgress(buf, *progress);
      
//      if(this->GradientDialog != NULL)
//        {
//        this->GradientDialog->UpdateProgress(*progress);
//        }
      return;
      }
    else if (eid==vtkCommand::ProgressEvent)
      {
      float progress=*((float*)callData);
      char buf[32] = "Rendering...";
      this->Gui->GetApplicationGUI()->SetExternalProgress(buf, progress);
      return;
      }     
    
    vtkDebugMacro("observed event but didn't process it");
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
    if(this->Gui->GetCurrentNode() && this->SVP_VolumeProperty->GetVolumeProperty()==this->Gui->GetCurrentNode()->GetVolumeProperty())
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
        if (this->Gui->GetCurrentNode())
          this->UpdateingGUI = 1;
          {
          double *croppingPlanes = this->Gui->GetCurrentNode()->GetCroppingRegionPlanes();
          for(int i=0;i<3;i++)
            {
            if(croppingPlanes[2*i  ]< croppingPlanes[2*i+1])
              {
              this->RA_Cropping[i]->SetRange(croppingPlanes[2*i], croppingPlanes[2*i+1]);
              }
            else
              {
              this->RA_Cropping[i]->SetRange(croppingPlanes[2*i+1], croppingPlanes[2*i]);
              }
            }
          this->UpdateingGUI = 0;
          }

        this->CB_Cropping->GetWidget()->SetSelectedState(this->Gui->GetCurrentNode()->GetCroppingEnabled());
        this->ProcessEnableDisableCropping(this->Gui->GetCurrentNode()->GetCroppingEnabled());
    }
    
    this->SVP_VolumeProperty->SetVolumeProperty(this->Gui->GetCurrentNode()->GetVolumeProperty());
    this->SVP_VolumeProperty->SetHSVColorSelectorVisibility(1);
    this->SVP_VolumeProperty->Update();
}

void vtkSlicerVRGrayscaleHelper::UpdateGUIElements(void)
{
    this->UpdateingGUI = 1;
    Superclass::UpdateGUIElements();
    
    this->UpdateSVP();
    
    double *croppingPlanes = this->Gui->GetCurrentNode()->GetCroppingRegionPlanes();
    for(int i = 0; i < 3; i++)
    {
        if(croppingPlanes[ 2*i ]< croppingPlanes[ 2*i+1 ])
        {
            this->RA_Cropping[i]->SetRange(croppingPlanes[2*i], croppingPlanes[2*i+1]);
        }
        else
        {
            this->RA_Cropping[i]->SetRange(croppingPlanes[2*i+1], croppingPlanes[2*i]);
        }
    }

    this->CB_Cropping->GetWidget()->SetSelectedState(this->Gui->GetCurrentNode()->GetCroppingEnabled());
    this->VRMB_ColorMode->SetColorTransferFunction(this->Gui->GetCurrentNode()->GetVolumeProperty()->GetRGBTransferFunction());
    this->VRMB_ColorMode->SetRange(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetPointData()->GetScalars()->GetRange());
    //this->ColorDisplay->SetMRMLScene(this->Gui->GetMRMLScene());

    this->UpdateingGUI = 0;
    this->ProcessCropping(0,0,0);
    this->ProcessEnableDisableCropping(this->Gui->GetCurrentNode()->GetCroppingEnabled());
}

void vtkSlicerVRGrayscaleHelper::AdjustMapping()
{
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

void vtkSlicerVRGrayscaleHelper::ProcessCropping(int index, double min,double max)
{
    if (this->UpdateingGUI)
        return;

    if(this->MapperTexture==NULL||this->MapperRaycast==NULL||this->MapperGPURaycast == NULL||this->MapperCUDARaycast==NULL)
        return;

    double pointA[3];
    double pointB[3];
    double croppingPlanes[6];

    for(int i = 0; i < 3; i++)
    {
        pointA[i]=this->RA_Cropping[i]->GetRange()[0];
        pointB[i]=this->RA_Cropping[i]->GetRange()[1];
        croppingPlanes[2*i  ] = this->RA_Cropping[i]->GetRange()[0];
        croppingPlanes[2*i+1] = this->RA_Cropping[i]->GetRange()[1];
    }

    this->Gui->GetCurrentNode()->SetCroppingRegionPlanes(croppingPlanes);

    this->ConvertBoxCoordinatesToWorld(pointA);
    this->ConvertBoxCoordinatesToWorld(pointB);
    if (this->BW_Clipping_Widget)
    {
        double pointPlace[6];
        pointPlace[0] = pointA[0];
        pointPlace[1] = pointB[0];
        pointPlace[2] = pointA[1];
        pointPlace[3] = pointB[1];
        pointPlace[4] = pointA[2];
        pointPlace[5] = pointB[2];
        this->BW_Clipping_Representation->PlaceWidget(pointPlace); //pointA[0],pointB[0],pointA[1],pointB[1],pointA[2],pointB[2]);
        this->BW_Clipping_Representation->SetTransform(this->AdditionalClippingTransform);
    }
    
    this->NoSetRangeNeeded = 1;
    this->ProcessVolumeRenderingEvents(this->BW_Clipping_Widget,vtkCommand::InteractionEvent,0);
    this->ProcessVolumeRenderingEvents(this->BW_Clipping_Widget,vtkCommand::EndInteractionEvent,0);
    this->NoSetRangeNeeded = 0;
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    this->CalculateBoxCoordinatesBoundaries();
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
    croppingFrame->SetLabelText("Cropping (Coordinates are relative to image origin)");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", croppingFrame->GetWidgetName() );

    int labelwidth=20;
    
    this->CB_Cropping=vtkKWCheckButtonWithLabel::New();
    this->CB_Cropping->SetParent(croppingFrame->GetFrame());
    this->CB_Cropping->Create();
    this->CB_Cropping->GetWidget()->SetSelectedState(0);
    this->CB_Cropping->SetBalloonHelpString("Enable/ Disable the configured clipping planes.");
    this->CB_Cropping->SetLabelText("Clipping in general");
    this->CB_Cropping->SetLabelWidth(labelwidth);
    this->CB_Cropping->GetWidget()->SetCommand(this, "ProcessEnableDisableCropping");
    this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10", this->CB_Cropping->GetWidgetName());
    
    this->CB_Clipping=vtkKWCheckButtonWithLabel::New();
    this->CB_Clipping->SetParent(croppingFrame->GetFrame());
    this->CB_Clipping->Create();
    this->CB_Clipping->SetBalloonHelpString("Display or suppress the clipping box. The configured clipping planes will still be enabled");
    this->CB_Clipping->GetWidget()->SetSelectedState(0);
    this->CB_Clipping->SetLabelText("Display Clipping Box");
    this->CB_Clipping->SetLabelWidth(labelwidth);
    this->CB_Clipping->GetWidget()->SetCommand(this, "ProcessDisplayClippingBox");
    this->Script("pack %s -side top -anchor nw -fill x -padx 10 -pady 10", this->CB_Clipping->GetWidgetName());

    for(int i = 0; i < 3; i++)
    {
        this->RA_Cropping[i]=vtkKWRange::New();
        this->RA_Cropping[i]->SetParent(croppingFrame->GetFrame());
        this->RA_Cropping[i]->Create();
        this->RA_Cropping[i]->SetEnabled(0);
        this->RA_Cropping[i]->SetBalloonHelpString("Configure the clipping planes relative to the center of the volume. You can also use the clipping box to do this.");
        this->RA_Cropping[i]->SymmetricalInteractionOff();
        std::stringstream str;
        str<<"ProcessCropping "<<i;
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

    //TODO this should be fixed after inclusion into KWWidgets
    this->ProcessConfigureCallback();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->SetBinding("<Configure>", this, "ProcessConfigureCallback");
}

void vtkSlicerVRGrayscaleHelper::CreateThreshold()
{
    vtkKWFrameWithLabel *thresholdFrame=vtkKWFrameWithLabel::New();
    thresholdFrame->SetParent(this->NB_Details->GetFrame("Threshold"));
    thresholdFrame->Create();
    thresholdFrame->SetLabelText("Threshold");
    this->Script("pack %s -side top -anchor nw -fill both -expand yes -padx 0 -pady 2", thresholdFrame->GetWidgetName());

    this->MB_ThresholdMode=vtkKWMenuButtonWithLabel::New();
    this->MB_ThresholdMode->SetParent(thresholdFrame->GetFrame());
    this->MB_ThresholdMode->Create();
    std::stringstream ss;
    ss<<"Select which kind of thresholding to use for transfer functions. ";
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
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_ThresholdMode->GetWidgetName());

    this->VRMB_ColorMode=vtkSlicerVRMenuButtonColorMode::New();
    this->VRMB_ColorMode->SetParent(thresholdFrame->GetFrame());
    this->VRMB_ColorMode->Create();
    ss.str("");
    ss<<"Which color should the volume have, ";
    ss<<"all \"static\" colors result in a single color for every gray value while \"dynamic\" produces a color gradient.";
    this->VRMB_ColorMode->SetBalloonHelpString(ss.str().c_str());
    this->VRMB_ColorMode->SetLabelText("Color Mode:"); 
    this->VRMB_ColorMode->SetLabelWidth(10);
    this->VRMB_ColorMode->EnabledOff();
    this->VRMB_ColorMode->AddObserver(vtkSlicerVRMenuButtonColorMode::ColorModeChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->VRMB_ColorMode->GetWidgetName());

    this->RA_RampRectangleOpacity=vtkKWRange::New();
    this->RA_RampRectangleOpacity->SetParent(thresholdFrame->GetFrame());
    this->RA_RampRectangleOpacity->Create();
    ss.str("");
    ss<<"Set the lower and upper opacity levels for the ramp and the rectangle. ";
    ss<<"Lower values make the volume more translucent.";
    this->RA_RampRectangleOpacity->SetBalloonHelpString(ss.str().c_str());
    this->RA_RampRectangleOpacity->SetLabelText("Opacity");
    this->RA_RampRectangleOpacity->SetWholeRange(0,1);
    this->RA_RampRectangleOpacity->SetRange(0,1);
    this->RA_RampRectangleOpacity->EnabledOff();
    this->RA_RampRectangleOpacity->SetCommand(this, "ProcessThresholdRange");
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->RA_RampRectangleOpacity->GetWidgetName());
    
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    this->RA_RampRectangleScalar=vtkKWRange::New();
    this->RA_RampRectangleScalar->SetParent(thresholdFrame->GetFrame());
    this->RA_RampRectangleScalar->Create();
    this->RA_RampRectangleScalar->SetBalloonHelpString("Apply thresholds to the gray values of volume.");
    this->RA_RampRectangleScalar->SetLabelText("Threshold");
    this->RA_RampRectangleScalar->SetWholeRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->RA_RampRectangleScalar->SetRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->RA_RampRectangleScalar->SetCommand(this, "ProcessThresholdRange");
    this->RA_RampRectangleScalar->EnabledOff();
    this->Script("pack %s -side left -anchor nw -expand n -fill x -padx 2 -pady 2", this->RA_RampRectangleScalar->GetWidgetName());

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
    this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2", this->PB_ThresholdZoomIn->GetWidgetName());

    this->PB_Reset=vtkKWPushButton::New();
    this->PB_Reset->SetParent(thresholdFrame->GetFrame());
    this->PB_Reset->Create();
    this->PB_Reset->SetBalloonHelpString("Cancel \"Zoom In\".");
    this->PB_Reset->SetText("Reset");
    this->PB_Reset->EnabledOff();
    this->PB_Reset->SetCommand(this,"ProcessThresholdReset");
    this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2", this->PB_Reset->GetWidgetName());

    thresholdFrame->Delete();
}

void vtkSlicerVRGrayscaleHelper::ProcessGPURayCastTechnique(int id)
{
    this->MapperGPURaycast->SetTechnique(id);
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessGPURayCastTechniqueII(int id)
{
    //this->MapperGPURaycast->SetTechnique(id);
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessGPURayCastColorOpacityFusion(int id)
{
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessGPUMemorySize(int id)
{
    switch(id)
    {
    case 0://128M
        this->MapperGPURaycast->SetInternalVolumeSize(256);//256^3
        this->MapperTexture->SetInternalVolumeSize(256);
        break;
    case 1://256M
        this->MapperGPURaycast->SetInternalVolumeSize(384);//384^3
        this->MapperTexture->SetInternalVolumeSize(256);
        break;
    case 2://512M
        this->MapperGPURaycast->SetInternalVolumeSize(512);//500^3
        this->MapperTexture->SetInternalVolumeSize(512);
        break;
    case 3://1024M
        this->MapperGPURaycast->SetInternalVolumeSize(640);//600^3
        this->MapperTexture->SetInternalVolumeSize(512);
        break;
    }
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessRenderingMethodEvents(int id)
{
    this->RenderingMethod = id;
  
    this->FrameCUDARayCasting->CollapseFrame();
    this->FrameGPURayCasting->CollapseFrame();
    this->FrameGPURayCastingII->CollapseFrame();
    this->FramePolygonBlending->CollapseFrame();
    this->FrameCPURayCasting->CollapseFrame();
    
    switch(id)
    {
    case 0://softwrae ray casting
      this->FrameCPURayCasting->ExpandFrame();
      if (this->Volume)
      {
        this->Volume->SetMapper(this->MapperRaycast);
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CPU Raycasting");
      }
      break;
    case 1://gpu ray casting
      this->FrameGPURayCasting->ExpandFrame();
      if (this->Volume && this->IsGPURayCastingSupported)
      {
        this->Volume->SetMapper(this->MapperGPURaycast);
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using GPU Raycasting");
      }
      else
        vtkErrorMacro("GPU ray casting is not supported by your computer.");
      break;
    case 2://gpu ray casting II
/*      this->FrameGPURayCastingII->ExpandFrame();
      if (this->Volume && this->IsGPURayCastingSupported)
      {
        this->Volume->SetMapper(this->MapperGPURaycastII);
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using GPU Raycasting II");
      }
      else
        vtkErrorMacro("GPU ray casting is not supported by your computer.");*/
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Under construction");  
      break;
    case 3://old school opengl 2D Polygon Texture 3D
      this->FramePolygonBlending->ExpandFrame();
      if (this->Volume && this->IsTextureMappingSupported)
      {
        this->Volume->SetMapper(this->MapperTexture);
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using OpenGL Polygon Texture 3D");
      }
      else
        vtkErrorMacro("OpenGL Polygon Texture 3D is not supported by your computer.");//seldom should we see this error message unless really low end graphics card...
      break;
    case 4://CUDA ray casting
      this->FrameCUDARayCasting->ExpandFrame();
      this->FrameFPS->ExpandFrame();
      if (this->Volume && this->IsCUDARayCastingSupported){
        this->Volume->SetMapper(this->MapperCUDARaycast);
        vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
        vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
        this->MapperCUDARaycast->SetOrientationMatrix(matrix);
        matrix->Delete();
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CUDA");
      }else{
        vtkErrorMacro("CUDA is not supported by your computer.");
      }
      break;
    }
    
    //update expected framerate
    this->MapperTexture->SetFramerate(this->SC_ExpectedFPS->GetValue());
    this->MapperCUDARaycast->SetIntendedFrameRate(this->SC_ExpectedFPS->GetValue());
    this->MapperGPURaycast->SetFramerate(this->SC_ExpectedFPS->GetValue());
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();//double rendering request to force mapper to adjust rendering quality for expected fps
    
    this->NB_Details->Resize();
}

void vtkSlicerVRGrayscaleHelper::ProcessThresholdModeEvents(int id)
{
    this->ThresholdMode = id;
    
    //Disable Everything and go back to standard
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    if(id == 0)
    {
        this->VRMB_ColorMode->EnabledOff();

        this->RA_RampRectangleScalar->SetRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
        this->RA_RampRectangleScalar->EnabledOff();
        this->RA_RampRectangleOpacity->SetRange(0,1);
        this->RA_RampRectangleOpacity->EnabledOff();
        this->PB_Reset->EnabledOff();
        this->PB_ThresholdZoomIn->EnabledOff();
        return;
    }
    
    //Before we continue enabled everything
    this->VRMB_ColorMode->EnabledOn();
    this->RA_RampRectangleScalar->EnabledOn();
    this->RA_RampRectangleOpacity->EnabledOn();
    this->PB_Reset->EnabledOn();
    this->PB_ThresholdZoomIn->EnabledOn();
    this->ProcessThresholdRange(.0,.0);
}

void vtkSlicerVRGrayscaleHelper::ProcessThresholdRange(double notUsed,double notUsedA)
{
    if(this->ThresholdMode == 0)
    {
        return;
    }
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    //Delete all old Mapping Points
    vtkPiecewiseFunction *opacity=this->Gui->GetCurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    opacity->RemoveAllPoints();
    //opacity->AdjustRange(iData->GetScalarRange());

    opacity->AddPoint(iData->GetScalarRange()[0],this->RA_RampRectangleOpacity->GetRange()[0]);
    opacity->AddPoint(iData->GetScalarRange()[1],this->RA_RampRectangleOpacity->GetRange()[0]);
    if(this->ThresholdMode==1)
    {
        opacity->AddPoint(iData->GetScalarRange()[0],this->RA_RampRectangleOpacity->GetRange()[0]);
        opacity->AddPoint(iData->GetScalarRange()[1],this->RA_RampRectangleOpacity->GetRange()[1]);
        opacity->AddPoint(this->RA_RampRectangleScalar->GetRange()[0],this->RA_RampRectangleOpacity->GetRange()[0]);
        opacity->AddPoint(this->RA_RampRectangleScalar->GetRange()[1],this->RA_RampRectangleOpacity->GetRange()[1]);

    }
    else if(this->ThresholdMode==2)
    {

        opacity->AddPoint(iData->GetScalarRange()[0],this->RA_RampRectangleOpacity->GetRange()[0]);
        opacity->AddPoint(iData->GetScalarRange()[1],this->RA_RampRectangleOpacity->GetRange()[0]);

        opacity->AddPoint(this->RA_RampRectangleScalar->GetRange()[0],this->RA_RampRectangleOpacity->GetRange()[0]);
        opacity->AddPoint(this->RA_RampRectangleScalar->GetRange()[0]+0.1,this->RA_RampRectangleOpacity->GetRange()[1]);
        opacity->AddPoint(this->RA_RampRectangleScalar->GetRange()[1]-0.1,this->RA_RampRectangleOpacity->GetRange()[1]);
        opacity->AddPoint(this->RA_RampRectangleScalar->GetRange()[1],this->RA_RampRectangleOpacity->GetRange()[0]);

    }
    this->SVP_VolumeProperty->Update();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessThresholdZoomIn(void)
{
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    double leftBorder=this->RA_RampRectangleScalar->GetRange()[0];
    double rightBorder=this->RA_RampRectangleScalar->GetRange()[1];
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
    this->RA_RampRectangleScalar->SetWholeRange(leftBorderWholeRange,rightBorderWholeRange);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetVisibleParameterRange(leftBorderWholeRange,rightBorderWholeRange);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetVisibleParameterRange(leftBorderWholeRange,rightBorderWholeRange);
}

void vtkSlicerVRGrayscaleHelper::ProcessThresholdReset(void)
{
    vtkImageData *iData=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();

    this->RA_RampRectangleScalar->SetWholeRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetVisibleParameterRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetVisibleParameterRange(iData->GetScalarRange()[0],iData->GetScalarRange()[1]);
}

void vtkSlicerVRGrayscaleHelper::ProcessEnableDisableCropping(int cbSelectedState)
{
    if(this->MapperTexture==NULL||this->MapperRaycast==NULL||this->MapperGPURaycast == NULL||this->MapperCUDARaycast == NULL)
        return;
    
    for(int i = 0; i < 3; i++)
    {
        this->RA_Cropping[i]->SetEnabled(cbSelectedState);
    }
    
    this->NS_TransformNode->SetEnabled(cbSelectedState);
    //There is not automatical enabling when
    //this->ProcessDisplayClippingBox(cbSelectedState);
    this->CB_Clipping->SetEnabled(cbSelectedState);
    if(this->Gui->GetCurrentNode()!=NULL)
    {
        this->Gui->GetCurrentNode()->SetCroppingEnabled(cbSelectedState);
    }
    
    if(cbSelectedState)
    {
        //If we enable clipping we choose the current state of CB_Clipping
        this->ProcessDisplayClippingBox(this->CB_Clipping->GetWidget()->GetSelectedState());
        this->ProcessCropping(0,0,0);
        //PlaceWidget

        this->MapperCUDARaycast->ClippingOn();
        this->MapperGPURaycast->ClippingOn();
    }
    else
    {
        this->ProcessDisplayClippingBox(0);
        this->MapperTexture->RemoveAllClippingPlanes();
        this->MapperRaycast->RemoveAllClippingPlanes();
        this->MapperGPURaycast->RemoveAllClippingPlanes();
        this->MapperGPURaycast->ClippingOff();
        this->MapperCUDARaycast->ClippingOff();
    }

    //Trigger a Render
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessDisplayClippingBox(int clippingEnabled)
{
    if(this->BW_Clipping_Widget == NULL)
    {
        this->Gui->GetApplicationGUI()->GetViewerWidget()->SetBoxWidgetInteractor();
        this->BW_Clipping_Widget = this->Gui->GetApplicationGUI()->GetViewerWidget()->GetBoxWidget();
        this->BW_Clipping_Representation = this->Gui->GetApplicationGUI()->GetViewerWidget()->GetBoxWidgetRepresentation();
        this->BW_Clipping_Representation->SetPlaceFactor(1);
      
        // this->BW_Clipping_Widget->SetProp3D(this->Volume);
        // get the bounding box of the volume
        double *bounds = this->Volume->GetBounds();
        if (bounds != NULL)
        {
            this->BW_Clipping_Representation->PlaceWidget(bounds);
        }
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
        
        for(int i = 0; i < 3; i++)
        {
            if(pointA[i] < pointB[i])
                this->RA_Cropping[i]->SetRange(pointA[i],pointB[i]);
            else
                this->RA_Cropping[i]->SetRange(pointB[i],pointA[i]);
        }

        double pointPlace[6];
        pointPlace[0] = pointA[0];
        pointPlace[1] = pointB[0];
        pointPlace[2] = pointA[0]; // is this correct?
        pointPlace[3] = pointB[1];
        pointPlace[4] = pointA[2];
        pointPlace[5] = pointB[2];
        
        this->BW_Clipping_Representation->PlaceWidget(pointPlace); //pointA[0],pointB[0],pointA[0],pointB[1],pointA[2],pointB[2]);
        this->BW_Clipping_Representation->InsideOutOn();
        this->BW_Clipping_Widget->RotationEnabledOff();
        this->BW_Clipping_Widget->TranslationEnabledOn();
        this->BW_Clipping_Representation->GetSelectedHandleProperty()->SetColor(0.2,0.6,0.15);
        this->NoSetRangeNeeded=0;

        this->BW_Clipping_Widget->AddObserver(vtkCommand::InteractionEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);
        this->BW_Clipping_Widget->AddObserver(vtkCommand::EndInteractionEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetInteractor()->ReInitialize();
    }
    
    if(clippingEnabled)
        this->BW_Clipping_Widget->On();
    else
        this->BW_Clipping_Widget->Off();
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::ProcessPauseResume(void)
{
    //Resume Rendering
    if(this->RenderingPaused)
    {
        this->RenderingPaused=0;
        this->Volume->VisibilityOn();
        this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
        this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());;
    }
    //Pause Rendering
    else if (!this->RenderingPaused)
    {
        this->RenderingPaused=1;
 
        this->Volume->VisibilityOff();
        this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
        this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetInvisibleIcon());
    }
    else
    {
        vtkErrorMacro("RenderingPaused is not a valid number");
    }
    
    this->Script("put \"ProcessPauseResume\"");
}

void vtkSlicerVRGrayscaleHelper::DestroyCropping(void)
{
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->RemoveBinding("<Configure>", this, "ProcessConfigureCallback");
    
    if(this->BW_Clipping_Widget)
    {
        this->BW_Clipping_Widget->RemoveObservers(vtkCommand::InteractionEvent,(vtkCommand*) this->VolumeRenderingCallbackCommand);
        this->BW_Clipping_Widget->RemoveObservers(vtkCommand::EndInteractionEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->BW_Clipping_Widget->Off();
        this->BW_Clipping_Widget=NULL;
    }
    if (this->BW_Clipping_Representation)
      {
        this->BW_Clipping_Representation =  NULL;
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
    if(this->VRMB_ColorMode)
    {
        this->VRMB_ColorMode->RemoveObservers(vtkSlicerVRMenuButtonColorMode::ColorModeChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->VRMB_ColorMode->SetParent(NULL);
        this->VRMB_ColorMode->Delete();
        this->VRMB_ColorMode=NULL;
    }
    if(this->RA_RampRectangleScalar)
    {
        this->RA_RampRectangleScalar->SetParent(NULL);
        this->RA_RampRectangleScalar->Delete();
        this->RA_RampRectangleScalar=NULL;
    }
    if(this->RA_RampRectangleOpacity)
    {
        this->RA_RampRectangleOpacity->SetParent(NULL);
        this->RA_RampRectangleOpacity->Delete();
        this->RA_RampRectangleOpacity=NULL;
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

void vtkSlicerVRGrayscaleHelper::ProcessExpectedFPS(void)
{
    this->MapperTexture->SetFramerate(this->SC_ExpectedFPS->GetValue());
    this->MapperCUDARaycast->SetIntendedFrameRate(this->SC_ExpectedFPS->GetValue());
    this->MapperGPURaycast->SetFramerate(this->SC_ExpectedFPS->GetValue());
    
    //CPU ray casting framerate is handled in SetupCPURayCastInteractive()
    
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVRGrayscaleHelper::CreatePerformance(void)
{
    this->FramePerformance = vtkKWFrameWithLabel::New();
    this->FramePerformance->SetParent(this->NB_Details->GetFrame("Performance"));
    this->FramePerformance->Create();
    this->FramePerformance->AllowFrameToCollapseOff();
    this->FramePerformance->SetLabelText("Performance/Quality");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FramePerformance->GetWidgetName() );
    
    int labelWidth = 20;
    
    //mapper selection combobox
    {
        this->MB_Mapper = vtkKWMenuButtonWithLabel::New();
        this->MB_Mapper->SetParent(this->FramePerformance->GetFrame());
        this->MB_Mapper->SetLabelText("Rendering Method");
        this->MB_Mapper->Create();
        this->MB_Mapper->SetLabelWidth(labelWidth);
        this->MB_Mapper->SetBalloonHelpString("Please select one rendering method");
        this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("Software Ray Casting");
        this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessRenderingMethodEvents 0");
        this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("GPU Ray Casting (GLSL)");
        this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessRenderingMethodEvents 1");
        this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("GPU Ray Casting II (GLSL)");
        this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessRenderingMethodEvents 2");
        this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("OpenGL Polygon Texture 3D");
        this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessRenderingMethodEvents 3");
        this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("CUDA Ray Casting");
        this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(4, this,"ProcessRenderingMethodEvents 4");
        
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_Mapper->GetWidgetName() );

    }
    
    //GPU memory size
    {
        this->MB_GPUMemorySize = vtkKWMenuButtonWithLabel::New();
        this->MB_GPUMemorySize->SetParent(this->FramePerformance->GetFrame());
        this->MB_GPUMemorySize->SetLabelText("GPU Memory Size");
        this->MB_GPUMemorySize->Create();
        this->MB_GPUMemorySize->SetLabelWidth(labelWidth);
        this->MB_GPUMemorySize->SetBalloonHelpString("Specify size of your GPU memory. Generally the larger GPU memory the better rendering quality. Do not select memory size larger than physical GPU memory size.");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("128M");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPUMemorySize 0");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("256M");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPUMemorySize 1");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("512M");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPUMemorySize 2");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("1024M");
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPUMemorySize 3");        
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPUMemorySize->GetWidgetName() );
        
        this->MB_GPUMemorySize->GetWidget()->SetValue("256M");
    }
    
    //Framerate
    {
        this->FrameFPS = vtkKWFrameWithLabel::New();
        this->FrameFPS->SetParent(this->FramePerformance->GetFrame());
        this->FrameFPS->Create();
        this->FrameFPS->AllowFrameToCollapseOff();
        this->FrameFPS->SetLabelText("Expected Framerate (FPS)");
        this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameFPS->GetWidgetName() );
    
        this->SC_ExpectedFPS=vtkKWScale::New();
        this->SC_ExpectedFPS->SetParent(this->FrameFPS->GetFrame());
        this->SC_ExpectedFPS->Create();
        this->SC_ExpectedFPS->SetBalloonHelpString("Adjust performance/quality. 1 fps: slow and high quality. 20 fps: fast and low quality.");
        this->SC_ExpectedFPS->SetRange(1,20); 
        this->SC_ExpectedFPS->SetResolution(1);
        this->SC_ExpectedFPS->SetValue(5.0);
        this->SC_ExpectedFPS->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_ExpectedFPS->GetWidgetName() );

    }
    
    //software ray casting
    {
        this->FrameCPURayCasting = vtkKWFrameWithLabel::New();
        this->FrameCPURayCasting->SetParent(this->FramePerformance->GetFrame());
        this->FrameCPURayCasting->Create();
        this->FrameCPURayCasting->AllowFrameToCollapseOff();
        this->FrameCPURayCasting->SetLabelText("Software Ray Casting");
        this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameCPURayCasting->GetWidgetName() );

        //enable/disable cpu ray casting MIP rendering
        this->CB_CPURayCastMIP=vtkKWCheckButtonWithLabel::New();
        this->CB_CPURayCastMIP->SetParent(this->FrameCPURayCasting->GetFrame());
        this->CB_CPURayCastMIP->Create();
        this->CB_CPURayCastMIP->SetBalloonHelpString("Enable MIP rendering in CPU ray cast.");
        this->CB_CPURayCastMIP->SetLabelText("Maximum Intensity Proj.");
        this->CB_CPURayCastMIP->SetLabelWidth(labelWidth);
        this->CB_CPURayCastMIP->GetWidget()->SetSelectedState(0);
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_CPURayCastMIP->GetWidgetName() );
        this->CB_CPURayCastMIP->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->VolumeRenderingCallbackCommand);

    }
    
    //GPU ray casting
    {
        this->FrameGPURayCasting = vtkKWFrameWithLabel::New();
        this->FrameGPURayCasting->SetParent(this->FramePerformance->GetFrame());
        this->FrameGPURayCasting->Create();
        this->FrameGPURayCasting->AllowFrameToCollapseOff();
        this->FrameGPURayCasting->SetLabelText("GPU Ray Casting");
        this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameGPURayCasting->GetWidgetName() );

        //set technique
        this->MB_GPURayCastTechnique = vtkKWMenuButtonWithLabel::New();
        this->MB_GPURayCastTechnique->SetParent(this->FrameGPURayCasting->GetFrame());
        this->MB_GPURayCastTechnique->SetLabelText("Rendering Technique");
        this->MB_GPURayCastTechnique->Create();
        this->MB_GPURayCastTechnique->SetLabelWidth(labelWidth);
        this->MB_GPURayCastTechnique->SetBalloonHelpString("Select different techniques in GPU ray casting");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Composite With Shading");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechnique 0");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Composite Psuedo Shading");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechnique 1");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechnique 2");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
        this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPURayCastTechnique 3");
        
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechnique->GetWidgetName() );
        
        this->MB_GPURayCastTechnique->GetWidget()->SetValue("Composite With Shading");
        
        //get scalar range 
        double scalarRange[2];
        vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
        this->SC_GPURayCastDepthPeelingThreshold=vtkKWScale::New();
        this->SC_GPURayCastDepthPeelingThreshold->SetParent(this->FrameGPURayCasting->GetFrame());
        this->SC_GPURayCastDepthPeelingThreshold->Create();
        this->SC_GPURayCastDepthPeelingThreshold->SetLabelText("Volumetric Depth Peeling Threshold");
        this->SC_GPURayCastDepthPeelingThreshold->SetBalloonHelpString("Set threshold for volumetric depth peeling. Volume rendering starts after we have met scalar values higher than the threshold. Use with transfer functions together.");
        this->SC_GPURayCastDepthPeelingThreshold->SetRange(scalarRange[0],scalarRange[1]); 
        this->SC_GPURayCastDepthPeelingThreshold->SetResolution(1);
        this->SC_GPURayCastDepthPeelingThreshold->SetValue(scalarRange[0]);
        this->SC_GPURayCastDepthPeelingThreshold->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastDepthPeelingThreshold->GetWidgetName() );
    }
    
    //GPU ray casting II
    {
        this->FrameGPURayCastingII = vtkKWFrameWithLabel::New();
        this->FrameGPURayCastingII->SetParent(this->FramePerformance->GetFrame());
        this->FrameGPURayCastingII->Create();
        this->FrameGPURayCastingII->AllowFrameToCollapseOff();
        this->FrameGPURayCastingII->SetLabelText("GPU Ray Casting II");
        this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameGPURayCastingII->GetWidgetName() );

        //set technique
        this->MB_GPURayCastTechniqueII = vtkKWMenuButtonWithLabel::New();
        this->MB_GPURayCastTechniqueII->SetParent(this->FrameGPURayCastingII->GetFrame());
        this->MB_GPURayCastTechniqueII->SetLabelText("Rendering Technique");
        this->MB_GPURayCastTechniqueII->Create();
        this->MB_GPURayCastTechniqueII->SetLabelWidth(labelWidth);
        this->MB_GPURayCastTechniqueII->SetBalloonHelpString("Select different techniques in GPU ray casting");
        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Composite With Shading");
        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechniqueII 0");
//        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Composite Psuedo Shading");
//        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechnique 1");
//        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
//        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechnique 2");
//        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
//        this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPURayCastTechnique 3");
        
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechniqueII->GetWidgetName() );
        
        this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Composite With Shading");
        
        this->MB_GPURayCastColorOpacityFusion = vtkKWMenuButtonWithLabel::New();
        this->MB_GPURayCastColorOpacityFusion->SetParent(this->FrameGPURayCastingII->GetFrame());
        this->MB_GPURayCastColorOpacityFusion->SetLabelText("Color/Opacity Fusion");
        this->MB_GPURayCastColorOpacityFusion->Create();
        this->MB_GPURayCastColorOpacityFusion->SetLabelWidth(labelWidth);
        this->MB_GPURayCastColorOpacityFusion->SetBalloonHelpString("Select color fusion method in multi-volume rendering");
        this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->AddRadioButton("Composite");
        this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastColorOpacityFusion 0");
        
        this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastColorOpacityFusion->GetWidgetName() );
        
        this->MB_GPURayCastColorOpacityFusion->GetWidget()->SetValue("Composite");
    }
    
    //opengl 2D Polygon Texture 3D
    {
        this->FramePolygonBlending = vtkKWFrameWithLabel::New();
        this->FramePolygonBlending->SetParent(this->FramePerformance->GetFrame());
        this->FramePolygonBlending->Create();
        this->FramePolygonBlending->AllowFrameToCollapseOff();
        this->FramePolygonBlending->SetLabelText("OpenGL Polygon Texture 3D");
        this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FramePolygonBlending->GetWidgetName() );
        
        //currently no parameters
    }
    
    //CUDA ray casting
    {
      this->FrameCUDARayCasting = vtkKWFrameWithLabel::New();
      this->FrameCUDARayCasting->SetParent(this->FramePerformance->GetFrame());
      this->FrameCUDARayCasting->Create();
      this->FrameCUDARayCasting->AllowFrameToCollapseOff();
      this->FrameCUDARayCasting->SetLabelText("CUDA Ray Casting");
      this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameCUDARayCasting->GetWidgetName() );

      //enable/disable CUDA ray casting shading
      this->CB_CUDARayCastShading=vtkKWCheckButtonWithLabel::New();
      this->CB_CUDARayCastShading->SetParent(this->FrameCUDARayCasting->GetFrame());
      this->CB_CUDARayCastShading->Create();
      this->CB_CUDARayCastShading->SetBalloonHelpString("Enable lighting/shading in CUDA ray cast.");
      this->CB_CUDARayCastShading->SetLabelText("Enable Lighting");
      this->CB_CUDARayCastShading->SetLabelWidth(labelWidth);
      this->CB_CUDARayCastShading->GetWidget()->SetSelectedState(0);
      this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_CUDARayCastShading->GetWidgetName() );
      this->CB_CUDARayCastShading->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->VolumeRenderingCallbackCommand);
        
    }
   
}

void vtkSlicerVRGrayscaleHelper::DestroyPerformance(void)
{
    //remember user choice on rendering method and GPU memory size
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","MB_GPUMemorySize","%d",
        this->MB_GPUMemorySize->GetWidget()->GetMenu()->GetIndexOfSelectedItem());
    this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","MB_Mapper","%d",
        this->MB_Mapper->GetWidget()->GetMenu()->GetIndexOfSelectedItem());
    
  if(this->CB_CUDARayCastShading!=NULL)
    {
      this->CB_CUDARayCastShading->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
      this->CB_CUDARayCastShading->SetParent(NULL);
      this->CB_CUDARayCastShading->Delete();
      this->CB_CUDARayCastShading=NULL;
    }
    
    if(this->MB_GPUMemorySize!=NULL)
    {
        this->MB_GPUMemorySize->SetParent(NULL);
        this->MB_GPUMemorySize->Delete();
        this->MB_GPUMemorySize=NULL;
    }
    
    if(this->SC_ExpectedFPS!=NULL)
    {
        this->SC_ExpectedFPS->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
        this->SC_ExpectedFPS->SetParent(NULL);
        this->SC_ExpectedFPS->Delete();
        this->SC_ExpectedFPS=NULL;
    }
    
    if(this->CB_CPURayCastMIP!=NULL)
    {
        this->CB_CPURayCastMIP->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
        this->CB_CPURayCastMIP->SetParent(NULL);
        this->CB_CPURayCastMIP->Delete();
        this->CB_CPURayCastMIP=NULL;
    }
    
    if(this->MB_Mapper != NULL)
    {
        this->MB_Mapper->SetParent(NULL);
        this->MB_Mapper->Delete();
        this->MB_Mapper=NULL;
    }
    
    if (this->FramePerformance != NULL)
    {
        this->FramePerformance->SetParent(NULL);
        this->FramePerformance->Delete();
        this->FramePerformance = NULL;
    }
    
    if (this->FrameCUDARayCasting != NULL)
    {
        this->FrameCUDARayCasting->SetParent(NULL);
        this->FrameCUDARayCasting->Delete();
        this->FrameCUDARayCasting = NULL;
    }

    if (this->FrameGPURayCasting != NULL)
    {
        this->FrameGPURayCasting->SetParent(NULL);
        this->FrameGPURayCasting->Delete();
        this->FrameGPURayCasting = NULL;
    }
    
    if (this->FramePolygonBlending != NULL)
    {
        this->FramePolygonBlending->SetParent(NULL);
        this->FramePolygonBlending->Delete();
        this->FramePolygonBlending = NULL;
    }
    
    if (this->FrameCPURayCasting != NULL)
    {
        this->FrameCPURayCasting->SetParent(NULL);
        this->FrameCPURayCasting->Delete();
        this->FrameCPURayCasting = NULL;
    }
    
    if (this->FrameFPS != NULL)
    {
        this->FrameFPS->SetParent(NULL);
        this->FrameFPS->Delete();
        this->FrameFPS = NULL;
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
        //matrix->Delete();
    }
    //Otherwise go back to Identity;
    else
    {
        this->AdditionalClippingTransform->Identity();
        this->InverseAdditionalClippingTransform->Identity();
    }
    this->BW_Clipping_Representation->SetTransform(this->AdditionalClippingTransform);
    this->BW_Clipping_Widget->InvokeEvent(vtkCommand::EndInteractionEvent);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();

}
//TODO: Scaling,translation with transform (scaling part of transform or part or placewidget)->preferred: part of transform
//TODO: Adjust initial range to size of volume, reduce until old volume is reached 

//Note: we save clipping planes in ijk space, show it in ras and

void vtkSlicerVRGrayscaleHelper::EstimateSampleDistances(void)
{
    double *spacing = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetSpacing();
    
    if (spacing)
    {
      double minSpace = spacing[0];
      double maxSpace = spacing[0];
    
      for(int i = 1; i < 3; i++)
      {
        if (spacing[i] > maxSpace)
            maxSpace = spacing[i];
        if (spacing[i] < minSpace)
            minSpace = spacing[i];
      }

      this->EstimatedSampleDistance = minSpace * 0.5f;
    }
    else
      this->EstimatedSampleDistance = 1.0f;
}

void vtkSlicerVRGrayscaleHelper::ConvertWorldToBoxCoordinates(double *inputOutput)
{
    int pointAint[3];
    double pointA[4];
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetDimensions(pointAint);
    for(int i=0;i<3;i++)
    {
        pointA[i]=pointAint[i]/2.;
    }
    pointA[3]=1;
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    matrix->MultiplyPoint(pointA,pointA);

    for(int i=0;i<3;i++)
    {
        inputOutput[i]=inputOutput[i]-pointA[i];
    }
    matrix->Delete();
}

void vtkSlicerVRGrayscaleHelper::ConvertBoxCoordinatesToWorld(double* inputOutput)
{
    int pointAint[3];
    double pointA[4];
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetDimensions(pointAint);
    for(int i=0;i<3;i++)
    {
        pointA[i]=pointAint[i]/2.;
    }
    pointA[3]=1;
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    matrix->MultiplyPoint(pointA,pointA);
    for(int i=0;i<3;i++)
    {
        inputOutput[i]=inputOutput[i]+pointA[i];
    }
    matrix->Delete();
}

void vtkSlicerVRGrayscaleHelper::ProcessConfigureCallback(void)
{
    vtkRenderWindowInteractor *interactor=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetInteractor();
    interactor->UpdateSize(this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetSize()[0],
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetSize()[1]);
}

void vtkSlicerVRGrayscaleHelper::CalculateBoxCoordinatesBoundaries(void)
{
    //Calculate origin in box coordinates
    //int pointAint[3];
    double pointA[4];
    //vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetOrigin(pointAint);
    for(int i=0;i<3;i++)
    {
        pointA[i]=0;
    }
    pointA[3]=1;
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    this->CalculateMatrix(matrix);
    matrix->MultiplyPoint(pointA,pointA);//Now we have point in world coordinates
    this->ConvertWorldToBoxCoordinates(pointA);//Now we have point in box coordinates


    //Calculate dimension in box coordinates
    int pointBint[3];
    double pointB[4];
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData()->GetDimensions(pointBint);
    for(int i=0;i<3;i++)
    {
        pointB[i]=pointBint[i];
    }
    pointB[3]=1;
    matrix->MultiplyPoint(pointB,pointB);//Now we have point in world coordinates
    this->ConvertWorldToBoxCoordinates(pointB);//Now we have point in box coordinates

    for(int i=0;i<3;i++)
    {
        if(pointA[i]<0)
        {
            this->VolumeBoundariesBoxCoordinates[0][i]=pointA[i];
            this->VolumeBoundariesBoxCoordinates[1][i]=pointB[i];
        }
        else
        {
            this->VolumeBoundariesBoxCoordinates[0][i]=pointB[i];
            this->VolumeBoundariesBoxCoordinates[1][i]=pointA[i];
        }
    }
    matrix->Delete();
}

void vtkSlicerVRGrayscaleHelper::CheckAbort(void)
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
        vtkSlicerVRHelperDebug("got an abort from gui","");
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        return;

    }
}

void vtkSlicerVRGrayscaleHelper::SetupCPURayCastInteractive()
{
    if (this->MapperRaycast == NULL || this->Volume == NULL)
        return;
        
    if (this->Volume->GetMapper() != this->MapperRaycast)
        return;
        
    //when start (rendering??) set CPU ray casting to be interactive
    if (this->ButtonDown == 1)
    {
        float desiredTime = 1.0f/this->SC_ExpectedFPS->GetValue();//expected fps will not be 0 so safe to do division here
  
        this->MapperRaycast->SetAutoAdjustSampleDistances(1);
        this->MapperRaycast->ManualInteractiveOn();
        this->MapperRaycast->SetManualInteractiveRate(desiredTime);
        
        this->CPURayCastingInteractionFlag = 1;
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CPU Raycasting: Interactive Quality");
    }
    else
    {
        //when end (rendering??) set CPU ray casting to be non-interactive high quality
        this->MapperRaycast->SetAutoAdjustSampleDistances(0);
        this->MapperRaycast->SetSampleDistance(this->EstimatedSampleDistance);
        this->MapperRaycast->SetImageSampleDistance(1.0f);
        this->MapperRaycast->ManualInteractiveOff();
        
        if (this->CPURayCastingInteractionFlag == 1)//avoid endless loop
        {
            this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CPU Raycasting: High Quality");
            this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
            this->CPURayCastingInteractionFlag = 0;
        }
    }
}

