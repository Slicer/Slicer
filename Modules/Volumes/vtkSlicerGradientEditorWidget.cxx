#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerGUICollection.h"
#include "vtkCommandLineModuleGUI.h"
#include "vtkSlicerTractographyFiducialSeedingGUI.h"
#include "vtkTimerLog.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget.h"
#include "vtkSlicerTractographyDisplayGUI.h"
//MRML nodes
#include "vtkMRMLNode.h"
#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiducialListNode.h"
//logics
#include "vtkCommandLineModuleLogic.h"
#include "vtkSlicerTractographyFiducialSeedingLogic.h"
#include "vtkSlicerGradientEditorLogic.h"
//widgets
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithLabel.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerGradientEditorWidget.h"
#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkKWCheckButton.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerGradientEditorWidget);
vtkCxxRevisionMacro (vtkSlicerGradientEditorWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerGradientEditorWidget::vtkSlicerGradientEditorWidget(void)
  {
  this->Application = NULL;
  this->ActiveVolumeNode = NULL;
  this->MeasurementFrameWidget = NULL;
  this->GradientsWidget = NULL;
  this->RestoreButton = NULL;
  this->UndoButton = NULL;
  this->RedoButton = NULL;
  this->ButtonFrame = NULL;
  this->Logic = vtkSlicerGradientEditorLogic::New();
  //Testframe maybe as an extra widget later
  this->TestFrame = NULL;
  this->RunButton = NULL;
  this->FiducialSelector = NULL;
  this->RunFrame = NULL;
  this->DTISelector = NULL;
  this->ModifiedForNewTensor = 1;
  this->TensorNode = NULL;
  this->FiberNode = NULL;
  this->BaselineNode = NULL;
  this->MaskNode = NULL;
  this->ViewGlyphs = NULL;
  this->ViewTracts = NULL;
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
  if (this->DTISelector)
    {
    this->DTISelector->SetParent (NULL);
    this->DTISelector->Delete();
    this->DTISelector = NULL;
    }
  if (this->RunFrame)
    {
    this->RunFrame->SetParent (NULL);
    this->RunFrame->Delete();
    this->RunFrame = NULL;
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
  if (this->Application)
    {
    this->Application->Delete();
    this->Application = NULL;
    }
  if (this->TensorNode)
    {
    this->TensorNode->Delete();
    this->TensorNode = NULL;
    }
  if (this->FiberNode)
    {
    this->FiberNode->Delete();
    this->FiberNode = NULL;
    }
  if (this->BaselineNode)
    {
    this->BaselineNode->Delete();
    this->BaselineNode = NULL;
    }
  if (this->MaskNode)
    {
    this->MaskNode->Delete();
    this->MaskNode = NULL;
    }
  if (this->ViewGlyphs)
    {
    this->ViewGlyphs->Delete();
    this->ViewGlyphs = NULL;
    }
  if (this->ViewTracts)
    {
    this->ViewTracts->Delete();
    this->ViewTracts = NULL;
    }
  this->ModifiedForNewTensor = 0;
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::AddWidgetObservers ( )
  {    
  this->RunButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RestoreButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->UndoButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RedoButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MeasurementFrameWidget->AddObserver(vtkSlicerMeasurementFrameWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GradientsWidget->AddObserver(vtkSlicerGradientsWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->ViewGlyphs->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewTracts->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::RemoveWidgetObservers( )
  {
  this->RunButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RestoreButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->UndoButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RedoButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MeasurementFrameWidget->RemoveObservers(vtkSlicerMeasurementFrameWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GradientsWidget->RemoveObservers(vtkSlicerGradientsWidget::ChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewGlyphs->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewTracts->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
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

  //enable undo/restore button, when values were changed
  if((this->MeasurementFrameWidget == vtkSlicerMeasurementFrameWidget::SafeDownCast(caller) && 
    event == vtkSlicerMeasurementFrameWidget::ChangedEvent) ||( event == vtkSlicerGradientsWidget::ChangedEvent && 
    this->GradientsWidget == vtkSlicerGradientsWidget::SafeDownCast(caller)))
    {
    this->UndoButton->SetEnabled(1);
    this->RestoreButton->SetEnabled(1);
    this->RedoButton->SetEnabled(0);
    this->ModifiedForNewTensor = 1; //tensor has to be estimated newly
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
    this->ModifiedForNewTensor = 1; //tensor has to be estimated newly
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
      this->ModifiedForNewTensor = 1; //tensor has to be estimated newly
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
      this->ModifiedForNewTensor = 1; //tensor has to be estimated newly
      }
    }

  //run test
  else if (this->RunButton->GetWidget() == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    this->RunButton->SetEnabled(0);
    if(this->ModifiedForNewTensor)
      {
      // create a command line module node
      vtkMRMLCommandLineModuleNode *tensorCML = vtkMRMLCommandLineModuleNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLCommandLineModuleNode"));

      // set its name  
      tensorCML->SetModuleDescription("Diffusion Tensor Estimation");
      tensorCML->SetName("GradientEditor: Tensor Estimation");

      // set the parameters
      tensorCML->SetParameterAsString("estimationMethod", "Least Squares");
      tensorCML->SetParameterAsDouble("otsuOmegaThreshold",0.5);
      tensorCML->SetParameterAsBool("removeIslands", 0);
      tensorCML->SetParameterAsBool("applyMask", 0);
      tensorCML->SetParameterAsString("inputVolume", this->ActiveVolumeNode->GetID());

      // create the output nodes
      this->BaselineNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      this->BaselineNode->SetScene(this->GetMRMLScene());
      this->BaselineNode->SetName("GradientenEditor: Baseline Node");
      this->MRMLScene->AddNode(this->BaselineNode);

      this->MaskNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      this->MaskNode->SetScene(this->GetMRMLScene());
      this->MaskNode->SetName("GradientenEditor: Threshold Mask");
      this->MRMLScene->AddNode(this->MaskNode);

      if(this->TensorNode)
        {
        this->TensorNode->Delete(); //delete previous data to avoid leaks
        }
      this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLDiffusionTensorVolumeNode"));
      this->TensorNode->SetScene(this->GetMRMLScene());
      this->TensorNode->SetName("GradientenEditor: Tensor Node");
      this->MRMLScene->AddNode(this->TensorNode);

      this->TensorNode->SetBaselineNodeID(this->BaselineNode->GetID());
      this->TensorNode->SetMaskNodeID(this->MaskNode->GetID());

      // set output parameters
      tensorCML->SetParameterAsString("outputTensor", this->TensorNode->GetID());
      tensorCML->SetParameterAsString("outputBaseline", this->BaselineNode->GetID());
      tensorCML->SetParameterAsString("thresholdMask", this->MaskNode->GetID());

      //get the existing GUI of the "Diffusion Tensor Estimation Command Line Module" 
      vtkCommandLineModuleGUI *moduleGUI = vtkCommandLineModuleGUI::SafeDownCast(
        this->Application->GetModuleGUIByName("Diffusion Tensor Estimation"));
      moduleGUI->Enter();

      //set command line node to GUI an logic
      moduleGUI->SetCommandLineModuleNode( tensorCML);
      moduleGUI->GetLogic()->SetCommandLineModuleNode( tensorCML); //use the GUI's Logic to invoke the task

      //estimate tensors
      moduleGUI->GetLogic()->Apply( tensorCML);

      //clean up
      tensorCML->Delete();
      this->BaselineNode->Delete();
      this->MaskNode->Delete();
      this->ModifiedForNewTensor = 0;
      }
    this->DTISelector->SetSelected(this->TensorNode);
    this->RunButton->SetEnabled(1);
    }

  if (this->DTISelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->DTISelector->GetSelected() != NULL) 
    {
    this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->DTISelector->GetSelected());
    this->CreateTracts();
    }

  //view of glyphs
  if(this->ViewGlyphs == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //get the existing GUI of the "Volumes Module"
    vtkSlicerVolumesGUI *volumesGUI = vtkSlicerVolumesGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("Volumes"));
    volumesGUI->Enter(); 
    volumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetDiffusionTensorVolumeNode(this->TensorNode);
    volumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(this->ViewGlyphs->GetSelectedState());
    }

  //view of tracts
  if(this->ViewTracts == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //get the existing GUI of the "Tractography Display Module"
    vtkSlicerTractographyDisplayGUI *tractGUI = vtkSlicerTractographyDisplayGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("DisplayLoadSave"));
    tractGUI->Enter();
    if(this->ViewTracts->GetSelectedState())
      {
      //visibility of tracts on
      tractGUI->GetFiberBundleDisplayWidget()->SetTractVisibility(this->ViewTracts->GetSelectedState());
      this->CreateTracts(); //start tractography seeding      
      }
    else
      {
      //visibility of tracts off
      tractGUI->GetFiberBundleDisplayWidget()->SetTractVisibility(this->ViewTracts->GetSelectedState());  
      }
    }
  }

void vtkSlicerGradientEditorWidget::CreateTracts ( )
  {
  if(this->TensorNode == NULL) return;
  //wait untill ImageData is set in other thread
  if(this->TensorNode->GetImageData() == NULL)
    {
    this->Script ( "update idletasks" );
    this->Script ( "after 5 \"%s CreateTracts \"",  this->GetTclName() );
    }
  else
    {
    //get fiducial list
    vtkMRMLFiducialListNode* fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(
      this->FiducialSelector->GetSelected());

    //create new fiber node
    if(this->FiberNode == NULL)
      {
      this->FiberNode = vtkMRMLFiberBundleNode::New();
      this->FiberNode->SetScene(this->GetMRMLScene());
      this->FiberNode->SetName("GradientenEditor: Fiber Node");
      this->MRMLScene->AddNode(this->FiberNode);
      }

    //get the existing GUI of the "Tractography Fiducial Seeding Module"
    vtkSlicerTractographyFiducialSeedingGUI *moduleGUI = vtkSlicerTractographyFiducialSeedingGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("FiducialSeeding"));    
    moduleGUI->Enter(); 

    //set the selectors to my nodes
    moduleGUI->SetVolumeSelector(this->TensorNode);
    moduleGUI->SetFiducialSelector(fiducialListNode);
    moduleGUI->SetOutFiberSelector(this->FiberNode);

    //create tracts
    moduleGUI->CreateTracts();
    this->Application->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Done");

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
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode); //set ActiveVolumeNode
  // update all when the active node changes
  this->MeasurementFrameWidget->SetMRMLScene(this->GetMRMLScene());
  this->MeasurementFrameWidget->UpdateWidget(this->ActiveVolumeNode);
  this->GradientsWidget->SetMRMLScene(this->GetMRMLScene());
  this->GradientsWidget->UpdateWidget(this->ActiveVolumeNode);
  this->Logic->SetActiveVolumeNode(this->ActiveVolumeNode);
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::CreateWidget( )
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
  this->MeasurementFrameWidget->Create();
  this->MeasurementFrameWidget->AddWidgetObservers();
  this->MeasurementFrameWidget->SetLogic(this->Logic);
  this->Script("pack %s -side top -anchor n -fill x -padx 2 -pady 2", 
    this->MeasurementFrameWidget->GetWidgetName());

  //create gradient widget 
  this->GradientsWidget = vtkSlicerGradientsWidget::New();
  this->GradientsWidget->SetParent(this->GetParent());
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
  this->UndoButton->SetText("Undo");  
  this->UndoButton->Create();
  this->UndoButton->SetWidth(10);
  this->UndoButton->SetEnabled(0);
  this->UndoButton->SetBalloonHelpString("Undo the last change in measurement frame/gradient values.");

  //create redoButton
  this->RedoButton = vtkKWPushButton::New();
  this->RedoButton->SetParent(this->ButtonFrame);
  this->RedoButton->SetText("Redo");  
  this->RedoButton->Create();
  this->RedoButton->SetWidth(10);
  this->RedoButton->SetEnabled(0);
  this->RedoButton->SetBalloonHelpString("Redo the last change in measurement frame/gradient values.");

  //create restore  button
  this->RestoreButton = vtkKWPushButton::New();
  this->RestoreButton->SetParent(this->ButtonFrame);
  this->RestoreButton->Create();
  this->RestoreButton->SetText("Restore");
  this->RestoreButton->SetBalloonHelpString("All parameters are restored to original");
  this->RestoreButton->SetWidth(10);
  this->RestoreButton->SetEnabled(0);

  //pack restoreButton and undoButton
  this->Script("pack %s %s %s -side right -anchor ne -fill x -padx 4 -pady 2", 
    this->RestoreButton->GetWidgetName(),
    this->RedoButton->GetWidgetName(),
    this->UndoButton->GetWidgetName());

  //create test frame 
  this->TestFrame = vtkKWFrameWithLabel::New();
  this->TestFrame->SetParent(this->GetParent());
  this->TestFrame->Create();
  this->TestFrame->CollapseFrame();
  this->TestFrame->SetLabelText("Test (Tensor Estimation & Tractography Fiducial Seeding)");
  this->Script("pack %s -side top -anchor n -fill x -padx 2 -pady 4", 
    this->TestFrame->GetWidgetName());

  ////create frame for run button and fiducial list
  //this->RunFrame = vtkKWFrame::New();
  //this->RunFrame->SetParent(this->TestFrame->GetFrame());
  //this->RunFrame->Create();
  //this->Script("pack %s -side top -anchor ne -padx 2 -pady 2", 
  //  this->RunFrame->GetWidgetName());

  //create run button
  this->RunButton = vtkKWPushButtonWithLabel::New();
  this->RunButton->SetParent(this->TestFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetLabelText("Estimate new Tensor: ");
  this->RunButton->GetWidget()->SetText("Run");
  this->RunButton->GetWidget()->SetWidth(10);
  this->RunButton->SetBalloonHelpString("Run test by computing tensors and tractography seeding.");
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 4 ", 
    this->RunButton->GetWidgetName()); 


  //create dti selector
  this->DTISelector = vtkSlicerNodeSelectorWidget::New();
  this->DTISelector->SetNodeClass("vtkMRMLDiffusionTensorVolumeNode", NULL, NULL, NULL);
  this->DTISelector->SetParent(this->TestFrame->GetFrame());
  this->DTISelector->Create();
  this->DTISelector->SetMRMLScene(this->GetMRMLScene());
  this->DTISelector->UpdateMenu();
  this->DTISelector->SetLabelText("Display a DTI Volume: ");
  this->DTISelector->SetBalloonHelpString("Select a DTI volume from the current mrml scene and see its tracts.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 4 ", 
    this->DTISelector->GetWidgetName()); 

  this->ViewGlyphs = vtkKWCheckButton::New();
  this->ViewGlyphs->SetParent(this->TestFrame->GetFrame());
  this->ViewGlyphs->Create();
  this->ViewGlyphs->SetText("View Glyphs ");
  this->Script("pack %s -side top -anchor nw -padx 2 ", 
    this->ViewGlyphs->GetWidgetName());

  this->ViewTracts = vtkKWCheckButton::New();
  this->ViewTracts->SetParent(this->TestFrame->GetFrame());
  this->ViewTracts->Create();
  this->ViewTracts->SetText("View Tracts based on Fiducial List");
  this->Script("pack %s -side left -anchor nw -padx 2 ", 
    this->ViewTracts->GetWidgetName());

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
  this->FiducialSelector->SetWidth(20);
  this->FiducialSelector->SetLabelPositionToRight();
  this->FiducialSelector->SetBalloonHelpString("Set Fiducial List for tractography seeding.");

  this->Script("pack %s %s -side left -anchor ne -padx 2 -pady 2", 
    this->ViewTracts->GetWidgetName(),
    this->FiducialSelector->GetWidgetName());
  } 
