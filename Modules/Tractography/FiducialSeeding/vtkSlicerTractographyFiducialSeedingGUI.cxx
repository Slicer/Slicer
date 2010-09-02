/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerTractographyFiducialSeedingGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkSlicerApplication.h"

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
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLTransformNode.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWCheckButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWScale.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWPushButton.h"

#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkSlicerTractographyFiducialSeedingLogic.h"

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
  this->SeedButton  = vtkKWCheckButton::New();
  this->SeedSelectedFiducialsButton  = vtkKWCheckButton::New();
  this->StoppingModeMenu = vtkKWMenuButtonWithLabel::New();
  this->StoppingValueScale = vtkKWScaleWithLabel::New();
  this->StoppingCurvatureScale = vtkKWScaleWithLabel::New();
  this->IntegrationStepLengthScale = vtkKWScaleWithLabel::New();
  this->MinimumPathLengthScale = vtkKWScaleWithLabel::New();
  this->RegionSizeScale = vtkKWScaleWithLabel::New();
  this->RegionSampleSizeScale = vtkKWScaleWithLabel::New();
  this->MaxNumberOfSeedsEntry = vtkKWEntryWithLabel::New();
  this->DisplayMenu = vtkKWPushButton::New();

  this->TractographyFiducialSeedingNodeSelector = vtkSlicerNodeSelectorWidget::New();

  this->TransformableNode = NULL;
  
  this->StoppingMode = NULL;
  this->StoppingThreshold=0.15;
  this->MaximumPropagationDistance = 600;
  this->OverwritePolyDataWarning =1;

//   this->RegisteredNode = 0;
  this->TractographyFiducialSeedingNode = NULL;

  UpdatingMRML = 0;
  UpdatingGUI = 0;

  // Try to load supporting libraries dynamically.  This is needed
  // since the toplevel is a loadable module but the other libraries
  // didn't get loaded
  Tcl_Interp* interp = this->GetApplication()->GetMainInterp();
  if (interp)
    {
    Vtkslicertractographymodulelogic_Init(interp);
    }
  else
    {
    vtkErrorMacro("Failed to obtain reference to application TCL interpreter");
    }
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
  
  if ( this->SeedButton ) 
  {
    this->SeedButton->SetParent(NULL);
    this->SeedButton->Delete();
    this->SeedButton = NULL;
  }  
  
  if ( this->StoppingModeMenu ) 
  {
    this->StoppingModeMenu->SetParent(NULL);
    this->StoppingModeMenu->Delete();
    this->StoppingModeMenu = NULL;
  }

  if ( this->DisplayMenu ) 
  {
    this->DisplayMenu->SetParent(NULL);
    this->DisplayMenu->Delete();
    this->DisplayMenu = NULL;
  }
  
  if ( this->StoppingValueScale ) 
  {
    this->StoppingValueScale->SetParent(NULL);
    this->StoppingValueScale->Delete();
    this->StoppingValueScale = NULL;
  }
  if ( this->StoppingCurvatureScale) 
  {
    this->StoppingCurvatureScale->SetParent(NULL);
    this->StoppingCurvatureScale->Delete();
    this->StoppingCurvatureScale = NULL;
  }
  if ( this->IntegrationStepLengthScale ) 
  {
    this->IntegrationStepLengthScale->SetParent(NULL);
    this->IntegrationStepLengthScale->Delete();
    this->IntegrationStepLengthScale = NULL;
  }
  if ( this->MinimumPathLengthScale ) 
  {
    this->MinimumPathLengthScale->SetParent(NULL);
    this->MinimumPathLengthScale->Delete();
    this->MinimumPathLengthScale = NULL;
  }
  if ( this->RegionSizeScale ) 
  {
    this->RegionSizeScale->SetParent(NULL);
    this->RegionSizeScale->Delete();
    this->RegionSizeScale = NULL;
  }
  
  if ( this->RegionSampleSizeScale ) 
  {
    this->RegionSampleSizeScale->SetParent(NULL);
    this->RegionSampleSizeScale->Delete();
    this->RegionSampleSizeScale = NULL;
  }
  
  if ( this->MaxNumberOfSeedsEntry ) 
  {
    this->MaxNumberOfSeedsEntry->SetParent(NULL);
    this->MaxNumberOfSeedsEntry->Delete();
    this->MaxNumberOfSeedsEntry = NULL;
  }

  if ( this->SeedSelectedFiducialsButton ) 
  {
    this->SeedSelectedFiducialsButton->SetParent(NULL);
    this->SeedSelectedFiducialsButton->Delete();
    this->SeedSelectedFiducialsButton = NULL;
  }

  if ( this->TractographyFiducialSeedingNodeSelector ) 
  {
    this->TractographyFiducialSeedingNodeSelector->SetParent(NULL);
    this->TractographyFiducialSeedingNodeSelector->Delete();
    this->TractographyFiducialSeedingNodeSelector = NULL;
  }

  vtkSetAndObserveMRMLNodeMacro(this->TransformableNode, NULL);
  
  vtkSetMRMLNodeMacro(this->TractographyFiducialSeedingNode, NULL);

  if (this->StoppingMode)
    {
    delete [] this->StoppingMode;
    }
}

void vtkSlicerTractographyFiducialSeedingGUI::SetModuleLogic ( vtkSlicerLogic *logic )
{
  this->ModuleLogic = vtkSlicerTractographyFiducialSeedingLogic::SafeDownCast(logic);
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::PrintSelf(
  ostream& vtkNotUsed(os), vtkIndent vtkNotUsed(indent) )
{
  
}

//---------------------------------------------------------------------------
vtkIntArray* vtkSlicerTractographyFiducialSeedingGUI::NewObservableEvents()
{
  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  return events;
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::AddGUIObservers ( ) 
{
  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutFiberSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->FiducialSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand ); 
  
  this->SeedSelectedFiducialsButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand ); 

  this->SeedButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand ); 

  this->StoppingValueScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->StoppingCurvatureScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->IntegrationStepLengthScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->MinimumPathLengthScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->RegionSizeScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->RegionSampleSizeScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
 
  this->MaxNumberOfSeedsEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TractographyFiducialSeedingNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->DisplayMenu->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::RemoveGUIObservers ( )
{
  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutFiberSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->FiducialSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->SeedSelectedFiducialsButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand ); 

  this->SeedButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand ); 

  this->StoppingValueScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->StoppingCurvatureScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->IntegrationStepLengthScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->MinimumPathLengthScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
 
  this->RegionSizeScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->RegionSampleSizeScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->MaxNumberOfSeedsEntry->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TractographyFiducialSeedingNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->DisplayMenu->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void * vtkNotUsed(callData) ) 
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
    vtkMRMLFiberBundleNode *fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(this->OutFiberSelector->GetSelected());
    
    int createFiber = 1;
    vtkMRMLDiffusionTensorVolumeNode *volumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->VolumeSelector->GetSelected());
    vtkMRMLTransformableNode *fiducialListNode = vtkMRMLTransformableNode::SafeDownCast(this->FiducialSelector->GetSelected());  

    int existingFiber = 0;
    if (this->OverwritePolyDataWarning && volumeNode && fiducialListNode  && fiberNode && fiberNode->GetPolyData() != NULL)
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
      message->SetStyleToYesNo();
      std::string msg = "TractographyFiducialSeeding is about to create new tracts for " + std::string(fiberNode->GetName()) + " which contains polydata. Do you want to override it?";
      message->SetText(msg.c_str());
      message->Create();
      createFiber = message->Invoke();
      if (!createFiber) 
        {
        this->OutFiberSelector->SetSelected(NULL);
        }
      existingFiber = 1;
      message->Delete();
      }

    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->FiducialSelector->GetSelected());
    if (!existingFiber && node) 
      {
      std::string name = std::string(node->GetName())+ std::string("_FiberTracts");
      fiberNode->SetName(name.c_str());
      this->OutFiberSelector->UpdateMenu();
      }

    if (createFiber)
      {
      this->CreateTracts();
      }
    }
  if (selector == this->FiducialSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->FiducialSelector->GetSelected() != NULL) 
    {
    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->FiducialSelector->GetSelected());
    if (node)
      {
      this->AddTransformableNodeObserver(node);
      }
    vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(node);
    vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(node);

    if (fiducialListNode)
      {
      this->MaxNumberOfSeedsEntry->SetEnabled(0);
      this->RegionSampleSizeScale->SetEnabled(1);
      this->RegionSizeScale->SetEnabled(1);
      this->SeedSelectedFiducialsButton->SetEnabled(1);
      this->RegionSizeScale->GetWidget()->SetValue(fiducialListNode->GetSymbolScale());
      this->RegionSampleSizeScale->GetWidget()->SetValue(fiducialListNode->GetSymbolScale()/3.0);
      }
    else if (modelNode)
      {
      this->MaxNumberOfSeedsEntry->SetEnabled(1);
      this->RegionSampleSizeScale->SetEnabled(0);
      this->RegionSizeScale->SetEnabled(0);
      this->SeedSelectedFiducialsButton->SetEnabled(0);
      }
    else
      {
      return;
      }
    this->CreateTracts();
    }

  if (selector == this->TractographyFiducialSeedingNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->TractographyFiducialSeedingNodeSelector->GetSelected() != NULL) 
    { 
    vtkMRMLTractographyFiducialSeedingNode* n = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(this->TractographyFiducialSeedingNodeSelector->GetSelected());
    vtkSetAndObserveMRMLNodeMacro( this->TractographyFiducialSeedingNode, n);
    this->UpdateGUI();
    }

  else if ( this->SeedButton == vtkKWCheckButton::SafeDownCast(caller) &&
          event == vtkKWCheckButton::SelectedStateChangedEvent ) 
    {
    this->CreateTracts();
    }
  else if ( this->SeedSelectedFiducialsButton == vtkKWCheckButton::SafeDownCast(caller) &&
          event == vtkKWCheckButton::SelectedStateChangedEvent ) 
    {
    this->CreateTracts();
    }
  else if (event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->CreateTracts();
    }
  else if (event == vtkKWEntry::EntryValueChangedEvent && 
          this->MaxNumberOfSeedsEntry->GetWidget() ==  vtkKWEntry::SafeDownCast(caller)) 
    {
    vtkMRMLTractographyFiducialSeedingNode* n = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(this->TractographyFiducialSeedingNodeSelector->GetSelected());
    if (n == NULL || (n && n->GetMaxNumberOfSeeds() != this->MaxNumberOfSeedsEntry->GetWidget()->GetValueAsInt()) )
      {
      this->CreateTracts();
      }
    }
  else if (vtkKWPushButton::SafeDownCast(caller) == this->DisplayMenu && 
        event == vtkKWPushButton::InvokedEvent)
    {
    this->GetApplicationGUI()->SelectModule("FiberBundles", 
                                     this->OutFiberSelector->GetSelected());
    }

}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::UpdateGUI ()
{
  if (this->UpdatingMRML)
    {
    return;
    }
  this->UpdatingGUI = 1;

  vtkMRMLTractographyFiducialSeedingNode* n = this->GetTractographyFiducialSeedingNode();
  if (n != NULL && this->GetMRMLScene() != NULL)
    {
    // set GUI widgest from parameter node
    
    if (n->GetInputVolumeRef())
      {
      vtkMRMLNode *vol = NULL;
      vol = this->GetMRMLScene()->GetNodeByID(n->GetInputVolumeRef());
      if (vol)
        {
        this->VolumeSelector->SetSelected(vol);
        }
      }

    if (n->GetInputFiducialRef())
      {
      vtkMRMLNode *fid = NULL;
      fid = this->GetMRMLScene()->GetNodeByID(n->GetInputFiducialRef());
      if (fid)
        {
        this->FiducialSelector->SetSelected(fid);
        }

      vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(fid);
      vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(fid);

      if (fiducialListNode)
        {
        this->MaxNumberOfSeedsEntry->SetEnabled(0);
        this->RegionSampleSizeScale->SetEnabled(1);
        this->RegionSizeScale->SetEnabled(1);
        this->RegionSizeScale->GetWidget()->SetValue(fiducialListNode->GetSymbolScale());
        this->RegionSampleSizeScale->GetWidget()->SetValue(fiducialListNode->GetSymbolScale()/3.0);
        }
      else if (modelNode)
        {
        this->MaxNumberOfSeedsEntry->SetEnabled(1);
        this->RegionSampleSizeScale->SetEnabled(0);
        this->RegionSizeScale->SetEnabled(0);
        }
      }

   
    if (n->GetOutputFiberRef())
      {
      vtkMRMLNode *fibre = NULL;
      fibre = this->GetMRMLScene()->GetNodeByID(n->GetOutputFiberRef());
      if (fibre)
        {
        this->OutFiberSelector->SetSelected(fibre);
        }
      }
    
    if(n->GetStoppingMode() == 0)
      {
      this->StoppingModeMenu->GetWidget()->SetValue("Linear Measure");
      }
    else
      {
      this->StoppingModeMenu->GetWidget()->SetValue ( "Fractional Anisotropy");
      }
    this->StoppingValueScale->GetWidget()->SetValue(n->GetStoppingValue());
    this->StoppingCurvatureScale->GetWidget()->SetValue(n->GetStoppingCurvature());
    this->IntegrationStepLengthScale->GetWidget()->SetValue(n->GetIntegrationStep());
    this->MinimumPathLengthScale->GetWidget()->SetValue(n->GetMinimumPathLength());
    this->RegionSizeScale->GetWidget()->SetValue(n->GetSeedingRegionSize());
    this->RegionSampleSizeScale->GetWidget()->SetValue(n->GetSeedingRegionStep());
    this->MaxNumberOfSeedsEntry->GetWidget()->SetValueAsInt(n->GetMaxNumberOfSeeds());
    this->SeedSelectedFiducialsButton->SetSelectedState(n->GetSeedSelectedFiducials());
    }

  this->UpdatingGUI = 0;
}
//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::UpdateMRML ()
{
  if (this->UpdatingGUI)
    {
    return;
    }
  this->UpdatingMRML = 1;

  vtkMRMLTractographyFiducialSeedingNode* n = this->GetTractographyFiducialSeedingNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->TractographyFiducialSeedingNodeSelector->SetSelectedNew("vtkMRMLTractographyFiducialSeedingNode");
    this->TractographyFiducialSeedingNodeSelector->ProcessNewNodeCommand("vtkMRMLTractographyFiducialSeedingNode", "Parameters");
    n = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(this->TractographyFiducialSeedingNodeSelector->GetSelected());

    // set an observe new node
    vtkSetAndObserveMRMLNodeMacro(this->TractographyFiducialSeedingNode, n);
   }
  // save node parameters for Undo
  this->GetMRMLScene()->SaveStateForUndo(n);

  if (this->VolumeSelector->GetSelected())
    {
    n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }
  if (this->FiducialSelector->GetSelected())
    {
    n->SetInputFiducialRef(this->FiducialSelector->GetSelected()->GetID());
    }
  if (this->OutFiberSelector->GetSelected())
    {
    n->SetOutputFiberRef(this->OutFiberSelector->GetSelected()->GetID());
    }
    
  if(!strcmp(this->StoppingModeMenu->GetWidget()->GetValue(), "Linear Measure"))
    {
    n->SetStoppingMode(0);
    }
  else
    if(!strcmp(this->StoppingModeMenu->GetWidget()->GetValue(), "Fractional Anisotropy"))
    {
    n->SetStoppingMode(1);
    }
  n->SetStoppingValue(this->StoppingValueScale->GetWidget()->GetValue() );
  n->SetStoppingCurvature( this->StoppingCurvatureScale->GetWidget()->GetValue() );
  n->SetIntegrationStep( this->IntegrationStepLengthScale->GetWidget()->GetValue() );
  n->SetMinimumPathLength( this->MinimumPathLengthScale->GetWidget()->GetValue() );
  n->SetSeedingRegionSize( this->RegionSizeScale->GetWidget()->GetValue() );
  n->SetSeedingRegionStep( this->RegionSampleSizeScale->GetWidget()->GetValue() );
  n->SetMaxNumberOfSeeds( this->MaxNumberOfSeedsEntry->GetWidget()->GetValueAsInt() );
  n->SetSeedSelectedFiducials( this->SeedSelectedFiducialsButton->GetSelectedState() );
  this->UpdatingMRML = 0;

 }

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  if (this->UpdatingMRML)
    {
    return;
    }
  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    vtkSetAndObserveMRMLNodeMacro( this->TractographyFiducialSeedingNode, NULL);
    this->AddTransformableNodeObserver(NULL);
    return;
    }
  // if parameter node has been added, update GUI widgets with new values
  vtkMRMLTractographyFiducialSeedingNode* snode = reinterpret_cast<vtkMRMLTractographyFiducialSeedingNode*> (callData);
  if (snode && event == vtkMRMLScene::NodeAddedEvent && snode->IsA("vtkMRMLTractographyFiducialSeedingNode") &&
    this->GetTractographyFiducialSeedingNode() == NULL)
    {
    vtkSetAndObserveMRMLNodeMacro( this->TractographyFiducialSeedingNode, snode);
   
    vtkMRMLTransformableNode *fn = NULL;
    if (this->GetMRMLScene() && this->TractographyFiducialSeedingNode->GetInputFiducialRef())
      {
      fn = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
          this->TractographyFiducialSeedingNode->GetInputFiducialRef()));
    
      }
    if (fn)
      {
      this->AddTransformableNodeObserver(fn);
      }
    this->UpdateGUI();
    this->CreateTracts();
    return;
    }

  // if parameter node has been changed externally, update GUI widgets with new values
  snode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(caller);
  if (snode != NULL && this->GetTractographyFiducialSeedingNode() == snode) 
    {
    vtkMRMLTransformableNode *fn = NULL;
    if (this->GetMRMLScene() && this->TractographyFiducialSeedingNode->GetInputFiducialRef())
      {
      fn = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
          this->TractographyFiducialSeedingNode->GetInputFiducialRef()));
    
      }
    if (fn != this->TransformableNode)
      {
      this->AddTransformableNodeObserver(fn);
      }
    this->UpdateGUI();
    this->CreateTracts();
    return;
    }

  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(caller);
  if (node != NULL && this->TransformableNode == node) 
    {
    this->CreateTracts();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::AddTransformableNodeObserver(vtkMRMLTransformableNode *n)
{
    vtkIntArray *events = vtkIntArray::New();
    //events->InsertNextValue( vtkCommand::ModifiedEvent);
    events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent);
    if (vtkMRMLModelNode::SafeDownCast(n) != NULL) 
      {
      events->InsertNextValue( vtkMRMLModelNode::PolyDataModifiedEvent);
      }
    else if (vtkMRMLFiducialListNode::SafeDownCast(n) != NULL) 
      {
      events->InsertNextValue( vtkMRMLFiducialListNode::FiducialModifiedEvent);
      }
    vtkSetAndObserveMRMLNodeEventsMacro(this->TransformableNode, n, events);
    events->Delete();
  }

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::CreateTracts()
{
  this->UpdateMRML();
  if ( this->SeedButton->GetSelectedState() == 0) 
    {
    return;
    }
  
  vtkMRMLDiffusionTensorVolumeNode *volumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->VolumeSelector->GetSelected());
  vtkMRMLTransformableNode *fiducialListNode = vtkMRMLTransformableNode::SafeDownCast(this->FiducialSelector->GetSelected());
  vtkMRMLFiberBundleNode *fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(this->OutFiberSelector->GetSelected());
  
  std::string stoppingMode = this->StoppingModeMenu->GetWidget()->GetValue ();
  int istoppingMode = 0;
  if ( stoppingMode == std::string("Linear Measurement") || 
       stoppingMode == std::string("Linear Measure")  )
    {
     istoppingMode = 0;
    }
  else 
    {  
     istoppingMode = 1;
    }  

  if(volumeNode == NULL || fiducialListNode == NULL || fiberNode == NULL)
    {
    return;
    }

  int displayMode = 0;

  this->ModuleLogic->CreateTracts(volumeNode, fiducialListNode, fiberNode,
                                                          istoppingMode,
                                                          this->StoppingValueScale->GetWidget()->GetValue(),
                                                          this->StoppingCurvatureScale->GetWidget()->GetValue(),
                                                          this->IntegrationStepLengthScale->GetWidget()->GetValue(),
                                                          this->MinimumPathLengthScale->GetWidget()->GetValue(),
                                                          this->RegionSizeScale->GetWidget()->GetValue(),
                                                          this->RegionSampleSizeScale->GetWidget()->GetValue(),
                                                          this->MaxNumberOfSeedsEntry->GetWidget()->GetValueAsInt(),
                                                          this->SeedSelectedFiducialsButton->GetSelectedState(),
                                                          displayMode);  
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::BuildGUI ( ) 
{
  if (this->Built)
    {
    return;
    }

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  this->UIPanel->AddPage ( "Tractography", "Tractography", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
  // Define your help text and build the help frame here.
  const char *help = "**FiducialSeeding Module:** Generate fiber tracts seeded at selected fiducal or model points \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:FiducialSeeding-Documentation-3.6</a>\n";
  const char *about = "This module was contributed by Alex Yarmarkovich, and Steve Pieper Isomics Inc., SPL/BWH.\nThis work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Tractography" );
  this->BuildHelpAndAboutFrame ( page, help, about );
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Tractography Seeding From Fiducial");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Tractography")->GetWidgetName());
  
  this->TractographyFiducialSeedingNodeSelector->SetNodeClass("vtkMRMLTractographyFiducialSeedingNode", NULL, NULL, "Parameters");
  this->TractographyFiducialSeedingNodeSelector->SetNewNodeEnabled(1);
  this->TractographyFiducialSeedingNodeSelector->NoneEnabledOff();
  this->TractographyFiducialSeedingNodeSelector->SetShowHidden(1);
  this->TractographyFiducialSeedingNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->TractographyFiducialSeedingNodeSelector->Create();
  this->TractographyFiducialSeedingNodeSelector->SetMRMLScene(this->GetMRMLScene());
  this->TractographyFiducialSeedingNodeSelector->UpdateMenu();

  this->TractographyFiducialSeedingNodeSelector->SetBorderWidth(2);
  this->TractographyFiducialSeedingNodeSelector->SetLabelText( "Parameters");
  this->TractographyFiducialSeedingNodeSelector->SetBalloonHelpString("select a parameter node from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TractographyFiducialSeedingNodeSelector->GetWidgetName());


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
  this->FiducialSelector->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->FiducialSelector->SetNewNodeEnabled(0);
  this->FiducialSelector->NoneEnabledOn();
  this->FiducialSelector->SetShowHidden(1);
  this->FiducialSelector->SetParent( moduleFrame->GetFrame() );
  this->FiducialSelector->Create();
  this->FiducialSelector->SetMRMLScene(this->GetApplicationLogic()->GetMRMLScene());
  this->FiducialSelector->UpdateMenu();
  this->FiducialSelector->SetBorderWidth(2);
  this->FiducialSelector->SetLabelText( "Select FiducialList or Model");
  this->FiducialSelector->SetBalloonHelpString("select a fiducial list or model for seeding.");
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

  this->StoppingModeMenu->SetParent( moduleFrame->GetFrame());
  this->StoppingModeMenu->Create();
  this->StoppingModeMenu->SetWidth(20);
  this->StoppingModeMenu->SetLabelWidth(12);
  this->StoppingModeMenu->SetLabelText("Stopping Mode:");
  this->StoppingModeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Linear Measure");
  this->StoppingModeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Fractional Anisotropy");
  this->StoppingModeMenu->GetWidget()->SetValue ( "Linear Measure" );
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->StoppingModeMenu->GetWidgetName());

  this->StoppingValueScale->SetParent ( moduleFrame->GetFrame() );
  this->StoppingValueScale->Create ( );
  this->StoppingValueScale->SetLabelText("Stopping Value");
  this->StoppingValueScale->GetWidget()->SetRange(0.01,1);
  this->StoppingValueScale->GetWidget()->SetResolution(0.01);
  this->StoppingValueScale->GetWidget()->SetValue(0.25);
  this->StoppingValueScale->SetBalloonHelpString("Tractography will stop when the stopping measurement drops below this value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->StoppingValueScale->GetWidgetName() );

  this->StoppingCurvatureScale->SetParent ( moduleFrame->GetFrame() );
  this->StoppingCurvatureScale->Create ( );
  this->StoppingCurvatureScale->SetLabelText("Stopping Track Curvature");
  this->StoppingCurvatureScale->GetWidget()->SetRange(0.01,10);
  this->StoppingCurvatureScale->GetWidget()->SetResolution(0.1);
  this->StoppingCurvatureScale->GetWidget()->SetValue(0.7);
  this->StoppingCurvatureScale->SetBalloonHelpString("Tractography will stop if radius of curvature becomes smaller than this number units are degrees per mm.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->StoppingCurvatureScale->GetWidgetName() );

  this->IntegrationStepLengthScale->SetParent ( moduleFrame->GetFrame() );
  this->IntegrationStepLengthScale->Create ( );
  this->IntegrationStepLengthScale->SetLabelText("Integration Step Length (mm)");
  this->IntegrationStepLengthScale->GetWidget()->SetRange(0.1,10);
  this->IntegrationStepLengthScale->GetWidget()->SetResolution(0.1);
  this->IntegrationStepLengthScale->GetWidget()->SetValue(0.5);
  this->IntegrationStepLengthScale->SetBalloonHelpString("Integration step size.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->IntegrationStepLengthScale->GetWidgetName() );

  this->MinimumPathLengthScale->SetParent ( moduleFrame->GetFrame() );
  this->MinimumPathLengthScale->Create ( );
  this->MinimumPathLengthScale->SetLabelText("Minimum Path Length (mm)");
  this->MinimumPathLengthScale->GetWidget()->SetRange(0,200);
  this->MinimumPathLengthScale->GetWidget()->SetResolution(1);
  this->MinimumPathLengthScale->GetWidget()->SetValue(20);
  this->MinimumPathLengthScale->SetBalloonHelpString("Path Length Threshold.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->MinimumPathLengthScale->GetWidgetName() );

  this->RegionSizeScale->SetParent ( moduleFrame->GetFrame() );
  this->RegionSizeScale->Create ( );
  this->RegionSizeScale->SetLabelText("Fiducial Seeding Region Size (mm)");
  this->RegionSizeScale->GetWidget()->SetRange(0,10);
  this->RegionSizeScale->GetWidget()->SetResolution(0.5);
  this->RegionSizeScale->GetWidget()->SetValue(2.5);
  this->RegionSizeScale->SetBalloonHelpString("The size of the seeding region for each fiducial.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->RegionSizeScale->GetWidgetName() );

  this->RegionSampleSizeScale->SetParent ( moduleFrame->GetFrame() );
  this->RegionSampleSizeScale->Create ( );
  this->RegionSampleSizeScale->SetLabelText("Fiducial Seeding Step Size (mm)");
  this->RegionSampleSizeScale->GetWidget()->SetRange(1,10);
  this->RegionSampleSizeScale->GetWidget()->SetResolution(0.5);
  this->RegionSampleSizeScale->GetWidget()->SetValue(1.0);
  this->RegionSampleSizeScale->SetBalloonHelpString("Step between seedin samples in the fiducial region");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->RegionSampleSizeScale->GetWidgetName() );

  this->SeedSelectedFiducialsButton->SetParent(moduleFrame->GetFrame());
  this->SeedSelectedFiducialsButton->Create();
  this->SeedSelectedFiducialsButton->SelectedStateOn();
  this->SeedSelectedFiducialsButton->SetText("Seed Selected Fiducials");
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->SeedSelectedFiducialsButton->GetWidgetName());

  this->MaxNumberOfSeedsEntry->SetParent ( moduleFrame->GetFrame() );
  this->MaxNumberOfSeedsEntry->Create ( );
  this->MaxNumberOfSeedsEntry->SetLabelText("Maximum number of seeds");
  this->MaxNumberOfSeedsEntry->GetWidget()->SetValueAsInt(100);
  this->MaxNumberOfSeedsEntry->SetBalloonHelpString("Maximum number of seeds");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->MaxNumberOfSeedsEntry->GetWidgetName() );

  // trackts display button
  this->DisplayMenu->SetParent(moduleFrame->GetFrame());
  this->DisplayMenu->Create();
  //this->DisplayMenu->SetWidth(60);
  this->DisplayMenu->SetText("Go to Fiber Bundles Module to Change Tracts Display");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->DisplayMenu->GetWidgetName());

  this->SeedButton->SetParent(moduleFrame->GetFrame());
  this->SeedButton->Create();
  this->SeedButton->SelectedStateOn();
  this->SeedButton->SetText("Enable Seeding Tracts");
  //this->SeedButton->SetActiveBackgroundColor(0.9, 0.5, 0.5);
  this->SeedButton->SetBackgroundColor(0.9, 0.6, 0.6);
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->SeedButton->GetWidgetName());

  moduleFrame->Delete();

  this->Built = 1;
}

void vtkSlicerTractographyFiducialSeedingGUI::SetVolumeSelector(vtkMRMLNode *node)
  {
  this->VolumeSelector->SetSelected(node);
  }

void vtkSlicerTractographyFiducialSeedingGUI::SetFiducialSelector(vtkMRMLNode *node)
{
  if (node)
    {
    this->FiducialSelector->SetSelected(node);
    }
}

void vtkSlicerTractographyFiducialSeedingGUI::SetOutFiberSelector(vtkMRMLNode *node)
  {
  this->OutFiberSelector->SetSelected(node);
  }

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingGUI::Init ( ) 
{
}
