#include "vtkSlicerVRGrayscaleHelper.h"
#include "vtkObjectFactory.h"
#include "vtkKWHistogram.h"
#include "vtkKWHistogramSet.h"
#include "vtkVolumeRenderingModuleGui.h"
#include "vtkVolume.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkTimerLog.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkKWProgressGauge.h" 
#include "vtkTexture.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkSlicerApplication.h"
#include "vtkKWEvent.h"



vtkCxxRevisionMacro(vtkSlicerVRGrayscaleHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRGrayscaleHelper);
vtkSlicerVRGrayscaleHelper::vtkSlicerVRGrayscaleHelper(void)
{
    this->Histograms=NULL;
    this->SVP_VolumeProperty=NULL;
    this->renViewport=NULL;
    this->renPlane=NULL;
    this->MapperTexture=NULL;
    this->MapperRaycast=NULL;
    this->timer=vtkTimerLog::New();
    this->RenderPlane=0;
    this->currentStage=0;
    this->scheduled=0;
    this->EventHandlerID="";

}

vtkSlicerVRGrayscaleHelper::~vtkSlicerVRGrayscaleHelper(void)
{
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
    if(this->renPlane!=NULL)
    {
        this->renPlane->Delete();
        this->renPlane=NULL;
    }
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
    if(this->timer!=NULL)
    {
        this->timer->StopTimer();
        this->timer->Delete();
        this->timer=NULL;
    }
}
void vtkSlicerVRGrayscaleHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    Superclass::Init(gui);
    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(this->Gui->GetdetailsFrame()->GetFrame());
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
    ((vtkSlicerApplication *)this->Gui->GetApplication())->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());
    //AddEvent for Interactive apply 
    this->SVP_VolumeProperty->AddObserver(vtkKWEvent::VolumePropertyChangingEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    grad->Delete();
    gradHisto->Delete(); 
}
void vtkSlicerVRGrayscaleHelper::InitializePipelineNewCurrentNode()
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
    vtkPiecewiseFunction *opacity=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetScalarOpacity();
    opacity->RemoveAllPoints();
    opacity->AddPoint(range[0],0.);
    opacity->AddPoint(tresholdLowIndex,0.0);
    opacity->AddPoint(tresholdHighIndex,0.2);
    opacity->AddPoint(range[1],0.2);
    vtkColorTransferFunction *colorTransfer=this->Gui->GetcurrentNode()->GetVolumeProperty()->GetRGBTransferFunction();
    colorTransfer->RemoveAllPoints();
    colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
    colorTransfer->AddRGBPoint(tresholdLowIndex,.3,.3,1.);
    colorTransfer->AddRGBPoint(tresholdLowIndex+.5*(tresholdHighIndex-tresholdLowIndex),.3,1.,.3);
    colorTransfer->AddRGBPoint(tresholdHighIndex,1.,.3,.3);
    colorTransfer->AddRGBPoint(range[1],1,.3,.3);
    this->UpdateSVP();
}

void vtkSlicerVRGrayscaleHelper::Rendering(void)
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
        //this->mapper=vtkSlicerFixedPointVolumeRayCastMapper::New();
        //vtkSlicerFixedPointVolumeRayCastMapper::SafeDownCast(this->mapper)->SetBlendModeToMaximumIntensity();
        //vtkSlicerFixedPointVolumeRayCastMapper::SafeDownCast(this->mapper)->Setlimit(1,2,3,4);
        this->MapperTexture=vtkVolumeTextureMapper3D::New();
        this->MapperTexture->SetSampleDistance(2);
        this->MapperTexture->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
        this->Volume->SetMapper(this->MapperTexture);
        //Also take care about Ray Cast
        this->MapperRaycast=vtkFixedPointVolumeRayCastMapper::New();
        this->MapperRaycast->SetInput(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData());
    }
    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsStartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);
    this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsEndEvent,(vtkCommand *) this->VolumeRenderingCallbackCommand);

    //Only needed if using performance enhancement
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->VolumeRenderingCallbackCommand);

    //TODO This is not the right place for this
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->VolumeRenderingCallbackCommand);
    this->Volume->SetProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
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
    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
    this->Volume->PokeMatrix(matrix);

    matrix->Delete();
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}
void vtkSlicerVRGrayscaleHelper::ShutdownPipeline(void)
{
    vtkKWRenderWidget *renderWidget=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
    //Remove Observers
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

    //Remove Volume
    if(this->Volume!=NULL)
    {
        renderWidget->RemoveViewProp(this->Volume);
        this->Volume->Delete();
        this->Volume=NULL;
        renderWidget->Render();
    }
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
    vtkSlicerVolumePropertyWidget *callerObjectSVP=vtkSlicerVolumePropertyWidget::SafeDownCast(caller);
    if(callerObjectSVP==this->SVP_VolumeProperty&&eid==vtkKWEvent::VolumePropertyChangingEvent)
    {
        this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->Render();
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
        //TODO what happens if volume==NULL and why does it happen?!
        this->Gui->Script("puts \"startevent scheduled %d\"",this->scheduled);
        this->Gui->Script("puts \"startevent currentstage %d\"",this->currentStage);
        this->Gui->Script("puts \"startevent id %s\"",this->EventHandlerID.c_str());
        //it is not a scheduled event so we use stage 0
        if(this->scheduled==0)
        {

            //go back to stage 0;
            //Check if we have an Event Scheduled, if this is the case abort it
            if(strcmp(this->EventHandlerID.c_str(),"")!=0)
            {
                const char* result=this->Gui->Script("after cancel %s",this->EventHandlerID.c_str());
                const char* resulta=this->Gui->Script("after info");
                this->Gui->Script("puts \"ResultCancel: %s\"",result);
                this->Gui->Script("puts \"Result info: %s\"",resulta);
                this->EventHandlerID="";
                //delete result;
                //delete resulta;



            }
            if(currentStage==2)
            {
                this->Volume->SetMapper(this->MapperTexture);
            }
            //always got to less sample distance
            this->MapperTexture->SetSampleDistance(2);
            this->currentStage=0;
            this->Gui->Script("puts \"Stage 0 started\"");
            //Decide if we REnder plane or not
            if(this->RenderPlane==1)
            {
                return;
            }
            timer->StartTimer();
            vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();

            double factor=.2;
            //We don't like to see, what we are doing here
            renWin->SwapBuffersOff();
            //get the viewport renderer up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renViewport);
            //Change viewport(simulation of "sample distance for "rays"" if using texture mapping)
            this->renViewport->SetViewport(0,0,factor,factor);


            //We are already in the Rendering Process
            //this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
        }//if

        //Stage 1
        if(this->currentStage==1)
        {
            this->Gui->Script("puts \"Stage 1 started\"");
            //Remove plane Renderer and get viewport Renderer Up
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->RemoveRenderer(this->renPlane);
            this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddRenderer(this->renViewport);
            //Go to Fullsize
            this->renViewport->SetViewport(0,0,1,1);
            this->MapperTexture->SetSampleDistance(.1);
            this->scheduled=0;
            return;
        }

        //Stage 2
        if(this->currentStage==2)
        {
            this->Gui->Script("puts \"Stage 2 started\"");
            this->Volume->SetMapper(this->MapperRaycast);

            this->EventHandlerID="";
            this->scheduled=0;
            return;
        }   
    }
    else if(caller==this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()&&eid==vtkCommand::EndEvent)
    {
        this->Gui->Script("puts \"endevent scheduled %d\"",this->scheduled);
        this->Gui->Script("puts \"endevent currentstage %d\"",this->currentStage);
        if(this->currentStage==1)
        {
            this->EventHandlerID=this->Gui->Script("after 100 %s ScheduleRender",this->GetTclName());
            this->Gui->Script("puts \"Stage 1 ended\"");
            return;
        }
        if(this->currentStage==2)
        {
            //We reached the highest Resolution, no scheduling
            this->Gui->Script("puts \"Stage 2 ended\"");
            return;
        }
        if(this->RenderPlane==1)
        {
            this->RenderPlane=0;
            this->timer->StopTimer();
            this->LastTimeLowRes=this->timer->GetElapsedTime();
            this->Gui->Script("puts %f",this->LastTimeLowRes);
            //It's time to start for the Scheduled Rendering
            this->EventHandlerID=this->Gui->Script("after 100 %s ScheduleRender",this->GetTclName());
            this->Gui->Script("puts \"Stage 0 ended\"");
            return;
        }


        vtkRenderWindow *renWin=this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow();
        double factor=.2;
        //Get current size of window
        int *size=renWin->GetSize();

        //RGB Data from the smaller viewport
        vtkUnsignedCharArray *image=vtkUnsignedCharArray::New();
        renWin->GetRGBACharPixelData(0,0,factor*size[0],factor*size[1],0,image);

        vtkImageData *imageData=vtkImageData::New();
        imageData->GetPointData()->SetScalars(image);
        imageData->SetDimensions(factor*size[0]+1,factor*size[1]+1,1);
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

    }
    //Check if caller equals mapper
    vtkAbstractMapper *callerMapper=vtkAbstractMapper::SafeDownCast(caller);
    if((this->Volume!=0)&&(callerMapper!=this->Volume->GetMapper()))
    {
        return;
    }
    if(eid==vtkCommand::VolumeMapperComputeGradientsStartEvent)
    {
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Calculate Gradients");
    }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsEndEvent)
    {
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Calculate Gradients finished");
    }
    else if(eid==vtkCommand::VolumeMapperComputeGradientsProgressEvent)
    {
        float *progress=(float*)callData;
        this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100**progress);
    }
}

void vtkSlicerVRGrayscaleHelper::ScheduleRender(void)
{
    this->scheduled=1;
    this->currentStage++;
    if(this->currentStage>2)
    {
        this->currentStage=0;
        this->scheduled=0;
        return;
    }
    this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
}

void vtkSlicerVRGrayscaleHelper::UpdateSVP(void)
{
        //First check if we have a SVP
    if(this->SVP_VolumeProperty==NULL)
    {
        vtkErrorMacro("SVP does not exist");
    }
    vtkTimerLog *timer=vtkTimerLog::New();
    timer->StartTimer();
    //First of all set New Property, Otherwise all Histograms will be overwritten
    //First check if we really need to update
    if(this->SVP_VolumeProperty->GetVolumeProperty()==this->Gui->GetcurrentNode()->GetVolumeProperty())
    {
        this->SVP_VolumeProperty->Update();
        timer->StopTimer();
        vtkErrorMacro("Update SVP calculated in "<<timer->GetElapsedTime()<<"seconds");
        timer->Delete();
        return;
    }
    this->SVP_VolumeProperty->SetVolumeProperty(this->Gui->GetcurrentNode()->GetVolumeProperty());
    this->SVP_VolumeProperty->SetHSVColorSelectorVisibility(1);

    //Adjust mapping
    //this->AdjustMapping();
    this->SVP_VolumeProperty->Update();
    timer->StopTimer();
    vtkDebugMacro("Update SVP calculated in "<<timer->GetElapsedTime()<<"seconds");
    timer->Delete();
}

void vtkSlicerVRGrayscaleHelper::UpdateGUIElements(void)
{
    Superclass::UpdateGUIElements();
    this->UpdateSVP();
}
