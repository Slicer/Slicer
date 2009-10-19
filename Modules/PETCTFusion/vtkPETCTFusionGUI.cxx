
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"

#include "vtkPETCTFusionGUI.h"

#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#include "vtkPointData.h"

#include "vtkKWCheckButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWRange.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWHistogram.h"
#include "vtkSlicerPopUpHelpWidget.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerWaitMessageWidget.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerColorGUI.h"

#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkPETCTFusionGUI, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkPETCTFusionGUI* vtkPETCTFusionGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkPETCTFusionGUI");

  if(ret)
    {
      return (vtkPETCTFusionGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPETCTFusionGUI;
}



//----------------------------------------------------------------------------
vtkPETCTFusionGUI::vtkPETCTFusionGUI()
{
  this->Logic = NULL;
  this->PETCTFusionNode = NULL;

  //---
  //--- widgets
  //---
  this->CTSelector = NULL;
  this->PETSelector = NULL;
  this->MaskSelector = NULL;
  this->LUTSelector = NULL;
  this->VolumeRenderCheckbox = NULL;
  this->BodyQuantifierSelector = NULL;
  this->TissueConcentrationEntry = NULL;
  this->DoseUnitsMenuButton = NULL;
  this->TissueUnitsMenuButton = NULL;
  this->WeightUnitsMenuButton = NULL;
  this->InjectedDoseEntry = NULL;
  this->LeanBodyMassEntry = NULL;
  this->BodySurfaceAreaEntry = NULL;
  this->PatientWeightEntry = NULL;
  this->ComputeButton = NULL;
  this->GetFromDICOMButton = NULL;
  this->SaveButton = NULL;
  this->SUVmaxLabel = NULL;
  this->SUVminLabel = NULL;
  this->SUVmaxmeanLabel = NULL;
  this->SUVmeanLabel = NULL;
  this->HelpButton = NULL;
  this->ColorRange = NULL;
  this->UpdatingLUT = 0;
  this->UpdatingGUI = 0;

  this->PatientWeightLabel = NULL;
  this->InjectedDoseLabel = NULL;
  this->NumberOfTemporalPositionsLabel = NULL;
  this->SeriesTimeLabel = NULL;
  this->RPStartTimeLabel = NULL;
  this->FrameReferenceTimeLabel = NULL;
  this->DecayCorrectionLabel = NULL;
  this->DecayFactorLabel = NULL;
  this->RTHalfLifeLabel = NULL;
  this->PhilipsSUVFactorLabel = NULL;  
  this->CalibrationFactorLabel = NULL;
}

//----------------------------------------------------------------------------
vtkPETCTFusionGUI::~vtkPETCTFusionGUI()
{
//    this->RemoveMRMLNodeObservers ( );
//    this->RemoveLogicObservers ( );
  if ( this->Logic )
    {
    this->SetLogic ( NULL );
    }
  if ( this->PETCTFusionNode )
    {
    this->RemoveMRMLNodeObservers();
    this->SetPETCTFusionNode ( NULL );
    }
  //---
  //--- widgets
  //---
  if ( this->CTSelector)
    {
    this->CTSelector->SetParent ( NULL );
    this->CTSelector->Delete();
    this->CTSelector = NULL;
    }
  if ( this->PETSelector)
    {
    this->PETSelector->SetParent ( NULL );
    this->PETSelector->Delete();
    this->PETSelector = NULL;
    }
  if ( this->MaskSelector)
    {
    this->MaskSelector->SetParent (NULL);
    this->MaskSelector->Delete();
    this->MaskSelector = NULL;
    }
  if ( this->LUTSelector)
    {
    this->LUTSelector->SetParent ( NULL );
    this->LUTSelector->Delete();
    this->LUTSelector = NULL;
    }
  if ( this->VolumeRenderCheckbox)
    {
    this->VolumeRenderCheckbox->SetParent ( NULL );
    this->VolumeRenderCheckbox->Delete();
    this->VolumeRenderCheckbox = NULL;
    }
  if ( this->BodyQuantifierSelector)
    {
    this->BodyQuantifierSelector->SetParent ( NULL );
    this->BodyQuantifierSelector->Delete();
    this->BodyQuantifierSelector = NULL;
    }
  if ( this->TissueConcentrationEntry )
    {
    this->TissueConcentrationEntry->SetParent ( NULL);
    this->TissueConcentrationEntry->Delete();
    this->TissueConcentrationEntry = NULL;
    }
  if ( this->DoseUnitsMenuButton )
    {
    this->DoseUnitsMenuButton->SetParent ( NULL );
    this->DoseUnitsMenuButton->Delete();
    this->DoseUnitsMenuButton = NULL;
    }
  if ( this->TissueUnitsMenuButton )
    {
    this->TissueUnitsMenuButton->SetParent ( NULL );
    this->TissueUnitsMenuButton->Delete();
    this->TissueUnitsMenuButton = NULL;
    }
  if ( this->WeightUnitsMenuButton )
    {
    this->WeightUnitsMenuButton->SetParent ( NULL );
    this->WeightUnitsMenuButton->Delete();
    this->WeightUnitsMenuButton = NULL;
    }
  if ( this->InjectedDoseEntry )
    {
    this->InjectedDoseEntry->SetParent ( NULL );
    this->InjectedDoseEntry->Delete();
    this->InjectedDoseEntry = NULL;
    }
  if ( this->LeanBodyMassEntry )
    {
    this->LeanBodyMassEntry->SetParent ( NULL );
    this->LeanBodyMassEntry->Delete();
    this->LeanBodyMassEntry = NULL;
    }
  if ( this->BodySurfaceAreaEntry )
    {
    this->BodySurfaceAreaEntry->SetParent ( NULL );
    this->BodySurfaceAreaEntry->Delete();
    this->BodySurfaceAreaEntry = NULL;
    }
  if ( this->PatientWeightEntry )
    {
    this->PatientWeightEntry->SetParent ( NULL );
    this->PatientWeightEntry->Delete();
    this->PatientWeightEntry = NULL;
    }
  if ( this->ComputeButton )
    {
    this->ComputeButton->SetParent ( NULL );
    this->ComputeButton->Delete();
    this->ComputeButton = NULL;
    }
  if ( this->GetFromDICOMButton )
    {
    this->GetFromDICOMButton->SetParent ( NULL );
    this->GetFromDICOMButton->Delete();
    this->GetFromDICOMButton = NULL;
    }
  if ( this->SaveButton )
    {
    this->SaveButton->SetParent ( NULL );
    this->SaveButton->Delete();
    this->SaveButton = NULL;
    }
  if ( this->SUVmaxLabel )
    {
    this->SUVmaxLabel->SetParent ( NULL );
    this->SUVmaxLabel->Delete();
    this->SUVmaxLabel = NULL;
    }
  if ( this->SUVminLabel )
    {
    this->SUVminLabel->SetParent ( NULL );
    this->SUVminLabel->Delete();
    this->SUVminLabel = NULL;
    }
  if ( this->SUVmeanLabel )
    {
    this->SUVmeanLabel->SetParent (NULL);
    this->SUVmeanLabel->Delete();
    this->SUVmeanLabel = NULL;
    }
  if ( this->SUVmaxmeanLabel )
    {
    this->SUVmaxmeanLabel->SetParent ( NULL );
    this->SUVmaxmeanLabel->Delete();
    this->SUVmaxmeanLabel = NULL;
    }
  if ( this->HelpButton )
    {
    this->HelpButton->SetParent ( NULL );
    this->HelpButton->Delete();
    this->HelpButton = NULL;
    }
  if ( this->ColorRange )
    {
    this->ColorRange->SetParent ( NULL );
    this->ColorRange->Delete();
    this->ColorRange = NULL;
    }

  if ( this->PatientWeightLabel)
    {
    this->PatientWeightLabel->SetParent ( NULL );
    this->PatientWeightLabel->Delete();
    this->PatientWeightLabel = NULL;    
    }
  if ( this->InjectedDoseLabel)
    {
    this->InjectedDoseLabel->SetParent ( NULL );
    this->InjectedDoseLabel->Delete();
    this->InjectedDoseLabel = NULL;    
    }
  if ( this->NumberOfTemporalPositionsLabel)
    {
    this->NumberOfTemporalPositionsLabel->SetParent ( NULL );
    this->NumberOfTemporalPositionsLabel->Delete();
    this->NumberOfTemporalPositionsLabel = NULL;    
    }
  if ( this->SeriesTimeLabel)
    {
    this->SeriesTimeLabel->SetParent ( NULL );
    this->SeriesTimeLabel->Delete ();
    this->SeriesTimeLabel = NULL;    
    }
  if ( this->RPStartTimeLabel)
    {
    this->RPStartTimeLabel->SetParent ( NULL);
    this->RPStartTimeLabel->Delete();
    this->RPStartTimeLabel = NULL;    
    }
  if ( this->FrameReferenceTimeLabel)
    {
    this->FrameReferenceTimeLabel->SetParent ( NULL );
    this->FrameReferenceTimeLabel->Delete();
    this->FrameReferenceTimeLabel = NULL;
    }
  if ( this->DecayCorrectionLabel)
    {
    this->DecayCorrectionLabel->SetParent ( NULL );
    this->DecayCorrectionLabel->Delete();
    this->DecayCorrectionLabel = NULL;    
    }
  if ( this->DecayFactorLabel)
    {
    this->DecayFactorLabel->SetParent ( NULL );
    this->DecayFactorLabel->Delete();
    this->DecayFactorLabel = NULL;    
    }
  if ( this->RTHalfLifeLabel)
    {
    this->RTHalfLifeLabel->SetParent ( NULL );
    this->RTHalfLifeLabel->Delete();
    this->RTHalfLifeLabel = NULL;    
    }
  if ( this->PhilipsSUVFactorLabel)  
    {
    this->PhilipsSUVFactorLabel->SetParent ( NULL ); 
    this->PhilipsSUVFactorLabel->Delete();
    this->PhilipsSUVFactorLabel = NULL;
    }
  if ( this->CalibrationFactorLabel )
    {
    this->CalibrationFactorLabel->SetParent ( NULL );
    this->CalibrationFactorLabel->Delete();
    this->CalibrationFactorLabel = NULL;
    }

  this->UpdatingLUT = 0;  
  this->UpdatingGUI = 0;
  this->SetAndObserveMRMLScene ( NULL );
}

//----------------------------------------------------------------------------
void vtkPETCTFusionGUI::Enter()
{
  this->Raised = true;
  this->Visited = true;
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddObserver (vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand());
    if ( this->Logic )
      {
      this->Logic->Enter();
      }
      vtkIntArray *guiEvents = this->NewObservableEvents ( );
    if ( guiEvents != NULL )
      {
      this->SetAndObserveMRMLSceneEvents ( this->MRMLScene, guiEvents );
      guiEvents->Delete();
      }
    }
  this->AddGUIObservers();    
  this->CreateModuleEventBindings();  
  this->UpdateGUIFromMRML(0);
}


//----------------------------------------------------------------------------
void vtkPETCTFusionGUI::Exit ( )
{
  this->Raised = false;
  this->RemoveGUIObservers();
  this->ReleaseModuleEventBindings();
  if ( this->Logic )
    {
    this->Logic->Exit();
    }
  this->TearDownGUI();
}


//----------------------------------------------------------------------------
vtkIntArray *vtkPETCTFusionGUI::NewObservableEvents()
{
  if ( !this->Visited)
    {
    return NULL;
    }

 vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::MetadataAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkPETCTFusionGUI::TearDownGUI ( )
{

  this->RemoveGUIObservers ( );

}


//----------------------------------------------------------------------------
void vtkPETCTFusionGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  //---
  //--- print widgets
  //---
  /*
  this->CTSelector
  this->PETSelector
  this->MaskSelector
  this->LUTSelector
  this->VolumeRenderCheckbox
  this->BodyQuantifierSelector
  this->TissueConcentrationEntry
  this->InjectedDoseEntry
  this->DoseUnitsMenuButton
  this->TissueUnitsMenuButton
  this->WeightUnitsMenuButton
  this->LeanBodyMassEntry
  this->BodySurfaceAreaEntry
  this->PatientWeightEntry
  this->ComputeButton
  this->SaveButton
  this->SUVmaxLabel
  this->SUVminLabel
  this->SUVmaxmeanLabel
  this->SUVmeanLabel
  this->HelpButton
  */

}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::AddGUIObservers ( ) 
{

  if ( !this->Built )
    {
    return;
    }
  if ( !this->Visited )
    {
    return;
    }
  
  if ( this->CTSelector )
    {
    this->CTSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PETSelector )
    {
    this->PETSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->MaskSelector )
    {
    this->MaskSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LUTSelector )
    {
    this->LUTSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->VolumeRenderCheckbox )
    {
    this->VolumeRenderCheckbox->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->BodyQuantifierSelector )
    {
    int rbID;
    vtkKWRadioButton *rb;
    for ( int i=0; i < this->BodyQuantifierSelector->GetNumberOfWidgets(); i++ )
      {
      rbID = this->BodyQuantifierSelector->GetIdOfNthWidget(i);
      rb = this->BodyQuantifierSelector->GetWidget(rbID);
      if ( rb )
        {
        rb->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
        }
      }
    }
  if (this->TissueConcentrationEntry )
    {
    this->TissueConcentrationEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->DoseUnitsMenuButton )
    {
    this->DoseUnitsMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->TissueUnitsMenuButton )
    {
    this->TissueUnitsMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->WeightUnitsMenuButton )
    {
    this->WeightUnitsMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->InjectedDoseEntry )
    {
    this->InjectedDoseEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LeanBodyMassEntry )
    {
    this->LeanBodyMassEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->BodySurfaceAreaEntry )
    {
    this->BodySurfaceAreaEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PatientWeightEntry )
    {
    this->PatientWeightEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ComputeButton )
    {
    this->ComputeButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->GetFromDICOMButton )
    {
    this->GetFromDICOMButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->SaveButton )
    {
    this->SaveButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::RemoveGUIObservers ( )
{

    if ( !this->Built )
    {
    return;
    }

  //--- include this to enable lazy building
  if ( !this->Visited )
    {
    return;
    }

  if ( this->CTSelector )
    {
    this->CTSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PETSelector )
    {
    this->PETSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->MaskSelector )
    {
    this->MaskSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LUTSelector )
    {
    this->LUTSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->VolumeRenderCheckbox )
    {
    this->VolumeRenderCheckbox->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->BodyQuantifierSelector )
    {
    int rbID;
    vtkKWRadioButton *rb;
    for ( int i=0; i < this->BodyQuantifierSelector->GetNumberOfWidgets(); i++ )
      {
      rbID = this->BodyQuantifierSelector->GetIdOfNthWidget(i);
      rb = this->BodyQuantifierSelector->GetWidget(rbID);
      if ( rb )
        {
        rb->RemoveObservers ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
        }
      }
    }
  if (this->TissueConcentrationEntry )
    {
    this->TissueConcentrationEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->DoseUnitsMenuButton )
    {
    this->DoseUnitsMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->TissueUnitsMenuButton )
    {
    this->TissueUnitsMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->WeightUnitsMenuButton )
    {
    this->WeightUnitsMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->InjectedDoseEntry )
    {
    this->InjectedDoseEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LeanBodyMassEntry )
    {
    this->LeanBodyMassEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->BodySurfaceAreaEntry )
    {
    this->BodySurfaceAreaEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->PatientWeightEntry )
    {
    this->PatientWeightEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ComputeButton )
    {
    this->ComputeButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->GetFromDICOMButton )
    {
    this->GetFromDICOMButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->SaveButton )
    {
    this->SaveButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::AddMRMLNodeObservers ()
{
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::RemoveMRMLNodeObservers ()
{
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  // lots of null pointer checking.
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast (this->GetApplicationGUI() );
  if (!appGUI)
    {
    vtkErrorMacro("ProcessGUIEvents: got Null SlicerApplicationGUI" );
    return;
    }
  if ( !this->PETCTFusionNode )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got NULL PETCTFusionNode. Can't record state and not processing GUI events." );
    return;
    }
  if ( !this->ApplicationLogic )
    {
    vtkErrorMacro ("ProcessGUIEvents: got NULL ApplicationLogic. Not processing GUI events.");
    return;
    }
  if ( ! (this->ApplicationLogic->GetSelectionNode()) )
    {
    vtkErrorMacro ("ProcessGUIEvents: got NULL SelectionNode in ApplicationLogic. Not processing GUI events.");
    return;    
    }
  
  
  vtkKWCheckButton *cb = vtkKWCheckButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  vtkKWRadioButton *rb = vtkKWRadioButton::SafeDownCast ( caller );
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkSlicerNodeSelectorWidget *s = vtkSlicerNodeSelectorWidget::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
  
  //---
  //--- node selectors  
  //---
  if ( (s != NULL) && (event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ))
    {
    vtkMRMLVolumeNode *v;
    if  ( s == this->CTSelector )
      {
      v = vtkMRMLVolumeNode::SafeDownCast (this->CTSelector->GetSelected() );
      if ( (v == NULL) || (v->GetID() == NULL) || (!strcmp(v->GetID(), "")) )
        {
        return;
        }

      //--- put CT volume in BG layer.
      this->ApplicationLogic->GetSelectionNode()->SetActiveVolumeID( v->GetID() );
      this->ApplicationLogic->PropagateVolumeSelection();
      this->PETCTFusionNode->SetInputCTReference ( v->GetID() );
      }
    else if  ( s == this->PETSelector )
      {
      v = vtkMRMLVolumeNode::SafeDownCast (this->PETSelector->GetSelected() );
      if ( (v == NULL) || (v->GetID() == NULL) || (!strcmp(v->GetID(), "")) )
        {
        this->ResetManualEntryGUI();
        this->ClearDICOMInformation();
        return;
        }
      //--- put PET volume in FG layer
      this->ApplicationLogic->GetSelectionNode()->SetSecondaryVolumeID( v->GetID() );
      this->ApplicationLogic->PropagateVolumeSelection();
      this->PETCTFusionNode->SetInputPETReference ( v->GetID() );
      this->InitializeGUI();

      if ( !strcmp(v->GetID(), "None" ) )
        {
        this->InitializePETMinAndMax();
        }
      else
        {
        this->UpdatePETRange();
        }
      this->UpdateDICOMPanel();
      this->UpdateLUTFromMRML();
      this->UpdatePETDisplayFromMRML();
      }
    else if ( s == this->MaskSelector )
      {
      this->SUVmaxLabel->SetText ( "" );
      this->SUVminLabel->SetText ( "");
      this->SUVmeanLabel->SetText ( "" );
      this->SUVmaxmeanLabel->SetText ( "" );

      v = vtkMRMLVolumeNode::SafeDownCast (this->MaskSelector->GetSelected() );
      if ( (v == NULL) || (v->GetID() == NULL) || (!strcmp(v->GetID(), "")) )
        {
        return;
        }

      //--- put label volume in LB layer.
      this->ApplicationLogic->GetSelectionNode()->SetActiveLabelVolumeID( v->GetID() );
      this->ApplicationLogic->PropagateVolumeSelection();
      this->PETCTFusionNode->SetInputMask(v->GetID());
      }
    else if ( s == this->LUTSelector )
      {
      this->ColorizePETVolume();
      }
    }
  
  //---
  //--- checkboxes
  //---  
  if ( (cb != NULL ) && (cb == this->VolumeRenderCheckbox ) &&
       (event == vtkKWCheckButton::SelectedStateChangedEvent ))
    {
    int vr = cb->GetSelectedState();

    if ( vr && !this->PETCTFusionNode->GetVolumeRendering() )
      {
      //--- if volume rendering is turned ON
      this->EnablePETCTVolumeRendering ( );
      }
    else if ( !vr && this->PETCTFusionNode->GetVolumeRendering() )
      {
      //--- if volume rendering is turned OFF
      this->DisablePETCTVolumeRendering ( );
      }

    //--- update node
    if ( this->PETCTFusionNode )
      {
      this->PETCTFusionNode->SetVolumeRendering ( vr );
      }
    }

  //---
  //--- radiobuttons
  //---
  if ( (rb != NULL ) && (this->BodyQuantifierSelector != NULL ) &&
       (event == vtkKWCheckButton::SelectedStateChangedEvent ))
    {
    int rbID;
    vtkKWRadioButton *_rb;
    for ( int i=0; i < this->BodyQuantifierSelector->GetNumberOfWidgets(); i++ )
      {
      rbID = this->BodyQuantifierSelector->GetIdOfNthWidget(i);
      _rb = this->BodyQuantifierSelector->GetWidget(rbID);
      if ( _rb == rb )
        {
        //--- which is it?
        }
      }
    }

  //---
  //--- menus
  //---
  if ( (m != NULL ) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
    {
    std::string ss;
    if ( this->DoseUnitsMenuButton != NULL && m == this->DoseUnitsMenuButton->GetMenu() )
      {
      ss.clear();
      ss = this->DoseUnitsMenuButton->GetValue();

      this->ClearDICOMInformation();
      if ( !strcmp ( ss.c_str(), "megabecquerels [MBq]" ))
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "MBq" );
        }
      else if ( !strcmp ( ss.c_str(), "kilobecquerels [kBq]" ))
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "kBq" );
        }
      else if ( !strcmp ( ss.c_str(), "millibecquerels [mBq]" ))
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "mBq" );
        }
      else if ( !strcmp ( ss.c_str(), "microbecquerels [uBq]" ))
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "uBq" );
        }
      else if ( !strcmp ( ss.c_str(), "becquerels [Bq]" ))
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "Bq" );
        }
      else if ( !strcmp ( ss.c_str(), "megacuries [MCi]" ) )
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "MCi" );
        }
      else if ( !strcmp ( ss.c_str(), "kilocuries [kCi]" ) )
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "kCi" );
        }
      else if ( !strcmp ( ss.c_str(), "millicuries [mCi]" ) )
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "mCi" );
        }
      else if ( !strcmp ( ss.c_str(), "microcuries [uCi]" ) )
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "uCi" );
        }
      else if ( !strcmp ( ss.c_str(), "curies [Ci]" ) )
        {
        this->PETCTFusionNode->SetDoseRadioactivityUnits ( "Ci" );
        }
      //--- make sure the weight gets set in the node too.
      if ( this->InjectedDoseEntry != NULL )
        {
        this->InjectedDoseEntry->InvokeEvent ( vtkKWEntry::EntryValueChangedEvent );
        }
      }
    if ( this->TissueUnitsMenuButton != NULL && m == this->TissueUnitsMenuButton->GetMenu() )
      {
      ss.clear();
      ss = this->TissueUnitsMenuButton->GetValue();

      this->ClearDICOMInformation();
      if ( !strcmp ( ss.c_str(), "megabecquerels [MBq]" ))
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "MBq" );
        }
      else if ( !strcmp ( ss.c_str(), "kilobecquerels [kBq]" ))
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "kBq" );
        }
      else if ( !strcmp ( ss.c_str(), "millibecquerels [mBq]" ))
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "mBq" );
        }
      else if ( !strcmp ( ss.c_str(), "microbecquerels [uBq]" ))
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "uBq" );
        }
      else if ( !strcmp ( ss.c_str(), "becquerels [Bq]" ))
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "Bq" );
        }
      else if ( !strcmp ( ss.c_str(), "megacuries [MCi]" ) )
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "MCi" );
        }
      else if ( !strcmp ( ss.c_str(), "kilocuries [kCi]" ) )
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "kCi" );
        }
      else if ( !strcmp ( ss.c_str(), "millicuries [mCi]" ) )
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "mCi" );
        }
      else if ( !strcmp ( ss.c_str(), "microcuries [uCi]" ) )
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "uCi" );
        }
      else if ( !strcmp ( ss.c_str(), "curies [Ci]" ) )
        {
        this->PETCTFusionNode->SetTissueRadioactivityUnits ( "Ci" );
        }
      }
    if ( this->WeightUnitsMenuButton != NULL && m == this->WeightUnitsMenuButton->GetMenu())
      {
      ss.clear(); 
      ss = this->WeightUnitsMenuButton->GetValue();

      this->ClearDICOMInformation();

      if (!strcmp (ss.c_str(), "kilograms [kg]"))
        {
        this->PETCTFusionNode->SetWeightUnits ( "kg" );
        }
      else if ( !strcmp (ss.c_str(), "grams [g]"))
        {
        this->PETCTFusionNode->SetWeightUnits ( "g" );
        }
      else if ( !strcmp (ss.c_str(), "pounds [lb]"))
        {
        this->PETCTFusionNode->SetWeightUnits ( "lb" );
        }        
      //--- make sure the weight gets set in the node too.
      if ( this->PatientWeightEntry != NULL )
        {
        this->PatientWeightEntry->InvokeEvent ( vtkKWEntry::EntryValueChangedEvent );
        }
      }
    }

  //---
  //--- entries
  //---
  if ( (e != NULL) && (event == vtkKWEntry::EntryValueChangedEvent ) )
    {
    if ( e == this->TissueConcentrationEntry )
      {
      }
    else if ( e == this->InjectedDoseEntry )
      {
      this->ClearDICOMInformation();

      if ( this->InjectedDoseEntry->GetValueAsDouble() )
        {
        if ( this->InjectedDoseEntry->GetValueAsDouble() != this->PETCTFusionNode->GetInjectedDose() )
          {
          this->PETCTFusionNode->SetInjectedDose(this->InjectedDoseEntry->GetValueAsDouble() );
          //--- make sure the units get set in the node too.
          if ( this->DoseUnitsMenuButton != NULL )
            {
            this->DoseUnitsMenuButton->GetMenu()->InvokeEvent ( vtkKWMenu::MenuItemInvokedEvent );
            }
          }
        }
      else
        {
        this->PETCTFusionNode->SetInjectedDose(0.0);
        }
      }
    else if ( e == this->LeanBodyMassEntry )
      {
      }
    else if ( e == this->BodySurfaceAreaEntry )
      {
      }
    else if ( e == this->PatientWeightEntry )
      {
      this->ClearDICOMInformation();
      if ( this->PatientWeightEntry->GetValueAsDouble() )
        {
        if ( this->PatientWeightEntry->GetValueAsDouble() != this->PETCTFusionNode->GetPatientWeight() )
          {
          this->PETCTFusionNode->SetPatientWeight(this->PatientWeightEntry->GetValueAsDouble() );
          //--- make sure the units get set in the node too.
          if ( this->WeightUnitsMenuButton != NULL )
            {
            this->WeightUnitsMenuButton->GetMenu()->InvokeEvent ( vtkKWMenu::MenuItemInvokedEvent );
            }
          }
        }
      else
        {
        this->PETCTFusionNode->SetPatientWeight ( 0.0);
        }
      }
    }

  //---
  //--- pushbuttons
  //---
  if ( (b != NULL ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    if ( b == this->ComputeButton )
      {
      //--- make sure we have the right stuff for computation
      //--- Need a pet volume and a mask volume.
      //--- CT volume is only for display.
      if ( this->PETCTFusionNode->GetInputPETReference() != NULL &&
           this->PETCTFusionNode->GetInputMask() != NULL  &&
           this->PETCTFusionNode->GetPatientWeight() != 0.0 &&
           this->PETCTFusionNode->GetInjectedDose() != 0.0 )
        {           
        //--- compute stuff.
        this->Logic->ComputeSUV();
        }
      else
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
        dialog->SetStyleToMessage();
        dialog->SetText ( "To compute the SUV, a valid PET volume\n and a Mask volume (label map) must be selected, \nand two parameters: Patient Weight and \nInjected Dose should be specified.\n \n At least one of these is missing." );
        dialog->Create();
        dialog->Invoke();
        dialog->Delete();
        return;
        }
      }
    if ( b == this->GetFromDICOMButton)
      {
      this->ResetManualEntryGUI();
      this->ClearDICOMInformation();
      this->UpdateDICOMPanel();
      }
    else if ( b == this->SaveButton )
      {
      }
    }
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ColorizePETVolume()
{
  vtkMRMLColorNode *c;
  c = vtkMRMLColorNode::SafeDownCast (this->LUTSelector->GetSelected() );
  if ( c && this->PETCTFusionNode->GetInputPETReference() )
    {
    //--- get the PET volume display node
    vtkMRMLScalarVolumeNode *svn;
    svn = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputPETReference() ));
    if ( svn )
      {
      vtkMRMLVolumeDisplayNode *dnode;
      dnode = vtkMRMLVolumeDisplayNode::SafeDownCast (svn->GetDisplayNode());
      if ( dnode && dnode->GetColorNodeID() )
        {
        //--- check to see if the change is new
        if ( strcmp (dnode->GetColorNodeID(), c->GetID() ) != 0 )
          {
          //--- apply LUT to PET volume
          dnode->SetAndObserveColorNodeID (c->GetID() );
          }
        }
      }
    //--- and update the node.
    this->PETCTFusionNode->SetPETLUT(c->GetID());
    this->UpdatePETDisplayFromMRML();
    }
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateLUTFromMRML ()
{
  if ( this->PETCTFusionNode->GetInputPETReference() )
    {
    //--- get the PET volume display node
    vtkMRMLScalarVolumeNode *svn;
    svn = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputPETReference() ));
    if ( svn )
      {
      vtkMRMLVolumeDisplayNode *dnode;
      dnode = vtkMRMLVolumeDisplayNode::SafeDownCast (svn->GetDisplayNode());
      if ( dnode )
        {
        vtkMRMLColorNode *cnode = dnode->GetColorNode();
        if ( cnode )
          {
          if ( this->LUTSelector->GetSelected() != cnode )
            {
            this->LUTSelector->SetSelected ( cnode );
            }
          if ( this->PETCTFusionNode->GetPETLUT() != cnode->GetID() )
            {
            this->PETCTFusionNode->SetPETLUT(cnode->GetID());
            }
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateGUIFromMRML ( int updateDICOMevent )
{

  if ( this->Logic == NULL )
    {
    vtkErrorMacro ( "UpdateGUIfromMRML: Got NULL Module Logic. ");
    return;    
    }
  if ( this->ApplicationLogic == NULL )
    {
    vtkErrorMacro ( "UpdateGUIFromMRML: Got NULL Application Logic. ");
    return;
    }
  if ( this->ApplicationLogic->GetSelectionNode() == NULL )
    {
    vtkErrorMacro ( "UpdateGUIFromMRML: Got NULL SelectionNode in Logic.");
    return;
    }
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "UpdateGUIFromMRML: Got NULL MRMLScene. ");
    return;
    }  

  this->UpdatingGUI = 1;
  //--- create a new node if we don't have one...
  vtkMRMLPETCTFusionNode* node = this->GetPETCTFusionNode();
  if (node == NULL )
    {
    node = vtkMRMLPETCTFusionNode::New();
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::ErrorEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::UpdateDisplayEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::StartUpdatingDisplayEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::ComputeDoneEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::DICOMUpdateEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::WaitEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::NonDICOMEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::StatusEvent );    
    vtkSetAndObserveMRMLNodeEventsMacro ( this->PETCTFusionNode, node, events );
    node->Delete(); 

    if ( this->Logic->GetPETCTFusionNode() != NULL )
      {
      this->Logic->SetPETCTFusionNode ( NULL );
      }
    this->Logic->SetPETCTFusionNode( this->GetPETCTFusionNode() );
    events->Delete();
    }
  node = this->GetPETCTFusionNode();
  if ( node == NULL )
    {
    vtkErrorMacro ( "UpdateGUIFromMRML: Got NULL PETCTFusion Node." );
    return;
    }


  //---  
  //--- Update DoseUnitsMenubutton if changed
  //---
  if ( this->DoseUnitsMenuButton && this->PETCTFusionNode->GetDoseRadioactivityUnits() )
    {
    std::string s = this->DoseUnitsMenuButton->GetValue();
    std::string ns = this->PETCTFusionNode->GetDoseRadioactivityUnits();

    if ( ns.find("MBq") != std::string::npos )
      {
      if ( strcmp (s.c_str(), "megabecquerels [MBq]"))
        {
        this->DoseUnitsMenuButton->SetValue ( "megabecquerels [MBq]");
        }
      }
    else if ( ns.find("kBq") != std::string::npos )
      {
      if ( strcmp (s.c_str(), "kilobecquerels [kBq]"))
        {
        this->DoseUnitsMenuButton->SetValue ( "kilobecquerels [kBq]");
        }
      }
    else if ( ns.find("mBq") != std::string::npos )
      {
      if ( strcmp (s.c_str(), "millibecquerels [mBq]"))
        {
        this->DoseUnitsMenuButton->SetValue ( "millibecquerels [mBq]");
        }
      }
    else if ( ns.find("uBq") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "microbecquerels [uBq]") )
        {
        this->DoseUnitsMenuButton->SetValue ( "microbecquerels [uBq]");
        }
      }
    else if ( ns.find("Bq") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "becquerels [Bq]"))
        {
        this->DoseUnitsMenuButton->SetValue ( "becquerels [Bq]");
        }
      }    
    else if ( ns.find("MCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "megacuries [MCi]"))
        {
        this->DoseUnitsMenuButton->SetValue ( "megacuries [MCi]");
        }
      }
    else if ( ns.find("kCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "kilocuries [kCi]") )
        {
        this->DoseUnitsMenuButton->SetValue ( "kilocuries [kCi]");
        }
      }
    else if ( ns.find("mCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "millicuries [mCi]") )
        {
        this->DoseUnitsMenuButton->SetValue ( "millicuries [mCi]");
        }
      }
    else if ( ns.find("uCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "microcuries [uCi]") )
        {
        this->DoseUnitsMenuButton->SetValue ( "microcuries [uCi]");
        }
      }
    else if ( ns.find("Ci") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "curies [Ci]"))
        {
        this->DoseUnitsMenuButton->SetValue ( "curies [Ci]");
        }
      }
    }

  //---  
  //--- Update TissueUnitsMenubutton if changed
  //---
  if ( this->TissueUnitsMenuButton && this->PETCTFusionNode->GetTissueRadioactivityUnits() )
    {
    std::string s = this->TissueUnitsMenuButton->GetValue();
    std::string ns = this->PETCTFusionNode->GetTissueRadioactivityUnits();

    if ( ns.find("MBq") != std::string::npos )
      {
      if ( strcmp (s.c_str(), "megabecquerels [MBq]"))
        {
        this->TissueUnitsMenuButton->SetValue ( "megabecquerels [MBq]");
        }
      }
    else if ( ns.find("kBq") != std::string::npos )
      {
      if ( strcmp (s.c_str(), "kilobecquerels [kBq]"))
        {
        this->TissueUnitsMenuButton->SetValue ( "kilobecquerels [kBq]");
        }
      }
    else if ( ns.find("mBq") != std::string::npos )
      {
      if ( strcmp (s.c_str(), "millibecquerels [mBq]"))
        {
        this->TissueUnitsMenuButton->SetValue ( "millibecquerels [mBq]");
        }
      }
    else if ( ns.find("uBq") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "microbecquerels [uBq]") )
        {
        this->TissueUnitsMenuButton->SetValue ( "microbecquerels [uBq]");
        }
      }
    else if ( ns.find("Bq") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "becquerels [Bq]"))
        {
        this->TissueUnitsMenuButton->SetValue ( "becquerels [Bq]");
        }
      }    
    else if ( ns.find("MCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "megacuries [MCi]"))
        {
        this->TissueUnitsMenuButton->SetValue ( "megacuries [MCi]");
        }
      }
    else if ( ns.find("kCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "kilocuries [kCi]") )
        {
        this->TissueUnitsMenuButton->SetValue ( "kilocuries [kCi]");
        }
      }
    else if ( ns.find("mCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "millicuries [mCi]") )
        {
        this->TissueUnitsMenuButton->SetValue ( "millicuries [mCi]");
        }
      }
    else if ( ns.find("uCi") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "microcuries [uCi]") )
        {
        this->TissueUnitsMenuButton->SetValue ( "microcuries [uCi]");
        }
      }
    else if ( ns.find("Ci") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "curies [Ci]"))
        {
        this->TissueUnitsMenuButton->SetValue ( "curies [Ci]");
        }
      }
    }

  //---  
  //--- Update WeightUnitsMenubutton if changed
  //---
  if ( this->WeightUnitsMenuButton && this->PETCTFusionNode->GetWeightUnits())
    {
    std::string s = this->WeightUnitsMenuButton->GetValue();  
    std::string ns = this->PETCTFusionNode->GetWeightUnits();
    if ( ns.find("kg") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "kilograms [kg]"))
        {
        this->WeightUnitsMenuButton->SetValue ( "kilograms [kg]");
        }
      }
    else if ( ns.find("g") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "grams [g]"))
        {
        this->WeightUnitsMenuButton->SetValue ( "grams [g]");
        }
      }
    else if ( ns.find("lb") != std::string::npos )
      {
      if ( strcmp ( s.c_str(), "pounds [lb]"))
        {
        this->WeightUnitsMenuButton->SetValue ( "pounds [lb]");
        }
      }
    }

  //vtkMRMLScalarVolumeNode *n = NULL; FIXME: unused variable
  //vtkMRMLVolumeDisplayNode *dnode = NULL; FIXME: unused variable
  if ( this->CTSelector )
    {
    //--- update the menu & selection
    this->CTSelector->UpdateMenu();
    const char *nid = this->PETCTFusionNode->GetInputCTReference();
    //--- check to see if the change is new
    if ( nid && strcmp(nid, this->CTSelector->GetSelected()->GetID()) )
      {
      //--- make GUI match MRML
      this->CTSelector->SetSelected ( this->MRMLScene->GetNodeByID(nid) );
      }
    }
    
  if ( this->PETSelector)
    {
    //--- update the menu & selection
    this->PETSelector->UpdateMenu();
    const char *nid = this->PETCTFusionNode->GetInputPETReference();
    //--- check to see if the change is new
    if (nid && strcmp(nid, this->PETSelector->GetSelected()->GetID()) )
      {
      //--- make GUI match MRML
      this->PETSelector->SetSelected ( this->MRMLScene->GetNodeByID(nid) );
      this->UpdatePETDisplayFromMRML();
      }
    }

  if ( this->MaskSelector)
    {
    //--- update the menu & selection
    this->MaskSelector->UpdateMenu();
    const char *nid = this->PETCTFusionNode->GetInputMask();
    //--- check to see if the change is new
    if ( nid && strcmp(nid, this->MaskSelector->GetSelected()->GetID()) )
      {
      //--- make GUI match MRML
      this->MaskSelector->SetSelected ( this->MRMLScene->GetNodeByID(nid) );
      }
    }

  if ( this->LUTSelector)
    {
    //--- makd sure PETLUT matches the display node's color node.
    if ( this->PETCTFusionNode->GetInputPETReference() )
      {
      vtkMRMLScalarVolumeNode *n;
      n = vtkMRMLScalarVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputPETReference()));
      vtkMRMLVolumeDisplayNode *d = vtkMRMLVolumeDisplayNode::SafeDownCast(n->GetDisplayNode());
      if ( d )
        {
        vtkMRMLColorNode *c = d->GetColorNode();
        if ( c )
          {
          if ( c->GetID() != this->PETCTFusionNode->GetPETLUT() )
            {
            this->PETCTFusionNode->SetPETLUT(c->GetID() );
            }
          }
        }
      }
    
    //--- update the menu & make sure selection matches mrml
    this->LUTSelector->UpdateMenu();
    const char *nid = this->PETCTFusionNode->GetPETLUT();
    //--- select inverted grey by default.
    if ( this->LUTSelector->GetSelected() == NULL )
      {
      this->LUTSelector->SetSelected ( this->MRMLScene->GetNodeByID("vtkMRMLColorTableNodeInvertedGrey" ));      
      }
    if ( nid && this->LUTSelector->GetSelected() )
      {
      if (strcmp (nid, this->LUTSelector->GetSelected()->GetID()) )
        {
        //--- make GUI match MRML
        this->LUTSelector->SetSelected ( this->MRMLScene->GetNodeByID(nid) );
        }
      }
    }

  std::stringstream sstr;
  if ( this->PETCTFusionNode->GetPatientWeight() != 0.0)
    {
    if ( this->PatientWeightEntry )
      {
      double d = this->PatientWeightEntry->GetValueAsDouble();
      if ( d != this->PETCTFusionNode->GetPatientWeight() )
        {
        this->PatientWeightEntry->SetValueAsDouble ( this->PETCTFusionNode->GetPatientWeight() );
        }
      }
    if ( this->PatientWeightLabel )
      {
      sstr.clear();
      sstr.str("");
      sstr <<this->PETCTFusionNode->GetPatientWeight();
      if ( this->PETCTFusionNode->GetWeightUnits() && strcmp (this->PETCTFusionNode->GetWeightUnits(), "" ))
        {
        sstr << " (" << this->PETCTFusionNode->GetWeightUnits() << ")";
        }
      std::string tmpstr;
      const char *tmpc;
      tmpstr = sstr.str();
      tmpc = tmpstr.c_str();
      this->PatientWeightLabel->SetText ( tmpc );
      }
    }
  else
    {
    if ( this->PatientWeightLabel )
      {
      if ( updateDICOMevent )
        {
        double d = this->PatientWeightEntry->GetValueAsDouble();
        if ( d != this->PETCTFusionNode->GetPatientWeight() )
          {
          this->PatientWeightEntry->SetValueAsDouble ( this->PETCTFusionNode->GetPatientWeight() );
          }
        this->PatientWeightLabel->SetText ("no value found");
        }
      else
        {
        this->PatientWeightLabel->SetText ("");
        }
      }
    }

  if ( this->PETCTFusionNode->GetInjectedDose() != 0.0 )
    {
    if ( this->InjectedDoseEntry )
      {
      double d = this->InjectedDoseEntry->GetValueAsDouble();
      if ( d != this->PETCTFusionNode->GetInjectedDose() )
        {
        this->InjectedDoseEntry->SetValueAsDouble ( this->PETCTFusionNode->GetInjectedDose() );
        }
      }
    if ( this->InjectedDoseLabel )
      {
      sstr.clear();
      sstr.str("");
      sstr <<this->PETCTFusionNode->GetInjectedDose();
      if ( this->PETCTFusionNode->GetDoseRadioactivityUnits() )
        {
        sstr << " (" << this->PETCTFusionNode->GetDoseRadioactivityUnits() << ")";
        }
      std::string tmpstr;
      const char *tmpc;
      tmpstr = sstr.str();
      tmpc = tmpstr.c_str();
      this->InjectedDoseLabel->SetText (tmpc );
      }
    }
  else
    {
    if ( this->InjectedDoseLabel )
      {    
      if ( updateDICOMevent )
        {
        double d = this->InjectedDoseEntry->GetValueAsDouble();
        if ( d != this->PETCTFusionNode->GetInjectedDose() )
          {
          this->InjectedDoseEntry->SetValueAsDouble (0.0);
          }
        this->InjectedDoseLabel->SetText ("no value found");
        }
      else
        {
        this->InjectedDoseLabel->SetText ("");
        }
      }
    }

  if (( this->PETCTFusionNode->GetNumberOfTemporalPositions() != 0)  &&
      ( this->NumberOfTemporalPositionsLabel ) )
    {
    sstr.clear();
    sstr.str("");
    sstr << this->PETCTFusionNode->GetNumberOfTemporalPositions();
    std::string tmpstr;
    const char *tmpc;
    tmpstr = sstr.str();
    tmpc = tmpstr.c_str();
    this->NumberOfTemporalPositionsLabel->SetText ( tmpc );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->NumberOfTemporalPositionsLabel->SetText ( "no value found" );
      }
    else
      {
      this->NumberOfTemporalPositionsLabel->SetText ( "" );
      }
    }
  
  if ( (this->PETCTFusionNode->GetSeriesTime() != NULL ) &&
       (this->SeriesTimeLabel ))
    {
    sstr.clear();
    sstr.str("");
    sstr << this->PETCTFusionNode->GetSeriesTime() << " (hh:mm:ss...)";
    std::string tmpstr;
    const char *tmpc;
    tmpstr = sstr.str();
    tmpc = tmpstr.c_str();
    this->SeriesTimeLabel->SetText ( tmpc );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->SeriesTimeLabel->SetText ( "no value found" );
      }
    else
      {
      this->SeriesTimeLabel->SetText ( "" );
      }
    }
  
  if ( (this->PETCTFusionNode->GetRadiopharmaceuticalStartTime() != NULL ) &&
       (this->RPStartTimeLabel ) )
    {
    sstr.clear();
    sstr.str("");
    sstr << this->PETCTFusionNode->GetRadiopharmaceuticalStartTime() << " (hh:mm:ss...)";
    std::string tmpstr;
    const char *tmpc;
    tmpstr = sstr.str();
    tmpc = tmpstr.c_str();
    this->RPStartTimeLabel->SetText ( tmpc );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->RPStartTimeLabel->SetText ( "no value found" );
      }
    else
      {
      this->RPStartTimeLabel->SetText ( "" );
      }
    }
  
  if ( (this->PETCTFusionNode->GetFrameReferenceTime() != NULL ) &&
       ( this->FrameReferenceTimeLabel ))
    {
    sstr.clear();
    sstr.str("");
    sstr << this->PETCTFusionNode->GetFrameReferenceTime() << " (msec)";
    
    std::string tmpstr;
    const char *tmpc;
    tmpstr = sstr.str();
    tmpc = tmpstr.c_str();
    this->FrameReferenceTimeLabel->SetText ( this->PETCTFusionNode->GetFrameReferenceTime());
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->FrameReferenceTimeLabel->SetText ( "no value found" );
      }
    else
      {
      this->FrameReferenceTimeLabel->SetText ( "" );
      }
    }

  if ( (this->PETCTFusionNode->GetDecayCorrection() != NULL ) &&
       (this->DecayCorrectionLabel ))
    {
    this->DecayCorrectionLabel->SetText ( this->PETCTFusionNode->GetDecayCorrection() );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->DecayCorrectionLabel->SetText ( "no value found" );
      }
    else
      {
      this->DecayCorrectionLabel->SetText ( "" );
      }
    }
  
  if (( this->PETCTFusionNode->GetDecayFactor() != NULL ) &&
      (this->DecayFactorLabel ))
    {
    this->DecayFactorLabel->SetText ( this->PETCTFusionNode->GetDecayFactor() );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->DecayFactorLabel->SetText ( "no value found" );
      }
    else
      {
      this->DecayFactorLabel->SetText ( "" );
      }
    }
  
  if ( ( this->PETCTFusionNode->GetRadionuclideHalfLife() != NULL ) &&
       (this->RTHalfLifeLabel ))
    {
    sstr.clear();
    sstr.str("");
    sstr << this->PETCTFusionNode->GetRadionuclideHalfLife();
    sstr << " (s)";
    std::string tmpstr;
    const char *tmpc;
    tmpstr = sstr.str();
    tmpc = tmpstr.c_str();
    this->RTHalfLifeLabel->SetText ( tmpc );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->RTHalfLifeLabel->SetText ( "no value found" );
      }
    else
      {
      this->RTHalfLifeLabel->SetText ( "" );
      }
    }
    
  if (( this->PETCTFusionNode->GetPhilipsSUVFactor()  != NULL ) &&
      (this->PhilipsSUVFactorLabel ))
    {
    this->PhilipsSUVFactorLabel->SetText ( this->PETCTFusionNode->GetPhilipsSUVFactor() );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->PhilipsSUVFactorLabel->SetText ( "no value found" );
      }
    else
      {
      this->PhilipsSUVFactorLabel->SetText ( "" );
      }
    }

  if (( this->PETCTFusionNode->GetCalibrationFactor()  != NULL ) &&
      (this->CalibrationFactorLabel ))
    {
    this->CalibrationFactorLabel->SetText ( this->PETCTFusionNode->GetCalibrationFactor() );
    }
  else
    {
    if ( updateDICOMevent )
      {
      this->CalibrationFactorLabel->SetText ( "no value found" );
      }
    else
      {
      this->CalibrationFactorLabel->SetText ( "" );
      }
    }

  std::stringstream ss;
  std::string s;
  //--- either clear or refresh labels.
  if ( this->SUVmaxLabel && this->PETCTFusionNode->GetSUVmax_t1() )
    {
    ss.str("");
    ss << this->PETCTFusionNode->GetSUVmax_t1();
    s = ss.str();
    this->SUVmaxLabel->SetText ( s.c_str() );
    }
  if ( this->SUVminLabel && this->PETCTFusionNode->GetSUVmin_t1() )
    {
    ss.str("");
    ss << this->PETCTFusionNode->GetSUVmin_t1();
    s = ss.str();
    this->SUVminLabel->SetText ( s.c_str() );
    }
  if ( this->SUVmeanLabel && this->PETCTFusionNode->GetSUVmean_t1() )
    {
    ss.str("");
    ss << this->PETCTFusionNode->GetSUVmean_t1();
    s = ss.str();
    this->SUVmeanLabel->SetText (s.c_str() );
    }
  if ( this->SUVmaxmeanLabel && this->PETCTFusionNode->GetSUVmaxmean_t1() )
    {
    ss.str("");
    ss << this->PETCTFusionNode->GetSUVmaxmean_t1();
    s = ss.str();
    this->SUVmaxmeanLabel->SetText ( s.c_str() );
    }

  this->UpdatingGUI = 0;
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  if ( this->MRMLScene == NULL )
    {
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }

  if ( this->PETCTFusionNode  == vtkMRMLPETCTFusionNode::SafeDownCast ( caller ) )
    {
    if ( event == vtkMRMLPETCTFusionNode::StartUpdatingDisplayEvent )
      {
      }
    else if ( event == vtkMRMLPETCTFusionNode::UpdateDisplayEvent )
      {
      //--- adjust color lut.
      if ( !this->UpdatingLUT )
        {
        this->UpdatePETDisplayFromMRML();
        return;
        }
      }
    else if ( event == vtkMRMLPETCTFusionNode::NonDICOMEvent )
      {
      if ( this->GetApplicationGUI () && this->GetApplicationGUI()->GetMainSlicerWindow() )
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
        dialog->SetStyleToMessage();
        dialog->SetText ( "The PET Volume is not a DICOM file,\nso parameters cannot be extracted automatically.\nParameters for computing SUV must be entered manually." );
        dialog->Create();
        dialog->Invoke();
        dialog->Delete();
        return;
        }
      }
    else if ( event == vtkMRMLPETCTFusionNode::WaitEvent )
      {
      }
    else if ( event == vtkMRMLPETCTFusionNode::DICOMUpdateEvent )
      {
      this->UpdateGUIFromMRML ( 1 );
      return;      
      }
    else if ( event == vtkMRMLPETCTFusionNode::ErrorEvent )
      {
      //--- post error message with node->MessageText
      if ( (this->PETCTFusionNode->GetMessageText() != NULL) &&
           (strcmp(this->PETCTFusionNode->GetMessageText(), "" )) )
        {
        }
      else
        {
        this->SetStatusText ( "" );
        }
      return;      
      }
    else if ( event == vtkMRMLPETCTFusionNode::StatusEvent )
      {
      if ( (this->PETCTFusionNode->GetMessageText() != NULL) &&
           (strcmp(this->PETCTFusionNode->GetMessageText(), "" )) )
        {
        this->SetStatusText ( this->PETCTFusionNode->GetMessageText() );
        }
      else
        {
        this->SetStatusText ( "" );
        }
      return;
      }
    else if ( event == vtkMRMLPETCTFusionNode::ComputeDoneEvent )
      {
      //--- update results panel.
      std::stringstream ss;
      std::string s;
      ss << this->PETCTFusionNode->GetSUVmax_t1();
      s.clear();
      s = ss.str();
      const char *suvmax = s.c_str();
      this->SUVmaxLabel->SetText ( suvmax );

      ss.str("");
      ss << this->PETCTFusionNode->GetSUVmin_t1();
      s.clear();
      s = ss.str();
      const char *suvmin = s.c_str();
      this->SUVminLabel->SetText ( suvmin );

      ss.str("");
      ss << this->PETCTFusionNode->GetSUVmean_t1();
      s.clear();
      s = ss.str();
      const char *suvmean = s.c_str();
      this->SUVmeanLabel->SetText ( suvmean );

      ss.str("");
      ss << this->PETCTFusionNode->GetSUVmaxmean_t1();
      s.clear();
      s = ss.str();
      const char *suvmaxmean = s.c_str();
      this->SUVmaxmeanLabel->SetText ( suvmaxmean );
      }
    return;    
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::EnablePETCTVolumeRendering ( )
{
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::DisablePETCTVolumeRendering ( )
{
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::SetStatusText(const char *txt)
{
  if ( this->GetApplicationGUI() )
    {
    if ( this->GetApplicationGUI()->GetMainSlicerWindow() )
      {
      this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
      }
    }
}




//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::BuildFusionFrame ( vtkKWWidget *page )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildFusionFrame: got Null SlicerApplication" );
    return;
    }

  vtkSlicerModuleCollapsibleFrame *dataFusionFrame = vtkSlicerModuleCollapsibleFrame::New();
  dataFusionFrame->SetParent(page);
  dataFusionFrame->Create();
  dataFusionFrame->SetLabelText("Data Fusion");
  dataFusionFrame->ExpandFrame();
  app->Script("pack %s -side top -expand y -fill x -padx 2 -pady 2", dataFusionFrame->GetWidgetName());
  //--- selector
  this->CTSelector = vtkSlicerNodeSelectorWidget::New() ;
  this->CTSelector->SetParent ( dataFusionFrame->GetFrame() );
  this->CTSelector->Create ( );
  this->CTSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->CTSelector->SetChildClassesEnabled(0);
  this->CTSelector->NoneEnabledOn();
  this->CTSelector->SetShowHidden(1);
  this->CTSelector->SetMRMLScene(this->GetMRMLScene());
  this->CTSelector->SetBorderWidth(2);
  this->CTSelector->SetPadX(2);
  this->CTSelector->SetPadY(2);
  this->CTSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->CTSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->CTSelector->SetLabelText( "CT Volume: ");
  this->CTSelector->GetLabel()->SetWidth (12 );
  this->CTSelector->UpdateMenu();
  this->CTSelector->SetBalloonHelpString("Select a CT volume to be displayed in the background layer.");
  //--- selector
  this->PETSelector = vtkSlicerNodeSelectorWidget::New() ;
  this->PETSelector->SetParent ( dataFusionFrame->GetFrame() );
  this->PETSelector->Create ( );
  this->PETSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->PETSelector->SetChildClassesEnabled(0);
  this->PETSelector->NoneEnabledOn();
  this->PETSelector->SetShowHidden(1);
  this->PETSelector->SetMRMLScene(this->GetMRMLScene());
  this->PETSelector->SetBorderWidth(2);
  this->PETSelector->SetPadX(2);
  this->PETSelector->SetPadY(2);
  this->PETSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->PETSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->PETSelector->SetLabelText( "PET Volume: ");
  this->PETSelector->GetLabel()->SetWidth ( 12) ;
  this->PETSelector->UpdateMenu();
  this->PETSelector->SetBalloonHelpString("Select a PET volume to be overlayed on the CT study.");
  //--- selector
  this->MaskSelector = vtkSlicerNodeSelectorWidget::New() ;
  this->MaskSelector->SetParent ( dataFusionFrame->GetFrame() );
  this->MaskSelector->Create ( );
  this->MaskSelector->SetNodeClass("vtkMRMLScalarVolumeNode", "LabelMap", "1", NULL);
  this->MaskSelector->SetChildClassesEnabled(0);
  this->MaskSelector->NoneEnabledOn();
  this->MaskSelector->SetShowHidden(1);
  this->MaskSelector->SetMRMLScene(this->GetMRMLScene());
  this->MaskSelector->SetBorderWidth(2);
  this->MaskSelector->SetPadX(2);
  this->MaskSelector->SetPadY(2);
  this->MaskSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->MaskSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->MaskSelector->SetLabelText( "Tumor Mask: ");
  this->MaskSelector->GetLabel()->SetWidth ( 12 );
  this->MaskSelector->UpdateMenu();
  this->MaskSelector->SetBalloonHelpString("Select a segmentation of tumor(s)--only required for analysis.");
  this->Script ( "pack %s %s %s -side top -anchor ne -fill x -expand y -padx 2 -pady 2",
                 this->CTSelector->GetWidgetName(),
                 this->PETSelector->GetWidgetName(),
                 this->MaskSelector->GetWidgetName());

  dataFusionFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::BuildDisplayFrame (  vtkKWWidget *page )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildDisplayFrame: got Null SlicerApplication" );
    return;
    }

  vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New();
  displayFrame->SetParent(page);
  displayFrame->Create();
  displayFrame->SetLabelText("Display");
  displayFrame->CollapseFrame();
  app->Script("pack %s -side top -expand y -fill x -padx 2 -pady 2", displayFrame->GetWidgetName());
  //--- selector
  this->LUTSelector = vtkSlicerNodeSelectorWidget::New() ;
  this->LUTSelector->SetParent ( displayFrame->GetFrame() );
  this->LUTSelector->Create ( );
  this->LUTSelector->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  this->LUTSelector->AddNodeClass("vtkMRMLColorTableNode", NULL, NULL, NULL);
  this->LUTSelector->AddNodeClass ("vtkMRMLPETProceduralColorNode", NULL, NULL, NULL );
  this->LUTSelector->AddExcludedChildClass ( "vtkMRMLDiffusionTensorDisplayPropertiesNode");
  this->LUTSelector->SetChildClassesEnabled(0);
  this->LUTSelector->NoneEnabledOn();
  this->LUTSelector->SetShowHidden(1);
  this->LUTSelector->SetMRMLScene(this->GetMRMLScene());
  this->LUTSelector->SetBorderWidth(2);
  this->LUTSelector->SetPadX(2);
  this->LUTSelector->SetPadY(2);
  this->LUTSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->LUTSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->LUTSelector->SetLabelText( "Color LUT: ");
  this->LUTSelector->GetLabel()->SetWidth ( 12);
  this->LUTSelector->UpdateMenu();
  this->LUTSelector->SetBalloonHelpString("Select a pseudocolor for PET volume (\"PETCT\" recommended).");
  this->Script ( "pack %s  -side top -anchor ne -fill x -expand y -padx 2 -pady 2",
                 this->LUTSelector->GetWidgetName() );
  //--- set range
  this->ColorRange = vtkKWRange::New();
  this->ColorRange->SetParent ( displayFrame->GetFrame() );
  this->ColorRange->Create();
  this->ColorRange->SetEntry1PositionToLeft();
  this->ColorRange->SetEntry2PositionToRight();
  this->ColorRange->SymmetricalInteractionOff();
  this->ColorRange->SetOrientationToHorizontal();
  this->ColorRange->SetCommand ( this, "ProcessColorRangeCommand" );
  this->ColorRange->SetStartCommand ( this, "ProcessColorRangeStartCommand" );
  this->ColorRange->SetEndCommand ( this, "ProcessColorRangeStopCommand" );
  this->ColorRange->SetEntriesCommand ( this, "ProcessColorRangeCommand" );
  this->Script ( "pack %s  -side top -anchor ne -fill x -expand y -padx 2 -pady 2",
                 this->ColorRange->GetWidgetName() );
  
  //--- Turn on/off volume rendering
  this->VolumeRenderCheckbox = vtkKWCheckButton::New();
  this->VolumeRenderCheckbox->SetParent ( displayFrame->GetFrame() );
  this->VolumeRenderCheckbox->Create();
  this->VolumeRenderCheckbox->SelectedStateOff();
  this->VolumeRenderCheckbox->SetText ( "Volume Rendering:");
  this->VolumeRenderCheckbox->SetStateToDisabled();
  this->Script ( "pack %s  -side top -anchor ne -fill x -expand y -padx 2 -pady 2",
                 this->VolumeRenderCheckbox->GetWidgetName() );


  displayFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::BuildAnalysisFrame ( vtkKWWidget *page  )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildAnalysisFrame: got Null SlicerApplication" );
    return;
    }

  vtkSlicerModuleCollapsibleFrame *analyzeFrame = vtkSlicerModuleCollapsibleFrame::New();
  analyzeFrame->SetParent(page);
  analyzeFrame->Create();
  analyzeFrame->SetLabelText("Study Parameters");
  analyzeFrame->ExpandFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", analyzeFrame->GetWidgetName());

  //--- AUTOMATIC parameterization from DICOM
  vtkKWFrameWithLabel *cfAuto = vtkKWFrameWithLabel::New();
  cfAuto->SetParent ( analyzeFrame->GetFrame() );
  cfAuto->Create();
  cfAuto->SetLabelText ("Parameters from DICOM" );
  cfAuto->ExpandFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", cfAuto->GetWidgetName());
  
  //--- MANUAL override.
  vtkKWFrameWithLabel *cfManual = vtkKWFrameWithLabel::New();
  cfManual->SetParent ( analyzeFrame->GetFrame() );
  cfManual->Create();
  cfManual->SetLabelText ("Set Parameters Manually" );
  cfManual->CollapseFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", cfManual->GetWidgetName());

  
  //---
  //--- Populate AUTOMATIC frame
  //---
  this->GetFromDICOMButton = vtkKWPushButton::New();
  this->GetFromDICOMButton->SetParent ( cfAuto->GetFrame() );
  this->GetFromDICOMButton->Create();
  this->GetFromDICOMButton->SetReliefToGroove();
  this->GetFromDICOMButton->SetText ( "Refresh" );

  vtkKWLabel *l5 = vtkKWLabel::New();
  l5->SetParent  ( cfAuto->GetFrame() );
  l5->Create();
  l5->SetText ( "   Patient's Weight: " );
  this->PatientWeightLabel = vtkKWLabel::New();
  this->PatientWeightLabel->SetParent  ( cfAuto->GetFrame() );
  this->PatientWeightLabel->Create();
  this->PatientWeightLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->PatientWeightLabel->SetText ( "" );  
//  this->PatientWeightLabel->SetWidth ( 24 );
  this->PatientWeightLabel->SetAnchorToWest();

  vtkKWLabel *l6 = vtkKWLabel::New();
  l6->SetParent  ( cfAuto->GetFrame() );
  l6->Create();
  l6->SetText ( "   Injected Dose: " );
  this->InjectedDoseLabel = vtkKWLabel::New();
  this->InjectedDoseLabel->SetParent  ( cfAuto->GetFrame() );
  this->InjectedDoseLabel->Create();
  this->InjectedDoseLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->InjectedDoseLabel->SetText ( "" );  
//  this->InjectedDoseLabel->SetWidth ( 24 );
  this->InjectedDoseLabel->SetAnchorToWest();

  vtkKWLabel *l7 = vtkKWLabel::New();
  l7->SetParent  ( cfAuto->GetFrame() );
  l7->Create();
  l7->SetText ( "   Temporal Positions: " );
  this->NumberOfTemporalPositionsLabel = vtkKWLabel::New();
  this->NumberOfTemporalPositionsLabel->SetParent  ( cfAuto->GetFrame() );
  this->NumberOfTemporalPositionsLabel->Create();
  this->NumberOfTemporalPositionsLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->NumberOfTemporalPositionsLabel->SetText ( "" );  
//  this->NumberOfTemporalPositionsLabel->SetWidth ( 24 );
  this->NumberOfTemporalPositionsLabel->SetAnchorToWest();
  
  vtkKWLabel *l8 = vtkKWLabel::New();
  l8->SetParent  ( cfAuto->GetFrame() );
  l8->Create();
  l8->SetText ( "   Series Time: ");
  this->SeriesTimeLabel = vtkKWLabel::New();
  this->SeriesTimeLabel->SetParent  ( cfAuto->GetFrame() );
  this->SeriesTimeLabel->Create();
  this->SeriesTimeLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->SeriesTimeLabel->SetText ( "" );  
//  this->SeriesTimeLabel->SetWidth ( 24 );
  this->SeriesTimeLabel->SetAnchorToWest();
  
  vtkKWLabel *l9 = vtkKWLabel::New();
  l9->SetParent  ( cfAuto->GetFrame() );
  l9->Create();
  l9->SetText ( "   Dose Time: ");
  this->RPStartTimeLabel = vtkKWLabel::New();
  this->RPStartTimeLabel->SetParent  ( cfAuto->GetFrame() );
  this->RPStartTimeLabel->Create();
  this->RPStartTimeLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->RPStartTimeLabel->SetText ( "" );  
//  this->RPStartTimeLabel->SetWidth ( 24 );
  this->RPStartTimeLabel->SetAnchorToWest();
  
  vtkKWLabel *l10 = vtkKWLabel::New();
  l10->SetParent  ( cfAuto->GetFrame() );
  l10->Create();
  l10->SetText ( "   Reference Time: ");
  this->FrameReferenceTimeLabel = vtkKWLabel::New();
  this->FrameReferenceTimeLabel->SetParent  ( cfAuto->GetFrame() );
  this->FrameReferenceTimeLabel->Create();
  this->FrameReferenceTimeLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->FrameReferenceTimeLabel->SetText ( "" );  
//  this->FrameReferenceTimeLabel->SetWidth ( 24 );
  this->FrameReferenceTimeLabel->SetAnchorToWest();

  vtkKWLabel *l11 = vtkKWLabel::New();
  l11->SetParent  ( cfAuto->GetFrame() );
  l11->Create();
  l11->SetText ( "   Decay Correction: ");
  this->DecayCorrectionLabel = vtkKWLabel::New();
  this->DecayCorrectionLabel->SetParent  ( cfAuto->GetFrame() );
  this->DecayCorrectionLabel->Create();
  this->DecayCorrectionLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->DecayCorrectionLabel->SetText ( "" );  
//  this->DecayCorrectionLabel->SetWidth ( 24 );
  this->DecayCorrectionLabel->SetAnchorToWest();

  vtkKWLabel *l12 = vtkKWLabel::New();
  l12->SetParent  ( cfAuto->GetFrame() );
  l12->Create();
  l12->SetText ( "   Decay Factor: ");
  this->DecayFactorLabel = vtkKWLabel::New();
  this->DecayFactorLabel->SetParent  ( cfAuto->GetFrame() );
  this->DecayFactorLabel->Create();
  this->DecayFactorLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->DecayFactorLabel->SetText ( "" );  
//  this->DecayFactorLabel->SetWidth ( 24 );
  this->DecayFactorLabel->SetAnchorToWest();

  vtkKWLabel *l13 = vtkKWLabel::New();
  l13->SetParent  ( cfAuto->GetFrame() );
  l13->Create();
  l13->SetText ( "   Radionuclide Half Life: ");
  this->RTHalfLifeLabel = vtkKWLabel::New();
  this->RTHalfLifeLabel->SetParent  ( cfAuto->GetFrame() );
  this->RTHalfLifeLabel->Create();
  this->RTHalfLifeLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->RTHalfLifeLabel->SetText ( "" );  
//  this->RTHalfLifeLabel->SetWidth ( 24 );
  this->RTHalfLifeLabel->SetAnchorToWest();

  vtkKWLabel *l14 = vtkKWLabel::New();
  l14->SetParent  ( cfAuto->GetFrame() );
  l14->Create();
  l14->SetText ( "   Philips SUV Factor: ");
  this->PhilipsSUVFactorLabel = vtkKWLabel::New();
  this->PhilipsSUVFactorLabel->SetParent  ( cfAuto->GetFrame() );
  this->PhilipsSUVFactorLabel->Create();
  this->PhilipsSUVFactorLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->PhilipsSUVFactorLabel->SetText ( "" );  
//  this->PhilipsSUVFactorLabel->SetWidth ( 24 );
  this->PhilipsSUVFactorLabel->SetAnchorToWest();

  vtkKWLabel *l15 = vtkKWLabel::New();
  l15->SetParent  ( cfAuto->GetFrame() );
  l15->Create();
  l15->SetText ( "   Calibration Factor: ");
  this->CalibrationFactorLabel = vtkKWLabel::New();
  this->CalibrationFactorLabel->SetParent  ( cfAuto->GetFrame() );
  this->CalibrationFactorLabel->Create();
  this->CalibrationFactorLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->CalibrationFactorLabel->SetText ( "" );  
//  this->CalibrationFactorLabel->SetWidth ( 24 );
  this->CalibrationFactorLabel->SetAnchorToWest();


  //--- grid
  app->Script ( "grid columnconfigure %s 0 -weight 0", cfAuto->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", cfAuto->GetFrame()->GetWidgetName() );
  app->Script("grid %s -row 0 -column 0 -sticky e  -padx 2 -pady 2", l5->GetWidgetName() );
  app->Script("grid %s -row 0 -column 1 -sticky w -padx 4 -pady 2", this->PatientWeightLabel->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l6->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky w -padx 4 -pady 2", this->InjectedDoseLabel->GetWidgetName() );
  app->Script("grid %s -row 2 -column 0 -sticky e -padx 2 -pady 2", l8->GetWidgetName() );
  app->Script("grid %s -row 2 -column 1 -sticky w -padx 4 -pady 2", this->SeriesTimeLabel->GetWidgetName() );
  app->Script("grid %s -row 3 -column 0 -sticky e -padx 2 -pady 2", l9->GetWidgetName() );
  app->Script("grid %s -row 3 -column 1 -sticky w -padx 4 -pady 2", this->RPStartTimeLabel->GetWidgetName() );
  app->Script("grid %s -row 4 -column 0 -sticky e -padx 2 -pady 2", l11->GetWidgetName() );
  app->Script("grid %s -row 4 -column 1 -sticky w -padx 4 -pady 2", this->DecayCorrectionLabel->GetWidgetName() );
  app->Script("grid %s -row 5 -column 0 -sticky e -padx 2 -pady 2", l12->GetWidgetName() );
  app->Script("grid %s -row 5 -column 1 -sticky w -padx 4 -pady 2", this->DecayFactorLabel->GetWidgetName() );
  app->Script("grid %s -row 6 -column 0 -sticky e -padx 2 -pady 2", l13->GetWidgetName() );
  app->Script("grid %s -row 6 -column 1 -sticky w -padx 4 -pady 2", this->RTHalfLifeLabel->GetWidgetName() );
  app->Script("grid %s -row 7 -column 0 -sticky e -padx 2 -pady 2", this->GetFromDICOMButton->GetWidgetName() );  
/*
  app->Script("grid %s -row 7 -column 0 -sticky e -padx 2 -pady 2", l10->GetWidgetName() );
  app->Script("grid %s -row 7 -column 1 -sticky w -padx 4 -pady 2", this->FrameReferenceTimeLabel->GetWidgetName() );
  app->Script("grid %s -row 8 -column 0 -sticky e -padx 2 -pady 2", l7->GetWidgetName() );
  app->Script("grid %s -row 8 -column 1 -sticky w -padx 4 -pady 2", this->NumberOfTemporalPositionsLabel->GetWidgetName() );
  app->Script("grid %s -row 9 -column 0 -sticky e -padx 2 -pady 2", l15->GetWidgetName() );
  app->Script("grid %s -row 9 -column 1 -sticky w -padx 4 -pady 2", this->CalibrationFactorLabel->GetWidgetName() );
  app->Script("grid %s -row 10 -column 0 -sticky e -padx 2 -pady 2", l14->GetWidgetName() );
  app->Script("grid %s -row 10 -column 1 -sticky w -padx 4 -pady 2", this->PhilipsSUVFactorLabel->GetWidgetName() );
*/


  //---
  //--- Populate MANUAL frame
  //---

  //--- injected dose labels, entry, unit menu
  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( cfManual->GetFrame() );
  l1->Create();
  l1->SetText ( "Injected Dose:" );
  this->InjectedDoseEntry = vtkKWEntry::New();
  this->InjectedDoseEntry->SetParent ( cfManual->GetFrame() );
  this->InjectedDoseEntry->Create();
  this->InjectedDoseEntry->SetWidth ( 11 );
  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( cfManual->GetFrame() );
  l2->Create();
  l2->SetText ( "Units:" );
  this->DoseUnitsMenuButton = vtkKWMenuButton::New ( );
  this->DoseUnitsMenuButton->SetParent ( cfManual->GetFrame() );
  this->DoseUnitsMenuButton->Create();  
  this->DoseUnitsMenuButton->IndicatorVisibilityOn();
  this->DoseUnitsMenuButton->SetWidth ( 26 );
  //--- populate menu
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "megabecquerels [MBq]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "kilobecquerels [kBq]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "becquerels [Bq]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "millibecquerels [mBq]" );  
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "microbecquerels [uBq]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "megacuries [MCi]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "kilocuries [kCi]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "curies [Ci]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "millicuries [mCi]" );
  this->DoseUnitsMenuButton->GetMenu()->AddRadioButton ( "microcuries [uCi]" );
  //--- set initial value
  this->DoseUnitsMenuButton->SetValue ( "megabecquerels [MBq]" );


  //--- injected dose labels, entry, unit menu
  vtkKWLabel *l16 = vtkKWLabel::New();
  l16->SetParent ( cfManual->GetFrame() );
  l16->Create();
  l16->SetText ( "Tissue Radioactivity Units:" );
  this->TissueUnitsMenuButton = vtkKWMenuButton::New ( );
  this->TissueUnitsMenuButton->SetParent ( cfManual->GetFrame() );
  this->TissueUnitsMenuButton->Create();  
  this->TissueUnitsMenuButton->IndicatorVisibilityOn();
  this->TissueUnitsMenuButton->SetWidth ( 26 );
  //--- populate menu
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "megabecquerels [MBq]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "kilobecquerels [kBq]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "becquerels [Bq]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "millibecquerels [mBq]" );  
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "microbecquerels [uBq]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "megacuries [MCi]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "kilocuries [kCi]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "curies [Ci]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "millicuries [mCi]" );
  this->TissueUnitsMenuButton->GetMenu()->AddRadioButton ( "microcuries [uCi]" );
  //--- set initial value
  this->TissueUnitsMenuButton->SetValue ( "megabecquerels [MBq]" );


  //-- patient weight labels, entry, unit menu
  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( cfManual->GetFrame() );
  l3->Create();
  l3->SetText ( "Patient Weight:" );
  this->PatientWeightEntry = vtkKWEntry::New();
  this->PatientWeightEntry->SetParent ( cfManual->GetFrame() );
  this->PatientWeightEntry->Create();
  this->PatientWeightEntry->SetWidth ( 11 );
  vtkKWLabel *l4 = vtkKWLabel::New();
  l4->SetParent ( cfManual->GetFrame() );
  l4->Create();
  l4->SetText ( "Units:" );
  this->WeightUnitsMenuButton = vtkKWMenuButton::New ( );
  this->WeightUnitsMenuButton->SetParent ( cfManual->GetFrame() );
  this->WeightUnitsMenuButton->Create();  
  this->WeightUnitsMenuButton->IndicatorVisibilityOn();
  this->WeightUnitsMenuButton->SetWidth ( 26 );
  this->WeightUnitsMenuButton->GetMenu()->AddRadioButton ( "kilograms [kg]" );
  this->WeightUnitsMenuButton->GetMenu()->AddRadioButton ( "grams [g]" );
  this->WeightUnitsMenuButton->GetMenu()->AddRadioButton ( "pounds [lb]" );
  //--- set initial value
  this->WeightUnitsMenuButton->SetValue ( "kilograms [kg]");
  
  app->Script ( "grid columnconfigure %s 0 -weight 0", cfManual->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", cfManual->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 2 -weight 0", cfManual->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 3 -weight 1", cfManual->GetFrame()->GetWidgetName() );

  app->Script("grid %s -row 0 -column 0 -columnspan 3 -sticky e -padx 2 -pady 2", l16->GetWidgetName() );
  app->Script("grid %s -row 0 -column 3 -sticky e -padx 2 -pady 2", this->TissueUnitsMenuButton->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l1->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky ew -padx 2 -pady 2", this->InjectedDoseEntry->GetWidgetName() );
  app->Script("grid %s -row 1 -column 2 -sticky e -padx 2 -pady 2", l2->GetWidgetName() );
  app->Script("grid %s -row 1 -column 3 -sticky ew -padx 2 -pady 2", this->DoseUnitsMenuButton->GetWidgetName() );
  app->Script("grid %s -row 2 -column 0 -sticky e -padx 2 -pady 2", l3->GetWidgetName() );
  app->Script("grid %s -row 2 -column 1 -sticky ew -padx 2 -pady 2", this->PatientWeightEntry->GetWidgetName() );
  app->Script("grid %s -row 2 -column 2 -sticky e -padx 2 -pady 2", l4->GetWidgetName() );
  app->Script("grid %s -row 2 -column 3 -sticky ew -padx 2 -pady 2", this->WeightUnitsMenuButton->GetWidgetName() );

  //--- clean up
  l1->Delete();
  l2->Delete();
  l3->Delete();
  l4->Delete();
  l5->Delete();
  l6->Delete();
  l7->Delete();
  l8->Delete();
  l9->Delete();
  l10->Delete();
  l11->Delete();
  l12->Delete();
  l13->Delete();
  l14->Delete();  
  l15->Delete();  
  l16->Delete();  
  cfManual->Delete();
  cfAuto->Delete();
  analyzeFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::BuildReportFrame ( vtkKWWidget *page  )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildReportFrame: got Null SlicerApplication" );
    return;
    }

  vtkSlicerModuleCollapsibleFrame *outputFrame = vtkSlicerModuleCollapsibleFrame::New();
  outputFrame->SetParent(page);
  outputFrame->Create();
  outputFrame->SetLabelText("Compute & View Results");
  outputFrame->ExpandFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", outputFrame->GetWidgetName());
  
  //--- get from dicom button
  this->ComputeButton = vtkKWPushButton::New();
  this->ComputeButton->SetParent ( outputFrame->GetFrame() );
  this->ComputeButton->Create();
  this->ComputeButton->SetReliefToGroove();
  this->ComputeButton->SetText ( "Compute SUV" );

  //--- suv max
  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( outputFrame->GetFrame() );
  l1->Create();
  l1->SetText ( "SUV_max: " );
  l1->SetWidth (14);
  l1->SetAnchorToEast();
  this->SUVmaxLabel = vtkKWLabel::New();
  this->SUVmaxLabel->SetParent ( outputFrame->GetFrame() );
  this->SUVmaxLabel->Create();
  this->SUVmaxLabel->SetWidth ( 11 );  
  this->SUVmaxLabel->SetText ( "" );  
  this->SUVmaxLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  vtkKWLabel *l4 = vtkKWLabel::New();
  l4->SetParent ( outputFrame->GetFrame() );
  l4->Create();
  l4->SetText ( "SUV_min: " );
  l4->SetWidth (14);
  l4->SetAnchorToEast();
  this->SUVminLabel = vtkKWLabel::New();
  this->SUVminLabel->SetParent ( outputFrame->GetFrame() );
  this->SUVminLabel->Create();
  this->SUVminLabel->SetWidth ( 11 );  
  this->SUVminLabel->SetText ( "" );
  this->SUVminLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  //--- normalized value
  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( outputFrame->GetFrame() );
  l2->Create();
  l2->SetText ( "SUV_mean: " );
  l2->SetWidth (14);
  l2->SetAnchorToEast();  
  this->SUVmeanLabel = vtkKWLabel::New();
  this->SUVmeanLabel->SetParent ( outputFrame->GetFrame() );
  this->SUVmeanLabel->Create();
  this->SUVmeanLabel->SetWidth ( 11 );  
  this->SUVmeanLabel->SetText ( "" );  
  this->SUVmeanLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  
  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( outputFrame->GetFrame() );
  l3->Create();
  l3->SetText ( "SUV_max_mean: " );
  l3->SetWidth (14);
  l3->SetAnchorToEast();
  this->SUVmaxmeanLabel = vtkKWLabel::New();
  this->SUVmaxmeanLabel->SetParent ( outputFrame->GetFrame() );
  this->SUVmaxmeanLabel->Create();
  this->SUVmaxmeanLabel->SetWidth ( 11 );  
  this->SUVmaxmeanLabel->SetText ( "" );  
  this->SUVmaxmeanLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  
  app->Script("grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", this->ComputeButton->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l1->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky w -padx 2 -pady 2", SUVmaxLabel->GetWidgetName() );
  app->Script("grid %s -row 2 -column 0 -sticky e -padx 2 -pady 2", l4->GetWidgetName() );
  app->Script("grid %s -row 2 -column 1 -sticky w -padx 2 -pady 2", SUVminLabel->GetWidgetName() );
  app->Script("grid %s -row 3 -column 0 -sticky e -padx 2 -pady 2", l2->GetWidgetName() );
  app->Script("grid %s -row 3 -column 1 -sticky w -padx 2 -pady 2", SUVmeanLabel->GetWidgetName() );
//  app->Script("grid %s -row 4 -column 0 -sticky e -padx 2 -pady 2", l3->GetWidgetName() );
//  app->Script("grid %s -row 4 -column 1 -sticky w -padx 2 -pady 2", SUVmaxmeanLabel->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", outputFrame->GetFrame()->GetWidgetName() );  
  app->Script ( "grid columnconfigure %s 1 -weight 1", outputFrame->GetFrame()->GetWidgetName() );  
  
  l1->Delete();
  l2->Delete();
  l3->Delete();
  l4->Delete();
  outputFrame->Delete();
    
}




//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::InitializeGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "InitializeGUI: got Null SlicerApplication" );
    return;
    }
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ( "InitializeGUI: Got NULL Module Logic. ");
    return;    
    }
  if ( this->ApplicationLogic == NULL )
    {
    vtkErrorMacro ( "InitializeGUI: Got NULL Application Logic. ");
    return;
    }
  if ( this->ApplicationLogic->GetSelectionNode() == NULL )
    {
    vtkErrorMacro ( "InitializeGUI: Got NULL SelectionNode in Logic.");
    return;
    }
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "InitializeGUI: Got NULL MRMLScene. ");
    return;
    }
  
  if ( this->SUVmaxLabel )
    {
    this->SUVmaxLabel->SetText ( "" );
    }
  if ( this->SUVminLabel )
    {
    this->SUVminLabel->SetText ( "" );
    }
  if ( this->SUVmeanLabel )
    {
    this->SUVmeanLabel->SetText ( "" );
    }
  if ( this->SUVmaxmeanLabel )
    {
    this->SUVmaxmeanLabel->SetText ( "" );
    }
  if ( this->PatientWeightLabel )
    {
    this->PatientWeightLabel->SetText ( "" );
    }
  if ( this->InjectedDoseLabel )
    {
    this->InjectedDoseLabel->SetText ( "" );
    }
  if ( this->NumberOfTemporalPositionsLabel )
    {
    this->NumberOfTemporalPositionsLabel->SetText ( "" );
    }
  if ( this->SeriesTimeLabel )
    {
    this->SeriesTimeLabel->SetText ( "" );
    }
  if ( this->RPStartTimeLabel )
    {
    this->RPStartTimeLabel->SetText ( "" );
    }
  if ( this->FrameReferenceTimeLabel )
    {
    this->FrameReferenceTimeLabel->SetText ( "" );
    }
  if ( this->DecayCorrectionLabel )
    {
    this->DecayCorrectionLabel->SetText ( "" );
    }
  if ( this->DecayFactorLabel )
    {
    this->DecayFactorLabel->SetText ( "" );
    }
  if ( this->RTHalfLifeLabel )
    {  
    this->RTHalfLifeLabel->SetText ( "" );
    }
  if ( this->PhilipsSUVFactorLabel )
    {
    this->PhilipsSUVFactorLabel->SetText ( "" );
    }
  if ( this->CalibrationFactorLabel )
    {
    this->CalibrationFactorLabel->SetText ( "" );
    }
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildGUI: got NULL MainSlicerWindow");
    return;
    }
  win->SetStatusText ( "Building Interface for PETCTFusion Module...." );
  app->Script ( "update idletasks" );

  this->UIPanel->AddPage ( "PETCTFusion", "PETCTFusion", NULL );

  //---
  //--- HELP
  //---
  const char* help_text = "The PETCTFusion displays PET/CT data and performs some PET quantifications.\n\nFrom: http://www.turkupetcentre.net/modelling/methods/suv.html: \n\nThe PET quantifier \"Standardized Uptake Value\", SUV, (or \"Dose Uptake Ratio\", DUR) is calculated as a ratio of tissue radioactivity concentration CPET(t)  at time t and in units (kBq/ml), and injected dose in units (MBq) at the time of injection divided by body weight (kg).\n\nForumula implemented: SUVbw = CPET(t) / (Injected dose / Patient's weight)\n\nThe input units are converted to those above, and the resulting SUV values are in units (g/ml).\n\nThe injected dose may also be corrected by the lean body mass, or body surface area (BSA) (Kim et al., 1994). Verbraecken et al. (2006) review the different formulas for calculating the BSA.\n\nForumula to be implemented: SUVbsa= CPET(T) / (Injected dose / BSA)\n\nCancer treatment responce is usually assessed with FDG PET by calculating the SUV on the highest image pixel in the tumour regions (SUVmax). Alternatively, tumour volume can be estimated using threshold or region growing techniques, and average SUV inside the region is reported as such or multiplied by tumour volume to calculate the total glycolytic volume, TGV (Boucek et al., 2008). Nahmias and Wahl (2008) reported that the use of SUVmax has worse reproducibility (3% ± 11%) than does the SUVmean value (1% ± 7%).\n\nCalculation of SUV does not require blood sampling or dynamic imaging. The imaging must take place at a late time point, and always at the same time point, if results are to be compared (Eckelman et al., 2000). \n\n The PET image should contain only one (late) frame. If the PET image contains more than one frame, the frames can be averaged together (check correctness of this). \n\n\nThis module is new and will be extended.";
  const char* ack_text = "PETCTFusion was developed by Wendy Plesniak with help from Jeffrey Yapp and Ron Kikinis. This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\n";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PETCTFusion" );    
  this->BuildHelpAndAboutFrame(page, help_text, ack_text);

  //---
  //--- ACKNOWLEDGEMENT
  //---
    vtkKWLabel *NACLabel = vtkKWLabel::New();
    NACLabel->SetParent ( this->GetLogoFrame() );
    NACLabel->Create();
    NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );   

    vtkKWLabel *NAMICLabel = vtkKWLabel::New();
    NAMICLabel->SetParent ( this->GetLogoFrame() );
    NAMICLabel->Create();
    NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

    vtkKWLabel *NCIGTLabel = vtkKWLabel::New();
    NCIGTLabel->SetParent ( this->GetLogoFrame() );
    NCIGTLabel->Create();
    NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
    vtkKWLabel *BIRNLabel = vtkKWLabel::New();
    BIRNLabel->SetParent ( this->GetLogoFrame() );
    BIRNLabel->Create();
    BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );

    vtkKWLabel *CTSCLabel = vtkKWLabel::New();
    CTSCLabel->SetParent ( this->GetLogoFrame() );
    CTSCLabel->Create();
    CTSCLabel->SetImageToIcon (this->GetAcknowledgementIcons()->GetCTSCLogo() );

    //
    //-- pack logos
    app->Script ( "grid %s -row 0 -columnspan 2 -padx 20 -pady 2 -sticky w",  CTSCLabel->GetWidgetName());                  
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w", NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky e", NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 2 -column 0 -padx 2 -pady 2 -sticky w",  BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 2 -column 1 -padx 2 -pady 2 -sticky e",  NCIGTLabel->GetWidgetName());                  

    app->Script ( "grid columnconfigure %s 0 -weight 0", this->GetLogoFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 0", this->GetLogoFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 1", this->GetLogoFrame()->GetWidgetName() );

    //
    //--- clean up.
    NACLabel->Delete();
    NAMICLabel->Delete();
    NCIGTLabel->Delete();
    BIRNLabel->Delete();
    CTSCLabel->Delete();

    //
    //--- Data Fusion Frame
    this->BuildFusionFrame ( page );

    //
    //--- Display Frame
    this->BuildDisplayFrame ( page );

    //
    //--- Analyze Frame
    this->BuildAnalysisFrame ( page );

    //
    //--- Output Frame
    this->BuildReportFrame ( page );

    this->InitializePETMinAndMax();
    this->InitializeGUI();
    this->Built = true;
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::Init ( )
{
}


//---------------------------------------------------------------------------
 void vtkPETCTFusionGUI::ProcessColorRangeCommand ( double min, double max)
{
  this->UpdateNodeColorRange();
}


//---------------------------------------------------------------------------
 void vtkPETCTFusionGUI::ProcessColorRangeStartCommand( double min, double max)
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->ColorRange == NULL )
    {
    return;
    }

  //--- Turn on UpdatingLUT
  this->UpdatingLUT = 1;

  //--- make sure node and gui are in synch.
  if ( this->ColorRange->GetEntry1()->GetValueAsDouble() !=
       this->PETCTFusionNode->GetColorRangeMin() )
    {
    this->PETCTFusionNode->SetColorRangeMin ( this->ColorRange->GetEntry1()->GetValueAsDouble() );
    }
  
  if ( this->ColorRange->GetEntry2()->GetValueAsDouble() !=
       this->PETCTFusionNode->GetColorRangeMax() )
    {
    this->PETCTFusionNode->SetColorRangeMax ( this->ColorRange->GetEntry2()->GetValueAsDouble() );
    }
}



//--------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessColorRangeStopCommand ( double min, double max)
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->ColorRange == NULL )
    {
    return;
    }
  //--- make sure node and gui are in synch.
  this->PETCTFusionNode->SetColorRange (this->ColorRange->GetEntry1()->GetValueAsDouble(),
                                        this->ColorRange->GetEntry2()->GetValueAsDouble() );

  //--- Turn off UpdatingLUT
  this->UpdatingLUT = 0;
}





//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ResetManualEntryGUI ( )
{
  if ( this->UpdatingGUI )
    {
    return;
    }

  if ( this->PatientWeightEntry )
    {
    this->PatientWeightEntry->SetValue ("");
    }
  if ( this->PETCTFusionNode )
    {
    this->PETCTFusionNode->SetPatientWeight ( 0.0);
    }

  
  if ( this->InjectedDoseEntry )
    {
    this->InjectedDoseEntry->SetValue ( "" );
    }
  if ( this->PETCTFusionNode )
    {
    this->PETCTFusionNode->SetInjectedDose ( 0.0);
    }

}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ClearDICOMInformation()
{
  if ( this->UpdatingGUI )
    {
    return;
    }
  if ( this->PatientWeightLabel )
    {
    this->PatientWeightLabel->SetText ("");
    }
  if ( this->InjectedDoseLabel )
    {
    this->InjectedDoseLabel->SetText ( "");
    }
  if ( this->NumberOfTemporalPositionsLabel )
    {
    this->NumberOfTemporalPositionsLabel->SetText ( "" );
    }
  if ( this->SeriesTimeLabel )
    {
    this->SeriesTimeLabel->SetText ("");
    }
  if ( this->RPStartTimeLabel )
    {
    this->RPStartTimeLabel->SetText ("");
    }
  if ( this->FrameReferenceTimeLabel )
    {
    this->FrameReferenceTimeLabel->SetText ("");
    }
  if ( this->DecayCorrectionLabel )
    {
    this->DecayCorrectionLabel->SetText ("");
    }
  if ( this->DecayFactorLabel )
    {
    this->DecayFactorLabel->SetText ("");
    }
  if ( this->RTHalfLifeLabel )
    {
    this->RTHalfLifeLabel->SetText ("");
    }
  if ( this->PhilipsSUVFactorLabel )
    {
    this->PhilipsSUVFactorLabel->SetText ("");
    }
  if ( this->CalibrationFactorLabel )
    {
    this->CalibrationFactorLabel->SetText ("");
    }
    
}




//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateDICOMPanel()
{
  
  this->ClearDICOMInformation();
  int found = 0;
  vtkSlicerWaitMessageWidget *wmss = NULL;
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //--- check to make sure there's a PET volume selected.
  if ( this->PETCTFusionNode->GetInputPETReference() == NULL )
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent ( appGUI->GetMainSlicerWindow() );
    dialog->SetStyleToMessage();
    dialog->SetText ( "Could not find selected PET study. No DICOM parameters retrieved." );
    dialog->Create();
    dialog->Invoke();
    dialog->Delete();
    return;
    }

  //--- pop up wait message...
  if ( appGUI && appGUI->GetMainSlicerWindow() )
    {
    wmss = vtkSlicerWaitMessageWidget::New();
    if ( wmss )
      {
      wmss->SetParent ( appGUI->GetMainSlicerWindow() );
      wmss->Create();
      wmss->SetText ( "Updating parameters from DICOM. This may take a little while.");
      wmss->DisplayWindow();
      }
    }

  if ( this->Logic )
    {
    std::string path;
    std::string filename;
    std::string tmp;
    //--- get path from node filename.
    if ( this->PETSelector && this->PETSelector->GetSelected() )
      {
      vtkMRMLScalarVolumeNode *n;
      n = vtkMRMLScalarVolumeNode::SafeDownCast ( this->PETSelector->GetSelected() );
      if (n->GetStorageNode())
        {
        tmp = n->GetStorageNode()->GetFileName();
        filename = vtksys::SystemTools::GetFilenameName ( tmp );
        size_t index = tmp.find(filename);
        if ( index  != std::string::npos )
          {
          path = tmp.substr ( 0, index );
          this->Logic->GetParametersFromDICOMHeader(path.c_str());
          found = 1;
          }
        }
      }
    }
  else
    {
    vtkErrorMacro ( "UpdateDICOMPanel: Got NULL Logic pointer." );
    }


  //--- withdraw wait message...
  if ( wmss )
    {
    wmss->WithdrawWindow();
    wmss->Delete();
    }

  //--- report error if appropriate
  if ( !found && appGUI && appGUI->GetMainSlicerWindow() )
    {

    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent ( appGUI->GetMainSlicerWindow() );
    dialog->SetStyleToMessage();
    dialog->SetText ( "Could not find selected PET study. No DICOM parameters retrieved." );
    dialog->Create();
    dialog->Invoke();
    dialog->Delete();
    }


}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdatePETDisplayFromMRML ()
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->ColorRange == NULL )
    {
    return;
    }
  this->ColorRange->SetWholeRange ( this->PETCTFusionNode->GetPETMin(), this->PETCTFusionNode->GetPETMax());

  //---
  //--- Method gets called if PETSelector changes
  //--- Scales current LUT to span the desired range of values
  //--- as specified in state.
  //---

  double tmpMin, tmpMax;
  tmpMin = this->ColorRange->GetEntry1()->GetValueAsDouble();
  tmpMax = this->ColorRange->GetEntry2()->GetValueAsDouble();
  
  if ( (tmpMin != this->PETCTFusionNode->GetColorRangeMin()) ||
       tmpMax != this->PETCTFusionNode->GetColorRangeMax() )
    {
    this->ColorRange->GetEntry1()->SetValueAsDouble ( this->PETCTFusionNode->GetColorRangeMin() );
    this->ColorRange->GetEntry2()->SetValueAsDouble ( this->PETCTFusionNode->GetColorRangeMax() );
    this->ScaleColormap( this->PETCTFusionNode->GetColorRangeMin(),
                         this->PETCTFusionNode->GetColorRangeMax());
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateNodeColorRange ()
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->ColorRange == NULL )
    {
    return;
    }

  //---
  //--- Method gets called if LUTSelector changes
  //--- Scales current LUT to span the desired range of values
  //--- as specified by GUI interaction
  //---
  double tmpMin, tmpMax;

  tmpMin = this->ColorRange->GetEntry1()->GetValueAsDouble();
  tmpMax = this->ColorRange->GetEntry2()->GetValueAsDouble();
  if ( (tmpMin != this->PETCTFusionNode->GetColorRangeMin()) ||
       tmpMax != this->PETCTFusionNode->GetColorRangeMax() )
    {
    this->PETCTFusionNode->SetColorRange (tmpMin, tmpMax );
    this->ScaleColormap ( this->PETCTFusionNode->GetColorRangeMin(),
                          this->PETCTFusionNode->GetColorRangeMax() );
    this->PETCTFusionNode->InvokeEvent (vtkMRMLPETCTFusionNode::UpdateDisplayEvent );
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ScaleColormap( double min, double max)
{
  //--- we want to scale the colors in the selected map
  //--- to span the range from:
  //--- PETCTFusionNode->GetColorRangeMin to
  //--- PETCTFusionNode->GetcolorRangeMax.

  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "ScaleColorMap: Got NULL MRMLScene." );
    return;
    }
  if ( this->GetApplication() == NULL )
    {
    vtkErrorMacro ( "ScaleColorMap: Got NULL Application." );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ScaleColorMap: Got NULL PETCTFusionNode." );
    return;
    }
  if ( this->PETCTFusionNode->GetInputPETReference() == NULL )
    {
    vtkErrorMacro ( "ScaleColorMap: Got NULL PET volume reference." );
    return;
    }
  if ( this->PETCTFusionNode->GetPETLUT() == NULL )
    {
    vtkErrorMacro ( "ScaleColorMap: Got NULL PET LUT." );
    return;
    }
  vtkMRMLScalarVolumeNode *node = vtkMRMLScalarVolumeNode::SafeDownCast (
                                                                         this->MRMLScene->GetNodeByID( this->PETCTFusionNode->GetInputPETReference() ));
  if ( node == NULL )
    {
    vtkErrorMacro ( "ScaleColorMap: Got NULL PET volume node." );
    return;
    }

  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node->GetDisplayNode());
  //--- check the volume -- if it doesn't yet have a display node,
  //--- we need to create one
  if (displayNode == NULL)
    {
    displayNode = vtkMRMLScalarVolumeDisplayNode::New ();
    displayNode->SetScene(this->MRMLScene);
    this->MRMLScene->AddNode (displayNode);
    displayNode->Delete();

    //--- Set the color map to the one selected in the PETLUTSelector.
    vtkMRMLColorNode *c;
    c = vtkMRMLColorNode::SafeDownCast (this->LUTSelector->GetSelected() );
    if ( c )
      {
      displayNode->SetAndObserveColorNodeID (c->GetID() );
      }
    node->SetAndObserveDisplayNodeID( displayNode->GetID() );
    }
  
  if ( displayNode )
    {
    displayNode->DisableModifiedEventOn();
    vtkMRMLScalarVolumeNode *svolumeNode = NULL;
    svolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

    if (displayNode->GetAutoWindowLevel() )
      {
      //update sliders with recomputed values
      svolumeNode->CalculateScalarAutoLevels(displayNode);
      }

    //--- Adjust node's window, level and threshold.
    double min = this->PETCTFusionNode->GetColorRangeMin();
    double max = this->PETCTFusionNode->GetColorRangeMax();
    displayNode->SetWindow( max - min );
    displayNode->SetLevel( min + (0.5 * (max - min)) );
    displayNode->SetUpperThreshold(max);
    displayNode->SetLowerThreshold(min);

    displayNode->DisableModifiedEventOff();
    displayNode->InvokePendingModifiedEvent();
    }
    return;
}
  


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdatePETRange ()
{
  if (this->ApplicationLogic == NULL)
    {
    return;
    }
  if ( this->ApplicationLogic->GetSelectionNode() == NULL )
    {
    return;
    }
  if ( this->MRMLScene == NULL )
    {
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }

  //---
  //--- Method gets called if PETSelector changes.
  //--- Checks range of PET image values.
  //---
  //--- get image data for pet
  vtkMRMLVolumeNode *v = vtkMRMLVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID (this->ApplicationLogic->GetSelectionNode()->GetSecondaryVolumeID() ));
  if ( v && v->GetImageData() )
    {
    //--- build a histogram
    vtkKWHistogram *h = vtkKWHistogram::New();
    h->BuildHistogram ( v->GetImageData()->GetPointData()->GetScalars(), 0);
    double *range = h->GetRange();
    this->PETCTFusionNode->SetPETMin( range[0]);
    this->PETCTFusionNode->SetPETMax( range[1]);

    //--- record max and min in node.
    this->PETCTFusionNode->SetColorRangeMin ( this->PETCTFusionNode->GetPETMin() );
    this->PETCTFusionNode->SetColorRangeMax ( this->PETCTFusionNode->GetPETMax() );

    this->ColorRange->GetEntry1()->SetValueAsDouble ( this->PETCTFusionNode->GetColorRangeMin() );
    this->ColorRange->GetEntry2()->SetValueAsDouble ( this->PETCTFusionNode->GetColorRangeMax() );
    h->Delete();
    }
}




//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::InitializePETMinAndMax()
{
  if ( this->PETCTFusionNode ==  NULL )
    {
    return;
    }

  this->PETCTFusionNode->SetPETMin(0.0);
  this->PETCTFusionNode->SetPETMax(255.0);
}

