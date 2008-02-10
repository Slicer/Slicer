#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"

#include "vtkSlicerGradientEditorWidget.h"
#include "vtkSlicerGradientEditorLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerGradientEditorWidget);
vtkCxxRevisionMacro (vtkSlicerGradientEditorWidget, "$Revision: 1.0 $");
//---------------------------------------------------------------------------
vtkSlicerGradientEditorWidget::vtkSlicerGradientEditorWidget(void)
  {
  this->ActiveVolumeNode = NULL;
  this->OriginalNode = NULL;
  this->MeasurementFrameWidget = NULL;
  this->GradientsWidget = NULL;

  //Testframe maybe as an extra widget later
  this->TestFrame = NULL;
  this->RunButton = NULL;
  this->FiducialSelector = NULL;

  this->RestoreButton = NULL;
  this->UndoButton = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerGradientEditorWidget::~vtkSlicerGradientEditorWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
    }
  if (this->OriginalNode)
    {
    this->OriginalNode->Delete();
    this->OriginalNode = NULL;
    }
  if (this->MeasurementFrameWidget)
    {
    this->MeasurementFrameWidget->Delete();
    this->MeasurementFrameWidget = NULL;
    }
  if (this->GradientsWidget)
    {
    this->GradientsWidget->Delete();
    this->GradientsWidget = NULL;
    }
  if (this->TestFrame)
    {
    this->TestFrame->SetParent (NULL);
    this->TestFrame->Delete();
    this->TestFrame = NULL;
    }
  if (this->FiducialSelector)
    {
    this->FiducialSelector->SetParent (NULL);
    this->FiducialSelector->Delete();
    this->FiducialSelector = NULL;
    }
  if (this->RunButton)
    {
    this->RunButton->SetParent (NULL);
    this->RunButton->Delete();
    this->RunButton = NULL;
    }
  if (this->RestoreButton)
    {
    this->RestoreButton->SetParent (NULL);
    this->RestoreButton->Delete();
    this->RestoreButton = NULL;
    }
  if (this->UndoButton)
    {
    this->UndoButton->SetParent (NULL);
    this->UndoButton->Delete();
    this->UndoButton = NULL;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::AddWidgetObservers ( )
  {    
  this->RunButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RestoreButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand); 
  this->UndoButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  }
//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::RemoveWidgetObservers( )
  {
  this->RunButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RestoreButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->UndoButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerGradientEditorWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {
  //restore 
  if (this->RestoreButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    this->ActiveVolumeNode->Copy(this->OriginalNode);
    this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode);
    this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode);
    }

  //undo 
  if (this->UndoButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    //if there are copys in the undoStack restore to previous version
    if(this->MRMLScene->GetNumberOfUndoLevels() > 0)
      {
      this->MRMLScene->Undo();
      this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode);
      this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode);
      }
    }

  //run test
  if (this->RunButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    //TODO
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  if (dwiNode == NULL)
    {
    vtkErrorMacro(<< this->GetClassName() << ": dwiNode in UpdateWidget() is NULL");
    return;
    }
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode);
  this->OriginalNode = vtkMRMLDiffusionWeightedVolumeNode::New();
  this->OriginalNode->Copy(this->ActiveVolumeNode);
  this->GetMRMLScene()->ClearUndoStack();
  // update the measurement frame, gradients and bValues 
  // when the active node changes
  this->MeasurementFrameWidget->SetMRMLScene(this->GetMRMLScene());
  this->GradientsWidget->SetMRMLScene(this->GetMRMLScene());
  this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode);
  this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode);
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::CreateWidget( )
  {
  //check if already created
  if (this->IsCreated()){
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  //call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  //create measurementFrame widget
  this->MeasurementFrameWidget = vtkSlicerMeasurementFrameWidget::New();
  this->MeasurementFrameWidget->SetParent(this->GetParent());
  this->MeasurementFrameWidget->Create();
  this->MeasurementFrameWidget->AddWidgetObservers();
  this->Script("pack %s -side top -anchor nw -fill x -padx 1 -pady 2", 
    this->MeasurementFrameWidget->GetWidgetName());

  //create gradient widget 
  this->GradientsWidget = vtkSlicerGradientsWidget::New();
  this->GradientsWidget->SetParent(this->GetParent());
  this->GradientsWidget->Create();
  this->GradientsWidget->AddWidgetObservers();
  this->Script("pack %s -side top -anchor nw -fill both expand true -padx 1 -pady 2", 
    this->GradientsWidget->GetWidgetName());

  //create test frame 
  this->TestFrame = vtkKWFrameWithLabel::New();
  this->TestFrame->SetParent(this->GetParent());
  this->TestFrame->Create();
  this->TestFrame->SetLabelText("Test");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->TestFrame->GetWidgetName());

  //create run button
  this->RunButton = vtkKWPushButton::New();
  this->RunButton->SetParent(this->TestFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetText("Run");
  this->RunButton->SetWidth(7);

  //create fiducial list
  this->FiducialSelector = vtkSlicerNodeSelectorWidget::New();
  this->FiducialSelector->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
  this->FiducialSelector->SetNewNodeEnabled(0);
  this->FiducialSelector->NoneEnabledOn();
  this->FiducialSelector->SetShowHidden(1);
  this->FiducialSelector->SetParent(this->TestFrame->GetFrame());
  this->FiducialSelector->SetMRMLScene(this->GetMRMLScene());
  this->FiducialSelector->Create();  
  this->FiducialSelector->UpdateMenu();
  this->FiducialSelector->SetLabelText("Fiducial List:");

  this->Script("pack %s %s -side right -anchor ne -padx 3 -pady 2", 
    this->RunButton->GetWidgetName(),
    this->FiducialSelector->GetWidgetName());

  //create save button
  this->RestoreButton = vtkKWPushButton::New();
  this->RestoreButton->SetParent(this->GetParent());
  this->RestoreButton->Create();
  this->RestoreButton->SetText("Restore");
  this->RestoreButton->SetWidth(10);

  //create save button
  this->UndoButton = vtkKWPushButton::New();
  this->UndoButton->SetParent(this->GetParent());
  this->UndoButton->Create();
  this->UndoButton->SetText("Undo");
  this->UndoButton->SetWidth(10);

  this->Script("pack %s %s -side right -anchor ne -padx 6 -pady 2", 
    this->RestoreButton->GetWidgetName(),
    this->UndoButton->GetWidgetName());
  } 
