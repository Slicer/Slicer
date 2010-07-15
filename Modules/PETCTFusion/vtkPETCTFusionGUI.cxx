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
#include "vtkLookupTable.h"
#include "vtkColorTransferFunction.h"

#include "vtkKWCheckButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButtonSetWithLabel.h"
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
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWScale.h"

#include "vtkSlicerPopUpHelpWidget.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerWaitMessageWidget.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerSlicesControlGUI.h"

#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLPETProceduralColorNode.h"

//--- goodies for plotting
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLArrayPlotNode.h"
#include "vtkMRMLXYPlotManagerNode.h"
#include "vtkSlicerXYPlotWidget.h"

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
  this->VolumeRenderCheckbox = NULL;
  this->TissueConcentrationEntry = NULL;
  this->DoseUnitsMenuButton = NULL;
  this->TissueUnitsMenuButton = NULL;
  this->WeightUnitsMenuButton = NULL;
  this->InjectedDoseEntry = NULL;
  this->PatientWeightEntry = NULL;
  this->ComputeButton = NULL;
  this->GetFromDICOMButton = NULL;
  this->SaveButton = NULL;
  this->HelpButton = NULL;
  this->PETRange = NULL;
  this->CTRange = NULL;
  this->UpdatingLUT = 0;
  this->UpdatingGUI = 0;

  this->ColorSet = NULL;
  this->PatientWeightLabel = NULL;
  this->InjectedDoseLabel = NULL;
  this->PatientNameLabel = NULL;
  this->StudyDateLabel = NULL;
  this->ResultList = NULL;
  this->ResultListWithScrollbars = NULL;

  this->ClearPlotArrayButton = NULL;
  this->AddToPlotArrayButton = NULL;
  this->SavePlotArrayButton = NULL;
  this->ShowPlotButton = NULL;

}

//----------------------------------------------------------------------------
vtkPETCTFusionGUI::~vtkPETCTFusionGUI()
{
  if ( this->Logic )
    {
    this->SetLogic ( NULL );
    }
  if ( this->PETCTFusionNode )
    {
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
  if ( this->VolumeRenderCheckbox)
    {
    this->VolumeRenderCheckbox->SetParent ( NULL );
    this->VolumeRenderCheckbox->Delete();
    this->VolumeRenderCheckbox = NULL;
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
  if ( this->HelpButton )
    {
    this->HelpButton->SetParent ( NULL );
    this->HelpButton->Delete();
    this->HelpButton = NULL;
    }
  if ( this->PETRange )
    {
    this->PETRange->SetParent ( NULL );
    this->PETRange->Delete();
    this->PETRange = NULL;
    }
  if ( this->CTRange )
    {
    this->CTRange->SetParent ( NULL );
    this->CTRange->Delete();
    this->CTRange = NULL;
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
  if ( this->PatientNameLabel )
    {
    this->PatientNameLabel->SetParent ( NULL );
    this->PatientNameLabel->Delete();
    this->PatientNameLabel = NULL;
    }
  if ( this->StudyDateLabel )
    {
    this->StudyDateLabel->SetParent ( NULL );
    this->StudyDateLabel->Delete();
    this->StudyDateLabel = NULL;    
    }
  if ( this->ResultList )
    {
    this->ResultList = NULL;
    }
  if ( this->ResultListWithScrollbars )
    {
    this->ResultListWithScrollbars->SetParent ( NULL );
    this->ResultListWithScrollbars->Delete();
    this->ResultListWithScrollbars = NULL;
    }
  if ( this->ColorSet )
    {
    this->ColorSet->SetParent ( NULL );
    this->ColorSet->Delete();
    this->ColorSet = NULL;
    }
  if (this->ClearPlotArrayButton)
    {
    this->ClearPlotArrayButton->SetParent ( NULL );
    this->ClearPlotArrayButton->Delete();
    this->ClearPlotArrayButton = NULL;
    }
  if ( this->AddToPlotArrayButton )
    {
    this->AddToPlotArrayButton->SetParent ( NULL );
    this->AddToPlotArrayButton->Delete();
    this->AddToPlotArrayButton = NULL;
    }
  if ( this->ShowPlotButton )
    {
    this->ShowPlotButton->SetParent ( NULL );
    this->ShowPlotButton->Delete();
    this->ShowPlotButton = NULL;
    }
  if ( this->SavePlotArrayButton )
    {
    this->SavePlotArrayButton->SetParent ( NULL );
    this->SavePlotArrayButton->Delete();
    this->SavePlotArrayButton = NULL;    
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

  //---
  //--- If PET, CT, or Mask volumes have had 
  //--- display adjusted in the volumes
  //--- module, make sure this module
  //--- tracks those changes.
  //---
  this->UpdateFusionDisplayFromMRML();
  
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
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
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
  Superclass::PrintSelf(os, indent);
  //---
  //--- print widgets
  //---
  /*
  this->CTSelector
  this->PETSelector
  this->MaskSelector
  this->VolumeRenderCheckbox
  this->TissueConcentrationEntry
  this->InjectedDoseEntry
  this->DoseUnitsMenuButton
  this->TissueUnitsMenuButton
  this->WeightUnitsMenuButton
  this->PatientWeightEntry
  this->ComputeButton
  this->SaveButton
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
  if ( this->VolumeRenderCheckbox )
    {
    this->VolumeRenderCheckbox->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  if ( this->ColorSet )
    {
    this->ColorSet->GetWidget()->GetWidget(0)->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ColorSet->GetWidget()->GetWidget(1)->AddObserver (  vtkKWRadioButton::SelectedStateChangedEvent,(vtkCommand *)this->GUICallbackCommand );
    this->ColorSet->GetWidget()->GetWidget(2)->AddObserver (  vtkKWRadioButton::SelectedStateChangedEvent,(vtkCommand *)this->GUICallbackCommand );
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
  if ( this->VolumeRenderCheckbox )
    {
    this->VolumeRenderCheckbox->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  if ( this->ColorSet )
    {
    this->ColorSet->GetWidget()->GetWidget(0)->RemoveObserver ( (vtkCommand *)this->GUICallbackCommand );
    this->ColorSet->GetWidget()->GetWidget(1)->RemoveObserver ( (vtkCommand *)this->GUICallbackCommand );
    this->ColorSet->GetWidget()->GetWidget(2)->RemoveObserver ( (vtkCommand *)this->GUICallbackCommand );
    }


}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *vtkNotUsed(callData) )
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
        this->PETCTFusionNode->SetInputCTReference ( NULL );
        this->InitializeCTMinAndMax();
        this->UpdateCTRangeFromMRML();
        return;
        }
      //--- put CT volume in BG layer.
      this->ApplicationLogic->GetSelectionNode()->SetActiveVolumeID( v->GetID() );
      this->ApplicationLogic->PropagateVolumeSelection(0);
      this->PETCTFusionNode->SetInputCTReference ( v->GetID() );
      this->ApplyDefaultCTLUT();
      //--- update display of PET
      this->UpdateCTRangeFromMRML();
      this->UpdateCTDisplayFromMRML();
      }

    else if  ( s == this->PETSelector )
      {
      this->ClearResultsTable();
      this->ResetManualEntryGUI();
      this->ClearDICOMInformation();
      v = vtkMRMLVolumeNode::SafeDownCast (this->PETSelector->GetSelected() );
      if ( (v == NULL) || (v->GetID() == NULL) || (!strcmp(v->GetID(), "")) )
        {
        this->PETCTFusionNode->SetInputPETReference ( NULL );
        this->InitializePETMinAndMax();
        this->UpdatePETRangeFromMRML();        
        return;
        }
      //--- put PET volume in FG layer
      this->ApplicationLogic->GetSelectionNode()->SetSecondaryVolumeID( v->GetID() );
      this->ApplicationLogic->PropagateVolumeSelection(0);
      this->PETCTFusionNode->SetInputPETReference ( v->GetID() );
      this->ApplyDefaultPETLUT();
      this->InitializeGUI();
      //--- update display of PET
      this->UpdateDICOMPanel();
      //--- little error checking and user-feedback...
      double weight = this->PETCTFusionNode->GetPatientWeight();
      double dose = this->PETCTFusionNode->GetInjectedDose();
      if ( weight == 0.0 || dose == 0.0 || this->PETCTFusionNode->GetTissueRadioactivityUnits() == NULL )
        {
        this->PETCTFusionNode->SetMessageText ( "Not all parameters were found to compute SUVmax for the PET volume. The PET color range will operate in voxel units (not in SUV units) until SUV attributes are manually specified." );
        this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::ErrorEvent );
        }
      this->UpdatePETRangeFromMRML();
      this->UpdateColorRadioButtonsFromMRML();
      this->UpdatePETDisplayFromMRML();
      if ( appGUI->GetSlicesControlGUI() )
        {
        if ( appGUI->GetSlicesControlGUI()->GetSliceFadeScale())
          {
          appGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue ( 0.8 );
          }
        }
      }

    else if ( s == this->MaskSelector )
      {
      this->ClearResultsTable();

      v = vtkMRMLVolumeNode::SafeDownCast (this->MaskSelector->GetSelected() );
      if ( (v == NULL) || (v->GetID() == NULL) || (!strcmp(v->GetID(), "")) )
        {
        this->PETCTFusionNode->SetInputMask ( NULL );
        return;
        }
      //--- put label volume in LB layer.
      this->ApplicationLogic->GetSelectionNode()->SetActiveLabelVolumeID( v->GetID() );
      this->ApplicationLogic->PropagateVolumeSelection(0);
      this->PETCTFusionNode->SetInputMask(v->GetID());
      }
    }
  
  //---
  //--- checkboxes (CURRENTLY NOT EXPOSED.)
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
  if ( (rb != NULL) && ( this->ColorSet->GetWidget()->GetWidget(0) == rb ) &&
            (event == vtkKWRadioButton::SelectedStateChangedEvent ) )
    {
    //---
    //--- Change the PET volume's color if setting is new
    //---
    if ( this->PETCTFusionNode->GetPETLUT() )
      {
      vtkMRMLPETProceduralColorNode *cnode = vtkMRMLPETProceduralColorNode::SafeDownCast( this->MRMLScene->GetNodeByID( this->PETCTFusionNode->GetPETLUT() ));
      if ( cnode == NULL )
        {
          this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETMIP );
        }
      else 
        {
        if ( (cnode->GetType() != vtkMRMLPETProceduralColorNode::PETMIP) &&
             (this->ColorSet->GetWidget()->GetWidget(0)->GetSelectedState() == 1) )
          {
          this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETMIP );
          }
        }
      }
    else
      {
      this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETMIP );
      }
    }
  else if ( (rb != NULL) && ( this->ColorSet->GetWidget()->GetWidget(1) == rb ) &&
            (event == vtkKWRadioButton::SelectedStateChangedEvent ) )
    {
    //---
    //--- Change the PET volume's color if setting is new
    //---
    if ( this->PETCTFusionNode->GetPETLUT() )
      {
      vtkMRMLPETProceduralColorNode *cnode = vtkMRMLPETProceduralColorNode::SafeDownCast( this->MRMLScene->GetNodeByID( this->PETCTFusionNode->GetPETLUT() ));
      if ( cnode == NULL )
        {
          this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETheat );
        }
      else
        {
        if ( (cnode->GetType() != vtkMRMLPETProceduralColorNode::PETheat) &&
             (this->ColorSet->GetWidget()->GetWidget(1)->GetSelectedState() == 1) )
          {
          this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETheat );
          }
        }
      }
    else
      {
      this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETheat );
      }
    }
  else if ( (rb != NULL) && ( this->ColorSet->GetWidget()->GetWidget(2) == rb ) &&
            (event == vtkKWRadioButton::SelectedStateChangedEvent ) )
    {
    //---
    //--- Change the PET volume's color if setting is new
    //---
    if ( this->PETCTFusionNode->GetPETLUT() )
      {
      vtkMRMLPETProceduralColorNode *cnode = vtkMRMLPETProceduralColorNode::SafeDownCast( this->MRMLScene->GetNodeByID( this->PETCTFusionNode->GetPETLUT() ));
      if ( cnode == NULL )
        {
          this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETrainbow );
        }
      else
        {
        if ( (cnode->GetType() != vtkMRMLPETProceduralColorNode::PETrainbow) &&
             (this->ColorSet->GetWidget()->GetWidget(2)->GetSelectedState() == 1) )
          {
          this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETrainbow );
          }
        }
      }
    else
      {
      this->ColorizePETVolume( vtkMRMLPETProceduralColorNode::PETrainbow );
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

      this->ClearResultsTable();
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
      //TEST
      this->UpdatePETRangeFromMRML();
      this->UpdatePETDisplayFromMRML();
      }
    if ( this->TissueUnitsMenuButton != NULL && m == this->TissueUnitsMenuButton->GetMenu() )
      {
      ss.clear();
      ss = this->TissueUnitsMenuButton->GetValue();

      this->ClearResultsTable();
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
      //TEST
      this->UpdatePETRangeFromMRML();
      this->UpdatePETDisplayFromMRML();
      }
    if ( this->WeightUnitsMenuButton != NULL && m == this->WeightUnitsMenuButton->GetMenu())
      {
      ss.clear(); 
      ss = this->WeightUnitsMenuButton->GetValue();

      this->ClearResultsTable();
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
      //TEST
      this->UpdatePETRangeFromMRML();
      this->UpdatePETDisplayFromMRML();
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
      if ( this->InjectedDoseEntry->GetValueAsDouble() )
        {
        if ( this->InjectedDoseEntry->GetValueAsDouble() != this->PETCTFusionNode->GetInjectedDose() )
          {
          this->ClearResultsTable();
          this->ClearDICOMInformation();
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
      //TEST
      this->UpdatePETRangeFromMRML();
      this->UpdatePETDisplayFromMRML();
      }
    else if ( e == this->PatientWeightEntry )
      {
      if ( this->PatientWeightEntry->GetValueAsDouble() )
        {
        if ( this->PatientWeightEntry->GetValueAsDouble() != this->PETCTFusionNode->GetPatientWeight() )
          {
          this->ClearResultsTable();
          this->ClearDICOMInformation();
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
      //TEST
      this->UpdatePETRangeFromMRML();
      this->UpdatePETDisplayFromMRML();
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
        //--- empty report gui
        this->ClearResultsTable();

        //--- empty out node.
        this->PETCTFusionNode->LabelResults.clear();
        
        //--- compute stuff.
        this->Logic->ComputeSUV();
        }
      else
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
        dialog->SetStyleToMessage();
        dialog->SetText ( "To compute the SUV, a valid PET volume\n and a Mask volume (label map) must be selected, \nand the following SUV attributes must be specified: Patient Weight,\nInjected Dose, and\nTissue Radioactivity Units.\nAt least one of these is missing.\n\n(Hint: if the PET study is non-DICOM, SUV attributes may have to be entered manually in the Study Parameters panel.)" );
        dialog->Create();
        dialog->Invoke();
        dialog->Delete();
        return;
        }
      }
    if ( b == this->GetFromDICOMButton)
      {
      this->ResetManualEntryGUI();
      this->ClearResultsTable();
      this->ClearDICOMInformation();
      this->UpdateDICOMPanel();
      }
    else if ( b == this->SaveButton )
      {
      }
    }
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ColorizePETVolume(int type)
{
  //---
  //--- Make sure we have what we need
  //---
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "Got NULL MRMLScene" );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->PETCTFusionNode->GetInputPETReference() == NULL )
    {
    return;
    }
  int numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLPETProceduralColorNode" );
  if ( numnodes == 0 )
    {
    //--- no PET color nodes. can't use it to colorize pet data.
    vtkErrorMacro ( "Can't find any vtkMRMLPETProceduralColorNodes in scene. Not colorizing PET volume." );
    return;
    }
  
  //---
  //--- for the PET volume display node
  //---
  vtkMRMLScalarVolumeNode *svn;
  svn = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputPETReference() ));
  if ( svn == NULL )
    {
    return;
    }
  //---
  //--- for the PET volume's display node
  //---
  vtkMRMLVolumeDisplayNode *dnode;
  dnode = vtkMRMLVolumeDisplayNode::SafeDownCast (svn->GetDisplayNode());
  if ( dnode == NULL )
    {
    return;
    }
  //---
  //--- for the PET volume's color node
  //---
  vtkMRMLColorNode *cnode;
  cnode = vtkMRMLColorNode::SafeDownCast ( dnode->GetColorNode() );
  if ( cnode == NULL )
    {
    return;
    }

  //---
  //--- find the selected LUT by type.
  //--- and change color node if it doesn't
  //--- match the correct type or selection.
  //--- Apply the LUT to the PET volume.
  //---
  vtkMRMLPETProceduralColorNode *pn = vtkMRMLPETProceduralColorNode::SafeDownCast ( cnode );
  if ( pn==NULL || pn->GetType() !=type )
    {
    const char *id = this->GetPETColorNodeIDByType ( type );
    dnode->SetAndObserveColorNodeID (id );
    this->PETCTFusionNode->SetPETLUT(id);
    this->UpdatePETDisplayFromMRML();
    }
  return;
}





//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateColorRadioButtonsFromMRML ()
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
        vtkMRMLPETProceduralColorNode *cnode = vtkMRMLPETProceduralColorNode::SafeDownCast(dnode->GetColorNode());
        if ( cnode )
          {
          int type = cnode->GetType();
          if ((type == vtkMRMLPETProceduralColorNode::PETMIP) &&
              (this->ColorSet->GetWidget()->GetWidget(0)->GetSelectedState() != 1) )
            {
            this->ColorSet->GetWidget()->GetWidget(0)->SetSelectedState(1);
            }
          if ((type == vtkMRMLPETProceduralColorNode::PETheat) &&
              (this->ColorSet->GetWidget()->GetWidget(1)->GetSelectedState() != 1) )
            {
            this->ColorSet->GetWidget()->GetWidget(1)->SetSelectedState(1);
            }
          if ((type == vtkMRMLPETProceduralColorNode::PETrainbow) &&
              (this->ColorSet->GetWidget()->GetWidget(2)->GetSelectedState() != 1) )
            {
            this->ColorSet->GetWidget()->GetWidget(2)->SetSelectedState(1);
            }          
          }
        else
          {
          //--- vtkErrorMacro ( "PET Volume has unexpected color node." );
          return;
          }
        }
      }
    }
}





//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateFusionDisplayFromMRML()
{
  //---
  //--- If PET, CT, or Mask volumes have had 
  //--- display adjusted in the volumes
  //--- module, make sure this module
  //--- tracks those changes.
  //--- Method is called from method Enter()
  //---
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->PETSelector->GetSelected() != NULL )
    {
    //--- update PETCTNode (get vol's win lev and set node)
    vtkMRMLScalarVolumeNode *vnode = vtkMRMLScalarVolumeNode::SafeDownCast (this->PETSelector->GetSelected() );
    if ( vnode != NULL )
      {
      vtkMRMLScalarVolumeDisplayNode *dnode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast (vnode->GetDisplayNode());
      if ( dnode )
        {
        //--- get display node.
        double win = dnode->GetWindow( );
        double lev = dnode->GetLevel( ); //min + (0.5 * (max - min)) );
        //double ut = dnode->GetUpperThreshold();
        //double lt = dnode->GetLowerThreshold();
        //--- compute settings for node
        double lw = lev - (win/2.0);
        double uw = lev + (win/2.0);
        this->PETCTFusionNode->SetColorRangeMin (lw);
        this->PETCTFusionNode->SetColorRangeMax (uw);
        //this->PETCTFusionNode->SetPETMin ( lt );
        //this->PETCTFusionNode->SetPETMax ( lt );
        //--- if the LUT is not one of the normal PET luts,
        //--- then assume user has selected something
        //--- else, and uncheck all PET Color radiobuttons.
        //---HERE
        vtkMRMLPETProceduralColorNode *cnode =
          vtkMRMLPETProceduralColorNode::SafeDownCast(dnode->GetColorNode());
        if ( cnode == NULL )
          {
          this->ColorSet->GetWidget()->GetWidget(0)->SetSelectedState(0);
          this->ColorSet->GetWidget()->GetWidget(1)->SetSelectedState(0);
          this->ColorSet->GetWidget()->GetWidget(2)->SetSelectedState(0);
          }
        }
      }
    //--- update Display
    this->UpdatePETDisplayFromMRML();
    }
  if ( this->CTSelector->GetSelected() != NULL )
    {
    //--- update PETCTNode (get vol's win lev, set node)
    vtkMRMLScalarVolumeNode *vnode = vtkMRMLScalarVolumeNode::SafeDownCast ( this->CTSelector->GetSelected() );
    if ( vnode != NULL )
      {
      vtkMRMLScalarVolumeDisplayNode *dnode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast (vnode->GetDisplayNode());
      if ( dnode )
        {
        //--- get display node.
        double win = dnode->GetWindow( );
        double lev = dnode->GetLevel( ); //min + (0.5 * (max - min)) );
        //double ut = dnode->GetUpperThreshold();
        //double lt = dnode->GetLowerThreshold();
        //--- compute settings for node
        double lw = lev - (win/2.0);
        double uw = lev + (win/2.0);
        this->PETCTFusionNode->SetCTRangeMin (lw);
        this->PETCTFusionNode->SetCTRangeMax (uw);
        //this->PETCTFusionNode->SetCTMin ( lt );
        //this->PETCTFusionNode->SetCTMax ( lt );
        }
      }
    //--- update Display
      this->UpdateCTDisplayFromMRML();
    }
  if ( this->MaskSelector->GetSelected() != NULL )
    {
    //--- update PETCTNode
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
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::UpdatePETDisplayEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::UpdateCTDisplayEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::StartUpdatingDisplayEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::ComputeDoneEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::DICOMUpdateEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::WaitEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::NonDICOMEvent );
    events->InsertNextValue ( vtkMRMLPETCTFusionNode::StatusEvent );    
//    events->InsertNextValue ( vtkMRMLPETCTFusionNode::PlotReadyEvent );    
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

  if ( this->CTSelector )
    {
    //--- update the menu & selection
    this->CTSelector->UpdateMenu();
    const char *nid = this->PETCTFusionNode->GetInputCTReference();
    //--- check to see if the change is new
    if ( this->CTSelector->GetSelected() == NULL ||
         ( nid && strcmp(nid, this->CTSelector->GetSelected()->GetID())) )
      {
      //--- make GUI match MRML
      this->CTSelector->SetSelected ( this->MRMLScene->GetNodeByID(nid) );
      this->UpdateCTDisplayFromMRML();
      }
    }
    
  if ( this->PETSelector)
    {
    //--- update the menu & selection
    this->PETSelector->UpdateMenu();
    const char *nid = this->PETCTFusionNode->GetInputPETReference();
    //--- check to see if the change is new
    if (this->PETSelector->GetSelected() == NULL ||
        (nid && strcmp(nid, this->PETSelector->GetSelected()->GetID())) )
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
    if ( this->MaskSelector->GetSelected() == NULL ||
         ( nid && strcmp(nid, this->MaskSelector->GetSelected()->GetID())) )
      {
      //--- make GUI match MRML
      this->MaskSelector->SetSelected ( this->MRMLScene->GetNodeByID(nid) );
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
  
  if ( this->PETCTFusionNode->GetPatientName() != NULL )
    {
    if ( this->PatientNameLabel )
      {
      sstr.clear();
      sstr.str("");
      sstr <<this->PETCTFusionNode->GetPatientName();
      std::string tmpstr;
      const char *tmpc;
      tmpstr = sstr.str();
      tmpc = tmpstr.c_str();
      this->PatientNameLabel->SetText ( tmpc );
      sstr.clear();
      sstr.str("");
      if ( this->PETCTFusionNode->GetPatientName() )
        {
        sstr <<this->PETCTFusionNode->GetPatientName();
        tmpstr = sstr.str();
        tmpc = tmpstr.c_str();
        this->PatientNameLabel->SetText ( tmpc );
        }
      else
        {
        this->PatientNameLabel->SetText ("no value found" );
        }
      }
    }
  else
    {
    if ( this->PatientNameLabel )
      {
      this->PatientNameLabel->SetText ("no value found");
      }
    }

  if ( this->PETCTFusionNode->GetStudyDate() != NULL )
    {
    if ( this->StudyDateLabel )
      {
      sstr.clear();
      sstr.str("");
      if ( this->PETCTFusionNode->GetStudyDate() )
        {
        sstr <<this->PETCTFusionNode->GetStudyDate();
        std::string tmpstr;
        const char *tmpc;
        tmpstr = sstr.str();
        tmpc = tmpstr.c_str();
        this->StudyDateLabel->SetText ( tmpc );
        }
      else
        {
        this->StudyDateLabel->SetText ( "no value found");
        }
      }
    }
  else
    {
    if ( this->StudyDateLabel )
      {
        this->StudyDateLabel->SetText ( "no value found");
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
      if ( this->PETCTFusionNode->GetDoseRadioactivityUnits())
        {
        //sstr << " (" << this->PETCTFusionNode->GetDoseRadioactivityUnits() << ")";
        double mbqs = this->Logic->ConvertRadioactivityUnits ( this->PETCTFusionNode->GetInjectedDose(),
                                                  this->PETCTFusionNode->GetDoseRadioactivityUnits(), "MBq" );
        sstr << mbqs;
        sstr << " (MBq)";
        std::string tmpstr;
        const char *tmpc;
        tmpstr = sstr.str();
        tmpc = tmpstr.c_str();
        this->InjectedDoseLabel->SetText (tmpc );
        }
      else
        {
        this->InjectedDoseLabel->SetText ("");
        }
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
  this->UpdatingGUI = 0;
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *vtkNotUsed(callData) )
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
    else if ( event == vtkMRMLPETCTFusionNode::UpdatePETDisplayEvent )
      {
      //--- adjust color lut.
      if ( !this->UpdatingLUT )
        {
        this->UpdatePETDisplayFromMRML();
        return;
        }
      }
    else if ( event == vtkMRMLPETCTFusionNode::UpdateCTDisplayEvent )
      {
      //--- adjust color lut.
      if ( !this->UpdatingLUT )
        {
        this->UpdateCTDisplayFromMRML();
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
        dialog->SetText ( "The PET Volume appears not to be a DICOM file.\nSince parameters cannot be extracted automatically, SUV attributes must be entered manually.\n\nThe use of DICOM studies is recommended for easiest use of this module." );
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
        if ( this->GetApplicationGUI () && this->GetApplicationGUI()->GetMainSlicerWindow() )
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
          dialog->SetStyleToMessage();
          dialog->SetText ( this->PETCTFusionNode->GetMessageText() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
          return;
          }
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
      this->UpdateResultsTableFromMRML();
      }
    else if ( event == vtkMRMLPETCTFusionNode::PlotReadyEvent )
      {
      this->RaisePlot ();
      }
    return;    
    }
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateResultsTableFromMRML()
{
  if ( this->ResultList == NULL )
    {
    return;
    }
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "Got NULL MRMLScene." );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "Got NULL PETCTFusionNode." );
    return;
    }



  vtkMRMLPETCTFusionNode* n = this->GetPETCTFusionNode();
  if(!n->LabelResults.empty()) 
    { 

    typedef std::list<vtkMRMLPETCTFusionNode::SUVEntry>::const_iterator LI;

    int i = 0;
    for (LI li = n->LabelResults.begin(); li != n->LabelResults.end(); ++li)
      {
      const vtkMRMLPETCTFusionNode::SUVEntry& label = *li;  
      this->ResultList->InsertCellTextAsInt(i, 0, label.Label);
      this->ResultList->InsertCellTextAsDouble(i, 2, label.Max);
      this->ResultList->InsertCellTextAsDouble(i, 3, label.Mean);
      int entry = this->ResultList->GetCellTextAsInt ( i, 0 );
      
      //---
      //--- now fill the color cell background with color
      //--- first, find the number of colors and the color range.
      //--- which will depend on what kind of color node is selected.
      bool isFSProcedural = false;
      bool isProcedural = false;
      int numColors = 0;
      double *range = NULL;

      if ( this->PETCTFusionNode->GetInputMask()==NULL )
        {
        return;
        }
      vtkMRMLVolumeNode *vn = vtkMRMLVolumeNode::SafeDownCast (
                                                               this->MRMLScene->GetNodeByID(
                                                                                            this->PETCTFusionNode->GetInputMask() ));
      if ( vn == NULL )
        {
        return;
        }
      vtkMRMLDisplayNode *dn = vn->GetDisplayNode();
      if ( dn == NULL )
        {
        return;
        }
      vtkMRMLColorNode *colorNode = dn->GetColorNode();
      
      if ( colorNode == NULL )
        {
        return;
        }

      if ( vtkMRMLColorTableNode::SafeDownCast (colorNode) != NULL )
        {
        //---
        //--- color table node.
        //---
        numColors = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetNumberOfColors();
        range = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetLookupTable()->GetRange();
        }
      else if ( vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode) != NULL &&
                vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable() != NULL )
        {
        //---
        //--- freesurfer procedural node
        //---
        isFSProcedural = true;
        range = vtkMRMLFreeSurferProceduralColorNode::SafeDownCast (colorNode)->GetLookupTable()->GetRange();
        if ( range )
          {
          numColors = (int)floor(range[1]-range[0]);
          if ( range[0] < 0 && range[1] >= 0)
            {
            numColors++;
            }
          }
        }
      else if ( vtkMRMLProceduralColorNode::SafeDownCast(colorNode) != NULL &&
                vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction() != NULL )
        {
        //---
        //--- procedural node
        //---
        isProcedural = true;
        range = vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction()->GetRange();
        if ( range )
          {
          numColors = (int)floor(range[1]-range[0]);
          if ( range[0] < 0 && range[1] >=0)
            {
            numColors++;
            }
          }
        }
      
        //---
        double color[3];
        if (isFSProcedural)
          {
          if ( entry <= range[1] )
            {
            vtkMRMLFreeSurferProceduralColorNode::SafeDownCast ( colorNode)->GetLookupTable()->GetColor(entry, color);
            this->ResultList->SetCellBackgroundColor(i, 1, color);
            this->ResultList->SetCellSelectionBackgroundColor( i, 1, color);
            }
          }
        else if ( isProcedural)
          {
          vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction()->GetColor(entry, color);
          this->ResultList->SetCellBackgroundColor(i, 1, color);
          this->ResultList->SetCellSelectionBackgroundColor( i, 1, color);
          }
        else if ( colorNode->GetLookupTable() != NULL )
          {
          colorNode->GetLookupTable()->GetColor((double)entry, color);
          this->ResultList->SetCellBackgroundColor(i, 1, color);
          this->ResultList->SetCellSelectionBackgroundColor( i, 1, color);
          }
        i++;
      }
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ClearResultsTable()
{
  if ( this->ResultList != NULL )
    {
    this->ResultList->DeleteAllRows();
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::RaisePlot()
{

/*
  // TODO: this is code in progress.
 // plot the node
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ( "Got NULL Logic");
    return;
    }
  if ( this->Logic->GetPlots() == NULL )
    {
    vtkErrorMacro ( "Got NULL Plots");
    return;
    }
 if ( this->Logic->GetPlots()->GetLabelList() == NULL )
    {
    vtkErrorMacro ( "Got NULL LabelList");
    return;
    }

  vtkIntArray *a = this->Logic->GetPlots()->GetLabelList();
  int size = a->GetNumberOfTuples();
  
  ///--- get the each node from the logic
  ///--- and plot it in correct color.
  int label;
  for (int i = 0; i < size; i++ )
    {
    label = a->GetValue(static_cast<vtkIdType>(i));
    vtkMRMLDoubleArrayNode *node = this->Logic->GetPlots()->GetPlotData(label);
    if (node != NULL && node->IsA("vtkMRMLDoubleArrayNode"))
      {
        vtkMRMLDoubleArrayNode *dnode = vtkMRMLDoubleArrayNode::SafeDownCast(node);

        vtkMRMLArrayPlotNode *plot = vtkMRMLArrayPlotNode::New();
        this->GetMRMLScene()->AddNode(plot);
        plot->SetAndObserveArray(dnode);
        plot->SetColor(1, 0, 0);
        
        
        vtkMRMLXYPlotManagerNode *manager = vtkMRMLXYPlotManagerNode::New();
        this->GetMRMLScene()->AddNode(manager);
        manager->AddPlotNode(plot);
//        vtkMRMLDoubleArrayNode::LabelsVectorType labels = dnode->GetLabels();
//        manager->SetXLabel(labels.at(0).c_str());
//        manager->SetYLabel(labels.at(1).c_str());
//        manager->SetErrorBarAll(1);
        manager->SetBackgroundColor(0.8,0.8,1);
        
        vtkKWTopLevel *top = vtkKWTopLevel::New();
        top->SetApplication(this->GetApplication());
        top->Create();
        
        vtkSlicerXYPlotWidget *widget = vtkSlicerXYPlotWidget::New();
        widget->SetParent(top);
        widget->SetAndObservePlotManagerNode(manager);
        widget->Create();
        
        this->Script ( "pack %s -fill both -expand true",
                   widget->GetWidgetName() );

        widget->UpdateGraph();
       
        top->SetSize(400, 200);
        top->Display();

        plot->Delete();
        manager->Delete();
        // top->Delete(); do I need to keep this around to have it display?
        // what about the widget? when do I delete that? Or will the toplevel delete it?
      }
    }
*/
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
  this->MaskSelector->SetLabelText( "VOI Mask: ");
  this->MaskSelector->GetLabel()->SetWidth ( 12 );
  this->MaskSelector->UpdateMenu();
  this->MaskSelector->SetBalloonHelpString("Select a segmentation of tumor(s)--only required for analysis.");
  this->Script ( "pack %s %s %s -side top -anchor ne -fill x -expand y -padx 2 -pady 2",
                 this->CTSelector->GetWidgetName(),
                 this->PETSelector->GetWidgetName(),
                 this->MaskSelector->GetWidgetName());


  //--- AUTOMATIC parameterization from DICOM
  vtkKWFrameWithLabel *cfCase = vtkKWFrameWithLabel::New();
  cfCase->SetParent ( dataFusionFrame->GetFrame() );
  cfCase->Create();
  cfCase->SetLabelText ("Study Information" );
  cfCase->ExpandFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", cfCase->GetWidgetName());

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent  ( cfCase->GetFrame() );
  l1->Create();
  l1->SetText ( "Patient Name:" );
  this->PatientNameLabel = vtkKWLabel::New();
  this->PatientNameLabel->SetParent  ( cfCase->GetFrame() );
  this->PatientNameLabel->Create();
  this->PatientNameLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->PatientNameLabel->SetText ( "" );  
  this->PatientNameLabel->SetAnchorToWest();

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent  ( cfCase->GetFrame() );
  l2->Create();
  l2->SetText ( "Study Date:" );
  this->StudyDateLabel = vtkKWLabel::New();
  this->StudyDateLabel->SetParent  ( cfCase->GetFrame() );
  this->StudyDateLabel->Create();
  this->StudyDateLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->StudyDateLabel->SetText ( "" );  
  this->StudyDateLabel->SetAnchorToWest();

  //--- grid
  app->Script ( "grid columnconfigure %s 0 -weight 0", cfCase->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", cfCase->GetFrame()->GetWidgetName() );
  app->Script("grid %s -row 0 -column 0 -sticky e  -padx 2 -pady 2", l1->GetWidgetName() );
  app->Script("grid %s -row 0 -column 1 -sticky w -padx 4 -pady 2", this->PatientNameLabel->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l2->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky w -padx 4 -pady 2", this->StudyDateLabel->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  cfCase->Delete();
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
  displayFrame->ExpandFrame();
  app->Script("pack %s -side top -expand y -fill x -padx 2 -pady 2", displayFrame->GetWidgetName());

  //--- colorizing options
  this->ColorSet = vtkKWRadioButtonSetWithLabel::New();
  this->ColorSet->SetParent ( displayFrame->GetFrame() );
  this->ColorSet->Create();
  this->ColorSet->SetLabelText ( "  PET Color:  ");
  this->ColorSet->GetWidget()->PackHorizontallyOn();
  this->ColorSet->GetWidget()->SetMaximumNumberOfWidgetsInPackingDirection ( 3 );
  this->ColorSet->GetWidget()->UniformColumnsOn();
  this->ColorSet->GetWidget()->UniformRowsOn();
  this->ColorSet->GetWidget()->AddWidget(0);
  this->ColorSet->GetWidget()->GetWidget(0)->SetText ( "Grey" );
  this->ColorSet->GetWidget()->AddWidget(1);
  this->ColorSet->GetWidget()->GetWidget(1)->SetText ( "Heat" );
  this->ColorSet->GetWidget()->AddWidget(2);
  this->ColorSet->GetWidget()->GetWidget(2)->SetText ( "Spectrum" );
  this->ColorSet->GetWidget()->GetWidget(1)->SetSelectedState ( 1 );
  app->Script ( "pack %s  -side top -anchor ne -fill x -expand y -padx 2 -pady 1",
                this->ColorSet->GetWidgetName() );

  //---
  //--- Adjust PET display
  //---
  vtkKWFrameWithLabel *f1 = vtkKWFrameWithLabel::New();
  f1->SetParent ( displayFrame->GetFrame() );
  f1->Create();
  f1->SetLabelText ( "Window/Level" );
  f1->ExpandFrame();
  this->Script ( "pack %s  -side top -anchor w -fill x -expand y -padx 2 -pady 0",
                 f1->GetWidgetName() );
  //--- set range
  vtkKWLabel *minl  = vtkKWLabel::New();
  minl->SetParent ( f1->GetFrame() );
  minl->Create();
  minl->SetText ( "PET:   min");

  vtkKWLabel *maxl  = vtkKWLabel::New();
  maxl->SetParent ( f1->GetFrame() );
  maxl->Create();
  maxl->SetText ( "max (SUVbw)");

  this->PETRange = vtkKWRange::New();
  this->PETRange->SetParent ( f1->GetFrame() );
  this->PETRange->Create();
  this->PETRange->SetEntry1PositionToTop();
  this->PETRange->SetEntry2PositionToTop();
  this->PETRange->SymmetricalInteractionOff();
  this->PETRange->SetOrientationToHorizontal();
  this->PETRange->SetCommand ( this, "ProcessPETRangeCommand" );
  this->PETRange->SetStartCommand ( this, "ProcessPETRangeStartCommand" );
  this->PETRange->SetEndCommand ( this, "ProcessPETRangeStopCommand" );
  this->PETRange->SetEntriesCommand ( this, "ProcessPETRangeCommand" );
  this->PETRange->SetWholeRange ( 0, 255 );
  this->PETRange->SetRange ( 0, 255 );
  
  //---
  //--- Adjust CT display
  //---
  vtkKWLabel *minl2  = vtkKWLabel::New();
  minl2->SetParent ( f1->GetFrame() );
  minl2->Create();
  minl2->SetText ( "CT:   min");

  vtkKWLabel *maxl2  = vtkKWLabel::New();
  maxl2->SetParent ( f1->GetFrame() );
  maxl2->Create();
  maxl2->SetText ( "max          ");

  this->CTRange = vtkKWRange::New();
  this->CTRange->SetParent ( f1->GetFrame() );
  this->CTRange->Create();
  this->CTRange->SetEntry1PositionToTop();
  this->CTRange->SetEntry2PositionToTop();
  this->CTRange->SymmetricalInteractionOff();
  this->CTRange->SetOrientationToHorizontal();
  this->CTRange->SetCommand ( this, "ProcessCTRangeCommand" );
  this->CTRange->SetStartCommand ( this, "ProcessCTRangeStartCommand" );
  this->CTRange->SetEndCommand ( this, "ProcessCTRangeStopCommand" );
  this->CTRange->SetEntriesCommand ( this, "ProcessCTRangeCommand" );
  this->CTRange->SetWholeRange ( 0, 255 );
  this->CTRange->SetRange ( 0, 255 );

  app->Script ( "grid columnconfigure %s 0 -weight 0", f1->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f1->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 2 -weight 0", f1->GetFrame()->GetWidgetName() );
  app->Script("grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", minl->GetWidgetName() );
  app->Script("grid %s -row 0 -column 1 -sticky new -padx 2 -pady 2", this->PETRange->GetWidgetName() );
  app->Script("grid %s -row 0 -column 2 -sticky w -padx 2 -pady 2", maxl->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", minl2->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky new -padx 2 -pady 2", this->CTRange->GetWidgetName() );
  app->Script("grid %s -row 1 -column 2 -sticky w -padx 2 -pady 2", maxl2->GetWidgetName() );

  //--- Turn on/off volume rendering
  this->VolumeRenderCheckbox = vtkKWCheckButton::New();
  this->VolumeRenderCheckbox->SetParent ( displayFrame->GetFrame() );
  this->VolumeRenderCheckbox->Create();
  this->VolumeRenderCheckbox->SelectedStateOff();
  this->VolumeRenderCheckbox->SetText ( "Volume Rendering:");
  this->VolumeRenderCheckbox->SetStateToDisabled();
//  this->Script ( "pack %s  -side top -anchor ne -fill x -expand y -padx 2 -pady 2",
//                 this->VolumeRenderCheckbox->GetWidgetName() );

  minl->Delete();
  maxl->Delete();
  minl2->Delete();
  maxl2->Delete();
  f1->Delete();
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
  cfAuto->SetLabelText ("Refresh SUV Attributes from DICOM" );
  cfAuto->CollapseFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", cfAuto->GetWidgetName());
  
  //--- MANUAL override.
  vtkKWFrameWithLabel *cfManual = vtkKWFrameWithLabel::New();
  cfManual->SetParent ( analyzeFrame->GetFrame() );
  cfManual->Create();
  cfManual->SetLabelText ("Set SUV Attributes Manually" );
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

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent  ( cfAuto->GetFrame() );
  l1->Create();
  l1->SetText ( "   Patient's Weight:" );
  this->PatientWeightLabel = vtkKWLabel::New();
  this->PatientWeightLabel->SetParent  ( cfAuto->GetFrame() );
  this->PatientWeightLabel->Create();
  this->PatientWeightLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->PatientWeightLabel->SetText ( "" );  
  this->PatientWeightLabel->SetAnchorToWest();

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent  ( cfAuto->GetFrame() );
  l2->Create();
  l2->SetText ( "   Injected Dose:" );
  this->InjectedDoseLabel = vtkKWLabel::New();
  this->InjectedDoseLabel->SetParent  ( cfAuto->GetFrame() );
  this->InjectedDoseLabel->Create();
  this->InjectedDoseLabel->SetForegroundColor ( 0.0, 0.2, 0.6 );
  this->InjectedDoseLabel->SetText ( "" );  
  this->InjectedDoseLabel->SetAnchorToWest();

  //--- grid
  app->Script ( "grid columnconfigure %s 0 -weight 0", cfAuto->GetFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", cfAuto->GetFrame()->GetWidgetName() );
  app->Script("grid %s -row 0 -column 0 -sticky e  -padx 2 -pady 2", l1->GetWidgetName() );
  app->Script("grid %s -row 0 -column 1 -sticky w -padx 4 -pady 2", this->PatientWeightLabel->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l2->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky w -padx 4 -pady 2", this->InjectedDoseLabel->GetWidgetName() );
  app->Script("grid %s -row 2 -column 0 -sticky ew -padx 2 -pady 2", this->GetFromDICOMButton->GetWidgetName() );  

  //---
  //--- Populate MANUAL frame
  //---

  //--- injected dose labels, entry, unit menu
  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( cfManual->GetFrame() );
  l3->Create();
  l3->SetText ( "Injected Dose:" );
  this->InjectedDoseEntry = vtkKWEntry::New();
  this->InjectedDoseEntry->SetParent ( cfManual->GetFrame() );
  this->InjectedDoseEntry->Create();
  this->InjectedDoseEntry->SetWidth ( 11 );
  vtkKWLabel *l4 = vtkKWLabel::New();
  l4->SetParent ( cfManual->GetFrame() );
  l4->Create();
  l4->SetText ( "Units:" );
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
  vtkKWLabel *l7 = vtkKWLabel::New();
  l7->SetParent ( cfManual->GetFrame() );
  l7->Create();
  l7->SetText ( "Tissue Radioactivity Units:" );
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
  vtkKWLabel *l5 = vtkKWLabel::New();
  l5->SetParent ( cfManual->GetFrame() );
  l5->Create();
  l5->SetText ( "Patient Weight:" );
  this->PatientWeightEntry = vtkKWEntry::New();
  this->PatientWeightEntry->SetParent ( cfManual->GetFrame() );
  this->PatientWeightEntry->Create();
  this->PatientWeightEntry->SetWidth ( 11 );
  vtkKWLabel *l6 = vtkKWLabel::New();
  l6->SetParent ( cfManual->GetFrame() );
  l6->Create();
  l6->SetText ( "Units:" );
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

  app->Script("grid %s -row 0 -column 0 -columnspan 3 -sticky e -padx 2 -pady 2", l7->GetWidgetName() );
  app->Script("grid %s -row 0 -column 3 -sticky e -padx 2 -pady 2", this->TissueUnitsMenuButton->GetWidgetName() );
  app->Script("grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l3->GetWidgetName() );
  app->Script("grid %s -row 1 -column 1 -sticky ew -padx 2 -pady 2", this->InjectedDoseEntry->GetWidgetName() );
  app->Script("grid %s -row 1 -column 2 -sticky e -padx 2 -pady 2", l4->GetWidgetName() );
  app->Script("grid %s -row 1 -column 3 -sticky ew -padx 2 -pady 2", this->DoseUnitsMenuButton->GetWidgetName() );
  app->Script("grid %s -row 2 -column 0 -sticky e -padx 2 -pady 2", l5->GetWidgetName() );
  app->Script("grid %s -row 2 -column 1 -sticky ew -padx 2 -pady 2", this->PatientWeightEntry->GetWidgetName() );
  app->Script("grid %s -row 2 -column 2 -sticky e -padx 2 -pady 2", l6->GetWidgetName() );
  app->Script("grid %s -row 2 -column 3 -sticky ew -padx 2 -pady 2", this->WeightUnitsMenuButton->GetWidgetName() );

  //--- clean up
  l1->Delete();
  l2->Delete();
  l3->Delete();
  l4->Delete();
  l5->Delete();
  l6->Delete();
  l7->Delete();
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
  outputFrame->SetLabelText("Quantitative Measures");
  outputFrame->ExpandFrame();
  app->Script("pack %s -side top -fill x -expand y -padx 2 -pady 2", outputFrame->GetWidgetName());
  
  
  //--- mc list with results per label.
  this->ResultListWithScrollbars = vtkKWMultiColumnListWithScrollbars::New();
  this->ResultListWithScrollbars->SetParent ( outputFrame->GetFrame() );
  this->ResultListWithScrollbars->Create();
  this->ResultList = this->ResultListWithScrollbars->GetWidget();
  this->ResultList->SetHeight(3);
  this->ResultList->SetWidth(4);
  int col_index;

  // Add the columns (make some of them editable)
  col_index = this->ResultList->AddColumn("Label");
  this->ResultList->ColumnEditableOff(col_index);

  // Add the columns (make some of them editable)
  col_index = this->ResultList->AddColumn("Color");
  this->ResultList->ColumnEditableOff(col_index);

  col_index = this->ResultList->AddColumn("Max SUVbw (g/ml)");
  this->ResultList->ColumnEditableOff(col_index);
  
  col_index = this->ResultList->AddColumn("Mean SUVbw (g/ml)");
  this->ResultList->ColumnEditableOff(col_index);
  app->Script(
    "pack %s -side top -anchor w -fill x -expand y -padx 2 -pady 2", 
    this->ResultListWithScrollbars->GetWidgetName());

  //--- get from dicom button
  this->ComputeButton = vtkKWPushButton::New();
  this->ComputeButton->SetParent ( outputFrame->GetFrame() );
  this->ComputeButton->Create();
  this->ComputeButton->SetReliefToGroove();
  this->ComputeButton->SetText ( "Compute / Refresh" );

  app->Script(
    "pack %s -side top -anchor w  -padx 2 -pady 2", 
    this->ComputeButton->GetWidgetName());

  
  // TO DO: finish and expose in the GUI.
  //--- button to add suvmax and suvmean to plot
  this->AddToPlotArrayButton = vtkKWPushButton::New();
  //--- button to display plot.
  this->ShowPlotButton = vtkKWPushButton::New();
  //--- button to save plot arrays to a csv file
  this->SavePlotArrayButton = vtkKWPushButton::New();
  //--- button to clear plot arrays.
  this->ClearPlotArrayButton = vtkKWPushButton::New();


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
  
  if ( this->PatientNameLabel )
    {
    this->PatientNameLabel->SetText ("");
    }
  if ( this->StudyDateLabel )
    {
    this->StudyDateLabel->SetText ( "" );
    }
  if ( this->PatientWeightLabel )
    {
    this->PatientWeightLabel->SetText ( "" );
    }
  if ( this->InjectedDoseLabel )
    {
    this->InjectedDoseLabel->SetText ( "" );
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
  const char* help_text = "For detailed documentation on this module, please see: <a>http://www.slicer.org/slicerWiki/index.php/Modules:PETCTFusion-Documentation-3.6</a>.\n\nThe **PETCTFusion Module** displays PET/CT data and performs some PET quantitative measurements (Standardized Uptake Value).\n\nThe PET quantifier \"Standardized Uptake Value\", SUV, (or \"Dose Uptake Ratio\", DUR) is calculated as a ratio of tissue radioactivity concentration CPET(t)  at time t and in units (kBq/ml), and injected dose in units (MBq) at the time of injection divided by body weight (kg).\n\nForumula implemented: SUVbw = CPET(t) / (Injected dose / Patient's weight)\n\nThe input units are converted to those above, and the resulting SUV values are in units (g/ml).\n\nThe Data Fusion panel allows the CT volume, PET volume and a tumor mask to be selected. It is recommended that the PET volume be a DICOM study so that SUV attributes can be extracted automatically from the DICOM header. The tumor mask may contain multiple tumor labels; each label ID/color will receive a separate SUVmax and SUVmean computation.\n\nThe Display panel allows the selection of different colorizing treatments for the PET volume, and Window/Level adjustments for the PET and CT volumes. The PET color range is displayed in SUV units, from 0 to the volume's SUVmax.\n\nThe Quantitative Measures panel provides a button for the computation of SUVmax and SUVmean for each label in the VOI Mask. The Study Parameters panel allows review of SUV attributes extracted from the DICOM header (and used in the SUV computations) and a panel for setting attributes manually if the PET Volume is a non-DICOM study.\n\n";
  const char* ack_text = "PETCTFusion was developed by Wendy Plesniak with help from Jeffrey Yapp and Ron Kikinis. This work was supported by NA-MIC, NAC, BIRN, NCIGT, Harvard CTSC, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\n";

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
    //--- Output Frame
    this->BuildReportFrame ( page );

    //
    //--- Analyze Frame
    this->BuildAnalysisFrame ( page );


    this->InitializeCTMinAndMax();
    this->InitializePETMinAndMax();
    this->InitializeGUI();
    this->Built = true;
}

//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::Init ( )
{
}


//---------------------------------------------------------------------------
 void vtkPETCTFusionGUI::ProcessPETRangeCommand ( double vtkNotUsed(min), double vtkNotUsed(max))
{
  if ( this->UpdatingLUT )
    {
    this->UpdateNodePETColorRange();
    }
}


//---------------------------------------------------------------------------
 void vtkPETCTFusionGUI::ProcessPETRangeStartCommand(double vtkNotUsed(min),
                                                     double vtkNotUsed(max))
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->PETRange == NULL )
    {
    return;
    }

  //--- Turn on UpdatingLUT
  this->UpdatingLUT = 1;

  double mmin = this->Logic->ConvertSUVUnitsToImageUnits(this->PETRange->GetEntry1()->GetValueAsDouble());
  double mmax = this->Logic->ConvertSUVUnitsToImageUnits(this->PETRange->GetEntry2()->GetValueAsDouble());
  double nodeMin = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMin() );
  double nodeMax = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMax() );

  //--- make sure node captures GUI
  //--- changes during interaction.
  if ( this->PETRange->GetEntry1()->GetValueAsDouble() != nodeMin )
    {
    this->PETCTFusionNode->SetColorRangeMin (mmin);
    }
  if ( this->PETRange->GetEntry2()->GetValueAsDouble() != nodeMax )
    {
    this->PETCTFusionNode->SetColorRangeMax ( mmax );
    }
}



//--------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessPETRangeStopCommand(double vtkNotUsed(min), double vtkNotUsed(max))
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->PETRange == NULL )
    {
    return;
    }

  double mmin = this->Logic->ConvertSUVUnitsToImageUnits(this->PETRange->GetEntry1()->GetValueAsDouble());
  double mmax = this->Logic->ConvertSUVUnitsToImageUnits(this->PETRange->GetEntry2()->GetValueAsDouble());

  //--- make sure node captures GUI
  //--- at interaction end.
  this->PETCTFusionNode->SetColorRange ( mmin, mmax );

  //--- Turn off UpdatingLUT
  this->UpdatingLUT = 0;
}




//---------------------------------------------------------------------------
 void vtkPETCTFusionGUI::ProcessCTRangeCommand(double vtkNotUsed(min), double vtkNotUsed(max))
{
  this->UpdateNodeCTColorRange();
}


//---------------------------------------------------------------------------
 void vtkPETCTFusionGUI::ProcessCTRangeStartCommand(double vtkNotUsed(min), double vtkNotUsed(max))
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->CTRange == NULL )
    {
    return;
    }

  //--- Turn on UpdatingLUT
  this->UpdatingLUT = 1;

  //--- make sure node and gui are in synch.
  if ( this->CTRange->GetEntry1()->GetValueAsDouble() !=
       this->PETCTFusionNode->GetCTRangeMin() )
    {
    this->PETCTFusionNode->SetCTRangeMin ( this->CTRange->GetEntry1()->GetValueAsDouble() );
    }
  
  if ( this->CTRange->GetEntry2()->GetValueAsDouble() !=
       this->PETCTFusionNode->GetCTRangeMax() )
    {
    this->PETCTFusionNode->SetCTRangeMax ( this->CTRange->GetEntry2()->GetValueAsDouble() );
    }
}



//--------------------------------------------------------------------------
void vtkPETCTFusionGUI::ProcessCTRangeStopCommand(double vtkNotUsed(min), double vtkNotUsed(max))
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->CTRange == NULL )
    {
    return;
    }
  //--- make sure node and gui are in synch.
  this->PETCTFusionNode->SetCTRange (this->CTRange->GetEntry1()->GetValueAsDouble(),
                                        this->CTRange->GetEntry2()->GetValueAsDouble() );

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
  if ( this->PatientNameLabel )
    {
    this->PatientNameLabel->SetText ("");
    }
  if ( this->StudyDateLabel )
    {
    this->StudyDateLabel->SetText ("");
    }
  if ( this->PatientWeightLabel )
    {
    this->PatientWeightLabel->SetText ("");
    }
  if ( this->InjectedDoseLabel )
    {
    this->InjectedDoseLabel->SetText ( "");
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
          if ( this->Logic->GetParametersFromDICOMHeader(path.c_str()) == 1)
            {
            found = 1;
            }
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
void vtkPETCTFusionGUI::UpdateCTDisplayFromMRML ()
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->CTRange == NULL )
    {
    return;
    }
  this->CTRange->SetWholeRange ( this->PETCTFusionNode->GetCTMin(), this->PETCTFusionNode->GetCTMax());
  //---
  //--- Method gets called if CTSelector changes
  //--- Scales current LUT to span the desired range of values
  //--- as specified in state.
  //---
  double tmpMin, tmpMax;
  tmpMin = this->CTRange->GetEntry1()->GetValueAsDouble();
  tmpMax = this->CTRange->GetEntry2()->GetValueAsDouble();
  
  if ( (tmpMin != this->PETCTFusionNode->GetCTRangeMin()) ||
       tmpMax != this->PETCTFusionNode->GetCTRangeMax() )
    {
    this->CTRange->GetEntry1()->SetValueAsDouble ( this->PETCTFusionNode->GetCTRangeMin() );
    this->CTRange->GetEntry2()->SetValueAsDouble ( this->PETCTFusionNode->GetCTRangeMax() );
    this->CTRange->SetWholeRange(this->PETCTFusionNode->GetCTMin(), this->PETCTFusionNode->GetCTMax());
    this->CTRange->SetRange(this->PETCTFusionNode->GetCTRangeMin(), this->PETCTFusionNode->GetCTRangeMax());
    this->ScaleCTColormap( this->PETCTFusionNode->GetCTRangeMin(),
                         this->PETCTFusionNode->GetCTRangeMax());
    }
}






//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdatePETDisplayFromMRML ()
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->Logic == NULL )
    {
    return;
    }
  if ( this->PETRange == NULL )
    {
    return;
    }

  //---
  //--- Method gets called if PETSelector changes
  //--- Scales current LUT to span the desired range of values
  //--- as specified in state.
  //---
  double tmpMin, tmpMax;
  tmpMin = this->PETRange->GetEntry1()->GetValueAsDouble();
  tmpMax = this->PETRange->GetEntry2()->GetValueAsDouble();
  
  double nodeColorMin = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMin());
  double nodeColorMax = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMax() );
  double nodePETMin = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetPETMin());
  double nodePETMax = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetPETMax() );

  if ( (tmpMin != nodeColorMin) || (tmpMax != nodeColorMax) )
    {
    this->PETRange->GetEntry1()->SetValueAsDouble ( nodeColorMin );
    this->PETRange->GetEntry2()->SetValueAsDouble ( nodeColorMax );
    this->PETRange->SetWholeRange( nodePETMin, nodePETMax );
    this->PETRange->SetRange( nodeColorMin, nodeColorMax );
    this->ScalePETColormap( this->PETCTFusionNode->GetColorRangeMin(),
                         this->PETCTFusionNode->GetColorRangeMax());
    }
}





//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateNodePETColorRange ()
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->Logic == NULL )
    {
    return;
    }
  if ( this->PETRange == NULL )
    {
    return;
    }

  //---
  //--- Method gets called if LUT for PET volume changes
  //--- or if Window/Level are adjusted.
  //--- Scales current LUT to span the desired range of values
  //--- as specified by GUI interaction
  //---
  double tmpMin, tmpMax;

  tmpMin = this->PETRange->GetEntry1()->GetValueAsDouble();
  tmpMax = this->PETRange->GetEntry2()->GetValueAsDouble();
  double nodeColorMin = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMin());
  double nodeColorMax = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMax() );
  
  if ( (tmpMin != nodeColorMin)  ||   (tmpMax != nodeColorMax) )
    {
    this->PETCTFusionNode->SetColorRange (
                                          this->Logic->ConvertSUVUnitsToImageUnits(tmpMin),
                                          this->Logic->ConvertSUVUnitsToImageUnits(tmpMax) );
    this->ScalePETColormap ( this->PETCTFusionNode->GetColorRangeMin(),
                          this->PETCTFusionNode->GetColorRangeMax() );
    }
}




//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateNodeCTColorRange ()
{
  if ( this->PETCTFusionNode == NULL )
    {
    return;
    }
  if ( this->CTRange == NULL )
    {
    return;
    }
  double tmpMin, tmpMax;

  tmpMin = this->CTRange->GetEntry1()->GetValueAsDouble();
  tmpMax = this->CTRange->GetEntry2()->GetValueAsDouble();
  if ( (tmpMin != this->PETCTFusionNode->GetCTRangeMin()) ||
       tmpMax != this->PETCTFusionNode->GetCTRangeMax() )
    {
    this->PETCTFusionNode->SetCTRange (tmpMin, tmpMax );
    this->ScaleCTColormap ( this->PETCTFusionNode->GetCTRangeMin(),
                          this->PETCTFusionNode->GetCTRangeMax() );
    }
}




//---------------------------------------------------------------------------
const char * vtkPETCTFusionGUI::GetCTColorTableNodeIDByType (int type)
{
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  basicNode->SetType(type);
  const char *id = basicNode->GetTypeAsIDString();
  basicNode->Delete();
  return (id);
}




//---------------------------------------------------------------------------
const char * vtkPETCTFusionGUI::GetPETColorNodeIDByType(int type)
{
  vtkMRMLPETProceduralColorNode *basicNode = vtkMRMLPETProceduralColorNode::New();
  basicNode->SetType(type);
  const char *id = basicNode->GetTypeAsIDString();
  basicNode->Delete();
  return (id);
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ApplyDefaultCTLUT()
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultCTLUT: Got NULL MRMLScene." );
    return;
    }
  if ( this->GetApplication() == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultCTLUT: Got NULL Application." );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultCTLUT: Got NULL PETCTFusionNode." );
    return;
    }

  vtkMRMLScalarVolumeNode *node =
    vtkMRMLScalarVolumeNode::SafeDownCast (
                                           this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputCTReference() ));
  if ( node == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultCTLUT: Got NULL CT volume node." );
    return;
    }
  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node->GetDisplayNode());


  //---
  //--- check the volume --
  //--- if it doesn't yet have a display node,
  //--- we need to create one
  //---
  if (displayNode == NULL)
    {
    //--- assign default volumeColorNode.
    const char *cID = this->GetCTColorTableNodeIDByType ( vtkMRMLColorTableNode::Grey);
    if ( cID == NULL)
      {
      vtkErrorMacro ( "Could not find default greyscale color node in scene." );
      return;
      }
    displayNode = vtkMRMLScalarVolumeDisplayNode::New ();
    displayNode->SetScene(this->MRMLScene);
    this->MRMLScene->AddNode (displayNode);
    displayNode->SetAndObserveColorNodeID ( cID );
    node->SetAndObserveDisplayNodeID( displayNode->GetID() );
    displayNode->Delete();
    }
  

  //---
  //--- Default color display & valid overrides:
  //--- Set the LUT to greyscale unless already specified.
  //---
  vtkMRMLColorTableNode *cnode = vtkMRMLColorTableNode::SafeDownCast ( displayNode->GetColorNode() );
  vtkMRMLColorNode *c = vtkMRMLColorNode::SafeDownCast ( displayNode->GetColorNode() );
  if ( c == NULL )
    {
    //--- TODO:
    //--- create a color node here.
    vtkErrorMacro ( "Display node has no color node!" );
    return;
    }


  if ( cnode == NULL )
    {
    //--- set the color node to be default Grey
    const char *id = this->GetCTColorTableNodeIDByType ( vtkMRMLColorTableNode::Grey);
    if ( id != NULL )
      {
      displayNode->SetAndObserveColorNodeID ( id );
      this->PETCTFusionNode->SetCTLUT(id);      
      }
    else
      {
      vtkErrorMacro ( "No Default (Grey) vtkMRMLColorTableNode found in scene. Cannot set color node." );
      return;
      }
    }
  else 
    {
    int type = cnode->GetType();
    //--- if type is not Grey
    const char *id = this->GetCTColorTableNodeIDByType ( vtkMRMLColorTableNode::Grey);
    if ( type != vtkMRMLColorTableNode::Grey ) 
      {
      //--- ... then set it to be Grey by default.
      if ( id == NULL )
        {
        vtkErrorMacro ( "No Default (Grey) vtkMRMLColorTableNode found in scene. Cannot set color node." );
        return;
        }
      displayNode->SetAndObserveColorNodeID (id );
      }
    this->PETCTFusionNode->SetCTLUT(displayNode->GetColorNodeID());      
    }

  //---
  //--- update win/lev sliders with recomputed values
  //--- TODO: this will get caught by display widgets
  //--- in volumes module: but ensure it gets caught
  //--- in display panel here too.
  //---
  displayNode->DisableModifiedEventOn();
  vtkMRMLScalarVolumeNode *svolumeNode = NULL;
  svolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (displayNode->GetAutoWindowLevel() )
    {
    displayNode->SetAutoWindowLevel(0);
//    svolumeNode->CalculateScalarAutoLevels(displayNode);
    }
  displayNode->DisableModifiedEventOff();
  displayNode->InvokePendingModifiedEvent();
}



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ApplyDefaultPETLUT()
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultPETLUT: Got NULL MRMLScene." );
    return;
    }
  if ( this->GetApplication() == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultPETLUT: Got NULL Application." );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultPETLUT: Got NULL PETCTFusionNode." );
    return;
    }

  vtkMRMLScalarVolumeNode *node =
    vtkMRMLScalarVolumeNode::SafeDownCast (
                                           this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputPETReference() ));
  if ( node == NULL )
    {
    vtkErrorMacro ( "ApplyDefaultPETLUT: Got NULL PET volume node." );
    return;
    }
  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node->GetDisplayNode());


  //---
  //--- check the volume --
  //--- if it doesn't yet have a display node,
  //--- we need to create one
  //---
  if (displayNode == NULL)
    {
    const char *cID = this->GetPETColorNodeIDByType ( vtkMRMLPETProceduralColorNode::PETheat);
    if ( cID == NULL)
      {
      vtkErrorMacro ( "Could not find default PETheat color node in scene." );
      return;
      }
    displayNode = vtkMRMLScalarVolumeDisplayNode::New ();
    displayNode->SetScene(this->MRMLScene);
    this->MRMLScene->AddNode (displayNode);
    displayNode->SetAndObserveColorNodeID ( cID );
    node->SetAndObserveDisplayNodeID( displayNode->GetID() );
    displayNode->Delete();
    }
  

  //---
  //--- Default color display & valid overrides:
  //--- If the node has a valid PET color LUT set, then allow it to
  //--- stay that way. Otherwise, set the color LUT to PETheat.
  //---
  vtkMRMLPETProceduralColorNode *cnode = vtkMRMLPETProceduralColorNode::SafeDownCast ( displayNode->GetColorNode() );
  vtkMRMLColorNode *c = vtkMRMLColorNode::SafeDownCast ( displayNode->GetColorNode() );
  if ( c == NULL )
    {
    //--- TODO:
    //--- create a color node here.
    vtkErrorMacro ( "Display node has no color node!" );
    return;
    }


  if ( cnode == NULL )
    {
    //--- set the color node to be default PETheat
    const char *id = this->GetPETColorNodeIDByType ( vtkMRMLPETProceduralColorNode::PETheat);
    if ( id != NULL )
      {
      displayNode->SetAndObserveColorNodeID ( id );
      this->PETCTFusionNode->SetPETLUT(id);      
      }
    else
      {
      vtkErrorMacro ( "No Default (PETheat) vtkMRMLPETProceduralColorNode found in scene. Cannot set color node." );
      return;
      }
    }
  else 
    {
    int type = cnode->GetType();
    //--- if type is not a PET LUT....
    if ( ( type != vtkMRMLPETProceduralColorNode::PETheat ) &&
         ( type != vtkMRMLPETProceduralColorNode::PETrainbow) &&
         ( type != vtkMRMLPETProceduralColorNode::PETMIP) )
      {
      //--- ... then set it to be PETheat by default.
      const char *id = this->GetPETColorNodeIDByType ( vtkMRMLPETProceduralColorNode::PETheat);
      if ( id == NULL )
        {
        vtkErrorMacro ( "No Default (PETheat) vtkMRMLPETProceduralColorNode found in scene. Cannot set color node." );
        return;
        }
      displayNode->SetAndObserveColorNodeID (id );
      }
    this->PETCTFusionNode->SetPETLUT(displayNode->GetColorNodeID());      
    }

  //---
  //--- update win/lev sliders with recomputed values
  //--- TODO: this will get caught by display widgets
  //--- in volumes module: but ensure it gets caught
  //--- in display panel here too.
  //---
  displayNode->DisableModifiedEventOn();
  vtkMRMLScalarVolumeNode *svolumeNode = NULL;
  svolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (displayNode->GetAutoWindowLevel() )
    {
    displayNode->SetAutoWindowLevel(0);
//    svolumeNode->CalculateScalarAutoLevels(displayNode);
    }
  displayNode->DisableModifiedEventOff();
  displayNode->InvokePendingModifiedEvent();
}


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ScaleCTColormap(double vtkNotUsed(min), double vtkNotUsed(max))
{
  //--- we want to scale the colors in the selected map
  //--- to span the range from:
  //--- PETCTFusionNode->GetColorRangeMin to
  //--- PETCTFusionNode->GetcolorRangeMax.

  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "ScaleCTColorMap: Got NULL MRMLScene." );
    return;
    }
  if ( this->GetApplication() == NULL )
    {
    vtkErrorMacro ( "ScaleCTColorMap: Got NULL Application." );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ScaleCTColorMap: Got NULL PETCTFusionNode." );
    return;
    }

  if ( this->PETCTFusionNode->GetInputCTReference() == NULL )
    {
    return;
    }
  if ( this->PETCTFusionNode->GetCTLUT() == NULL )
    {
    return;
    }

  vtkMRMLScalarVolumeNode *node =
    vtkMRMLScalarVolumeNode::SafeDownCast (
                                           this->MRMLScene->GetNodeByID(this->PETCTFusionNode->GetInputCTReference() ));
  if ( node == NULL )
    {
    vtkErrorMacro ( "ScaleCTColorMap: Got NULL CT volume node." );
    return;
    }

  vtkMRMLScalarVolumeDisplayNode *displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node->GetDisplayNode());
  //---
  //--- check the volume --
  //--- if it doesn't yet have a display node,
  //--- we need to create one
  //---
  if (displayNode == NULL)
    {
    const char *greyID = this->GetCTColorTableNodeIDByType ( vtkMRMLColorTableNode::Grey);
    if ( greyID == NULL )
      {
        vtkErrorMacro ( "No vtkMRMLColorTableNode for Grey in scene; cannot set default color node." );
        return;
      }
    displayNode = vtkMRMLScalarVolumeDisplayNode::New ();
    displayNode->SetScene(this->MRMLScene);
    this->MRMLScene->AddNode (displayNode);
    displayNode->SetAndObserveColorNodeID ( greyID );
    node->SetAndObserveDisplayNodeID( displayNode->GetID() );
    displayNode->Delete();
    }
  
  //---
  //--- otherwise, modify win and lev
  //--- in the node
  //---
  if ( displayNode )
    {
    displayNode->DisableModifiedEventOn();
    if (displayNode->GetAutoWindowLevel() )
      {
      displayNode->SetAutoWindowLevel(0);
      }
    //--- Adjust node's window, level and threshold.
    double min = this->PETCTFusionNode->GetCTRangeMin();
    double max = this->PETCTFusionNode->GetCTRangeMax();
    displayNode->SetWindow( max - min );
    displayNode->SetLevel( min + (0.5 * (max - min)) );
    //--- apply no threshold... (may need to change this for vol render)
    displayNode->SetUpperThreshold(this->PETCTFusionNode->GetCTMax());
    displayNode->SetLowerThreshold(this->PETCTFusionNode->GetCTMin());

    displayNode->DisableModifiedEventOff();
    displayNode->InvokePendingModifiedEvent();
    }
    return;
}
  



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::ScalePETColormap(double vtkNotUsed(min), double vtkNotUsed(max))
{
  //--- we want to scale the colors in the selected map
  //--- to span the range from:
  //--- PETCTFusionNode->GetColorRangeMin to
  //--- PETCTFusionNode->GetcolorRangeMax.

  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "ScalePETColormap: Got NULL MRMLScene." );
    return;
    }
  if ( this->GetApplication() == NULL )
    {
    vtkErrorMacro ( "ScalePETColormap: Got NULL Application." );
    return;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ScalePETColormap: Got NULL PETCTFusionNode." );
    return;
    }

  if ( this->PETCTFusionNode->GetInputPETReference() == NULL )
    {
    return;
    }
  if ( this->PETCTFusionNode->GetPETLUT() == NULL )
    {
    return;
    }

  vtkMRMLScalarVolumeNode *node = vtkMRMLScalarVolumeNode::SafeDownCast (
                                                                         this->MRMLScene->GetNodeByID( this->PETCTFusionNode->GetInputPETReference() ));
  if ( node == NULL )
    {
    vtkErrorMacro ( "ScalePETColormap: Got NULL PET volume node." );
    return;
    }

  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node->GetDisplayNode());
  //---
  //--- check the volume --
  //--- if it doesn't yet have a display node,
  //--- we need to create one
  //---
  if (displayNode == NULL)
    {
    const char *petID = this->GetPETColorNodeIDByType (vtkMRMLPETProceduralColorNode::PETheat);
    if ( petID == NULL )
      {
        vtkErrorMacro ( "No vtkMRMLPETProceduralColorNode in scene. Cannot set color node." );
        return;
      }
    displayNode = vtkMRMLScalarVolumeDisplayNode::New ();
    displayNode->SetScene(this->MRMLScene);
    this->MRMLScene->AddNode (displayNode);
    displayNode->SetAndObserveColorNodeID ( petID );
    node->SetAndObserveDisplayNodeID( displayNode->GetID() );
    displayNode->Delete();
    }
  
  //---
  //--- otherwise, modify win and lev
  //--- in the node
  //---
  if ( displayNode )
    {
    displayNode->DisableModifiedEventOn();
    vtkMRMLScalarVolumeNode *svolumeNode = NULL;
    svolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

    if (displayNode->GetAutoWindowLevel() )
      {
      displayNode->SetAutoWindowLevel(0);
      }

    //--- Adjust node's window, level and threshold.
    double min = this->PETCTFusionNode->GetColorRangeMin();
    double max = this->PETCTFusionNode->GetColorRangeMax();
    double win = max - min;
    double lev =  min + (0.5 * win);
    displayNode->SetWindow( win );
    displayNode->SetLevel( lev );
    //--- apply no threshold... (may need to change this for vol render...)
    displayNode->SetUpperThreshold(this->PETCTFusionNode->GetPETMax());
    displayNode->SetLowerThreshold(this->PETCTFusionNode->GetPETMin());

    displayNode->DisableModifiedEventOff();
    displayNode->InvokePendingModifiedEvent();
    }
    return;
}
  


//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdatePETRangeFromMRML ()
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
  if ( this->Logic == NULL )
    {
    return;
    }

  //---
  //--- Method gets called if PETSelector changes.
  //--- Checks range of PET image values.
  //---
  //--- get image data for pet
  vtkMRMLVolumeNode *v = vtkMRMLVolumeNode::SafeDownCast (this->PETSelector->GetSelected());
  if ( v == NULL )
    {
    this->PETRange->GetEntry1()->SetValueAsDouble ( this->PETCTFusionNode->GetColorRangeMin() );
    this->PETRange->GetEntry2()->SetValueAsDouble ( this->PETCTFusionNode->GetColorRangeMax() );
    this->PETRange->SetWholeRange(this->PETCTFusionNode->GetPETMin(),
                                  this->PETCTFusionNode->GetPETMax());
    this->PETRange->SetRange(this->PETCTFusionNode->GetColorRangeMin(),
                             this->PETCTFusionNode->GetColorRangeMax());
    return;
    }

  
  if ( v && v->GetImageData() && v->GetImageData()->GetPointData() )
    {
    //--- build a histogram
    vtkKWHistogram *h = vtkKWHistogram::New();
    h->BuildHistogram ( v->GetImageData()->GetPointData()->GetScalars(), 0);
    double *range = h->GetRange();
    this->PETCTFusionNode->SetPETMin( range[0]);
    this->PETCTFusionNode->SetPETMax( range[1]);
    this->Logic->ComputeSUVmax();
    if ( this->PETCTFusionNode->GetPETSUVmax() < 0.0 )
      {
      this->PETCTFusionNode->SetPETSUVmax(0.0);
      }

    //--- Get Win/Lev from Volume node
    //--- and initialize the display range
    //--- from those values.
    //--- First get display node.
    vtkMRMLScalarVolumeDisplayNode *dnode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast( v->GetDisplayNode() );
    if ( dnode )
      {
      double win = dnode->GetWindow( );
      double lev = dnode->GetLevel( ); //min + (0.5 * (max - min)) );
      //double ut = dnode->GetUpperThreshold();
      //double lt = dnode->GetLowerThreshold();
      //--- compute settings for node
      double lw = lev - (win/2.0);
      double uw = lev + (win/2.0);
      //--- record max and min in node.
      this->PETCTFusionNode->SetColorRangeMin (lw);
      this->PETCTFusionNode->SetColorRangeMax (uw);
      }
    else
      {
      this->PETCTFusionNode->SetColorRangeMin ( this->PETCTFusionNode->GetPETMin() );
      this->PETCTFusionNode->SetColorRangeMax ( this->PETCTFusionNode->GetPETMax() );
      }

    double nodeColorMin = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMin());
    double nodeColorMax = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetColorRangeMax() );

    double nodePETMin = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetPETMin());
    double nodePETMax = this->Logic->ConvertImageUnitsToSUVUnits(this->PETCTFusionNode->GetPETMax() );

    this->PETRange->GetEntry1()->SetValueAsDouble ( nodeColorMin );
    this->PETRange->GetEntry2()->SetValueAsDouble ( nodeColorMax );
    this->PETRange->SetWholeRange( nodePETMin, nodePETMax );
    this->PETRange->SetRange( nodeColorMin,  nodeColorMax );

    h->Delete();
    }
  return;
}




//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::UpdateCTRangeFromMRML ()
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
  //--- Method gets called if CTSelector changes.
  //--- Checks range of CT image values.
  //---
  //--- get image data for ct
  vtkMRMLVolumeNode *v = vtkMRMLVolumeNode::SafeDownCast (this->CTSelector->GetSelected() );

  if ( v == NULL )
    {
    this->PETCTFusionNode->SetCTRangeMin (this->PETCTFusionNode->GetCTMin() );
    this->PETCTFusionNode->SetCTRangeMax (this->PETCTFusionNode->GetCTMin() );
    this->CTRange->SetWholeRange(this->PETCTFusionNode->GetCTMin(), this->PETCTFusionNode->GetCTMax());
    this->CTRange->SetRange(this->PETCTFusionNode->GetCTRangeMin(), this->PETCTFusionNode->GetCTRangeMax());
    return;
    }

  if ( v && v->GetImageData() && v->GetImageData()->GetPointData() )
    {
    //--- build a histogram
    vtkKWHistogram *h = vtkKWHistogram::New();
    h->BuildHistogram ( v->GetImageData()->GetPointData()->GetScalars(), 0);
    double *range = h->GetRange();
    this->PETCTFusionNode->SetCTMin( range[0]);
    this->PETCTFusionNode->SetCTMax( range[1]);

    //--- Get Win/Lev from Volume node
    //--- and initialize the display range
    //--- from those values.
    //--- First get display node.
    vtkMRMLScalarVolumeDisplayNode *dnode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast( v->GetDisplayNode() );
    if ( dnode )
      {
      double win = dnode->GetWindow( );
      double lev = dnode->GetLevel( ); //min + (0.5 * (max - min)) );
      //double ut = dnode->GetUpperThreshold();
      //double lt = dnode->GetLowerThreshold();
      //--- compute settings for node
      double lw = lev - (win/2.0);
      double uw = lev + (win/2.0);
      //--- record max and min in node.
      this->PETCTFusionNode->SetCTRangeMin (lw);
      this->PETCTFusionNode->SetCTRangeMax (uw);
      }
    else
      {
      this->PETCTFusionNode->SetCTRangeMin (this->PETCTFusionNode->GetCTMin() );
      this->PETCTFusionNode->SetCTRangeMax (this->PETCTFusionNode->GetCTMin() );
      }

    this->CTRange->GetEntry1()->SetValueAsDouble ( this->PETCTFusionNode->GetCTRangeMin() );
    this->CTRange->GetEntry2()->SetValueAsDouble ( this->PETCTFusionNode->GetCTRangeMax() );

    this->CTRange->SetWholeRange(this->PETCTFusionNode->GetCTMin(), this->PETCTFusionNode->GetCTMax());
    this->CTRange->SetRange(this->PETCTFusionNode->GetCTRangeMin(), this->PETCTFusionNode->GetCTRangeMax());

    h->Delete();
    }
  return;
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



//---------------------------------------------------------------------------
void vtkPETCTFusionGUI::InitializeCTMinAndMax()
{
  if ( this->PETCTFusionNode ==  NULL )
    {
    return;
    }

  this->PETCTFusionNode->SetCTMin(0.0);
  this->PETCTFusionNode->SetCTMax(255.0);
}
