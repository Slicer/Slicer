#include "vtkObject.h"
#include "vtkObjectFactory.h"

//MRML nodes
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
//logics
#include "vtkSlicerDiffusionEditorLogic.h"
//widgets
#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkSlicerGradientsWidget.h"
#include "vtkSlicerDWITestingWidget.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerDiffusionEditorWidget.h"
#include "vtkSlicerMeasurementFrameWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDiffusionEditorWidget);
vtkCxxRevisionMacro (vtkSlicerDiffusionEditorWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerDiffusionEditorWidget::vtkSlicerDiffusionEditorWidget(void)
  {
  this->Application = NULL;
  this->ActiveVolumeNode = NULL;
  this->MeasurementFrameWidget = NULL;
  this->GradientsWidget = NULL;
  this->TestingWidget = NULL;
  this->Logic = vtkSlicerDiffusionEditorLogic::New();
  //buttons
  this->ButtonFrame = NULL;
  this->RestoreButton = NULL;
  this->UndoButton = NULL;
  this->RedoButton = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerDiffusionEditorWidget::~vtkSlicerDiffusionEditorWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    vtkSetMRMLNodeMacro(this->ActiveVolumeNode, NULL);
    }
  if (this->Application)
    {
    this->Application->Delete();
    this->Application = NULL;
    }
  if (this->Logic)
    {
    this->Logic->Delete();
    this->Logic = NULL;
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
  if (this->TestingWidget)
    {
    this->TestingWidget->Delete();
    this->TestingWidget = NULL;
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
  if (this->RedoButton)
    {
    this->RedoButton->SetParent (NULL);
    this->RedoButton->Delete();
    this->RedoButton = NULL;
    }
  if (this->ButtonFrame)
    {
    this->ButtonFrame->SetParent (NULL);
    this->ButtonFrame->Delete();
    this->ButtonFrame = NULL;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorWidget::AddWidgetObservers ( )
  {    
  this->RestoreButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->UndoButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RedoButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MeasurementFrameWidget->AddObserver(vtkSlicerMeasurementFrameWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GradientsWidget->AddObserver(vtkSlicerGradientsWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorWidget::RemoveWidgetObservers( )
  {
  this->RestoreButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->UndoButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RedoButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MeasurementFrameWidget->RemoveObservers(vtkSlicerMeasurementFrameWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GradientsWidget->RemoveObservers(vtkSlicerGradientsWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerDiffusionEditorWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {
  //enable undo/restore button, when values were changed
  if((this->MeasurementFrameWidget == vtkSlicerMeasurementFrameWidget::SafeDownCast(caller) && 
    event == vtkSlicerMeasurementFrameWidget::ChangedEvent) ||( event == vtkSlicerGradientsWidget::ChangedEvent && 
    this->GradientsWidget == vtkSlicerGradientsWidget::SafeDownCast(caller)))
    {
    this->UndoButton->SetEnabled(1);
    this->RestoreButton->SetEnabled(1);
    this->RedoButton->SetEnabled(0);
    }

  //restore to original
  else if (this->RestoreButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    this->Logic->Restore();
    this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode); //update GUI
    this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode); //update GUI
    this->RestoreButton->SetEnabled(0); //disable restoreButton until next change
    this->UndoButton->SetEnabled(0); //disable undoButton until next change
    this->RedoButton->SetEnabled(0);
    }

  //undo
  else if(event == vtkKWPushButton::InvokedEvent && this->UndoButton == vtkKWPushButton::SafeDownCast(caller))
    {
    //if there are more copys in the undoStack
    if(this->Logic->IsUndoable())
      {
      this->Logic->Undo();
      this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode); //update GUI
      this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode); //update GUI
      this->RedoButton->SetEnabled(1);
      //disable buttons, when no more copys are in the stack
      if(!this->Logic->IsUndoable())
        {
        this->UndoButton->SetEnabled(0); //disable undoButton until next change
        this->RestoreButton->SetEnabled(0); //disable restoreButton until next change
        }
      }
    }

  //redo
  else if(event == vtkKWPushButton::InvokedEvent && this->RedoButton == vtkKWPushButton::SafeDownCast(caller))
    {
    //if there are more copys in the undoStack
    if(this->Logic->IsRedoable())
      {
      this->Logic->Redo();
      this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode); //update GUI
      this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode); //update GUI
      this->UndoButton->SetEnabled(1);
      this->RestoreButton->SetEnabled(1);
      //disable buttons, when no more copys are in the stack
      if(!this->Logic->IsRedoable())
        {
        this->RedoButton->SetEnabled(0); //disable undoButton until next change
        }
      }
    }
  this->TestingWidget->SetModifiedForNewTensor(1);
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorWidget::UpdateWidget(vtkMRMLVolumeNode *node)
  {
  if (node == NULL)
    {
    vtkErrorMacro(<< this->GetClassName() << ": dwiNode in UpdateWidget() is NULL");
    return;
    }

  //set ActiveVolumeNode
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, node); 

  if(node->IsA("vtkMRMLDiffusionTensorVolumeNode"))
    {
    //gradients widget will be inaktiv
    this->GradientsWidget->SetStatus(0);
    }

  if(node->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
    {
    //gradients widget will be aktiv
    this->GradientsWidget->SetStatus(1);
    //update gradients widget
    this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode);
    }

  //update measurement frame
  this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode);
  //update testing widget
  this->TestingWidget->UpdateWidget(this->ActiveVolumeNode);
  //update editor logic
  this->Logic->SetActiveVolumeNode(this->ActiveVolumeNode);
  //undo/redo/restore are enabled at the beginning
  this->RedoButton->EnabledOff();
  this->UndoButton->EnabledOff();
  this->RestoreButton->EnabledOff();
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorWidget::CreateWidget( )
  {
  //check if already created
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  //call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  //create measurementFrame widget
  this->MeasurementFrameWidget = vtkSlicerMeasurementFrameWidget::New();
  this->MeasurementFrameWidget->SetParent(this->GetParent());  
  this->MeasurementFrameWidget->SetMRMLScene(this->GetMRMLScene());
  this->MeasurementFrameWidget->Create();
  this->MeasurementFrameWidget->AddWidgetObservers();
  this->MeasurementFrameWidget->SetLogic(this->Logic);
  this->Script("pack %s -side top -anchor n -fill x -padx 2 -pady 2", 
    this->MeasurementFrameWidget->GetWidgetName());

  //create gradient widget 
  this->GradientsWidget = vtkSlicerGradientsWidget::New();
  this->GradientsWidget->SetParent(this->GetParent());
  this->GradientsWidget->SetMRMLScene(this->GetMRMLScene());
  this->GradientsWidget->SetApplication(this->Application);
  this->GradientsWidget->Create();
  this->GradientsWidget->AddWidgetObservers();
  this->GradientsWidget->SetLogic(this->Logic);
  this->Script("pack %s -side top -anchor n -fill both -expand true -padx 2 -pady 2", 
    this->GradientsWidget->GetWidgetName());

  //create frame for undo, redo and restore button
  this->ButtonFrame = vtkKWFrame::New();
  this->ButtonFrame->SetParent(this->GetParent());
  this->ButtonFrame->Create();
  this->Script("pack %s -side top -anchor ne ", 
    this->ButtonFrame->GetWidgetName());

  //create undoButton
  this->UndoButton = vtkKWPushButton::New();
  this->UndoButton->SetParent(this->ButtonFrame);
  this->UndoButton->Create();
  this->UndoButton->SetWidth(10);
  this->UndoButton->SetText("Undo");
  this->UndoButton->SetBalloonHelpString("Undo the last change in measurement frame/gradient values.");

  //create redoButton
  this->RedoButton = vtkKWPushButton::New();
  this->RedoButton->SetParent(this->ButtonFrame);
  this->RedoButton->SetText("Redo");  
  this->RedoButton->Create();
  this->RedoButton->SetWidth(10);
  this->RedoButton->SetBalloonHelpString("Redo the last change in measurement frame/gradient values.");

  //create restoreButton
  this->RestoreButton = vtkKWPushButton::New();
  this->RestoreButton->SetParent(this->ButtonFrame);
  this->RestoreButton->Create();
  this->RestoreButton->SetText("Restore");
  this->RestoreButton->SetWidth(10);
  this->RestoreButton->SetBalloonHelpString("All parameters are restored to original");

  //pack restoreButton, redoButton, undoButton
  this->Script("pack %s %s %s -side right -anchor ne -fill x -padx 4 -pady 2", 
    this->RestoreButton->GetWidgetName(),
    this->RedoButton->GetWidgetName(),
    this->UndoButton->GetWidgetName());

  //create testing widget 
  this->TestingWidget = vtkSlicerDWITestingWidget::New();
  this->TestingWidget->SetParent(this->GetParent());
  this->TestingWidget->SetMRMLScene(this->GetMRMLScene());
  this->TestingWidget->SetApplication(this->Application);
  this->TestingWidget->Create();
  this->TestingWidget->AddWidgetObservers();
  this->Script("pack %s -side top -anchor n -fill both -expand true -padx 2 -pady 2", 
    this->TestingWidget->GetWidgetName());
  } 
