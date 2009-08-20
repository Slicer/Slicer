/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRIBiasFieldCorrectionGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRIBiasFieldCorrectionGUI.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWSpinBox.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

#include "vtkMRMLSliceNode.h"
#include "vtkSlicerSliceGUI.h"

//------------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI* vtkMRIBiasFieldCorrectionGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRIBiasFieldCorrectionGUI");
  if(ret)
    {
      return (vtkMRIBiasFieldCorrectionGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionGUI;
}


//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI::vtkMRIBiasFieldCorrectionGUI()
{
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->StorageVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->MaskVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  
  this->GADNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->CatchButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->MRIBiasFieldCorrectionNode = NULL;

  this->Sagittal2Scale = vtkKWScaleWithEntry::New();
  
  this->Coronal1Scale = vtkKWScaleWithEntry::New();
  this->Coronal2Scale = vtkKWScaleWithEntry::New();
  
  this->VolumeSelectionFrame = vtkKWFrameWithLabel::New();

  this->ParametersFrame = vtkKWFrameWithLabel::New();
  this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
  
  this->ShrinkFactor = vtkKWScaleWithEntry::New();
  this->MaxNumOfIt   = vtkKWScaleWithEntry::New();
  this->NumOfFitLev  = vtkKWScaleWithEntry::New();
  this->WienFilNoi   = vtkKWScaleWithEntry::New();
  this->BiasField    = vtkKWScaleWithEntry::New();
  this->Conv         = vtkKWScaleWithEntry::New();
  
  this->SlidePositionScale = vtkKWScaleWithEntry::New();

  this->BiasIntensityScale         = vtkKWScaleWithEntry::New();
}

//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI::~vtkMRIBiasFieldCorrectionGUI()
{
   if ( this->BiasIntensityScale ) {
        this->BiasIntensityScale->SetParent(NULL);
        this->BiasIntensityScale->Delete();
        this->BiasIntensityScale = NULL;
    }


   if ( this->SlidePositionScale ) {
        this->SlidePositionScale->SetParent(NULL);
        this->SlidePositionScale->Delete();
        this->SlidePositionScale = NULL;
    }
   

  if ( this->MaxNumOfIt ) {
        this->MaxNumOfIt->SetParent(NULL);
        this->MaxNumOfIt->Delete();
        this->MaxNumOfIt = NULL;
    }
    
   if ( this->NumOfFitLev ) {
        this->NumOfFitLev->SetParent(NULL);
        this->NumOfFitLev->Delete();
        this->NumOfFitLev = NULL;
    }
    
  if ( this->WienFilNoi ) {
        this->WienFilNoi->SetParent(NULL);
        this->WienFilNoi->Delete();
        this->WienFilNoi = NULL;
    }
    
  if ( this->BiasField ) {
        this->BiasField->SetParent(NULL);
        this->BiasField->Delete();
        this->BiasField = NULL;
    }     

  if ( this->Conv ) {
        this->Conv->SetParent(NULL);
        this->Conv->Delete();
        this->Conv = NULL;
    }


   if ( this->ShrinkFactor ) {
        this->ShrinkFactor->SetParent(NULL);
        this->ShrinkFactor->Delete();
        this->ShrinkFactor = NULL;
    }

    if ( this->ParametersFrame ) {
        this->ParametersFrame->SetParent(NULL);
        this->ParametersFrame->Delete();
        this->ParametersFrame = NULL;
    }

    if ( this->AdvancedParametersFrame ) {
        this->AdvancedParametersFrame->SetParent(NULL);
        this->AdvancedParametersFrame->Delete();
        this->AdvancedParametersFrame = NULL;
    }
    
    if ( this->TimeStepScale ) {
        this->TimeStepScale->SetParent(NULL);
        this->TimeStepScale->Delete();
        this->TimeStepScale = NULL;
    }
    if ( this->NumberOfIterationsScale ) {
        this->NumberOfIterationsScale->SetParent(NULL);
        this->NumberOfIterationsScale->Delete();
        this->NumberOfIterationsScale = NULL;
    }
    if ( this->VolumeSelector ) {
        this->VolumeSelector->SetParent(NULL);
        this->VolumeSelector->Delete();
        this->VolumeSelector = NULL;
    }
    if ( this->OutVolumeSelector ) {
        this->OutVolumeSelector->SetParent(NULL);
        this->OutVolumeSelector->Delete();
        this->OutVolumeSelector = NULL;
    }
   if ( this->StorageVolumeSelector ) {
        this->StorageVolumeSelector->SetParent(NULL);
        this->StorageVolumeSelector->Delete();
        this->StorageVolumeSelector = NULL;
    }
   if ( this->MaskVolumeSelector ) {
        this->MaskVolumeSelector->SetParent(NULL);
        this->MaskVolumeSelector->Delete();
        this->MaskVolumeSelector = NULL;
    } 
   
    if ( this->GADNodeSelector ) {
        this->GADNodeSelector->SetParent(NULL);
        this->GADNodeSelector->Delete();
        this->GADNodeSelector = NULL;
    }
    if ( this->ApplyButton ) {
        this->ApplyButton->SetParent(NULL);
        this->ApplyButton->Delete();
        this->ApplyButton = NULL;
    }
    if ( this->CatchButton ) {
        this->CatchButton->SetParent(NULL);
        this->CatchButton->Delete();
        this->CatchButton = NULL;
    }
    
    if (this->VolumeSelectionFrame ) {
        this->VolumeSelectionFrame->SetParent(NULL);
        this->VolumeSelectionFrame->Delete();
        this->VolumeSelectionFrame = NULL;
    }

    if (this->Sagittal2Scale ) {
        this->Sagittal2Scale->SetParent(NULL);
        this->Sagittal2Scale->Delete();
        this->Sagittal2Scale = NULL;
    }
    if (this->Coronal1Scale ) {
        this->Coronal1Scale->SetParent(NULL);
        this->Coronal1Scale->Delete();
        this->Coronal1Scale = NULL;
    }
    if (this->Coronal2Scale ) {
        this->Coronal2Scale->SetParent(NULL);
        this->Coronal2Scale->Delete();
        this->Coronal2Scale = NULL;
    }
    
  this->SetLogic (NULL);
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, NULL);

}

//----------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::AddGUIObservers ( ) 
{
  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand ); 
  
  this->StorageVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->MaskVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand ); 

  this->GADNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->CatchButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::RemoveGUIObservers ( )
{
  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->MaskVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->OutVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->StorageVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->GADNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  this->CatchButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->VolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if (selector == this->OutVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->OutVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML(); 
    this->UpdateGUI();
    }
  else if (selector == this->MaskVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->MaskVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML(); 
    this->UpdateGUI();
    }
   else if (selector == this->StorageVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->StorageVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    this->UpdateGUI();
    }
  if (selector == this->GADNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->GADNodeSelector->GetSelected() != NULL) 
    { 
    vtkMRMLMRIBiasFieldCorrectionNode* n = vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(this->GADNodeSelector->GetSelected());
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->MRIBiasFieldCorrectionNode, n);
    this->UpdateGUI();
    }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
    }
    
  else if (b == this->CatchButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    
    vtkSlicerApplicationGUI *applicationGUI     = this->GetApplicationGUI();
    
    double oldSliceSetting[3];
    oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->GetValue());
    oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale()->GetValue());
    oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale()->GetValue());
    
    double oldSliceSetting2[3];
    oldSliceSetting2[0] = double(applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetSliceOffset());
    oldSliceSetting2[1] = double(applicationGUI->GetApplicationLogic()->GetSliceLogic("Yellow")->GetSliceOffset());
    oldSliceSetting2[2] = double(applicationGUI->GetApplicationLogic()->GetSliceLogic("Green")->GetSliceOffset());
    
    // GET DATA FROM SLICES
    
    vtkImageData* image = vtkImageData::New(); 
    image->DeepCopy( applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetBackgroundLayer()->GetSlice()->GetOutput());//GetReslice()->GetOutput());
    vtkMRMLVolumeNode *backgroundNode = NULL;
    
    backgroundNode = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetLayerVolumeNode (0);//->GetImageData()->GetDimensions();
    //applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetLayerVolumeNode (0)->GetXYToIJK(xyToijk);
    int* dims = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetLayerVolumeNode (0)->GetImageData()->GetDimensions();
    
    vtkMatrix4x4* xyToras = vtkMatrix4x4::New();
    //backgroundNode->GetRASToIJKMatrix(xyToijk);
    //vtkImageData* image2 = applicationGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetImageData();
    //applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetBackgroundLayer()->GetVolumeNode();
    image->UpdateInformation();
    double bounds[6];
    image->GetBounds(bounds);
    /*std::cout<<"RESLICE BOUNDS" << std::endl;
    std::cout<<"bound 1: "<< bounds[0] << "to: "<< bounds[1] << std::endl;
    std::cout<<"bound 2: "<< bounds[2] << "to: "<< bounds[3] << std::endl;
    std::cout<<"bound 3: "<< bounds[4] << "to: "<< bounds[5] << std::endl;
    */
   int extent[6];
   image->GetWholeExtent(extent);
    /*
  std::cout<<"NODE DIMS"<<std::endl;
  std::cout<<"0: "<< dims[0]<<std::endl;
  std::cout<<"1: "<< dims[1]<< std::endl;
  std::cout<<"2: "<< dims[2]<< std::endl;
  */
  
    //vtkMRMLVolumeNode *backgroundNode = NULL;
    //backgroundNode = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetLayerVolumeNode (0);
    //vtkImageData* image2 = backgroundNode->GetImageData();
    //double bounds[3];
    double center[3];
    //applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->FitSliceToVolume(backgroundNode,50,50);
    
    vtkMRMLSliceNode *RedSliceNode = vtkMRMLSliceNode::New();
    vtkSlicerSliceGUI *sgui = NULL;
    double fov, fovX, fovY;
    
    fovX = RedSliceNode->GetFieldOfView()[0];
    fovY = RedSliceNode->GetFieldOfView()[1];
    /*
    std::cout<<"FOV RED" << std::endl;
    std::cout<<"X: "<< fovX << std::endl;
    std::cout<<"Y: "<< fovY << std::endl;
    */
    /*int *windowSize =
              applicationGUI->GetMainSliceGUI("Red")->GetOrientationString();//->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow()->GetSize();
             */
             
    double xyPt[4];
    xyPt[0] = 10;
    xyPt[1] = 10;
    xyPt[2] = 0;
    xyPt[3] = 1;
    
    vtkTransform* xyToijk = vtkTransform::New();
    xyToijk = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetBackgroundLayer()->GetXYToIJKTransform();
    
    double ijkPt[3];
    xyToijk->MultiplyPoint(xyPt,ijkPt);
    /*std::cout<<"XY TO IJK MATRIX" << std::endl;
    std::cout<< xyToijk[0] <<std::endl;
    
    std::cout<<"IN LOGIC IJK"<<std::endl;
    std::cout<<"i: "<< ijkPt[0] <<std::endl;
    std::cout<<"j: "<< ijkPt[1] <<std::endl;
    std::cout<<"k: "<< ijkPt[2] <<std::endl;*/
    
    vtkMRMLVolumeNode *vnode = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetBackgroundLayer()->GetVolumeNode();
    
    vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")->GetSliceNode();
    xyToras = snode->GetXYToRAS();
    int active = snode->GetActiveSlice();
    /*std::cout<<"ACTIVE SLICE" << std::endl;
    std::cout<< "Nb: "<< active <<std::endl;
    */
    // XY TO RAS MATRIX
    /*std::cout<<"XY TO RAS MATRIX" << std::endl;
    std::cout<< xyToras[0] <<std::endl;*/
    
    double rasPt[3];
    xyToras->MultiplyPoint(xyPt,rasPt);
  /*
  std::cout<<"IN LOGIC RAS"<<std::endl;
  std::cout<<"RED: "<< rasPt[0] <<std::endl;
  std::cout<<"YEL: "<< rasPt[1] <<std::endl;
  std::cout<<"GRE: "<< rasPt[2] <<std::endl;*/
    
    unsigned int dimensions[3];
    snode->GetDimensions(dimensions);
    /*std::cout<<"DIMENSIONS"<<std::endl;
    std::cout<<"1: "<<dimensions[0] <<std::endl;
    std::cout<<"2: "<<dimensions[1] <<std::endl;
    std::cout<<"3: "<<dimensions[2] <<std::endl;*/
    /*<< xyToras[1] << xyToras[2] <<std::endl;
    std::cout<< xyToras[3] << xyToras[4] << xyToras[5] <<std::endl;
    std::cout<< xyToras[6] << xyToras[7] << xyToras[8] <<std::endl;*/
    /*
    applicationGUI->GetApplicationLogic()->GetSliceLogic("Green")->GetSliceOffset();
    
    std::cout<<"ORIENTATION" << std::endl;
    //std::cout<<"VALUE: "<< applicationGUI->GetMainSliceGUI("Red")->GetOrientationString() << std::endl;
    std::cout<<"VALUE: "<< snode->GetOrientationString() << std::endl;
    
    double XY1[4];
    XY1[0] = 0;
    XY1[1] = 0;
    XY1[2] = 0;
    XY1[3] = 1;
    double IJK1[3];
    
    xyToijk->MultiplyPoint(XY1,IJK1);
    
    std::cout<<"IN LOGIC IJK"<<std::endl;
    std::cout<<"I1: "<< IJK1[0] <<std::endl;
    std::cout<<"J1: "<< IJK1[1] <<std::endl;
    std::cout<<"K1: "<< IJK1[2] <<std::endl;
    
    double XY2[4];
    XY2[0] = dimensions[0];
    XY2[1] = 0;
    XY2[2] = 0;
    XY2[3] = 1;
    double IJK2[3];
    
    xyToijk->MultiplyPoint(XY2,IJK2);
    
    std::cout<<"IN LOGIC IJK"<<std::endl;
    std::cout<<"I2: "<< IJK2[0] <<std::endl;
    std::cout<<"J2: "<< IJK2[1] <<std::endl;
    std::cout<<"K2: "<< IJK2[2] <<std::endl;
    
    double XY3[4];
    XY3[0] = 0;
    XY3[1] = dimensions[1];
    XY3[2] = 0;
    XY3[3] = 1;
    double IJK3[3];
    
    xyToijk->MultiplyPoint(XY3,IJK3);
    
    std::cout<<"IN LOGIC IJK"<<std::endl;
    std::cout<<"I3: "<< IJK3[0] <<std::endl;
    std::cout<<"J3: "<< IJK3[1] <<std::endl;
    std::cout<<"K3: "<< IJK3[2] <<std::endl;
    
    double XY4[4];
    XY4[0] = dimensions[0];
    XY4[1] = dimensions[1];
    XY4[2] = 0;
    XY4[3] = 1;
    double IJK4[3];
    
    xyToijk->MultiplyPoint(XY4,IJK4);
    
    std::cout<<"IN LOGIC IJK"<<std::endl;
    std::cout<<"I4: "<< IJK4[0] <<std::endl;
    std::cout<<"J4: "<< IJK4[1] <<std::endl;
    std::cout<<"K4: "<< IJK4[2] <<std::endl;
    */
    this->Logic->SliceProcess(xyToijk,dimensions[0],dimensions[1]);
#if 0
    /*
    int xyz[3];
    int x, y;
   // vtkSlicerInteractorStyle *RedEvents = GetRedSliceEvents();
    /*
    vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")->GetSliceNode();
  
    vtkRenderWindowInteractor *iren = applicationGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    int *windowSize = iren->GetRenderWindow()->GetSize();

    double tx = x / (double) windowSize[0];
    double ty = (windowSize[1] - y) / (double) windowSize[1];
  
    vtkRenderer *ren = iren->FindPokedRenderer(x, y);
    int *origin = ren->GetOrigin();
  
    xyz[0] = x - origin[0];
    xyz[1] = y - origin[1];
    xyz[2] = static_cast<int> ( floor(ty*snode->GetLayoutGridRows())
                              *snode->GetLayoutGridColumns()
                              + floor(tx*snode->GetLayoutGridColumns()) );
    
    
    */
    
    
    
    //sgui = applicationGUI->GetMainSliceGUI("Red");
    //sliceNode->GetFieldOfView()[2]
    /*applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetVolumeSliceDimensions(backgroundNode,bounds,center);
    std::cout<<"RED BOUNDS" << std::endl;
    std::cout<<"bound 1: "<< bounds[0] << std::endl;
    std::cout<<"bound 2: "<< bounds[1] << std::endl;
    std::cout<<"bound 3: "<< bounds[2] << std::endl;
    std::cout<<"RED CENTER" << std::endl;
    std::cout<<"center 1: "<< center[0] << std::endl;
    std::cout<<"center 2: "<< center[1] << std::endl;
    std::cout<<"center 3: "<< center[2] << std::endl;*/
    //GetLowestVolumeSliceBounds(bounds)
    //GetBackgroundSliceBounds
    /*std::cout<<"RED BOUNDS" << std::endl;
    std::cout<<"1: "<< bounds[0] <<" to: " << bounds[1] << std::endl;
    std::cout<<"2: "<< bounds[2] <<" to: " << bounds[3] << std::endl;
    std::cout<<"3: "<< bounds[4] <<" to: " << bounds[5] << std::endl;
    */
    //applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetVolumeRASBox
    
    
    //vtkKWEntryWithLabel* val;
    //val = applicationGUI->RedFOVEntry->GetWidget()->GetValueAsDouble();
    /*vtkSlicerSlicesControlGUI* test = vtkSlicerSlicesControlGUI::New();*/
    //val = test->GetRedFOVEntry();//->GetWidget()->GetValueAsDouble();
    
    //std::cout<<"FOV RED" << std::endl;
    //std::cout<<"content: "<< val->GetLabelText() << std::endl;
    
    //vtkSlicerApplicationLogic *applicationLogic = this->Logic->GetApplicationLogic();
    
    //applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    //applicationLogic->PropagateVolumeSelection();
    //vtkSlicerSliceLogic::GetSliceOffset()
     
    //applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->FitSliceToBackground(1);
   /* std::cout<<"FROM GUI" << std::endl;
    std::cout<<"ID RED: "<< oldSliceSetting[0] << std::endl;
    std::cout<<"ID YEL: "<< oldSliceSetting[1] << std::endl;
    std::cout<<"ID GRE: "<< oldSliceSetting[2] << std::endl;
    
    std::cout<<"FROM LOGIC" << std::endl;
    std::cout<<"ID RED: "<< oldSliceSetting2[0] << std::endl;
    std::cout<<"ID YEL: "<< oldSliceSetting2[1] << std::endl;
    std::cout<<"ID GRE: "<< oldSliceSetting2[2] << std::endl;*/
  /*
    std::cout<<"Slices positions RAS" << std::endl;
    std::cout<<"RED: "<< oldSliceSetting[0] << std::endl;
    std::cout<<"YEL: "<< oldSliceSetting[1] << std::endl;
    std::cout<<"GRE: "<< oldSliceSetting[2] << std::endl;
    */
    /*vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
    vtkIdType* inc  = inVolume->GetImageData()->GetIncrements(); */
    //vtkIdType inc[3];
   /* std::cout<<"INCS"<<std::endl;
    std::cout<<"0: "<< inc[0] <<std::endl;
    std::cout<<"1: "<< inc[1] <<std::endl;
    std::cout<<"2: "<< inc[2] <<std::endl;
    */
   // this->Logic->ApplyPreview(oldSliceSetting2[0],oldSliceSetting2[1],oldSliceSetting2[2],image);

    //applicationGUI->GetMainSliceGUI("Green")->GetSliceViewer()->SetImageData(image);//RequestRender();
    //applicationGUI->GetMainSliceGUI("Green")->GetSliceViewer()->RequestRender();
    
    //this->UpdateGUI();
      /*    vtkSlicerApplicationGUI *applicationGUI     = this->GetApplicationGUI();

      double oldSliceSetting;
      oldSliceSetting = double(applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->GetValue());
      //std::cout<<oldSliceSetting<<std::endl;
      this->Logic->Cut(oldSliceSetting);
    //this->Logic->Apply();*/
#endif
    }
  
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateMRML ()
{
  vtkMRMLMRIBiasFieldCorrectionNode* n = this->GetMRIBiasFieldCorrectionNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->GADNodeSelector->SetSelectedNew("vtkMRMLMRIBiasFieldCorrectionNode");
    this->GADNodeSelector->ProcessNewNodeCommand("vtkMRMLMRIBiasFieldCorrectionNode", "GADParameters");
    n = vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(this->GADNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);
  
  n->SetShrink((int)floor(this->ShrinkFactor->GetValue()));  
  
  n->SetMax((int)floor(this->MaxNumOfIt->GetValue()));  
  
  n->SetNum(this->NumOfFitLev->GetValue());
  
  n->SetWien(this->WienFilNoi->GetValue());
  
  n->SetField(this->BiasField->GetValue());
  
  n->SetCon(this->Conv->GetValue());
 
  if (this->VolumeSelector->GetSelected() != NULL)
    {
    n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }
  if (this->StorageVolumeSelector->GetSelected() != NULL)
    {
    n->SetStorageVolumeRef(this->StorageVolumeSelector->GetSelected()->GetID());
    }
  if (this->MaskVolumeSelector->GetSelected() != NULL)
    {
    n->SetMaskVolumeRef(this->MaskVolumeSelector->GetSelected()->GetID());
    }
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateGUI ()
{
  vtkMRMLMRIBiasFieldCorrectionNode* n = this->GetMRIBiasFieldCorrectionNode();
  if (n != NULL)
    {
    std::cout<<"Update GUI"<<std::endl;
    // set GUI widgest from parameter node

    this->ShrinkFactor->SetValue(n->GetShrink());
    
    this->MaxNumOfIt->SetValue(n->GetMax());
    
    this->NumOfFitLev->SetValue(n->GetNum());
    
    this->WienFilNoi->SetValue(n->GetWien());
    
    this->BiasField->SetValue(n->GetField());
    
    this->Conv->SetValue(n->GetCon());
/*
    vtkSlicerApplicationLogic *applicationLogic = this->Logic->GetApplicationLogic();
    applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    applicationLogic->PropagateVolumeSelection();
    
    vtkSlicerApplicationGUI *applicationGUI     = this->GetApplicationGUI();
    
    applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->FitSliceToBackground(1);
  */     
    }
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLMRIBiasFieldCorrectionNode* node = vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(caller);
  if (node != NULL && this->GetMRIBiasFieldCorrectionNode() == node) 
    {
    this->UpdateGUI();
    //this->Logic->Cut();
    std::cout<<"MRML changed"<<std::endl;
    }
}




//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  vtkMRMLMRIBiasFieldCorrectionNode* gadNode = vtkMRMLMRIBiasFieldCorrectionNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(gadNode);
  gadNode->Delete();

  this->UIPanel->AddPage ( "MRIBiasFieldCorrection", "MRIBiasFieldCorrection", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
   // Define your help text and build the help frame here.
    const char *help = "Documentation and screenshots are available at <a>http://wiki.slicer.org/slicerWiki/index.php/Modules:MRIBiasFieldCorrection-Documentation-3.5</a>\n\nInstructions:\n- Create a mask volume using the Editor module (the Threshold tool should give a good result)\n- Select the MRIBiasFieldCorrection module\n- Select the Input Volume\n- Select the Mask Volume\n- In Preview Volume, select Create New Volume\n- In Output Volume, select Create New Volume\n- Modify parameter values if desired (The default parameters gave good results during our experiments)\n- Click on Apply\n\nWe measured a 32 min duration to process a 512x512x30 voxel volume on a MacBook laptop.";
    const char *about = "The MRIBiasFieldCorrection module was developed by Nicolas Rannou (BWH) and Sylvain Jaume (MIT).\nThis work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "MRIBiasFieldCorrection" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    
    
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "MRIBiasFieldCorrection" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("MRI Bias Correction Module");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("MRIBiasFieldCorrection")->GetWidgetName());
  
  
  
  this->GADNodeSelector->SetNodeClass("vtkMRMLMRIBiasFieldCorrectionNode", NULL, NULL, "GADParameters");
  this->GADNodeSelector->SetNewNodeEnabled(1);
  this->GADNodeSelector->NoneEnabledOn();
  this->GADNodeSelector->SetShowHidden(1);
  this->GADNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->GADNodeSelector->Create();
  this->GADNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->GADNodeSelector->UpdateMenu();

  this->GADNodeSelector->SetBorderWidth(2);
  this->GADNodeSelector->SetLabelText( "IN Parameters");
  this->GADNodeSelector->SetBalloonHelpString("select a GAD node from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->GADNodeSelector->GetWidgetName());
  
  

  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText( "Input Volume: ");
  this->VolumeSelector->SetBalloonHelpString("select an input volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->VolumeSelector->GetWidgetName());
                
  this->MaskVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->MaskVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->MaskVolumeSelector->Create();
  this->MaskVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->MaskVolumeSelector->UpdateMenu();

  this->MaskVolumeSelector->SetBorderWidth(2);
  this->MaskVolumeSelector->SetLabelText( "Mask Volume: ");
  this->MaskVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->MaskVolumeSelector->GetWidgetName());
  
  this->StorageVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, "GADVolumeStorage");
  this->StorageVolumeSelector->SetNewNodeEnabled(1);
  this->StorageVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->StorageVolumeSelector->Create();
  this->StorageVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->StorageVolumeSelector->UpdateMenu();

  this->StorageVolumeSelector->SetBorderWidth(2);
  this->StorageVolumeSelector->SetLabelText( "Preview Volume: ");
  this->StorageVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->StorageVolumeSelector->GetWidgetName());
  
  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, "GADVolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText( "Output Volume: ");
  this->OutVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->OutVolumeSelector->GetWidgetName());
                  
////////////////////////////////////////////////////////////////////////////////////////////  

 if (!this->AdvancedParametersFrame)
    {
    this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
   }
  if (!this->AdvancedParametersFrame->IsCreated())
    {
    this->AdvancedParametersFrame->SetParent(moduleFrame->GetFrame());
    this->AdvancedParametersFrame->Create();
    //this->AdvancedParametersFrame->CollapseFrame();
    this->AdvancedParametersFrame->SetLabelText(
      "Parameters");
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->AdvancedParametersFrame->GetWidgetName());

  vtkKWFrame* aparametersFrame = this->AdvancedParametersFrame->GetFrame();
////////////////////////////////////////////////////////////////////////////////////////////


if (!this->ShrinkFactor)
    {
    this->ShrinkFactor = vtkKWScaleWithEntry::New();
    }
 if (!this->ShrinkFactor->IsCreated())
    {
    this->ShrinkFactor->SetParent(aparametersFrame);
    this->ShrinkFactor->Create();
    this->ShrinkFactor->SetRange(0,10);
    this->ShrinkFactor->SetResolution(1);
    this->ShrinkFactor->SetValue(3);
    this->ShrinkFactor->SetLabelText(
      "Set Shrink Factor");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->ShrinkFactor->GetWidgetName());


 if (!this->MaxNumOfIt)
    {
    this->MaxNumOfIt = vtkKWScaleWithEntry::New();
   }
 if (!this->MaxNumOfIt->IsCreated())
    {
    this->MaxNumOfIt->SetParent(aparametersFrame);
    this->MaxNumOfIt->Create();
    this->MaxNumOfIt->SetRange(0,100);
    this->MaxNumOfIt->SetResolution(1);
    this->MaxNumOfIt->SetValue(20);
    this->MaxNumOfIt->SetLabelText(
      "Set Max Number of Iterations");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->MaxNumOfIt->GetWidgetName());

 if (!this->NumOfFitLev)
    {
    this->NumOfFitLev = vtkKWScaleWithEntry::New();
    }
 if (!this->NumOfFitLev->IsCreated())
    {
    this->NumOfFitLev->SetParent(aparametersFrame);
    this->NumOfFitLev->Create();
    this->NumOfFitLev->SetRange(0,20);
    this->NumOfFitLev->SetResolution(1);
    this->NumOfFitLev->SetValue(4);
    this->NumOfFitLev->SetLabelText(
      "Set Number of Fitting Level");
    }

    //this->Script(
    //"pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    //this->NumOfFitLev->GetWidgetName());


 if (!this->WienFilNoi)
    {
    this->WienFilNoi = vtkKWScaleWithEntry::New();
    }
 if (!this->WienFilNoi->IsCreated())
    {
    this->WienFilNoi->SetParent(aparametersFrame);
    this->WienFilNoi->Create();
    this->WienFilNoi->SetRange(0,10);
    this->WienFilNoi->SetResolution(0.1);
    this->WienFilNoi->SetValue(0.1);
    this->WienFilNoi->SetLabelText(
      "Set Wien Filter Noise");
    }
    
    //this->Script(
    //"pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    //this->WienFilNoi->GetWidgetName());


 if (!this->BiasField)
    {
    this->BiasField = vtkKWScaleWithEntry::New();
    }
 if (!this->BiasField->IsCreated())
    {
    this->BiasField->SetParent(aparametersFrame);
    this->BiasField->Create();
    this->BiasField->SetRange(0,10);
    this->BiasField->SetResolution(0.05);
    this->BiasField->SetValue(4.1);
    this->BiasField->SetLabelText(
      "Set Bias Field Full.");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->BiasField->GetWidgetName());

 if (!this->Conv)
    {
    this->Conv = vtkKWScaleWithEntry::New();
    }
 if (!this->Conv->IsCreated())
    {
    this->Conv->SetParent(aparametersFrame);
    this->Conv->Create();
    this->Conv->SetRange(0,10);
    this->Conv->SetResolution(0.001);
    this->Conv->SetValue(0.001);
    this->Conv->SetLabelText(
      "Set Convergence Threshold");
    }
    
    //this->Script(
    //"pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    //this->Conv->GetWidgetName());




////////////////////////////////////////////////////////////////////////////////////////////

  this->CatchButton->SetParent(moduleFrame->GetFrame());
  this->CatchButton->Create();
  this->CatchButton->SetText("Preview");
  this->CatchButton->SetWidth ( 8 );
  //app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
    //            this->CatchButton->GetWidgetName());

  this->ApplyButton->SetParent(moduleFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());

  moduleFrame->Delete();
                               
    /*        
  mask_page->Delete();
  bias_page->Delete();              
  treshFrame->Delete();
  moduleFrame->Delete();
  aparametersFrame->Delete();
  parametersFrame->Delete();
  this->Notebook->Delete();
*/

vtkMRMLMRIBiasFieldCorrectionNode* n = this->GetMRIBiasFieldCorrectionNode();
  
}
