/*==========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDAnalysis/vtkFourDAnalysisGUI.cxx $
  Date:      $Date: 2009-02-10 18:26:32 -0500 (Tue, 10 Feb 2009) $
  Version:   $Revision: 3674 $

==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkFourDAnalysisGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkMRMLColorNode.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWMultiColumnList.h"

#include "vtkKWScaleWithEntry.h"
#include "vtkKWScale.h"
#include "vtkKWPushButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWMenuButton.h"
#include "vtkKWSpinBox.h"
#include "vtkKWCanvas.h"
#include "vtkKWRange.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"

#include "vtkKWProgressDialog.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"

#include "vtkKWProgressGauge.h"

#include "vtkCornerAnnotation.h"
#include "vtkCommandLineModuleGUI.h"

#include "vtkMRMLTimeSeriesBundleNode.h"

#include "vtkMRMLPlotNode.h"
#include "vtkMRMLArrayPlotNode.h"
#include "vtkMRMLOrthogonalLinePlotNode.h"
#include "vtkMRMLXYPlotManagerNode.h"
#include "vtkMRMLCurveAnalysisNode.h"
#include "vtkMRMLDoubleArrayNode.h"

#include "vtkCurveAnalysisPythonInterface.h"
#ifdef Slicer3_USE_PYTHON
#include <Python.h>
#endif



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFourDAnalysisGUI );
vtkCxxRevisionMacro ( vtkFourDAnalysisGUI, "$Revision: 3674 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkFourDAnalysisGUI::vtkFourDAnalysisGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkFourDAnalysisGUI::DataCallback);

  this->BundleNameCount = 0;

  this->IntensityCurves = vtkIntensityCurves::New();
  //this->FittedCurve     = vtkDoubleArray::New();

  this->FittedCurveNode     = NULL;

  this->CurveAnalysisScript = NULL;

  //----------------------------------------------------------------
  // GUI widgets
  this->ProgressDialog = NULL;

  this->Active4DBundleSelectorWidget  = NULL;

  // Frame control
  this->ForegroundVolumeSelectorScale = NULL;
  this->BackgroundVolumeSelectorScale = NULL;

  // Curve fitting / parameter map
  //this->AcqTimeEntry        = NULL;
  //this->MaskSelectMenu           = NULL;
  this->MaskNodeSelector         = NULL;
  this->RunPlotButton            = NULL;
  this->ErrorBarCheckButton      = NULL;
  this->PlotList                 = NULL;
  this->FittingLabelMenu         = NULL;
  this->CurveScriptSelectButton  = NULL;
  this->CurveFittingStartIndexSpinBox = NULL;
  this->CurveFittingEndIndexSpinBox        = NULL;
  this->RunFittingButton    = NULL;
  this->SaveFittedCurveButton = NULL;
  this->CurveScriptMethodName = NULL;

  this->InitialParameterList = NULL;
  this->SavePlotButton = NULL;
  this->IntensityPlot  = NULL;

  this->MapOutputVolumePrefixEntry = NULL;
  //this->MapOutputSelector   = NULL;
  //this->MapOutputVolumeMenu = NULL;
  //this->ScriptSelectButton  = NULL;
  this->RunScriptButton = NULL;
  this->ResultParameterList = NULL;

  this->MapIMinSpinBox     = NULL;
  this->MapIMaxSpinBox     = NULL;
  this->MapJMinSpinBox     = NULL;
  this->MapJMaxSpinBox      = NULL;
  this->MapKMinSpinBox     = NULL;
  this->MapKMaxSpinBox     = NULL;

  //----------------------------------------------------------------
  // Time
  this->TimerFlag = 0;

  this->PlotManagerNode = NULL;

}

//---------------------------------------------------------------------------
vtkFourDAnalysisGUI::~vtkFourDAnalysisGUI ( )
{

  //----------------------------------------------------------------
  // Remove Callbacks

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  //----------------------------------------------------------------
  // Remove Observers

  this->RemoveGUIObservers();

  //----------------------------------------------------------------
  // Remove GUI widgets

  if (this->ProgressDialog)
    {
    this->ProgressDialog->SetParent(NULL);
    this->ProgressDialog->Delete();
    }
  if (this->Active4DBundleSelectorWidget)
    {
    this->Active4DBundleSelectorWidget->SetParent(NULL);
    this->Active4DBundleSelectorWidget->Delete();
    }
  if (this->ForegroundVolumeSelectorScale)
    {
    this->ForegroundVolumeSelectorScale->SetParent(NULL);
    this->ForegroundVolumeSelectorScale->Delete();
    }
  if (this->BackgroundVolumeSelectorScale)
    {
    this->BackgroundVolumeSelectorScale->SetParent(NULL);
    this->BackgroundVolumeSelectorScale->Delete();
    }
  if (this->WindowLevelRange)
    {
    this->WindowLevelRange->SetParent(NULL);
    this->WindowLevelRange->Delete();
    }
  if (this->ThresholdRange)
    {
    this->ThresholdRange->SetParent(NULL);
    this->ThresholdRange->Delete();
    }
  if (this->MaskNodeSelector)
    {
    this->MaskNodeSelector->SetParent(NULL);
    this->MaskNodeSelector->Delete();
    }

  if (this->RunPlotButton)
    {
    this->RunPlotButton->SetParent(NULL);
    this->RunPlotButton->Delete();
    }
  if (this->IntensityPlot)
    {
    this->IntensityPlot->SetParent(NULL);
    this->IntensityPlot->Delete();
    }
  if (this->ErrorBarCheckButton)
    {
    this->ErrorBarCheckButton->SetParent(NULL);
    this->ErrorBarCheckButton->Delete();
    }
  if (this->PlotList)
    {
    this->PlotList->SetParent(NULL);
    this->PlotList->Delete();
    }
  if (this->FittingLabelMenu)
    {
    this->FittingLabelMenu->SetParent(NULL);
    this->FittingLabelMenu->Delete();
    }
  if (this->CurveScriptSelectButton)
    {
    this->CurveScriptSelectButton->SetParent(NULL);
    this->CurveScriptSelectButton->Delete();
    }
  if (this->CurveFittingStartIndexSpinBox)
    {
    this->CurveFittingStartIndexSpinBox->SetParent(NULL);
    this->CurveFittingStartIndexSpinBox->Delete();
    }
  if (this->CurveFittingEndIndexSpinBox)
    {
    this->CurveFittingEndIndexSpinBox->SetParent(NULL);
    this->CurveFittingEndIndexSpinBox->Delete();
    }
  if (this->InitialParameterList)
    {
    this->InitialParameterList->SetParent(NULL);
    this->InitialParameterList->Delete();
    }
  if (this->RunFittingButton)
    {
    this->RunFittingButton->SetParent(NULL);
    this->RunFittingButton->Delete();
    }
  if (this->SaveFittedCurveButton)
    {
    this->SaveFittedCurveButton->SetParent(NULL);
    this->SaveFittedCurveButton->Delete();
    }
  if (this->CurveScriptMethodName)
    {
    this->CurveScriptMethodName->SetParent(NULL);
    this->CurveScriptMethodName->Delete();
    }

  if (this->ResultParameterList)
    {
    this->ResultParameterList->SetParent(NULL);
    this->ResultParameterList->Delete();
    }
  if (this->SavePlotButton)
    {
    this->SavePlotButton->SetParent(NULL);
    this->SavePlotButton->Delete();
    }

  if (this->MapOutputVolumePrefixEntry)
    {
    this->MapOutputVolumePrefixEntry->SetParent(NULL);
    this->MapOutputVolumePrefixEntry->Delete();
    }
  if (this->RunScriptButton)
    {
    this->RunScriptButton->SetParent(NULL);
    this->RunScriptButton->Delete();
    }
  if (this->MapIMinSpinBox)
    {
    this->MapIMinSpinBox->SetParent(NULL);
    this->MapIMinSpinBox->Delete();
    }
  if (this->MapIMaxSpinBox)
    {
    this->MapIMaxSpinBox->SetParent(NULL);
    this->MapIMaxSpinBox->Delete();
    }
  if (this->MapJMinSpinBox)
    {
    this->MapJMinSpinBox->SetParent(NULL);
    this->MapJMinSpinBox->Delete();
    }
  if (this->MapJMaxSpinBox)
    {
    this->MapJMaxSpinBox->SetParent(NULL);
    this->MapJMaxSpinBox->Delete();
    }
  if (this->MapKMinSpinBox)
    {
    this->MapKMinSpinBox->SetParent(NULL);
    this->MapKMinSpinBox->Delete();
    }
  if (this->MapKMaxSpinBox)
    {
    this->MapKMaxSpinBox->SetParent(NULL);
    this->MapKMaxSpinBox->Delete();
    }


  //----------------------------------------------------------------
  // Unregister Logic class

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::Init()
{

  if (this->GetMRMLScene())
    {
    vtkDoubleArray* array = vtkDoubleArray::New();
    this->FittedCurveNode = vtkMRMLDoubleArrayNode::New();
    this->GetMRMLScene()->AddNode(this->FittedCurveNode);
    this->FittedCurveNode->SetName("Fitted");
    this->FittedCurveNode->SetArray(array);
    array->Delete();

    if (this->IntensityCurves)
      {
      this->IntensityCurves->SetMRMLScene(this->GetMRMLScene());
      }


    // Register node classes. SmartPointer is used.
    {
      this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLArrayPlotNode >::New() );
      this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLCurveAnalysisNode >::New() );
      this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLDoubleArrayNode >::New() );
      this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLOrthogonalLinePlotNode >::New() );
      this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLPlotNode >::New() );
      this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLXYPlotManagerNode >::New() );
    }

    // Setup plot manager node
    vtkMRMLXYPlotManagerNode* node = vtkMRMLXYPlotManagerNode::New();
    this->GetMRMLScene()->AddNode(node);

    vtkIntArray  *events = vtkIntArray::New();
    events->InsertNextValue ( vtkCommand::ModifiedEvent );
    vtkSetAndObserveMRMLNodeEventsMacro ( this->PlotManagerNode, node, events );

    node->Delete();
    events->Delete();
    node = NULL;
    events = NULL;

    this->IntensityPlot->SetAndObservePlotManagerNode(this->PlotManagerNode);
    this->PlotManagerNode->Refresh();

    }
    
}

//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

  // register node type to the MRML scene
  vtkMRMLScene* scene = this->GetMRMLScene();

  // 4D bundle node (vtkMRMLTimeSeriesBundleNode)
  vtkMRMLTimeSeriesBundleNode* bundleNode = vtkMRMLTimeSeriesBundleNode::New();
  scene->RegisterNodeClass(bundleNode);
  bundleNode->Delete();

  // Curve analysis node (vtkMRMLCurveAnalysisNode)
  vtkMRMLCurveAnalysisNode* curveNode = vtkMRMLCurveAnalysisNode::New();
  scene->RegisterNodeClass(curveNode);
  curveNode->Delete();

  //this->FourDImageGUI = 
  //  vtkFourDImageGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())
  //                                 ->GetModuleGUIByName("4D Image"));


}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "FourDAnalysisGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (this->Active4DBundleSelectorWidget)
    {
    this->Active4DBundleSelectorWidget
      ->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ForegroundVolumeSelectorScale)
    {
    this->ForegroundVolumeSelectorScale
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->BackgroundVolumeSelectorScale)
    {
    this->BackgroundVolumeSelectorScale
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->WindowLevelRange)
    {
    this->WindowLevelRange
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ThresholdRange)
    {
    this->ThresholdRange
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MaskNodeSelector)
    {
    this->MaskNodeSelector
      ->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RunPlotButton)
    {
    this->RunPlotButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ErrorBarCheckButton)
    {
    this->ErrorBarCheckButton->GetWidget()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->PlotList)
    {
    this->PlotList->GetWidget()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->FittingLabelMenu)
    {
    this->FittingLabelMenu
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CurveScriptSelectButton)
    {
    this->CurveScriptSelectButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CurveFittingStartIndexSpinBox)
    {
    this->CurveFittingStartIndexSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CurveFittingEndIndexSpinBox)
    {
    this->CurveFittingEndIndexSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->RunFittingButton)
    {
    this->RunFittingButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapIMinSpinBox)
    {
    this->MapIMinSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapIMaxSpinBox)
    {
    this->MapIMaxSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapJMinSpinBox)
    {
    this->MapJMinSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapJMaxSpinBox)
    {
    this->MapJMaxSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapKMinSpinBox)
    {
    this->MapKMinSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapKMaxSpinBox)
    {
    this->MapKMaxSpinBox
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->SavePlotButton)
    {
    this->SavePlotButton->GetWidget()->GetLoadSaveDialog()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapOutputVolumePrefixEntry)
    {
    this->MapOutputVolumePrefixEntry
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->RunScriptButton)
    {
    this->RunScriptButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->SaveFittedCurveButton)
    {
    this->SaveFittedCurveButton->GetWidget()->GetLoadSaveDialog()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  //----------------------------------------------------------------
  // GUI Observers

  if (this->Active4DBundleSelectorWidget)
    {
    this->Active4DBundleSelectorWidget
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ForegroundVolumeSelectorScale)
    {
    this->ForegroundVolumeSelectorScale
      ->AddObserver(vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->BackgroundVolumeSelectorScale)
    {
    this->BackgroundVolumeSelectorScale
      ->AddObserver(vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->WindowLevelRange)
    {
    this->WindowLevelRange
      ->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ThresholdRange)
    {
    this->ThresholdRange
      ->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MaskNodeSelector)
    {
    this->MaskNodeSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RunPlotButton)
    {
    this->RunPlotButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ErrorBarCheckButton)
    {
    this->ErrorBarCheckButton->GetWidget()
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  //if (this->PlotList)
  //  {
  //  this->PlotList->GetWidget()
  //    ->AddObserver(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand);
  //  }

  if (this->FittingLabelMenu)
    {
    this->FittingLabelMenu
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CurveScriptSelectButton)
    {
    this->CurveScriptSelectButton->GetWidget()->GetLoadSaveDialog()
      ->AddObserver(vtkKWLoadSaveDialog::FileNameChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CurveFittingStartIndexSpinBox)
    {
    this->CurveFittingStartIndexSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CurveFittingEndIndexSpinBox)
    {
    this->CurveFittingEndIndexSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->RunFittingButton)
    {
    this->RunFittingButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->SaveFittedCurveButton)
    {
    this->SaveFittedCurveButton->GetWidget()->GetLoadSaveDialog()
      ->AddObserver(vtkKWLoadSaveDialog::FileNameChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->SavePlotButton)
    {
    this->SavePlotButton->GetWidget()->GetLoadSaveDialog()
      ->AddObserver(vtkKWLoadSaveDialog::FileNameChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapOutputVolumePrefixEntry)
    {
    this->MapOutputVolumePrefixEntry
      ->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->RunScriptButton)
    {
    this->RunScriptButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }

  if (this->MapIMinSpinBox)
    {
    this->MapIMinSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapIMaxSpinBox)
    {
    this->MapIMaxSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapJMinSpinBox)
    {
    this->MapJMinSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapJMaxSpinBox)
    {
    this->MapJMaxSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapKMinSpinBox)
    {
    this->MapKMinSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MapKMaxSpinBox)
    {
    this->MapKMaxSpinBox
      ->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }

  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkFourDAnalysisLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  else if (this->Active4DBundleSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
           && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLTimeSeriesBundleNode *bundleNode = 
      vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    SelectActive4DBundle(bundleNode);
    }
  else if (this->ForegroundVolumeSelectorScale == vtkKWScaleWithEntry::SafeDownCast(caller)
      && event == vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/)
    {
    vtkMRMLTimeSeriesBundleNode *bundleNode = 
      vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    int volume = (int)this->ForegroundVolumeSelectorScale->GetValue();
    if (bundleNode)
      {
      SetForeground(bundleNode->GetID(), volume);
      }
    }
  else if (this->BackgroundVolumeSelectorScale == vtkKWScaleWithEntry::SafeDownCast(caller)
      && event == vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/ )
    {
    int volume = (int)this->BackgroundVolumeSelectorScale->GetValue();
    vtkMRMLTimeSeriesBundleNode *bundleNode = 
      vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    if (bundleNode)
      {
      SetBackground(bundleNode->GetID(), volume);
      }
    }
  else if (this->WindowLevelRange == vtkKWRange::SafeDownCast(caller)
      && event == vtkKWRange::RangeValueChangingEvent)
    {
    double wllow, wlhigh;
    this->WindowLevelRange->GetRange(wllow, wlhigh);
    this->Window = wlhigh - wllow;
    this->Level  = (wlhigh + wllow) / 2.0;
    SetWindowLevelForCurrentFrame();
    }
  else if (this->ThresholdRange == vtkKWRange::SafeDownCast(caller)
      && event == vtkKWRange::RangeValueChangingEvent)
    {
    double thlow, thhigh;
    this->ThresholdRange->GetRange(thlow, thhigh);
    this->ThresholdUpper  = thhigh; 
    this->ThresholdLower  = thlow; 
    SetWindowLevelForCurrentFrame();
    }
  else if (this->MaskNodeSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
           && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    }
  else if (this->RunPlotButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    //int selected = this->MaskSelectMenu->GetMenu()->GetIndexOfSelectedItem();
    //const char* maskID   = this->MaskNodeIDList[selected].c_str();
    vtkMRMLTimeSeriesBundleNode *bundleNode = 
      vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    //vtkMRMLScalarVolumeNode* maskNode =
    //  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(maskID));
    vtkMRMLScalarVolumeNode* maskNode =
      vtkMRMLScalarVolumeNode::SafeDownCast(this->MaskNodeSelector->GetSelected());
    if (bundleNode && maskNode)
      {
      this->IntensityCurves->SetBundleNode(bundleNode);
      this->IntensityCurves->SetMaskNode(maskNode);
      }

    GeneratePlotNodes();
    UpdatePlotList();

    }
  else if (this->ErrorBarCheckButton->GetWidget() == vtkKWCheckButton::SafeDownCast(caller)
           && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    if (this->ErrorBarCheckButton->GetWidget()->GetSelectedState() == 1)
      {
      //this->IntensityPlot->ErrorBarOn();
      //this->IntensityPlot->UpdateGraph();
      this->PlotManagerNode->SetErrorBarAll(1);
      this->PlotManagerNode->Refresh();
      }
    else
      {
      //this->IntensityPlot->ErrorBarOff();
      //this->IntensityPlot->UpdateGraph();
      this->PlotManagerNode->SetErrorBarAll(0);
      this->PlotManagerNode->Refresh();
      }
    }
  else if (this->CurveScriptSelectButton->GetWidget()->GetLoadSaveDialog()
           == vtkKWLoadSaveDialog::SafeDownCast(caller)
           && event == vtkKWLoadSaveDialog::FileNameChangedEvent)
    {
    vtkMRMLCurveAnalysisNode* curveNode = vtkMRMLCurveAnalysisNode::New();
    this->GetMRMLScene()->AddNode(curveNode);

    //const char* script = this->CurveScriptSelectButton->GetWidget()->GetFileName();
    if (this->CurveAnalysisScript)
      {
      this->CurveAnalysisScript->Delete();
      }
    this->CurveAnalysisScript = vtkCurveAnalysisPythonInterface::New();
    this->CurveAnalysisScript->SetScript(this->CurveScriptSelectButton->GetWidget()->GetFileName());
    //this->GetLogic()->GetCurveAnalysisInfo(script, curveNode);
    this->CurveAnalysisScript->SetCurveAnalysisNode(curveNode);
    this->CurveAnalysisScript->GetInfo();

    // Update method's name field
    if (this->CurveScriptMethodName)
      {
      this->CurveScriptMethodName->GetWidget()->SetValue(curveNode->GetMethodName());
      }

    UpdateInitialParameterList(curveNode);

    this->GetMRMLScene()->RemoveNode(curveNode);
    curveNode->Delete();
    }
  else if (this->RunFittingButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    int n = this->FittingLabelMenu->GetMenu()->GetIndexOfSelectedItem();
    vtkIntArray* labels = this->IntensityCurves->GetLabelList();
    int label = labels->GetValue(n);

    // Get the path to the script
    //const char* script = this->CurveScriptSelectButton->GetWidget()->GetFileName();
    if (!this->CurveAnalysisScript)
      {
      vtkErrorMacro("Script is not selected.");
      return;
      /*
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
      dialog->SetStyleToMessage();
      std::string msg = std::string("Please select Python script for curve analysis");
      dialog->SetText(msg.c_str());
      dialog->Create ( );
      dialog->Invoke();
      dialog->Delete();
      */
      }

    //vtkDoubleArray* curve = this->IntensityCurves->GetCurve(label);
    vtkMRMLDoubleArrayNode* anode = this->IntensityCurves->GetCurve(label);
    vtkDoubleArray* curve = anode->GetArray();

    if (curve)
      {
      // Add a new vtkMRMLCurveAnalysisNode to the MRML scene
      vtkMRMLCurveAnalysisNode* curveNode = vtkMRMLCurveAnalysisNode::New();
      this->GetMRMLScene()->AddNode(curveNode);
      this->CurveAnalysisScript->SetCurveAnalysisNode(curveNode);
      this->CurveAnalysisScript->GetInfo();

      // Prepare vtkDoubleArray to pass the source cueve data
      vtkDoubleArray* srcCurve = vtkDoubleArray::New();
      srcCurve->SetNumberOfComponents( curve->GetNumberOfComponents() );

      // Check the range of the curve to fit
      int max   = curve->GetNumberOfTuples();
      int start = (int)this->CurveFittingStartIndexSpinBox->GetValue();
      int end   = (int)this->CurveFittingEndIndexSpinBox->GetValue();
      if (start < 0)   start = 0;
      if (end >= max)  end   = max-1;
      if (start > end) start = end;
      for (int i = start; i <= end; i ++)
        {
        double* xy = curve->GetTuple(i);
        srcCurve->InsertNextTuple(xy);
        }

      // Get initial parameters for the curve fitting
      GetInitialParametersAndInputCurves(curveNode, start, end);

      // Prepare vtkDoubleArray to receive a fitted curve from the script
      // (The size of the fitted curve should be the same as original
      // intensity curve)
      vtkDoubleArray* fittedCurve = vtkDoubleArray::New();
      fittedCurve->SetNumberOfComponents(2);
      for (int i = start; i < end; i ++)
        {
        double* xy = curve->GetTuple(i);
        fittedCurve->InsertNextTuple(xy);
        }

      // Set source and fitted curve arrays to the curve analysis node.
      curveNode->SetTargetCurve(srcCurve);
      curveNode->SetFittedCurve(fittedCurve);

      // Call Logic to excecute the curve fitting script
      //this->GetLogic()->RunCurveFitting(script, curveNode);
      this->CurveAnalysisScript->Run();

      // Display result parameters
      UpdateOutputParameterList(curveNode);

      vtkDoubleArray* resultCurve = curveNode->GetFittedCurve();
      if (this->FittedCurveNode)
        {
        vtkDoubleArray* array = this->FittedCurveNode->GetArray();
        array->SetNumberOfComponents(2);
        array->SetNumberOfTuples(0);
        int n = resultCurve->GetNumberOfTuples();
        for (int i = 0; i < n; i ++)
          {
          double* values = resultCurve->GetTuple(i);
          array->InsertNextTuple(values);
          }
        }

      //// NOTE: this might not be necessary... just use the fittedCurve pointer defined above.
      //if (resultCurve)
      //  {
      //  int num = resultCurve->GetNumberOfTuples();
      //  //std::cerr << "result curve number of tuples = " << num << std::endl;
      //  //for (int i = 0; i < num; i ++)
      //  //  {
      //  //  double* xy = resultCurve->GetTuple(i);
      //  //  std::cerr << "xy = " << xy[0]  << ", " << xy[1] << std::endl;
      //  //  }
      //  UpdateIntensityPlotWithFittedCurve(this->IntensityCurves, resultCurve);
      //  this->GetMRMLScene()->RemoveNode(curveNode);
      //  }
      UpdateIntensityPlotWithFittedCurve(this->IntensityCurves);

      //UpdatePlotList(); // called by event
      }
    }
  else if (this->SaveFittedCurveButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller)
           && event == vtkKWLoadSaveDialog::FileNameChangedEvent)
    {
    const char* filename = (const char*)callData;
    //int selected = this->MaskSelectMenu->GetMenu()->GetIndexOfSelectedItem();
    this->GetLogic()->SaveCurve(this->FittedCurveNode->GetArray(), filename);
    }
  else if (this->SavePlotButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller)
           && event == vtkKWLoadSaveDialog::FileNameChangedEvent)
    {
    const char* filename = (const char*)callData;
    //int selected = this->MaskSelectMenu->GetMenu()->GetIndexOfSelectedItem();
    this->GetLogic()->SaveIntensityCurves(this->IntensityCurves, filename);
    }

  else if (this->RunScriptButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    // Add a new vtkMRMLCurveAnalysisNode to the MRML scene
    vtkMRMLCurveAnalysisNode* curveNode = vtkMRMLCurveAnalysisNode::New();
    this->GetMRMLScene()->AddNode(curveNode);

    this->CurveAnalysisScript->SetCurveAnalysisNode(curveNode);
    this->CurveAnalysisScript->GetInfo();

    int start = (int)this->CurveFittingStartIndexSpinBox->GetValue();
    int end   = (int)this->CurveFittingEndIndexSpinBox->GetValue();
    
    GetInitialParametersAndInputCurves(curveNode, start, end);
    
    const char* prefix   = this->MapOutputVolumePrefixEntry->GetValue();
    
    int imin = (int)this->MapIMinSpinBox->GetValue();
    int imax = (int)this->MapIMaxSpinBox->GetValue();
    int jmin = (int)this->MapJMinSpinBox->GetValue();
    int jmax = (int)this->MapJMaxSpinBox->GetValue();
    int kmin = (int)this->MapKMinSpinBox->GetValue();
    int kmax = (int)this->MapKMaxSpinBox->GetValue();
    
    vtkMRMLTimeSeriesBundleNode *bundleNode = 
      vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    if (prefix && bundleNode && this->CurveAnalysisScript)
      {
      int start = (int)this->CurveFittingStartIndexSpinBox->GetValue();
      int end   = (int)this->CurveFittingEndIndexSpinBox->GetValue();
      this->GetLogic()->AddObserver(vtkFourDAnalysisLogic::ProgressDialogEvent,  this->LogicCallbackCommand);

      this->GetLogic()->GenerateParameterMap(this->CurveAnalysisScript,
                                              curveNode,
                                              bundleNode,
                                              prefix,
                                              start, end,
                                              imin, imax, jmin, jmax, kmin, kmax);
      this->GetLogic()->RemoveObservers(vtkFourDAnalysisLogic::ProgressDialogEvent,  this->LogicCallbackCommand);
      }
    else
      {
      
      }
    }
} 


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkFourDAnalysisGUI *self = reinterpret_cast<vtkFourDAnalysisGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkFourDAnalysisGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
  if (event == vtkFourDAnalysisLogic::StatusUpdateEvent)
    {
    //this->UpdateDeviceStatus();
    }
  else if (event ==  vtkCommand::ProgressEvent) 
    {
    double progress = *((double *)callData);
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100*progress);
    }
  else if (event == vtkFourDAnalysisLogic::ProgressDialogEvent)
    {
    vtkFourDAnalysisLogic::StatusMessageType* smsg
      = (vtkFourDAnalysisLogic::StatusMessageType*)callData;
    if (smsg->show)
      {
      if (!this->ProgressDialog)
        {
        this->ProgressDialog = vtkKWProgressDialog::New();
        this->ProgressDialog->SetParent(this->GetApplicationGUI()->GetMainSlicerWindow());
        this->ProgressDialog->SetMasterWindow(this->GetApplicationGUI()->GetMainSlicerWindow());
        this->ProgressDialog->Create();
        }
      this->ProgressDialog->SetMessageText(smsg->message.c_str());
      this->ProgressDialog->UpdateProgress(smsg->progress);
      this->ProgressDialog->Display();
      }
    else
      {
      if (this->ProgressDialog)
        {
        this->ProgressDialog->SetParent(NULL);
        this->ProgressDialog->Delete();
        this->ProgressDialog = NULL;
        }
      }
    }

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  if (event == vtkMRMLScene::NodeAddedEvent)
    {
    UpdateSeriesSelectorMenus();
    //UpdateMaskSelectMenu();
    }
  else if (event == vtkMRMLScene::SceneCloseEvent)
    {
    }
  else if (event == vtkCommand::ModifiedEvent && 
           this->PlotManagerNode == vtkMRMLXYPlotManagerNode::SafeDownCast(caller))
    {
    UpdatePlotList();
    }
  /*
  else if (event == vtkMRMLVolumeNode::ImageDataModifiedEvent)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
    }
  */
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "FourDAnalysis", "FourDAnalysis", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForActiveBundleSelectorFrame();
  BuildGUIForFrameControlFrame(0);
  BuildGUIForFunctionViewer(0);
  BuildGUIForScriptSetting(0);
  BuildGUIForCurveFitting(0);
  BuildGUIForMapGenerator(0);

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForActiveBundleSelectorFrame ()
{
  //vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDAnalysis");

  //  Volume to select
  this->Active4DBundleSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->Active4DBundleSelectorWidget->SetParent(page);
  this->Active4DBundleSelectorWidget->Create();
  this->Active4DBundleSelectorWidget->SetNodeClass("vtkMRMLTimeSeriesBundleNode", NULL, NULL, NULL);
  this->Active4DBundleSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->Active4DBundleSelectorWidget->SetBorderWidth(2);
  this->Active4DBundleSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->Active4DBundleSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->Active4DBundleSelectorWidget->SetLabelText( "Active 4D Bundle: ");
  this->Active4DBundleSelectorWidget->SetBalloonHelpString("Select a volume from the current scene.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->Active4DBundleSelectorWidget->GetWidgetName());

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForHelpFrame ()
{
  // ----------------------------------------------------------------
  // HELP FRAME         
  // ----------------------------------------------------------------

  // Define your help text here.
  const char *help = 
    "**The 4D Analysis Module** helps you to load, view and analyze a series of 3D images (4D image),"
    "such as perfusion MRI, DCE MRI, and fMRI. "
    "<a>http://www.slicer.org/slicerWiki/index.php/Modules:FourDAnalysis</a> for details.";
  const char *about =
    "This project is directed by Hiroto Hatabu, MD, PhD (BWH)."
    "The module is designed and implemented by Junichi Tokuda, PhD (BWH), under supports from"
    "Center for Pulmonary Functional Imaging at BWH, NCIGT and NA-MIC.";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "FourDAnalysis" );
  this->BuildHelpAndAboutFrame (page, help, about);

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForFrameControlFrame(int show)
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDAnalysis");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Frame Control");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Frame Control

  vtkKWFrameWithLabel *fframe = vtkKWFrameWithLabel::New();
  fframe->SetParent(conBrowsFrame->GetFrame());
  fframe->Create();
  fframe->SetLabelText ("Frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 fframe->GetWidgetName() );
  
  //vtkKWFrame *sframe = vtkKWFrame::New();
  //sframe->SetParent(fframe->GetFrame());
  //sframe->Create();
  //this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
  //               sframe->GetWidgetName() );

  vtkKWFrame *fgframe = vtkKWFrame::New();
  fgframe->SetParent(fframe->GetFrame());
  fgframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 fgframe->GetWidgetName() );

  this->ForegroundVolumeSelectorScale = vtkKWScaleWithEntry::New();
  this->ForegroundVolumeSelectorScale->SetParent(fgframe);
  this->ForegroundVolumeSelectorScale->Create();
  this->ForegroundVolumeSelectorScale->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionRight);
  this->ForegroundVolumeSelectorScale->SetOrientationToHorizontal();
  this->ForegroundVolumeSelectorScale->SetLabelText("FG");
  this->ForegroundVolumeSelectorScale->SetRange(0.0, 100.0);
  this->ForegroundVolumeSelectorScale->SetResolution(1.0);
  this->ForegroundVolumeSelectorScale->ExpandEntryOff();
  //this->ForegroundVolumeSelectorScale->SetWidth(30);

  this->Script("pack %s -side right -fill x -expand y -padx 2 -pady 2", 
               this->ForegroundVolumeSelectorScale->GetWidgetName());

  vtkKWFrame *bgframe = vtkKWFrame::New();
  bgframe->SetParent(fframe->GetFrame());
  bgframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 bgframe->GetWidgetName() );

  this->BackgroundVolumeSelectorScale = vtkKWScaleWithEntry::New();
  this->BackgroundVolumeSelectorScale->SetParent(bgframe);
  this->BackgroundVolumeSelectorScale->Create();
  this->BackgroundVolumeSelectorScale->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionRight);
  this->BackgroundVolumeSelectorScale->SetOrientationToHorizontal();
  this->BackgroundVolumeSelectorScale->SetLabelText("BG");
  this->BackgroundVolumeSelectorScale->SetRange(0.0, 100.0);
  this->BackgroundVolumeSelectorScale->SetResolution(1.0);
  this->BackgroundVolumeSelectorScale->ExpandEntryOff();
  //this->BackgroundVolumeSelectorScale->SetWidth(30);
  
  this->Script("pack %s -side right -fill x -expand y -padx 2 -pady 2", 
               this->BackgroundVolumeSelectorScale->GetWidgetName());

  vtkKWFrame *apframe = vtkKWFrame::New();
  apframe->SetParent(fframe->GetFrame());
  apframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 apframe->GetWidgetName() );
  
  // -----------------------------------------
  // Contrast control

  vtkKWFrameWithLabel *cframe = vtkKWFrameWithLabel::New();
  cframe->SetParent(conBrowsFrame->GetFrame());
  cframe->Create();
  cframe->SetLabelText ("Contrast");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 cframe->GetWidgetName() );

  vtkKWFrame *lwframe = vtkKWFrame::New();
  lwframe->SetParent(cframe->GetFrame());
  lwframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 lwframe->GetWidgetName() );

  vtkKWLabel *lwLabel = vtkKWLabel::New();
  lwLabel->SetParent(lwframe);
  lwLabel->Create();
  lwLabel->SetText("Window/Level: ");

  this->WindowLevelRange = vtkKWRange::New();
  this->WindowLevelRange->SetParent(lwframe);
  this->WindowLevelRange->Create();
  this->WindowLevelRange->SymmetricalInteractionOn();
  this->WindowLevelRange->EntriesVisibilityOff ();  
  this->WindowLevelRange->SetWholeRange(0.0, 1.0);
  /*
  this->WindowLevelRange->SetCommand(this, "ProcessWindowLevelCommand");
  this->WindowLevelRange->SetStartCommand(this, "ProcessWindowLevelStartCommand");
  */
  this->Script("pack %s %s -side left -anchor nw -expand yes -fill x -padx 2 -pady 2",
               lwLabel->GetWidgetName(),
               this->WindowLevelRange->GetWidgetName());

  vtkKWFrame *thframe = vtkKWFrame::New();
  thframe->SetParent(cframe->GetFrame());
  thframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 thframe->GetWidgetName() );

  vtkKWLabel *thLabel = vtkKWLabel::New();
  thLabel->SetParent(thframe);
  thLabel->Create();
  thLabel->SetText("Threashold:   ");

  this->ThresholdRange = vtkKWRange::New();
  this->ThresholdRange->SetParent(thframe);
  this->ThresholdRange->Create();
  this->ThresholdRange->SymmetricalInteractionOff();
  this->ThresholdRange->EntriesVisibilityOff ();
  this->ThresholdRange->SetWholeRange(0.0, 1.0);
  /*
  this->ThresholdRange->SetCommand(this, "ProcessThresholdCommand");
  this->ThresholdRange->SetStartCommand(this, "ProcessThresholdStartCommand");
  */
  this->Script("pack %s %s -side left -anchor w -expand y -fill x -padx 2 -pady 2", 
               thLabel->GetWidgetName(),
               this->ThresholdRange->GetWidgetName());


  // -----------------------------------------
  // Delete pointers

  conBrowsFrame->Delete();
  fframe->Delete();
  //sframe->Delete();

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForFunctionViewer(int show)
{

  std::cerr << "void vtkFourDAnalysisGUI::BuildGUIForFunctionViewer(int show)" << std::endl;

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDAnalysis");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Intensity Plot");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Mask selector frame
  
  vtkKWFrameWithLabel *msframe = vtkKWFrameWithLabel::New();
  msframe->SetParent(conBrowsFrame->GetFrame());
  msframe->Create();
  msframe->SetLabelText ("Plot curves");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 msframe->GetWidgetName() );

  vtkKWFrame* mframe = vtkKWFrame::New();
  mframe->SetParent(msframe->GetFrame());
  mframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 mframe->GetWidgetName() );


  this->MaskNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->MaskNodeSelector->SetParent(mframe);
  this->MaskNodeSelector->Create();
  this->MaskNodeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->MaskNodeSelector->SetNewNodeEnabled(0);
  this->MaskNodeSelector->SetMRMLScene(this->GetMRMLScene());
  this->MaskNodeSelector->SetBorderWidth(2);
  this->MaskNodeSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->MaskNodeSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->MaskNodeSelector->SetLabelText( "Mask: ");
  this->MaskNodeSelector->SetBalloonHelpString("Select a mask to specify regions of interest.");

  this->RunPlotButton = vtkKWPushButton::New();
  this->RunPlotButton->SetParent(mframe);
  this->RunPlotButton->Create();
  this->RunPlotButton->SetText ("Plot");
  this->RunPlotButton->SetWidth (4);

  this->SavePlotButton = vtkKWLoadSaveButtonWithLabel::New();
  this->SavePlotButton->SetParent(mframe);
  this->SavePlotButton->Create();
  this->SavePlotButton->SetWidth(50);
  this->SavePlotButton->GetWidget()->SetText ("Save");
  this->SavePlotButton->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();

  this->Script("pack %s %s %s -side left -fill x -expand y -anchor w -padx 2 -pady 2", 
               this->MaskNodeSelector->GetWidgetName(),
               this->RunPlotButton->GetWidgetName(),
               this->SavePlotButton->GetWidgetName());

  msframe->Delete();
  //menuLabel->Delete();

  
  // -----------------------------------------
  // Plot frame
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  frame->SetLabelText ("Intensity Plot");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 frame->GetWidgetName() );
  
  //this->IntensityPlot = vtkKWPlotGraph::New();
  this->IntensityPlot = vtkSlicerXYPlotWidget::New();
  this->IntensityPlot->SetParent(frame->GetFrame());
  this->IntensityPlot->Create();
  this->IntensityPlot->SetHeight(250);

  this->IntensityPlot->SetAxisLineColor(1.0, 1.0, 1.0);
  this->IntensityPlot->SetMRMLScene(this->GetMRMLScene());

  this->ErrorBarCheckButton = vtkKWCheckButtonWithLabel::New();
  this->ErrorBarCheckButton->SetParent(frame->GetFrame());
  this->ErrorBarCheckButton->Create();
  this->ErrorBarCheckButton->GetWidget()->SelectedStateOn();
  this->ErrorBarCheckButton->SetLabelText("Show SD");

  this->PlotList = vtkKWMultiColumnListWithScrollbars::New();
  this->PlotList->SetParent(frame->GetFrame());
  this->PlotList->Create();
  this->PlotList->GetWidget()->SetHeight(6);
  this->PlotList->GetWidget()->SetReliefToRidge();
  this->PlotList->GetWidget()->SetBorderWidth(1);
  //this->PlotList->GetWidget()->SetSelectionTypeToRow();
  this->PlotList->GetWidget()->SetSelectionTypeToCell();
  this->PlotList->GetWidget()->SetSelectionModeToSingle();
  this->PlotList->GetWidget()->MovableRowsOff();
  this->PlotList->GetWidget()->MovableColumnsOff();
  this->PlotList->GetWidget()->AddColumn("");
  this->PlotList->GetWidget()->AddColumn("Color");
  this->PlotList->GetWidget()->AddColumn("Curve name");
  this->PlotList->GetWidget()->AddColumn("MRML ID");

  // On/off column
  this->PlotList->GetWidget()->SetColumnEditWindowToCheckButton(COLUMN_VISIBLE);
  this->PlotList->GetWidget()->SetColumnStretchable(COLUMN_VISIBLE, 0);
  this->PlotList->GetWidget()->SetColumnResizable(COLUMN_VISIBLE, 0);
  this->PlotList->GetWidget()->SetColumnWidth(COLUMN_VISIBLE, 0);
  this->PlotList->GetWidget()->ColumnEditableOn(COLUMN_VISIBLE);
  this->PlotList->GetWidget()->SetColumnFormatCommandToEmptyOutput (COLUMN_VISIBLE);

  // Color column
  this->PlotList->GetWidget()->ColumnEditableOff(COLUMN_COLOR);
  this->PlotList->GetWidget()->SetColumnWidth(COLUMN_COLOR, 5);
  this->PlotList->GetWidget()->SetColumnResizable(COLUMN_COLOR, 0);

  // Curve name column
  this->PlotList->GetWidget()->ColumnEditableOn(COLUMN_NODE_NAME);
  this->PlotList->GetWidget()->SetColumnWidth(COLUMN_NODE_NAME, 15);
  this->PlotList->GetWidget()->SetColumnEditWindowToEntry(COLUMN_NODE_NAME);
  this->PlotList->GetWidget()->SetColumnAlignmentToLeft(COLUMN_NODE_NAME);
  this->PlotList->GetWidget()->SetColumnStretchable(COLUMN_NODE_NAME, 1);

  // MRML ID column
  this->PlotList->GetWidget()->ColumnEditableOff(COLUMN_MRML_ID);
  this->PlotList->GetWidget()->SetColumnWidth(COLUMN_MRML_ID, 15);
  this->PlotList->GetWidget()->SetColumnAlignmentToLeft(COLUMN_MRML_ID);
  this->PlotList->GetWidget()->SetColumnStretchable(COLUMN_MRML_ID, 1);

  
  this->Script("pack %s %s %s -side top -fill x -expand y -anchor w -padx 2 -pady 2", 
               this->IntensityPlot->GetWidgetName(),
               this->ErrorBarCheckButton->GetWidgetName(),
               this->PlotList->GetWidgetName());

  // Event handlers:
  this->PlotList->GetWidget()->SetCellUpdatedCommand(this, "UpdatePlotListElement");
  //this->PlotList->GetWidget()->SetRightClickCommand(this, "JumpSlicesCallback");
  //this->PlotList->GetWidget()->SetSelectionCommand( this, "CurveVisibilityToggle");

  frame->Delete();
  conBrowsFrame->Delete();

  std::cerr << "void vtkFourDAnalysisGUI::BuildGUIForFunctionViewer(int show) -- end" << std::endl;

}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForScriptSetting(int show)
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDAnalysis");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Script / Parameters");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Curve fitting Setting frame
  
  vtkKWFrameWithLabel *cframe = vtkKWFrameWithLabel::New();
  cframe->SetParent(conBrowsFrame->GetFrame());
  cframe->Create();
  cframe->SetLabelText ("Configurations / Initial Parameters");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 cframe->GetWidgetName() );

  vtkKWFrame *scriptframe = vtkKWFrame::New();
  scriptframe->SetParent(cframe->GetFrame());
  scriptframe->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                scriptframe->GetWidgetName() );

  vtkKWLabel *scriptLabel = vtkKWLabel::New();
  scriptLabel->SetParent(scriptframe);
  scriptLabel->Create();
  scriptLabel->SetText("Script:");

  this->CurveScriptSelectButton = vtkKWLoadSaveButtonWithLabel::New();
  this->CurveScriptSelectButton->SetParent(scriptframe);
  this->CurveScriptSelectButton->Create();
  //this->CurveScriptSelectButton->SetWidth(50);
  this->CurveScriptSelectButton->GetWidget()->SetText ("Script Path");
  this->CurveScriptSelectButton->GetWidget()->GetLoadSaveDialog()->SaveDialogOff();

  this->Script("pack %s -side right -fill x -expand y -padx 2 -pady 2", 
               this->CurveScriptSelectButton->GetWidgetName());
  this->Script("pack %s -side right -anchor w -padx 2 -pady 2", 
               scriptLabel->GetWidgetName());

  scriptLabel->Delete();
  scriptframe->Delete();

  this->CurveScriptMethodName = vtkKWEntryWithLabel::New();
  this->CurveScriptMethodName->SetParent(cframe->GetFrame());
  this->CurveScriptMethodName->Create();
  //this->CurveScriptMethodName->SetWidth(20);
  this->CurveScriptMethodName->SetLabelText("Method: ");
  this->CurveScriptMethodName->GetWidget()->ReadOnlyOn();
  this->CurveScriptMethodName->GetWidget()->SetValue("----");
  
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 this->CurveScriptMethodName->GetWidgetName() );


  vtkKWFrame *rangeframe = vtkKWFrame::New();
  rangeframe->SetParent(cframe->GetFrame());
  rangeframe->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                rangeframe->GetWidgetName() );

  vtkKWLabel *startLabel = vtkKWLabel::New();
  startLabel->SetParent(rangeframe);
  startLabel->Create();
  startLabel->SetText("From:");
  
  this->CurveFittingStartIndexSpinBox = vtkKWSpinBox::New();
  this->CurveFittingStartIndexSpinBox->SetParent(rangeframe);
  this->CurveFittingStartIndexSpinBox->Create();
  this->CurveFittingStartIndexSpinBox->SetWidth(3);

  vtkKWLabel *endLabel = vtkKWLabel::New();
  endLabel->SetParent(rangeframe);
  endLabel->Create();
  endLabel->SetText("to:");

  this->CurveFittingEndIndexSpinBox = vtkKWSpinBox::New();
  this->CurveFittingEndIndexSpinBox->SetParent(rangeframe);
  this->CurveFittingEndIndexSpinBox->Create();
  this->CurveFittingEndIndexSpinBox->SetWidth(3);

  this->Script("pack %s %s %s %s -side left -anchor w -anchor w -padx 2 -pady 2",
               startLabel->GetWidgetName(),
               this->CurveFittingStartIndexSpinBox->GetWidgetName(),
               endLabel->GetWidgetName(),
               this->CurveFittingEndIndexSpinBox->GetWidgetName());

  startLabel->Delete();
  endLabel->Delete();
  rangeframe->Delete();
  
  this->InitialParameterList = vtkKWMultiColumnListWithScrollbars::New();
  this->InitialParameterList->SetParent(cframe->GetFrame());
  this->InitialParameterList->Create();
  this->InitialParameterList->GetWidget()->SetHeight(10);
  this->InitialParameterList->GetWidget()->SetSelectionTypeToRow();
  this->InitialParameterList->GetWidget()->SetSelectionModeToSingle();
  this->InitialParameterList->GetWidget()->MovableRowsOff();
  this->InitialParameterList->GetWidget()->MovableColumnsOff();
  this->InitialParameterList->GetWidget()->AddColumn("Parameter name");
  this->InitialParameterList->GetWidget()->AddColumn("Initial value / curve index");
  this->InitialParameterList->GetWidget()->SetColumnWidth(0, 16);
  this->InitialParameterList->GetWidget()->SetColumnWidth(1, 20);
  this->InitialParameterList->GetWidget()->SetColumnAlignmentToLeft(1);
  this->InitialParameterList->GetWidget()->ColumnEditableOff(0);
  this->InitialParameterList->GetWidget()->ColumnEditableOn(1);
  this->InitialParameterList->GetWidget()->SetSelectionTypeToCell();

  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 this->InitialParameterList->GetWidgetName() );


  
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForCurveFitting(int show)
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDAnalysis");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Curve Fitting");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Curve fitting
  
  vtkKWFrameWithLabel *oframe = vtkKWFrameWithLabel::New();
  oframe->SetParent(conBrowsFrame->GetFrame());
  oframe->Create();
  oframe->SetLabelText ("Results");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 oframe->GetWidgetName() );

  vtkKWFrame *runframe = vtkKWFrame::New();
  runframe->SetParent(oframe->GetFrame());
  runframe->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                runframe->GetWidgetName() );


  vtkKWLabel *fittingLabelLabel = vtkKWLabel::New();
  fittingLabelLabel->SetParent(runframe);
  fittingLabelLabel->Create();
  fittingLabelLabel->SetText("Label:");

  this->FittingLabelMenu = vtkKWMenuButton::New();
  this->FittingLabelMenu->SetParent(runframe);
  this->FittingLabelMenu->Create();
  this->FittingLabelMenu->SetWidth(5);

  this->RunFittingButton = vtkKWPushButton::New();
  this->RunFittingButton->SetParent(runframe);
  this->RunFittingButton->Create();
  this->RunFittingButton->SetText ("Run");
  this->RunFittingButton->SetWidth (4);

  this->SaveFittedCurveButton = vtkKWLoadSaveButtonWithLabel::New();
  this->SaveFittedCurveButton->SetParent(runframe);
  this->SaveFittedCurveButton->Create();
  this->SaveFittedCurveButton->SetWidth(50);
  this->SaveFittedCurveButton->GetWidget()->SetText ("Save");
  this->SaveFittedCurveButton->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
  
  this->Script("pack %s %s %s %s -side left -fill x -expand y -anchor w -padx 2 -pady 2", 
               fittingLabelLabel->GetWidgetName(),
               this->FittingLabelMenu->GetWidgetName(),
               this->RunFittingButton->GetWidgetName(),
               this->SaveFittedCurveButton->GetWidgetName());

  this->ResultParameterList = vtkKWMultiColumnListWithScrollbars::New();
  this->ResultParameterList->SetParent(oframe->GetFrame());
  this->ResultParameterList->Create();
  this->ResultParameterList->GetWidget()->SetHeight(10);
  this->ResultParameterList->GetWidget()->SetSelectionTypeToRow();
  this->ResultParameterList->GetWidget()->SetSelectionModeToSingle();
  this->ResultParameterList->GetWidget()->MovableRowsOff();
  this->ResultParameterList->GetWidget()->MovableColumnsOff();
  this->ResultParameterList->GetWidget()->AddColumn("Parameter name");
  this->ResultParameterList->GetWidget()->AddColumn("Output value");
  this->ResultParameterList->GetWidget()->SetColumnWidth(0, 16);
  this->ResultParameterList->GetWidget()->SetColumnWidth(1, 20);
  this->ResultParameterList->GetWidget()->SetColumnAlignmentToLeft(1);
  this->ResultParameterList->GetWidget()->ColumnEditableOff(0);
  this->ResultParameterList->GetWidget()->ColumnEditableOff(1);
  this->ResultParameterList->GetWidget()->SetSelectionTypeToCell();

  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 this->ResultParameterList->GetWidgetName() );
  



  conBrowsFrame->Delete();
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::BuildGUIForMapGenerator(int show)
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDAnalysis");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Parameter Map");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  
  // -----------------------------------------
  // Map Generator

  vtkKWFrameWithLabel *dframe = vtkKWFrameWithLabel::New();
  dframe->SetParent(conBrowsFrame->GetFrame());
  dframe->Create();
  dframe->SetLabelText ("Configurations / Input parameters");
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                dframe->GetWidgetName() );

  // i range

  vtkKWFrame *iframe = vtkKWFrame::New();
  iframe->SetParent(dframe->GetFrame());
  iframe->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                iframe->GetWidgetName() );
  
  vtkKWLabel *ilabel1 = vtkKWLabel::New();
  ilabel1->SetParent(iframe);
  ilabel1->Create();
  ilabel1->SetText("i range: ");

  vtkKWLabel *ilabel2 = vtkKWLabel::New();
  ilabel2->SetParent(iframe);
  ilabel2->Create();
  ilabel2->SetText("min:");

  this->MapIMinSpinBox = vtkKWSpinBox::New();
  this->MapIMinSpinBox->SetParent(iframe);
  this->MapIMinSpinBox->Create();
  this->MapIMinSpinBox->SetWidth(3);
  this->MapIMinSpinBox->SetRange(0, 0);

  vtkKWLabel *ilabel3 = vtkKWLabel::New();
  ilabel3->SetParent(iframe);
  ilabel3->Create();
  ilabel3->SetText("max:");

  this->MapIMaxSpinBox = vtkKWSpinBox::New();
  this->MapIMaxSpinBox->SetParent(iframe);
  this->MapIMaxSpinBox->Create();
  this->MapIMaxSpinBox->SetWidth(3);
  this->MapIMaxSpinBox->SetRange(0, 0);

  this->Script("pack %s %s %s %s %s -side left -fill x -expand y -anchor w -padx 2 -pady 2",
               ilabel1->GetWidgetName(),
               ilabel2->GetWidgetName(),
               this->MapIMinSpinBox->GetWidgetName(),
               ilabel3->GetWidgetName(),
               this->MapIMaxSpinBox->GetWidgetName());

  iframe->Delete();
  ilabel1->Delete();
  ilabel2->Delete();
  ilabel3->Delete();


  // j range

  vtkKWFrame *jframe = vtkKWFrame::New();
  jframe->SetParent(dframe->GetFrame());
  jframe->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                jframe->GetWidgetName() );
  
  vtkKWLabel *jlabel1 = vtkKWLabel::New();
  jlabel1->SetParent(jframe);
  jlabel1->Create();
  jlabel1->SetText("j range: ");

  vtkKWLabel *jlabel2 = vtkKWLabel::New();
  jlabel2->SetParent(jframe);
  jlabel2->Create();
  jlabel2->SetText("min:");

  this->MapJMinSpinBox = vtkKWSpinBox::New();
  this->MapJMinSpinBox->SetParent(jframe);
  this->MapJMinSpinBox->Create();
  this->MapJMinSpinBox->SetWidth(3);
  this->MapJMinSpinBox->SetRange(0, 0);

  vtkKWLabel *jlabel3 = vtkKWLabel::New();
  jlabel3->SetParent(jframe);
  jlabel3->Create();
  jlabel3->SetText("max:");

  this->MapJMaxSpinBox = vtkKWSpinBox::New();
  this->MapJMaxSpinBox->SetParent(jframe);
  this->MapJMaxSpinBox->Create();
  this->MapJMaxSpinBox->SetWidth(3);
  this->MapJMaxSpinBox->SetRange(0, 0);
  
  this->Script("pack %s %s %s %s %s -side left -fill x -expand y -anchor w -padx 2 -pady 2",
               jlabel1->GetWidgetName(),
               jlabel2->GetWidgetName(),
               this->MapJMinSpinBox->GetWidgetName(),
               jlabel3->GetWidgetName(),
               this->MapJMaxSpinBox->GetWidgetName());

  jframe->Delete();
  jlabel1->Delete();
  jlabel2->Delete();
  jlabel3->Delete();


  // k range

  vtkKWFrame *kframe = vtkKWFrame::New();
  kframe->SetParent(dframe->GetFrame());
  kframe->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                kframe->GetWidgetName() );
  
  vtkKWLabel *klabel1 = vtkKWLabel::New();
  klabel1->SetParent(kframe);
  klabel1->Create();
  klabel1->SetText("k range: ");

  vtkKWLabel *klabel2 = vtkKWLabel::New();
  klabel2->SetParent(kframe);
  klabel2->Create();
  klabel2->SetText("min:");

  this->MapKMinSpinBox = vtkKWSpinBox::New();
  this->MapKMinSpinBox->SetParent(kframe);
  this->MapKMinSpinBox->Create();
  this->MapKMinSpinBox->SetWidth(3);
  this->MapKMinSpinBox->SetRange(0, 0);

  vtkKWLabel *klabel3 = vtkKWLabel::New();
  klabel3->SetParent(kframe);
  klabel3->Create();
  klabel3->SetText("max:");

  this->MapKMaxSpinBox = vtkKWSpinBox::New();
  this->MapKMaxSpinBox->SetParent(kframe);
  this->MapKMaxSpinBox->Create();
  this->MapKMaxSpinBox->SetWidth(3);
  this->MapKMaxSpinBox->SetRange(0, 0);

  this->Script("pack %s %s %s %s %s -side left -fill x -expand y -anchor w -padx 2 -pady 2",
               klabel1->GetWidgetName(),
               klabel2->GetWidgetName(),
               this->MapKMinSpinBox->GetWidgetName(),
               klabel3->GetWidgetName(),
               this->MapKMaxSpinBox->GetWidgetName());

  kframe->Delete();
  klabel1->Delete();
  klabel2->Delete();
  klabel3->Delete();

  vtkKWFrame *outputFrame = vtkKWFrame::New();
  outputFrame->SetParent(dframe->GetFrame());
  outputFrame->Create();
  this->Script ("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                outputFrame->GetWidgetName());

  vtkKWLabel *outputLabel = vtkKWLabel::New();
  outputLabel->SetParent(outputFrame);
  outputLabel->Create();
  outputLabel->SetText("Output prefix:");
  
  this->MapOutputVolumePrefixEntry = vtkKWEntry::New();
  this->MapOutputVolumePrefixEntry->SetParent(outputFrame);
  this->MapOutputVolumePrefixEntry->Create();
  //this->MapOutputVolumePrefixEntry->SetWidth(20);
  this->MapOutputVolumePrefixEntry->SetValue("ParameterMap");

  this->RunScriptButton = vtkKWPushButton::New();
  this->RunScriptButton->SetParent(outputFrame);
  this->RunScriptButton->Create();
  this->RunScriptButton->SetText ("Run");
  this->RunScriptButton->SetWidth (10);
  
  this->Script("pack %s %s %s -side left -fill x -expand y -anchor w -padx 2 -pady 2", 
               outputLabel->GetWidgetName(),
               this->MapOutputVolumePrefixEntry->GetWidgetName(),
               this->RunScriptButton->GetWidgetName());

  dframe->Delete();
  outputFrame->Delete();
  outputLabel->Delete();

  conBrowsFrame->Delete();

}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdateAll()
{
  
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::SelectActive4DBundle(vtkMRMLTimeSeriesBundleNode* bundleNode)
{
  if (bundleNode == NULL)
    {
    return;
    }

  int volume = (int)this->ForegroundVolumeSelectorScale->GetValue();

  // frame control
  int n = bundleNode->GetNumberOfFrames();
  this->ForegroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  this->BackgroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  bundleNode->SwitchDisplayBuffer(0, volume);
  bundleNode->SwitchDisplayBuffer(1, volume);
  
  // plot
  this->CurveFittingStartIndexSpinBox->SetRange(0, n-1);
  this->CurveFittingEndIndexSpinBox->SetRange(0, n-1);
  this->CurveFittingStartIndexSpinBox->SetValue(0);
  this->CurveFittingStartIndexSpinBox->Modified();
  this->CurveFittingEndIndexSpinBox->SetValue(n-1);
  this->CurveFittingEndIndexSpinBox->Modified();
  
  // set spin box ranges for 3D map generation and cropping
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(0));
  if (volumeNode)
    {
    vtkImageData* imageData = volumeNode->GetImageData();
    int* dimensions = imageData->GetDimensions();
    
    // Map
    this->MapIMinSpinBox->SetRange(0, dimensions[0]-1);
    this->MapIMinSpinBox->SetValue(0);
    this->MapIMaxSpinBox->SetRange(0, dimensions[0]-1);
    this->MapIMaxSpinBox->SetValue(dimensions[0]-1);
    this->MapJMinSpinBox->SetRange(0, dimensions[1]-1);
    this->MapJMinSpinBox->SetValue(0);
    this->MapJMaxSpinBox->SetRange(0, dimensions[1]-1);
    this->MapJMaxSpinBox->SetValue(dimensions[1]-1);
    this->MapKMinSpinBox->SetRange(0, dimensions[2]-1);
    this->MapKMinSpinBox->SetValue(0);
    this->MapKMaxSpinBox->SetRange(0, dimensions[2]-1);
    this->MapKMaxSpinBox->SetValue(dimensions[2]-1);
    
    }

  this->ForegroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  this->BackgroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  SetForeground(bundleNode->GetID(), 0);
  SetBackground(bundleNode->GetID(), 0);
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::SetForeground(const char* bundleID, int index)
{

  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  vtkMRMLTimeSeriesBundleNode* bundleNode 
    = vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bundleID));

  if (!bundleNode)
    {
    return;
    }

  vtkMRMLVolumeNode* volNode;
  //volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetFrameNode(index));
  bundleNode->SwitchDisplayBuffer(0, index);
  volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetDisplayBufferNode(0));

  if (volNode)
    {
    //std::cerr << "volume node name  = " <<  volNode->GetName() << std::endl;
    nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
    for ( i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      if ( cnode != NULL)
        {
        cnode->SetForegroundVolumeID(volNode->GetID());
        }
      }
    //SetWindowLevelForCurrentFrame();
    }
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::SetBackground(const char* bundleID, int index)
{
  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  vtkMRMLTimeSeriesBundleNode* bundleNode 
    = vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bundleID));

  if (!bundleNode)
    {
    return;
    }

  vtkMRMLVolumeNode* volNode;
  //volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetFrameNode(index));
  bundleNode->SwitchDisplayBuffer(1, index);
  volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetDisplayBufferNode(1));

  if (volNode)
    {
    //std::cerr << "node id = " << nodeID << std::endl;
    nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
    for ( i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      if ( cnode != NULL)
        {
        cnode->SetBackgroundVolumeID(volNode->GetID());
        }
      }
    //SetWindowLevelForCurrentFrame();
    }
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::SetWindowLevelForCurrentFrame()
{

  vtkMRMLSliceCompositeNode *cnode = 
    vtkMRMLSliceCompositeNode::SafeDownCast (this->GetMRMLScene()->GetNthNodeByClass (0, "vtkMRMLSliceCompositeNode"));
  if (cnode != NULL)
    {
    const char* fgNodeID = cnode->GetForegroundVolumeID();
    const char* bgNodeID = cnode->GetBackgroundVolumeID();
    if (fgNodeID)
      {
      vtkMRMLVolumeNode* fgNode =
        vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(fgNodeID));
      vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(fgNode->GetDisplayNode());
      if (displayNode)
        {
        //double r[2];
        //fgNode->GetImageData()->GetScalarRange(r);
        double lower = this->RangeLower;
        double upper = this->RangeUpper;
        double range = upper - lower;
        double thLower = lower + range * this->ThresholdLower;
        double thUpper = lower + range * this->ThresholdUpper;
        double window  = range * this->Window;
        double level   = lower + range * this->Level;
        displayNode->SetAutoWindowLevel(0);
        displayNode->SetAutoThreshold(0);
        double cThLower = displayNode->GetLowerThreshold();
        double cThUpper = displayNode->GetUpperThreshold();
        double cWindow  = displayNode->GetWindow();
        double cLevel   = displayNode->GetLevel();

        int m = 0;
        if (cThLower!=thLower) { displayNode->SetLowerThreshold(thLower); m = 1;}
        if (cThUpper!=thUpper) { displayNode->SetUpperThreshold(thUpper); m = 1;}
        if (cWindow!=window)   { displayNode->SetWindow(window); m = 1;}
        if (cLevel!=level)     { displayNode->SetLevel(level); m = 1;}
        if (m) { displayNode->UpdateImageDataPipeline(); }
        }
      }
    if (bgNodeID && strcmp(fgNodeID, bgNodeID) != 0)
      {
      vtkMRMLVolumeNode* bgNode =
        vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bgNodeID));
      vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(bgNode->GetDisplayNode());
      if (displayNode)
        {
        //double r[2];
        //bgNode->GetImageData()->GetScalarRange(r);
        double lower = this->RangeLower;
        double upper = this->RangeUpper;
        double range = upper - lower;
        double thLower = lower + range * this->ThresholdLower;
        double thUpper = lower + range * this->ThresholdUpper;
        double window  = range * this->Window;
        double level   = lower + range * this->Level;
        displayNode->SetAutoWindowLevel(0);
        displayNode->SetAutoThreshold(0);
        double cThLower = displayNode->GetLowerThreshold();
        double cThUpper = displayNode->GetUpperThreshold();
        double cWindow  = displayNode->GetWindow();
        double cLevel   = displayNode->GetLevel();

        int m = 0;
        if (cThLower!=thLower) { displayNode->SetLowerThreshold(thLower); m = 1;}
        if (cThUpper!=thUpper) { displayNode->SetUpperThreshold(thUpper); m = 1;}
        if (cWindow!=window)   { displayNode->SetWindow(window); m = 1;}
        if (cLevel!=level)     { displayNode->SetLevel(level); m = 1;}
        if (m) { displayNode->UpdateImageDataPipeline(); }
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdateSeriesSelectorMenus()
{

  // generate a list of 4D Bundles
  std::vector<vtkMRMLNode*> nodes;
  std::vector<std::string>  names;

  this->GetApplicationLogic()->GetMRMLScene()->GetNodesByClass("vtkMRMLTimeSeriesBundleNode", nodes);

  this->BundleNodeIDList.clear();
  names.clear();
  std::vector<vtkMRMLNode*>::iterator niter;
  for (niter = nodes.begin(); niter != nodes.end(); niter ++)
    {
    this->BundleNodeIDList.push_back((*niter)->GetID());
    names.push_back((*niter)->GetName());
    }

}

//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdatePlotList()
{
  std::cerr << "void vtkFourDAnalysisGUI::UpdatePlotList() begin" << std::endl;
  if (this->PlotList && this->PlotManagerNode && this->GetMRMLScene())
    {
    std::cerr << "void vtkFourDAnalysisGUI::UpdatePlotList()" << std::endl;

    // Obtain a list of plot nodes
    std::vector<vtkMRMLNode*> nodes;
    this->GetMRMLScene()->GetNodesByClass("vtkMRMLArrayPlotNode", nodes);

    this->PlotList->GetWidget()->DeleteAllRows();
    this->PlotList->GetWidget()->AddRows(nodes.size());
    
    for (int i = 0; i < nodes.size(); i ++)
      {
      vtkMRMLPlotNode* node = vtkMRMLPlotNode::SafeDownCast(nodes[i]);
      if (node)
        {
        double r, g, b;
        node->GetColor(r, g, b);
        int v = node->GetVisible();
        
        if (v > 0)
          {
          this->PlotList->GetWidget()->SetCellTextAsInt(i, COLUMN_VISIBLE, 1);
          this->PlotList->GetWidget()->SetCellWindowCommandToCheckButton(i, COLUMN_VISIBLE);
          }
        else
          {
          this->PlotList->GetWidget()->SetCellTextAsInt(i, COLUMN_VISIBLE, 0);
          this->PlotList->GetWidget()->SetCellWindowCommandToCheckButton(i, COLUMN_VISIBLE);
          }

        // First column: color panel
        this->PlotList->GetWidget()->SetCellSelectionBackgroundColor(i, COLUMN_COLOR, r, g, b);
        this->PlotList->GetWidget()->SetCellBackgroundColor(i, COLUMN_COLOR, r, g, b);

        // Second column: check button to turn on / off
        //this->PlotList->GetWidget()->SetCellEditWindowToCheckButton(i, 1);

        // Third column: curve name entry
        //this->PlotList->GetWidget()->SetCellEditWindowToEntry(i, 2);
        this->PlotList->GetWidget()->SetCellText(i, COLUMN_NODE_NAME, node->GetID());

        // Forth column: MRML node ID
        this->PlotList->GetWidget()->SetCellText(i, COLUMN_MRML_ID, node->GetID());
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdatePlotListElement(int row, int col, char * str)
{
  if (this->PlotManagerNode &&
      this->GetMRMLScene() &&
      (row >= 0) && (row < this->PlotList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->PlotList->GetWidget()->GetNumberOfColumns()))
    {
    const char* nodeID = this->PlotList->GetWidget()->GetCellText(row, COLUMN_MRML_ID);
    vtkMRMLPlotNode* pnode = vtkMRMLPlotNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
    if (pnode)
      {
      if (col == COLUMN_VISIBLE)
        {
        int v = this->PlotList->GetWidget()->GetCellTextAsInt(row, col);
        if (v == 0)
          {
          pnode->SetVisible(0);
          }
        else
          {
          pnode->SetVisible(1);
          }
        this->PlotManagerNode->Refresh();
        }
      else if (col == COLUMN_NODE_NAME)
        {
        const char* name = this->PlotList->GetWidget()->GetCellText(row, col);
        pnode->SetName(name);
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdateInitialParameterList(vtkMRMLCurveAnalysisNode* curveNode)
{
  if (this->InitialParameterList == NULL)
    {
    return;
    }

  // Adjust number of rows
  //int numRows = this->InitialParameterList->GetWidget()->GetNumberOfRows();

  vtkStringArray* paramNames      = curveNode->GetInitialParameterNameArray();
  vtkStringArray* inputParamNames = curveNode->GetConstantNameArray();
  vtkStringArray* inputDataNames  = curveNode->GetInputArrayNameArray();
  int numParameters      = paramNames->GetNumberOfTuples();
  int numInputParameters = inputParamNames->GetNumberOfTuples();
  int numInputCurves     = inputDataNames->GetNumberOfTuples();

  this->InitialParameterList->GetWidget()->DeleteAllRows();
  this->InitialParameterList->GetWidget()->AddRows(numParameters + numInputParameters + numInputCurves);

  char label[256];

  for (int i = 0; i < numParameters; i ++)
    {
    const char* name = paramNames->GetValue(i);
    double value     = curveNode->GetInitialParameter(name);
    sprintf(label, "[P] %s", name);
    this->InitialParameterList->GetWidget()->SetCellText(i, 0, label);
    this->InitialParameterList->GetWidget()->SetCellEditWindowToEntry(i, 1);
    this->InitialParameterList->GetWidget()->SetCellTextAsDouble(i, 1, value);
    }

  for (int i = 0; i < numInputParameters; i ++)
    {
    int row = i + numParameters;
    const char* name = inputParamNames->GetValue(i);
    double value     = curveNode->GetConstant(name);
    sprintf(label, "[I] %s", name);
    this->InitialParameterList->GetWidget()->SetCellText(row, 0, label);
    this->InitialParameterList->GetWidget()->SetCellEditWindowToEntry(row, 1);
    this->InitialParameterList->GetWidget()->SetCellTextAsDouble(row, 1, value);
    }

  for (int i = 0; i < numInputCurves; i ++)
    {
    int row = i + numParameters + numInputParameters;
    const char* name = inputDataNames->GetValue(i);
    sprintf(label, "[C] %s", name);
    this->InitialParameterList->GetWidget()->SetCellText(row, 0, label);
    this->InitialParameterList->GetWidget()->SetCellEditWindowToEntry(row, 1);
    //this->InitialParameterList->GetWidget()->SetCellEditWindowToSpinBox(row, 1);
    this->InitialParameterList->GetWidget()->SetCellTextAsInt(row, 1, 0);
    }

}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::GetInitialParametersAndInputCurves(vtkMRMLCurveAnalysisNode* curveNode,
                                                             int start, int end)
{
  if (this->InitialParameterList == NULL)
    {
    return;
    }

  vtkStringArray* paramNames      = curveNode->GetInitialParameterNameArray();
  vtkStringArray* inputParamNames = curveNode->GetConstantNameArray();
  vtkStringArray* inputDataNames  = curveNode->GetInputArrayNameArray();
  int numParameters      = paramNames->GetNumberOfTuples();
  int numInputParameters = inputParamNames->GetNumberOfTuples();
  int numInputCurves     = inputDataNames->GetNumberOfTuples();

  for (int i = 0; i < numParameters; i ++)
    {
    const char* name = paramNames->GetValue(i);
    double value = this->InitialParameterList->GetWidget()->GetCellTextAsDouble(i, 1);
    curveNode->SetInitialParameter(name, value);
    std::cerr << name << " = " << value << std::endl;
    }

  for (int i = 0; i < numInputParameters; i ++)
    {
    int row = i + numParameters;
    const char* name = inputParamNames->GetValue(i);
    double value = this->InitialParameterList->GetWidget()->GetCellTextAsDouble(row, 1);
    curveNode->SetConstant(name, value);
    std::cerr << name << " = " << value << std::endl;
    }

  for (int i = 0; i < numInputCurves; i ++)
    {
    int row = i + numParameters + numInputParameters;
    const char* name = inputDataNames->GetValue(i);
    int label = this->InitialParameterList->GetWidget()->GetCellTextAsInt(row, 1);
    
    std::cerr << "label = " << label << std::endl;;

    //vtkDoubleArray* curve = this->IntensityCurves->GetCurve(label);
    vtkMRMLDoubleArrayNode* anode = this->IntensityCurves->GetCurve(label);
    vtkDoubleArray* curve = anode->GetArray();

    if (curve)
      {
      vtkDoubleArray* inputCurve = vtkDoubleArray::New();
      inputCurve->SetNumberOfComponents( curve->GetNumberOfComponents() );
      int max   = curve->GetNumberOfTuples();
      
      if (start < 0)   start = 0;
      if (end >= max)  end   = max-1;
      if (start > end) start = end;
      for (int i = start; i <= end; i ++)
        {
        double* xy = curve->GetTuple(i);
        inputCurve->InsertNextTuple(xy);
        std::cerr << "input xy = " << xy[0] << ", " << xy[1] << std::endl;
        }
      curveNode->SetInputArray(name, inputCurve);
      }
    }

}

//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdateOutputParameterList(vtkMRMLCurveAnalysisNode* curveNode)
{
  if (this->ResultParameterList == NULL)
    {
    return;
    }

  // Adjust number of rows
  //int numRows = this->ResultParameterList->GetWidget()->GetNumberOfRows();

  vtkStringArray* paramNames = curveNode->GetOutputValueNameArray();
  int numParameters  = paramNames->GetNumberOfTuples();

  this->ResultParameterList->GetWidget()->DeleteAllRows();
  this->ResultParameterList->GetWidget()->AddRows(numParameters);

  for (int i = 0; i < numParameters; i ++)
    {
    const char* name = paramNames->GetValue(i);
    double value     = curveNode->GetOutputValue(name);
    this->ResultParameterList->GetWidget()->SetCellText(i, 0, name);
    this->ResultParameterList->GetWidget()->SetCellTextAsDouble(i, 1, value);
    }

}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::GeneratePlotNodes()
{
  
  this->IntensityCurves->Update();
  vtkMRMLScalarVolumeNode* node = this->IntensityCurves->GetMaskNode();
  if (node == NULL || !node->GetLabelMap())
    {
    return;
    }
  vtkMRMLVolumeDisplayNode* dnode = node->GetVolumeDisplayNode();
  if (dnode == NULL)
    {
    return;
    }
  vtkMRMLColorNode* cnode = dnode->GetColorNode();
  if (cnode == NULL)
    {
    return;
    }
  vtkLookupTable* lt = cnode->GetLookupTable();

  vtkIntArray* labels = this->IntensityCurves->GetLabelList();
  int n = labels->GetNumberOfTuples();

  //this->IntensityPlot->ClearPlot();

  for (int i = 0; i < n; i ++)
    {
    int label = labels->GetValue(i);
    vtkMRMLDoubleArrayNode* anode = this->IntensityCurves->GetCurve(label);
    vtkMRMLArrayPlotNode* cnode = vtkMRMLArrayPlotNode::New();

    this->GetMRMLScene()->AddNode(cnode);
    cnode->SetAndObserveArray(anode);
    cnode->SetErrorBar(1);
    int id = this->PlotManagerNode->AddPlotNode(cnode);

    double color[3];
    lt->GetColor(label, color);
    if (color[0] > 0.99 && color[1] > 0.99  && color[2] > 0.99)
      {
      // if the line color is white, change the color to black
      color[0] = 0.0;
      color[1] = 0.0;
      color[2] = 0.0;
      }
    cnode->SetColor(color[0], color[1], color[2]);
    }

  this->PlotManagerNode->SetAutoXRange(1);
  this->PlotManagerNode->SetAutoYRange(1);

  this->PlotManagerNode->SetXLabel("Time (s)");
  this->PlotManagerNode->Refresh();

  // Update FittingLabelMenu
  if (this->FittingLabelMenu)
    {
    this->FittingLabelMenu->GetMenu()->DeleteAllItems();
    for (int i = 0; i < n; i ++)
      {
      char str[256];
      sprintf(str, "%d", labels->GetValue(i));
      this->FittingLabelMenu->GetMenu()->AddRadioButton(str);
      }
    }
}


//----------------------------------------------------------------------------
void vtkFourDAnalysisGUI::UpdateIntensityPlotWithFittedCurve(vtkIntensityCurves* intensityCurves)
{
  this->IntensityCurves->Update();
  vtkMRMLScalarVolumeNode* node = this->IntensityCurves->GetMaskNode();
  if (node == NULL || !node->GetLabelMap())
    {
    return;
    }
  vtkMRMLVolumeDisplayNode* dnode = node->GetVolumeDisplayNode();
  if (dnode == NULL)
    {
    return;
    }
  vtkMRMLColorNode* cnode = dnode->GetColorNode();
  if (cnode == NULL)
    {
    return;
    }
  vtkLookupTable* lt = cnode->GetLookupTable();

  vtkIntArray* labels = this->IntensityCurves->GetLabelList();
  int n = labels->GetNumberOfTuples();

  //this->IntensityPlot->ClearPlot();
  this->PlotManagerNode->ClearPlotNodes();
  for (int i = 0; i < n; i ++)
    {
    int label = labels->GetValue(i);
    vtkMRMLDoubleArrayNode* anode = this->IntensityCurves->GetCurve(label);
    vtkMRMLArrayPlotNode* cnode = vtkMRMLArrayPlotNode::New();
    this->GetMRMLScene()->AddNode(cnode);
    cnode->SetAndObserveArray(anode);
    int id = this->PlotManagerNode->AddPlotNode(cnode);

    double color[3];
    lt->GetColor(label, color);
    if (color[0] > 0.99 && color[1] > 0.99  && color[2] > 0.99)
      {
      // if the line color is white, change the color to black
      color[0] = 0.0;
      color[1] = 0.0;
      color[2] = 0.0;
      }
    //this->PlotNode->SetColor(id, color[0], color[1], color[2]);
    cnode->SetColor(color[0], color[1], color[2]);
    }

  if (this->FittedCurveNode)
    {
    //int id = this->IntensityPlot->AddPlot(array, "Fitted");
    vtkMRMLArrayPlotNode* cnode = vtkMRMLArrayPlotNode::New();
    this->GetMRMLScene()->AddNode(cnode);
    cnode->SetAndObserveArray(this->FittedCurveNode);
    int id = this->PlotManagerNode->AddPlotNode(cnode);
    double color[3];
    color[0] = 1.0;
    color[1] = 0.0;
    color[2] = 0.0;
    cnode->SetColor(color[0], color[1], color[2]);
    }
  
  this->PlotManagerNode->SetAutoXRange(1);
  this->PlotManagerNode->SetAutoYRange(1);
  this->PlotManagerNode->Refresh();

}


