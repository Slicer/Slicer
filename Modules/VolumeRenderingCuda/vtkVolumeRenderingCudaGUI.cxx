#include "vtkVolumeRenderingCudaGUI.h"
#include "vtkVolumeRenderingCudaLogic.h"
#include "vtkSlicerApplication.h"

#include "vtkImageGradientMagnitude.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkImageData.h"
#include "vtkVolume.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkPiecewiseFunction.h"

// Widgets
#include "vtkKWPushButton.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWLabel.h"
#include "vtkKWVolumePropertyWidget.h"
#include "vtkKWEvent.h"
#include "vtkKWScale.h"
#include "vtkKWRange.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWHistogram.h"

// MRML
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

// vtkCuda
#include "vtkCudaVolumeMapper.h"
#include "vtkCudaMemoryTexture.h"

vtkVolumeRenderingCudaGUI::vtkVolumeRenderingCudaGUI()
{
    this->CudaMapper = NULL;
    this->CudaVolume = NULL;
    this->CudaVolumeProperty = NULL;

    this->Histograms = NULL;

    this->RenderModeChooser = NULL;
    this->RayCastingMethodChooser = NULL;
    this->InterpolationMethodChooser = NULL;
    this->RenderObjectModeChooser = NULL;
    this->VolumeRenderDirectionChooser = NULL;
    this->VolumePropertyWidget = NULL;

    this->ThresholdRange = NULL;

    this->RenderScheduled = false;

    this->SelectedImageData = NULL;
    this->SelectedSliceMatrix = NULL;

    this->SliceMatrix = NULL;
    this->ImageData = NULL;
}


vtkVolumeRenderingCudaGUI::~vtkVolumeRenderingCudaGUI()
{
    if (this->CudaMapper != NULL)
    {
        this->CudaMapper->Delete();
    }
    if (this->CudaVolume != NULL)
    {
        this->CudaVolume->Delete();  
    }

    if (this->Histograms != NULL)
        this->Histograms->Delete();

    if (this->CudaVolumeProperty != NULL)
        this->CudaVolumeProperty->Delete();

    DeleteWidget(this->RenderModeChooser);
    DeleteWidget(this->RayCastingMethodChooser);
    DeleteWidget(this->InterpolationMethodChooser);
    DeleteWidget(this->RenderObjectModeChooser);
    DeleteWidget(this->VolumeRenderDirectionChooser);
    DeleteWidget(this->VolumePropertyWidget);
}

void vtkVolumeRenderingCudaGUI::DeleteWidget(vtkKWWidget* widget)
{
    if (widget != NULL)
    {
        widget->SetParent(NULL);
        widget->Delete();
    }
}

vtkVolumeRenderingCudaGUI* vtkVolumeRenderingCudaGUI::New()
{
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingCudaGUI");
    if (ret)
        return (vtkVolumeRenderingCudaGUI*)ret;
    // If the Factory was unable to create the object, we do it ourselfes.
    return new vtkVolumeRenderingCudaGUI();
}


void vtkVolumeRenderingCudaGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    this->GetUIPanel()->AddPage("VolumeRenderingCuda","VolumeRenderingCuda",NULL);

    // Define your help text and build the help frame here.
    const char *help = "VolumeRenderingCuda. 3D Segmentation This module is currently a prototype and will be under active development throughout 3DSlicer's Beta release.";
    const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRenderingCuda" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    //
    //Load and save
    //
    vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadSaveDataFrame->SetParent (page);
    loadSaveDataFrame->Create();
    loadSaveDataFrame->ExpandFrame();
    loadSaveDataFrame->SetLabelText("Load and Save");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        loadSaveDataFrame->GetWidgetName(), page->GetWidgetName());

     //NodeSelector  for Node from MRML Scene
    this->NS_ImageData=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageData->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_ImageData->Create();
    this->NS_ImageData->NoneEnabledOn();
    this->NS_ImageData->SetLabelText("Source Volume: ");
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageData->GetWidgetName());

    this->NS_SliceMatrix=vtkSlicerNodeSelectorWidget::New();
    this->NS_SliceMatrix->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_SliceMatrix->Create();
    this->NS_SliceMatrix->NoneEnabledOn();
    this->NS_SliceMatrix->SetLabelText("Slice Matrix: ");
    this->NS_SliceMatrix->SetNodeClass("vtkMRMLTransformNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_SliceMatrix->GetWidgetName());

    this->VolumePropertyWidget = vtkKWVolumePropertyWidget::New();
    this->VolumePropertyWidget->SetParent(loadSaveDataFrame->GetFrame());
    this->VolumePropertyWidget->Create();
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->VolumePropertyWidget->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName()); 

    this->ThresholdRange = vtkKWRange::New();
    this->ThresholdRange->SetParent(loadSaveDataFrame->GetFrame());
    this->ThresholdRange->Create();
    this->ThresholdRange->SetWholeRange(0, 255);
    this->ThresholdRange->SetRange(1, 255);
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->ThresholdRange->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName()); 

    this->RenderModeChooser = vtkKWMenuButton::New();
    this->RenderModeChooser->SetParent(loadSaveDataFrame->GetFrame());
    this->RenderModeChooser->Create();
    this->RenderModeChooser->GetMenu()->AddRadioButton("To Texture");
    this->RenderModeChooser->GetMenu()->AddRadioButton("To Memory");
    this->RenderModeChooser->GetMenu()->SelectItem(0);
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->RenderModeChooser->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName());

    this->RayCastingMethodChooser = vtkKWMenuButton::New();
    this->RayCastingMethodChooser->SetParent(loadSaveDataFrame->GetFrame());
    this->RayCastingMethodChooser->Create();
    this->RayCastingMethodChooser->GetMenu()->AddRadioButton("Isosurface");
    this->RayCastingMethodChooser->GetMenu()->AddRadioButton("MIP");
    this->RayCastingMethodChooser->GetMenu()->AddRadioButton("Composite");
    this->RayCastingMethodChooser->GetMenu()->AddRadioButton("Composite Shaded");
    this->RayCastingMethodChooser->GetMenu()->SelectItem(2);
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->RayCastingMethodChooser->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName());  

    this->InterpolationMethodChooser = vtkKWMenuButton::New();
    this->InterpolationMethodChooser->SetParent(loadSaveDataFrame->GetFrame());
    this->InterpolationMethodChooser->Create();
    this->InterpolationMethodChooser->GetMenu()->AddRadioButton("Nearest Neighbor");
    this->InterpolationMethodChooser->GetMenu()->AddRadioButton("Trilinear");
    this->InterpolationMethodChooser->GetMenu()->SelectItem(0);
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->InterpolationMethodChooser->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName());  

    this->RenderObjectModeChooser = vtkKWMenuButton::New();
    this->RenderObjectModeChooser->SetParent(loadSaveDataFrame->GetFrame());
    this->RenderObjectModeChooser->Create();
    this->RenderObjectModeChooser->GetMenu()->AddRadioButton("Volume");
    this->RenderObjectModeChooser->GetMenu()->AddRadioButton("Slice");
    this->RenderObjectModeChooser->GetMenu()->SelectItem(0);
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->RenderObjectModeChooser->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName());

    this->VolumeRenderDirectionChooser = vtkKWMenuButton::New();
    this->VolumeRenderDirectionChooser->SetParent(loadSaveDataFrame->GetFrame());
    this->VolumeRenderDirectionChooser->Create();
    this->VolumeRenderDirectionChooser->GetMenu()->AddRadioButton("Plus/Minus");
    this->VolumeRenderDirectionChooser->GetMenu()->AddRadioButton("Plus");
    this->VolumeRenderDirectionChooser->GetMenu()->AddRadioButton("Minus");
    this->VolumeRenderDirectionChooser->GetMenu()->SelectItem(0);
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        this->VolumeRenderDirectionChooser->GetWidgetName(), loadSaveDataFrame->GetFrame()->GetWidgetName());

    //Build IV viewer
    
    this->Built=true;
}

void vtkVolumeRenderingCudaGUI::TearDownGUI ( )
{
    this->Exit();
    if ( this->Built )
    {
        this->RemoveGUIObservers();
    }
}

void vtkVolumeRenderingCudaGUI::CreateModuleEventBindings ( )
{
    vtkDebugMacro("VolumeRenderingCuda: CreateModuleEventBindings: No ModuleEventBindings yet");
}
void vtkVolumeRenderingCudaGUI::ReleaseModuleEventBindings ( )
{
    vtkDebugMacro("VolumeRenderingCuda: ReleaseModuleEventBindings: No ModuleEventBindings to remove yet");
}

void vtkVolumeRenderingCudaGUI::AddGUIObservers ( )
{
    this->NS_ImageData->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->NS_SliceMatrix->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->RenderModeChooser->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);

    this->RayCastingMethodChooser->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);

    this->InterpolationMethodChooser->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);

    this->RenderObjectModeChooser->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);

    this->VolumeRenderDirectionChooser->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);

    this->VolumePropertyWidget->AddObserver(vtkKWEvent::VolumePropertyChangedEvent, (vtkCommand*)this->GUICallbackCommand);

    this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,(vtkCommand *)this->GUICallbackCommand);
    this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,(vtkCommand *)this->GUICallbackCommand);

    this->ThresholdRange->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand*)this->GUICallbackCommand);
}

void vtkVolumeRenderingCudaGUI::RemoveGUIObservers ( )
{
  this->RenderModeChooser->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->RayCastingMethodChooser->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->InterpolationMethodChooser->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->RenderObjectModeChooser->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->VolumeRenderDirectionChooser->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
}
void vtkVolumeRenderingCudaGUI::RemoveMRMLNodeObservers ( )
{
  if(this->SliceMatrix!=NULL){
    this->SliceMatrix->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand);
  }
}

void vtkVolumeRenderingCudaGUI::RemoveLogicObservers ( )
{
}

void vtkVolumeRenderingCudaGUI::CreateMapper()
{
    if (this->CudaMapper == NULL)
    {
        this->CudaMapper = vtkCudaVolumeMapper::New();
        this->CudaMapper->SetThreshold(this->ThresholdRange->GetRange());
    }
    if (this->CudaVolume == NULL)
    {
        this->CudaVolume = vtkVolume::New();
        this->CudaVolume->SetMapper(this->CudaMapper);
        
        this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderer()->AddVolume(this->CudaVolume);
        
    }

}

void vtkVolumeRenderingCudaGUI::DeleteMapper()
{
    if (this->CudaVolume != NULL)
      {
        this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderer()->RemoveVolume(this->CudaVolume);
        this->CudaVolume->Delete();
        this->CudaVolume = NULL;
    }
    if (this->CudaMapper != NULL)
    {
        this->CudaMapper->Delete();
        this->CudaMapper = NULL;
    }
}

void vtkVolumeRenderingCudaGUI::ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                                         void *callData )
{
  vtkDebugMacro("vtkVolumeRenderingModuleGUI::ProcessGUIEvents: event = " << event);

  if (caller == this->RenderModeChooser->GetMenu())
    {
      if (this->CudaMapper != NULL)
        if (!strcmp (this->RenderModeChooser->GetValue(), "To Texture"))
          this->CudaMapper->SetRenderMode(vtkCudaMemoryTexture::RenderToTexture);
        else
          this->CudaMapper->SetRenderMode(vtkCudaMemoryTexture::RenderToMemory);
    }

  else if (caller == this->RayCastingMethodChooser->GetMenu())
    {
      if (this->CudaMapper != NULL)
        if (!strcmp (this->RayCastingMethodChooser->GetValue(), "Isosurface"))
          this->CudaMapper->SetRayCastingMethod(0);
        else if (!strcmp (this->RayCastingMethodChooser->GetValue(), "MIP"))
          this->CudaMapper->SetRayCastingMethod(1);
        else if (!strcmp (this->RayCastingMethodChooser->GetValue(), "Composite"))
          this->CudaMapper->SetRayCastingMethod(2);
        else if (!strcmp (this->RayCastingMethodChooser->GetValue(), "Composite Shaded"))
          this->CudaMapper->SetRayCastingMethod(3);

      if(this->RenderScheduled == false){
       this->RenderScheduled = true;
       this->ScheduleRender();
      }
      if(this->RenderScheduled == false){
        this->RenderScheduled = true;
        this->ScheduleRender();
      }
    }

  else if (caller == this->InterpolationMethodChooser->GetMenu())
    {
      if (this->CudaMapper != NULL)
        if (!strcmp (this->InterpolationMethodChooser->GetValue(), "Nearest Neighbor"))
          this->CudaMapper->SetInterpolationMethod(0);
        else if (!strcmp (this->InterpolationMethodChooser->GetValue(), "Trilinear"))
          this->CudaMapper->SetInterpolationMethod(1);

      if(this->RenderScheduled == false){
       this->RenderScheduled = true;
       this->ScheduleRender();
      }
      if(this->RenderScheduled == false){
        this->RenderScheduled = true;
        this->ScheduleRender();
      }
    }

  else if (caller == this->RenderObjectModeChooser->GetMenu())
    {
      if (!strcmp (this->RenderObjectModeChooser->GetValue(), "Volume")){
        this->CudaMapper->SetRenderObjectMode(0);
      }else if (!strcmp (this->RenderObjectModeChooser->GetValue(), "Slice")){
        this->CudaMapper->SetRenderObjectMode(1);
        this->SelectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
        if(this->SelectedImageData!=NULL){
          int extent[6];
          this->SelectedImageData->GetImageData()->GetExtent(extent);
        }
      }else if (!strcmp (this->RenderObjectModeChooser->GetValue(), "Arbitrary Slice")){
          
      }
      
      if(this->RenderScheduled == false){
       this->RenderScheduled = true;
       this->ScheduleRender();
      }
      if(this->RenderScheduled == false){
        this->RenderScheduled = true;
        this->ScheduleRender();
      }
    }

  else if (caller == this->VolumeRenderDirectionChooser->GetMenu())
    {
      if (!strcmp (this->VolumeRenderDirectionChooser->GetValue(), "Plus/Minus")){
        this->CudaMapper->SetVolumeRenderDirection(0);
      }else if (!strcmp (this->VolumeRenderDirectionChooser->GetValue(), "Plus")){
        this->CudaMapper->SetVolumeRenderDirection(1);
      }else if (!strcmp (this->VolumeRenderDirectionChooser->GetValue(), "Minus")){
        this->CudaMapper->SetVolumeRenderDirection(2);
      }
        
      if(this->RenderScheduled == false){
       this->RenderScheduled = true;
       this->ScheduleRender();
      }
      if(this->RenderScheduled == false){
        this->RenderScheduled = true;
        this->ScheduleRender();
      }
    }

  else if (caller == this->NS_ImageData)
    {
      if(this->ImageData!=NULL){
        this->ImageData->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand);
        this->DeleteMapper();
      }
      
      this->SelectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
      if (this->SelectedImageData != NULL && this->SelectedImageData->GetImageData() != NULL)
        {
          this->ImageData=this->SelectedImageData->GetImageData();
          this->ImageData->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand );
          
          this->CreateMapper();
          this->SelectedImageData->GetImageData()->SetSpacing(this->SelectedImageData->GetSpacing());
          
          this->CudaMapper->SetInput(this->SelectedImageData->GetImageData());
          
          float min=this->SelectedImageData->GetImageData()->GetScalarTypeMin();
          float max=this->SelectedImageData->GetImageData()->GetScalarTypeMax();
          double range[2]={min, max};
          
          this->ThresholdRange->SetWholeRange(min, max);
          this->ThresholdRange->SetRange(min, max);
          
          if(this->CudaVolumeProperty!=NULL)this->CudaVolumeProperty->Delete();
          this->CudaVolumeProperty = vtkVolumeProperty::New();
          this->CudaVolumeProperty->GetRGBTransferFunction()->RemoveAllPoints();
          this->CudaVolumeProperty->GetRGBTransferFunction()->AddRGBPoint(min, 0, 0, 0);
          this->CudaVolumeProperty->GetRGBTransferFunction()->AddRGBPoint(max, 1, 1, 1);

          this->CudaVolumeProperty->GetScalarOpacity()->RemoveAllPoints();
          this->CudaVolumeProperty->GetScalarOpacity()->AddPoint(min, 0);
          this->CudaVolumeProperty->GetScalarOpacity()->AddPoint(max, 1);
            
          this->CudaVolume->SetProperty(this->CudaVolumeProperty);
            
          this->VolumePropertyWidget->SetVolumeProperty(this->CudaVolumeProperty);
          this->VolumePropertyWidget->ScalarOpacityUnitDistanceVisibilityOff ();
          this->VolumePropertyWidget->GradientOpacityFunctionVisibilityOff ();
            
          if(this->Histograms!=NULL)this->Histograms->Delete();
          this->Histograms = vtkKWHistogramSet::New();

          //Add Histogram for image data

          vtkKWHistogram *histo = vtkKWHistogram::New();
            
          histo->SetRange(min, max+1);
          histo->AccumulateHistogram(this->CudaMapper->GetInput()->GetPointData()->GetScalars(), 0);
          this->Histograms->AddHistogram(histo,"0");
            
          this->VolumePropertyWidget->SetHistogramSet(this->Histograms);

          histo->Delete();
            
          //Delete

          vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
          this->SelectedImageData->GetIJKToRASMatrix(matrix);
          this->CudaMapper->SetOrientationMatrix(matrix);
          matrix->Delete();
        }
      else
        this->DeleteMapper();

      if (this->RenderScheduled == false){    
        this->RenderScheduled=true;
        this->ScheduleRender();
      } 
    }
  else if (caller == this->NS_SliceMatrix){
    if(this->SliceMatrix!=NULL){
      this->SliceMatrix->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand);
    }
    this->SelectedSliceMatrix=vtkMRMLLinearTransformNode::SafeDownCast(this->NS_SliceMatrix->GetSelected());
    if (this->SelectedSliceMatrix != NULL){
      this->SliceMatrix=this->SelectedSliceMatrix->GetMatrixTransformToParent();
      this->SliceMatrix->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand );
      if(this->CudaMapper != NULL){
       vtkMatrix4x4* mat=vtkMatrix4x4::New();
       this->SelectedSliceMatrix->GetMatrixTransformToWorld(mat);
       this->CudaMapper->SetSliceMatrix(mat);
       mat->Delete();
     }
   }else{
     if(this->CudaMapper != NULL){
       vtkMatrix4x4* mat=vtkMatrix4x4::New();
       this->CudaMapper->SetSliceMatrix(mat);
       mat->Delete();
     }
   }
 }
    
 else if (caller == this->VolumePropertyWidget)
   {
     if (this->CudaMapper != NULL){
       this->CudaMapper->Update();
     }
     if(this->RenderScheduled == false){
       this->RenderScheduled = true;
       this->ScheduleRender();
     }
   }

 else if (caller == this->ThresholdRange){
   if(this->CudaMapper !=NULL){
     this->CudaMapper->SetThreshold(this->ThresholdRange->GetRange());
   }
 }
  
 else if(this->CudaMapper != NULL &&
         caller==this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow() && event==vtkCommand::EndEvent)
   {
      
     this->SelectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
     if(this->SelectedImageData->GetParentTransformNode()){
       vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
       this->SelectedImageData->GetParentTransformNode()->GetMatrixTransformToWorld(matrix);
       this->CudaMapper->SetTransformationMatrix(matrix);
       matrix->Delete();
     }
      
     if (this->RenderScheduled == false)
       {    
         this->RenderScheduled=true;
         this->ScheduleRender();
       }
   
   }
  
}

void vtkVolumeRenderingCudaGUI::ScheduleRender()
{
  this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->Render();
  this->RenderScheduled = false;   
}

void vtkVolumeRenderingCudaGUI::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData)
{
  
  if(caller == this->SliceMatrix){
    if(this->CudaMapper!=NULL){
      this->CudaMapper->SetSliceMatrix(this->SliceMatrix);
      if(this->RenderScheduled==false){
        this->RenderScheduled = true;
        this->ScheduleRender();
      }
    }
  }
  else if(caller == this->ImageData){
    if(this->CudaMapper!=NULL){
      this->CudaMapper->SetInput(this->ImageData);
      if(this->RenderScheduled==false){
        this->RenderScheduled = true;
        this->ScheduleRender();
      }
    }
  }
  
}


void vtkVolumeRenderingCudaGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingCudaGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}


void vtkVolumeRenderingCudaGUI::Enter ( )
{
    vtkDebugMacro("Enter Volume Rendering Cuda Module");

    if ( this->Built == false )
    {
        this->BuildGUI();
        this->AddGUIObservers();
    }
    this->CreateModuleEventBindings();

    this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageData->UpdateMenu();

    this->NS_SliceMatrix->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_SliceMatrix->UpdateMenu();

}
void vtkVolumeRenderingCudaGUI::Exit ( )
{
    vtkDebugMacro("Exit: removeObservers for VolumeRenderingModule");
    this->ReleaseModuleEventBindings();
}


void vtkVolumeRenderingCudaGUI::PrintSelf(ostream& os, vtkIndent indent)
{
    this->SuperClass::PrintSelf(os, indent);

    os<<indent<<"vtkVolumeRenderingCudaGUI"<<endl;
    os<<indent<<"vtkVolumeRenderingCudaLogic"<<endl;
    if(this->GetLogic())
    {
        this->GetLogic()->PrintSelf(os,indent.GetNextIndent());
    }
}
