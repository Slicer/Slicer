/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerTractographyFiducialSeedingGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkSlicerTractographyFiducialSeedingGUI.h"

#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkMath.h"
#include "vtkCommand.h"

#include "vtkDiffusionTensorMathematics.h"
#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkSeedTracts.h"

#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiberBundleNode.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

//------------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingGUI* vtkSlicerTractographyFiducialSeedingGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerTractographyFiducialSeedingGUI");
  if(ret)
    {
      return (vtkSlicerTractographyFiducialSeedingGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerTractographyFiducialSeedingGUI;
}


//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingGUI::vtkSlicerTractographyFiducialSeedingGUI()
{
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutFiberSelector = vtkSlicerNodeSelectorWidget::New();
  this->FiducialSelector = vtkSlicerNodeSelectorWidget::New();
  this->FiducialListNode = NULL;
  
  this->StoppingMode = NULL;
  this->StoppingThreshold=0.15;
  this->MaximumPropagationDistance = 600;

}

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingGUI::~vtkSlicerTractographyFiducialSeedingGUI()
{
    
  if ( this->VolumeSelector ) 
  {
    this->VolumeSelector->SetParent(NULL);
    this->VolumeSelector->Delete();
    this->VolumeSelector = NULL;
  }
  if ( this->OutFiberSelector ) 
  {
    this->OutFiberSelector->SetParent(NULL);
    this->OutFiberSelector->Delete();
    this->OutFiberSelector = NULL;
  }
  if ( this->FiducialSelector ) 
  {
    this->FiducialSelector->SetParent(NULL);
    this->FiducialSelector->Delete();
    this->FiducialSelector = NULL;
  }
  vtkSetAndObserveMRMLNodeMacro(this->FiducialListNode, NULL);
  
  if (this->StoppingMode)
    {
    delete [] this->StoppingMode;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::AddGUIObservers ( ) 
{
  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutFiberSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->FiducialSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

}



//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::RemoveGUIObservers ( )
{
  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutFiberSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->FiducialSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

}



//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->VolumeSelector->GetSelected() != NULL) 
    { 
    this->CreateTracts();
    }
  else if (selector == this->OutFiberSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->OutFiberSelector->GetSelected() != NULL) 
    { 
    this->CreateTracts();
    }
  if (selector == this->FiducialSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->FiducialSelector->GetSelected() != NULL) 
    { 
    vtkMRMLFiducialListNode* n = vtkMRMLFiducialListNode::SafeDownCast(this->FiducialSelector->GetSelected());
    vtkIntArray *events = vtkIntArray::New();
    //events->InsertNextValue( vtkCommand::ModifiedEvent);
    events->InsertNextValue( vtkMRMLFiducialListNode::FiducialModifiedEvent);
    events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro(this->FiducialListNode, n, events);
    events->Delete();

    this->CreateTracts();
    }
  
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLFiducialListNode* node = vtkMRMLFiducialListNode::SafeDownCast(caller);
  if (node != NULL && this->FiducialListNode == node) 
    {
    this->CreateTracts();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI:: CreateTracts()
{
  vtkMRMLDiffusionTensorVolumeNode *volumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->VolumeSelector->GetSelected());
  vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(this->FiducialSelector->GetSelected());
  vtkMRMLFiberBundleNode *fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(this->OutFiberSelector->GetSelected());
  
  // 0. check inputs
  if (volumeNode == NULL || fiducialListNode == NULL || fiberNode == NULL ||
      volumeNode->GetImageData() == NULL || fiducialListNode->GetNumberOfFiducials() == 0)
    {
    if (fiberNode && fiberNode->GetPolyData())
      {
      fiberNode->GetPolyData()->Reset();
      }
    return;
    }
    
  vtkSeedTracts *seed = vtkSeedTracts::New();
  
  //1. Set Input
  seed->SetInputTensorField(volumeNode->GetImageData());
  
  //2. Set Up matrices
  vtkMatrix4x4 *mat = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(mat);
  
  vtkMatrix4x4 *TensorRASToIJK = vtkMatrix4x4::New();
  TensorRASToIJK->DeepCopy(mat);
  mat->Delete();
  
  //Do scale IJK
  double sp[3];
  double spold[3];
  volumeNode->GetSpacing(sp);
  // putr spacing into image so that tractography knows about
  volumeNode->GetImageData()->GetSpacing(spold);
  volumeNode->GetImageData()->SetSpacing(sp);

  vtkTransform *trans = vtkTransform::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(TensorRASToIJK);
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  trans->Inverse();
  
  //Set Transformation to seeding class
  seed->SetWorldToTensorScaledIJK(trans);
  
  vtkMatrix4x4 *TensorRASToIJKRotation = vtkMatrix4x4::New();
  TensorRASToIJKRotation->DeepCopy(TensorRASToIJK);
  
  //Set Translation to zero
  for (int i=0;i<3;i++)
    {
    TensorRASToIJKRotation->SetElement(i,3,0);
    }
  //Remove scaling in rasToIjk to make a real roation matrix
  double col[3];
  for (int jjj = 0; jjj < 3; jjj++) 
    {
    for (int iii = 0; iii < 3; iii++)
      {
      col[iii]=TensorRASToIJKRotation->GetElement(iii,jjj);
      }
    vtkMath::Normalize(col);
    for (int iii = 0; iii < 3; iii++)
      {
      TensorRASToIJKRotation->SetElement(iii,jjj,col[iii]);
     }  
  }
  TensorRASToIJKRotation->Invert();
  seed->SetTensorRotationMatrix(TensorRASToIJKRotation);  
  

  //ROI comes from tensor, IJKToRAS is the same
  // as the tensor
  vtkTransform *trans2 = vtkTransform::New();
  trans2->Identity();
  trans2->SetMatrix(TensorRASToIJK);
  trans2->Inverse();
  seed->SetROIToWorld(trans2);
  
  seed->UseVtkHyperStreamlinePoints();
  vtkHyperStreamlineDTMRI *streamer=vtkHyperStreamlineDTMRI::New();
  seed->SetVtkHyperStreamlinePointsSettings(streamer);
 
  if (this->StoppingMode && std::string(this->StoppingMode) == std::string("LinearMeasurement"))
    {
     streamer->SetStoppingModeToLinearMeasure();
    }
  else if (this->StoppingMode && std::string(this->StoppingMode) == std::string("PlanarMeasurement"))
    {  
    streamer->SetStoppingModeToPlanarMeasure();
    }
  else if (this->StoppingMode && std::string(this->StoppingMode) == std::string("FractionalAnisotropy"))
    {  
    streamer->SetStoppingModeToFractionalAnisotropy();
    }
    
  streamer->SetStoppingThreshold(this->StoppingThreshold);
  streamer->SetMaximumPropagationDistance(this->MaximumPropagationDistance);
  
  // Temp fix to provide a scalar
  seed->GetInputTensorField()->GetPointData()->SetScalars(volumeNode->GetImageData()->GetPointData()->GetScalars());
  
  // loop over fiducials
  int nf = fiducialListNode->GetNumberOfFiducials();
  for (int f=0; f<nf; f++)
    {
    float *xyz = fiducialListNode->GetNthFiducialXYZ(f);
    //Run the thing
    seed->SeedStreamlineFromPoint(xyz[0], xyz[1], xyz[2]);
    }
    
  //6. Extra5ct PolyData in RAS
  vtkPolyData *outFibers = vtkPolyData::New();
  
  seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers);
  
  fiberNode->SetAndObservePolyData(outFibers);
  
  vtkMRMLFiberBundleDisplayNode *dnode = fiberNode->GetLineDisplayNode();
  if (dnode == NULL)
    {
    dnode = fiberNode->AddLineDisplayNode();
    dnode->SetVisibility(1);
    }
    
  dnode = fiberNode->GetTubeDisplayNode();
  if (dnode == NULL)
    {
    dnode = fiberNode->AddTubeDisplayNode();
    dnode->SetVisibility(0);
    }
  
  dnode = fiberNode->GetGlyphDisplayNode();
  if (dnode == NULL)
    {
    dnode = fiberNode->AddGlyphDisplayNode();
    dnode->SetVisibility(0);
    }
  // Restore the original spacing
  volumeNode->GetImageData()->SetSpacing(spold);

  fiberNode->InvokeEvent(vtkMRMLFiberBundleNode::PolyDataModifiedEvent, NULL);
  
  // Delete everything: Still trying to figure out what is going on
  outFibers->Delete();
  seed->Delete();
  TensorRASToIJK->Delete();
  TensorRASToIJKRotation->Delete();
  trans2->Delete();
  trans->Delete();
  streamer->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  this->UIPanel->AddPage ( "Tractography", "Tractography", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
  // Define your help text and build the help frame here.
  const char *help = "The FiducialSeeding module....";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Tractography" );
  this->BuildHelpAndAboutFrame ( page, help, about );
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Tractography Seeding From Fiducial");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Tractography")->GetWidgetName());
  
  this->VolumeSelector->SetNodeClass("vtkMRMLDiffusionTensorVolumeNode", NULL, NULL, NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->GetApplicationLogic()->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();
  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText( "Select DTI Volume: ");
  this->VolumeSelector->SetBalloonHelpString("select an input DT volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->VolumeSelector->GetWidgetName());
  
  this->FiducialSelector->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
  this->FiducialSelector->SetNewNodeEnabled(0);
  this->FiducialSelector->NoneEnabledOn();
  this->FiducialSelector->SetShowHidden(1);
  this->FiducialSelector->SetParent( moduleFrame->GetFrame() );
  this->FiducialSelector->Create();
  this->FiducialSelector->SetMRMLScene(this->GetApplicationLogic()->GetMRMLScene());
  this->FiducialSelector->UpdateMenu();
  this->FiducialSelector->SetBorderWidth(2);
  this->FiducialSelector->SetLabelText( "Select FiducialList");
  this->FiducialSelector->SetBalloonHelpString("select a fiducial list for seeding.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->FiducialSelector->GetWidgetName());


  this->OutFiberSelector->SetNodeClass("vtkMRMLFiberBundleNode", NULL, NULL, "FiberBundleNode");
  this->OutFiberSelector->SetNewNodeEnabled(1);
  this->OutFiberSelector->SetParent( moduleFrame->GetFrame() );
  this->OutFiberSelector->Create();
  this->OutFiberSelector->SetMRMLScene(this->GetApplicationLogic()->GetMRMLScene());
  this->OutFiberSelector->UpdateMenu();
  this->OutFiberSelector->SetBorderWidth(2);
  this->OutFiberSelector->SetLabelText( "Output FiberBundleNode: ");
  this->OutFiberSelector->SetBalloonHelpString("select an output fiber bundle node.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->OutFiberSelector->GetWidgetName());


  moduleFrame->Delete();

  
}
