#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerVolumeFileHeaderWidget.h"
//MRML nodes
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
//KWwidgets
#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenu.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWTopLevel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumesGUI );
vtkCxxRevisionMacro (vtkSlicerVolumesGUI, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::vtkSlicerVolumesGUI ( )
  {
  this->Logic = NULL;
  this->VolumeNode = NULL;
  this->SelectedVolumeID = NULL;

  this->VolumeSelectorWidget = NULL;
  this->LoadVolumeButton = NULL;
  this->SaveVolumeButton = NULL;
  this->VolumeHeaderWidget = NULL;
  this->VolumeDisplayWidget = NULL;
  this->scalarVDW = NULL;
  this->labelVDW = NULL;
  this->dwiVDW = NULL;
  this->dtiVDW = NULL;

  this->HelpFrame = NULL;
  this->LoadFrame = NULL;
  this->DisplayFrame = NULL;
  this->InfoFrame = NULL;
  this->OptionFrame = NULL;
  this->SaveFrame = NULL;
  this->GradientFrame = NULL;

  this->ScalarDisplayFrame = NULL;
  this->LabelMapDisplayFrame = NULL;
  this->DWIDisplayFrame = NULL;
  this->DTIDisplayFrame = NULL;
  this->VolumeDisplayFrame = NULL;

  this->NameEntry = NULL;
  this->CenterImageMenu = NULL;
  this->LabelMapCheckButton = NULL;
  this->SingleFileCheckButton = NULL;
  this->UseCompressionCheckButton = NULL;
  this->ApplyButton=NULL;

  this->VolumeFileHeaderWidget = NULL;
  this->DiffusionEditorWidget = NULL;

  NACLabel = NULL;
  NAMICLabel = NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::~vtkSlicerVolumesGUI ( )
  {
  this->RemoveGUIObservers();

  if (this->SelectedVolumeID)
    {
    delete [] this->SelectedVolumeID;
    this->SelectedVolumeID = NULL;
    }
  if (this->LoadVolumeButton )
    {
    this->LoadVolumeButton->SetParent(NULL );
    this->LoadVolumeButton->Delete ( );
    }
  if (this->SaveVolumeButton )
    {
    this->SaveVolumeButton->SetParent(NULL );
    this->SaveVolumeButton->Delete ( );
    }
  if (this->VolumeFileHeaderWidget)
    {
    this->VolumeFileHeaderWidget->SetParent(NULL );
    this->VolumeFileHeaderWidget->Delete ( );
    }
  if (this->VolumeHeaderWidget)
    {
    this->VolumeHeaderWidget->SetParent(NULL );
    this->VolumeHeaderWidget->Delete ( );
    }
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->SetParent(NULL );
    this->VolumeSelectorWidget->Delete ( );
    }
  if (this->CenterImageMenu)
    {
    this->CenterImageMenu->SetParent(NULL );
    this->CenterImageMenu->Delete ( );
    }
  if (this->LabelMapCheckButton)
    {
    this->LabelMapCheckButton->SetParent(NULL );
    this->LabelMapCheckButton->Delete ( );
    }
  if (this->SingleFileCheckButton)
    {
    this->SingleFileCheckButton->SetParent(NULL );
    this->SingleFileCheckButton->Delete ( );
    }
  if (this->UseCompressionCheckButton)
    {
    this->UseCompressionCheckButton->SetParent(NULL );
    this->UseCompressionCheckButton->Delete ( );
    }
  if (this->ApplyButton)
    {
    this->ApplyButton->SetParent(NULL );
    this->ApplyButton->Delete ( );
    }
  if (this->NameEntry)
    {
    this->NameEntry->SetParent(NULL );
    this->NameEntry->Delete ( );
    }
  if (this->labelVDW)
    {
    this->labelVDW->SetParent(NULL );
    this->labelVDW->Delete ( );
    }
  if (this->scalarVDW)
    {
    this->scalarVDW->SetParent(NULL );
    this->scalarVDW->Delete ( );
    }
  if (this->dwiVDW)
    {
    this->dwiVDW->SetParent(NULL );
    this->dwiVDW->Delete ( );
    }
  if (this->dtiVDW) 
    {
    this->dtiVDW->SetParent(NULL );
    this->dtiVDW->Delete ( );
    }
  if (this->LabelMapDisplayFrame)
    {
    this->LabelMapDisplayFrame->SetParent(NULL );
    this->LabelMapDisplayFrame->Delete( );
    }
  if (this->ScalarDisplayFrame)
    {
    this->ScalarDisplayFrame->SetParent(NULL );
    this->ScalarDisplayFrame->Delete( );
    }
  if (this->DWIDisplayFrame)
    {
    this->DWIDisplayFrame->SetParent(NULL );
    this->DWIDisplayFrame->Delete( );
    }
  if (this->DTIDisplayFrame)
    {
    this->DTIDisplayFrame->SetParent(NULL );
    this->DTIDisplayFrame->Delete( );
    }
  if ( this->HelpFrame )
    {
    this->HelpFrame->SetParent (NULL);
    this->HelpFrame->Delete ( );
    this->HelpFrame = NULL;
    }
  if ( this->LoadFrame )
    {
    this->LoadFrame->SetParent ( NULL );
    this->LoadFrame->Delete ( );
    this->LoadFrame = NULL;
    }
  if ( this->SaveFrame )
    {
    this->SaveFrame->SetParent ( NULL );
    this->SaveFrame->Delete ( );
    this->SaveFrame = NULL;
    }
  if ( this->DisplayFrame )
    {
    this->DisplayFrame->SetParent ( NULL );
    this->DisplayFrame->Delete ( );
    this->DisplayFrame = NULL;
    }
  if ( this->GradientFrame )
    {
    this->GradientFrame->SetParent ( NULL );
    this->GradientFrame->Delete ( );
    this->GradientFrame = NULL;
    }
  if ( this->DiffusionEditorWidget )
    {
    this->DiffusionEditorWidget->SetParent ( NULL );
    this->DiffusionEditorWidget->Delete ( );
    this->DiffusionEditorWidget = NULL;
    }
  if ( this->InfoFrame )
    {
    this->InfoFrame->SetParent ( NULL );
    this->InfoFrame->Delete ( );
    this->InfoFrame = NULL;
    }
  if ( this->OptionFrame )
    {
    this->OptionFrame->SetParent ( NULL );
    this->OptionFrame->Delete ( );
    this->OptionFrame = NULL;
    }
  if ( this->NACLabel )
    {
    this->NACLabel->SetParent ( NULL );
    this->NACLabel->Delete();
    this->NACLabel = NULL;
    }
  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  if ( this->NCIGTLabel )
    {
    this->NCIGTLabel->SetParent ( NULL );
    this->NCIGTLabel->Delete();
    this->NCIGTLabel = NULL;
    }
  if ( this->BIRNLabel )
    {
    this->BIRNLabel->SetParent ( NULL );
    this->BIRNLabel->Delete();
    this->BIRNLabel = NULL;
    }

  this->Built = false;
  this->SetModuleLogic ( NULL );  
  vtkSetMRMLNodeMacro (this->VolumeNode, NULL );
  }


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::PrintSelf ( ostream& os, vtkIndent indent )
  {
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "SlicerVolumesGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "VolumeNode: " << this->GetVolumeNode ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
  os << indent << "HelpFrame: " << this->GetHelpFrame ( ) << "\n";
  os << indent << "LoadFrame: " << this->GetLoadFrame ( ) << "\n";
  os << indent << "DisplayFrame: " << this->GetDisplayFrame ( ) << "\n";    
  os << indent << "OptionFrame: " << this->GetOptionFrame ( ) << "\n";
  os << indent << "SaveFrame: " << this->GetSaveFrame ( ) << "\n";
  // print widgets?
  }


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveGUIObservers ( )
  {
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadVolumeButton)
    {
    this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveVolumeButton)
    {
    this->SaveVolumeButton->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ApplyButton)
    {
    this->ApplyButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->UseCompressionCheckButton)
    {
    this->UseCompressionCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  }


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddGUIObservers ( )
  {
  // Fill in
  // observer load volume button    
  this->VolumeSelectorWidget->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveVolumeButton->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UseCompressionCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  }


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData )
  {
  if (event == vtkKWCheckButton::SelectedStateChangedEvent && 
    this->UseCompressionCheckButton == vtkKWCheckButton::SafeDownCast(caller))
    {
    vtkMRMLVolumeNode *refNode = 
      vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    if (refNode != NULL)
      {
      if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
        {
        // set UI widgets for Archetype storage node
        vtkMRMLVolumeArchetypeStorageNode *snode = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(
          refNode->GetStorageNode());
        if (snode == NULL) 
          {
          snode = vtkMRMLVolumeArchetypeStorageNode::New();
          snode->SetScene(this->GetMRMLScene());
          this->GetMRMLScene()->AddNode(snode);
          refNode->SetAndObserveStorageNodeID(snode->GetID());
          snode->Delete();
          }
        snode->SetUseCompression(this->UseCompressionCheckButton->GetSelectedState());
        }
      else 
        {
        // set UI widgets for NRRD storage node
        vtkMRMLNRRDStorageNode *snode = vtkMRMLNRRDStorageNode::SafeDownCast(
          refNode->GetStorageNode());
        if (snode == NULL) 
          {
          snode = vtkMRMLNRRDStorageNode::New();
          snode->SetScene(this->GetMRMLScene());
          this->GetMRMLScene()->AddNode(snode);
          refNode->SetAndObserveStorageNodeID(snode->GetID());
          snode->Delete();
          }
        snode->SetUseCompression(this->UseCompressionCheckButton->GetSelectedState());
        }
      }
    }
  if (this->VolumeFileHeaderWidget == vtkSlicerVolumeFileHeaderWidget::SafeDownCast(caller) && 
    event == vtkSlicerVolumeFileHeaderWidget::FileHeaderOKEvent )
    {
    const char *fileName = this->LoadVolumeButton->GetWidget()->GetFileName();
    vtkKWMenuButton *mb = this->CenterImageMenu->GetWidget();
    int loadingOptions = 0;
    if ( !strcmp (mb->GetValue(), "Centered") )
      {
      loadingOptions += 2;
      }

    if ( this->LabelMapCheckButton->GetSelectedState() )
      {
      loadingOptions += 1;
      }

    if ( this->SingleFileCheckButton->GetSelectedState() )
      {
      loadingOptions += 4;
      }

    vtkSlicerVolumesLogic* volumeLogic = this->Logic;
    vtkMRMLVolumeHeaderlessStorageNode* snode = this->VolumeFileHeaderWidget->GetVolumeHeaderlessStorageNode();

    this->VolumeNode = volumeLogic->AddHeaderVolume( fileName, this->NameEntry->GetWidget()->GetValue(), 
      snode, loadingOptions );
    return;
    }
  if (this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
    const char * filename = this->LoadVolumeButton->GetWidget()->GetFileName();
    if (filename)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      this->NameEntry->GetWidget()->SetValue(name.c_str());
      }
    else
      {
      this->NameEntry->GetWidget()->SetValue("");
      }
    this->LoadVolumeButton->GetWidget()->SetText ("Select Volume File");
    }
  else if (this->ApplyButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadVolumeButton->GetWidget()->GetFileName();
    if ( fileName ) 
      {

      vtkKWMenuButton *mb = this->CenterImageMenu->GetWidget();

      int loadingOptions = 0;
      if ( !strcmp (mb->GetValue(), "Centered") )
        {
        loadingOptions += 2;
        }

      if ( this->LabelMapCheckButton->GetSelectedState() )
        {
        loadingOptions += 1;
        }

      if ( this->SingleFileCheckButton->GetSelectedState() )
        {
        loadingOptions += 4;
        }

      std::string fileString(fileName);
      for (unsigned int i = 0; i < fileString.length(); i++)
        {
        if (fileString[i] == '\\')
          {
          fileString[i] = '/';
          }
        }

      vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      volumeLogic->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

      vtkMRMLVolumeNode *volumeNode = NULL;
      std::string archetype( this->NameEntry->GetWidget()->GetValue() );
      volumeNode = volumeLogic->AddArchetypeVolume( fileString.c_str(), archetype.c_str(), loadingOptions );
      if ( volumeNode == NULL ) 
        {
        this->VolumeNode = NULL;
        this->VolumeFileHeaderWidget->Invoke();  

        if (this->VolumeNode == NULL) 
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->LoadFrame->GetFrame() );
          dialog->SetStyleToMessage();
          std::string msg = std::string("Unable to read volume file ") + std::string(fileName);
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          }
        else
          {
          volumeNode = this->VolumeNode;
          }
        }      
      this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      if (volumeNode)
        {
        if ( loadingOptions & 1 )   // volume loaded as a label map
          {
          this->ApplicationLogic->GetSelectionNode()->SetActiveLabelVolumeID( volumeNode->GetID() );
          } 
        else
          {
          this->ApplicationLogic->GetSelectionNode()->SetActiveVolumeID( volumeNode->GetID() );
          } 
        this->ApplicationLogic->PropagateVolumeSelection();
        this->VolumeSelectorWidget->SetSelected( volumeNode );
        if (this->VolumeDisplayWidget == NULL)
          {
          this->UpdateFramesFromMRML();
          }
        this->VolumeDisplayWidget->SetVolumeNode(volumeNode);
        }
      volumeLogic->RemoveObservers(vtkCommand::ProgressEvent,  this->GUICallbackCommand);
      }

    return;
    }
  else if (this->VolumeSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) &&
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLVolumeNode *volume = 
      vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());

    if (volume != NULL)
      {
      // Deactivate GradientsEditor, as it should only enabled when activenode is a DWI
      this->GradientFrame->EnabledOff();
      this->GradientFrame->SetAllowFrameToCollapse(0);
      this->GradientFrame->CollapseFrame();
      this->UpdateFramesFromMRML();
      }
    return;
    }
  else if (this->SaveVolumeButton->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
    const char * fileName = this->SaveVolumeButton->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      vtkMRMLVolumeNode *volNode = vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
      if ( !volumeLogic->SaveArchetypeVolume( fileName, volNode ))
        {
        // TODO: generate an error...
        }
      else
        {
        this->SaveVolumeButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        }
      }
    return;
    }
  } 


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData )
  {
  if (event ==  vtkCommand::ProgressEvent) 
    {
    double progress = *((double *)callData);
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100*progress);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData )
  {
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateModuleEventBindings ( )
  {
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ReleaseModuleEventBindings ( )
  {
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::Enter ( )
  {
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  this->UpdateFramesFromMRML();
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::Exit ( )
  {
  this->ReleaseModuleEventBindings();
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::UpdateFramesFromMRML()
  {

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  //vtkWarningMacro(<< "this->VolumeDisplayWidget: " << this->VolumeDisplayWidget->GetTclName() << "; " << this->VolumeDisplayWidget->GetWidgetName() << endl);

  vtkMRMLVolumeNode *refNode = 
    vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());


  // Update Display Widget according to the selected Volume Node
  vtkSlicerVolumeDisplayWidget *oldDisplayWidget = this->VolumeDisplayWidget;
  vtkKWFrame *oldFrame = this->VolumeDisplayFrame;

  int tearDown = 0;
  vtkKWFrame *frame = NULL;
  if (refNode != NULL)
    {
    if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
      {
      // set UI widgets for Archetype storage node
      vtkMRMLVolumeArchetypeStorageNode *snode = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(
        refNode->GetStorageNode());
      if (snode == NULL) 
        {
        snode = vtkMRMLVolumeArchetypeStorageNode::New();
        snode->SetScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(snode);
        refNode->SetAndObserveStorageNodeID(snode->GetID());
        snode->Delete();
        }
      this->UseCompressionCheckButton->SetSelectedState(snode->GetUseCompression());

      // update the load frame
      if (snode->GetCenterImage())
        {
        if (strcmp("Centered", this->CenterImageMenu->GetWidget()->GetValue ()) != 0)
          {
          this->CenterImageMenu->GetWidget()->SetValue ( "Centered" );
          }
        }
      else
        {
        if (strcmp("From File", this->CenterImageMenu->GetWidget()->GetValue ()) != 0)
          {
          this->CenterImageMenu->GetWidget()->SetValue ( "From File" );
          }
        }
      }
    else 
      {
      // set UI widgets for NRRD storage node
      vtkMRMLNRRDStorageNode *snode = vtkMRMLNRRDStorageNode::SafeDownCast(
        refNode->GetStorageNode());
      if (snode == NULL) 
        {
        snode = vtkMRMLNRRDStorageNode::New();
        snode->SetScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(snode);
        refNode->SetAndObserveStorageNodeID(snode->GetID());
        snode->Delete();
        }
      this->UseCompressionCheckButton->SetSelectedState(snode->GetUseCompression());
      // update the load frame
      if (snode->GetCenterImage())
        {
        if (strcmp("Centered", this->CenterImageMenu->GetWidget()->GetValue ()) != 0)
          {
          this->CenterImageMenu->GetWidget()->SetValue ( "Centered" );
          }
        }
      else
        {
        if (strcmp("From File", this->CenterImageMenu->GetWidget()->GetValue ()) != 0)
          {
          this->CenterImageMenu->GetWidget()->SetValue ( "From File" );
          }
        }
      }

    if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
      {
      vtkMRMLScalarVolumeNode *svol = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);
      if (!svol->GetLabelMap() && (this->VolumeDisplayWidget == NULL || this->VolumeDisplayWidget != scalarVDW))
        {
        if (this->VolumeDisplayWidget != NULL)
          {
          this->VolumeDisplayWidget->TearDownWidget();
          }
        tearDown = 1;
        this->CreateScalarDisplayWidget();
        this->VolumeDisplayWidget = this->scalarVDW;
        this->VolumeDisplayFrame = this->ScalarDisplayFrame;
        }
      else if (svol->GetLabelMap() && (this->VolumeDisplayWidget == NULL || this->VolumeDisplayWidget != labelVDW))
        {
        if (this->VolumeDisplayWidget != NULL)
          {
          this->VolumeDisplayWidget->TearDownWidget();
          }
        tearDown = 1;
        this->CreateLabelMapDisplayWidget();
        this->VolumeDisplayWidget = this->labelVDW;
        this->VolumeDisplayFrame = this->LabelMapDisplayFrame;
        }
      if (svol->GetLabelMap() != this->LabelMapCheckButton->GetSelectedState())
        {
        this->LabelMapCheckButton->SetSelectedState(svol->GetLabelMap());
        }
      }
    else if ( refNode->IsA("vtkMRMLVectorVolumeNode") ) 
      {
      /* TODO: 
      if (this->VolumeDisplayWidget != vectorVDW)
      {
      if (this->VolumeDisplayWidget != NULL)
      {
      this->VolumeDisplayWidget->TearDownWidget();
      }
      tearDown = 1;
      this->CreateVectorDisplayWidget();
      this->VolumeDisplayWidget = this->vectorVDW;
      this->VolumeDisplayFrame = this->VectorDisplayFrame;
      }
      */
      }
    else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
      {
      if (this->VolumeDisplayWidget == NULL || this->VolumeDisplayWidget != dwiVDW)
        {
        if (this->VolumeDisplayWidget != NULL)
          {
          this->VolumeDisplayWidget->TearDownWidget();
          }
        tearDown = 1;
        this->CreateDTIDisplayWidget();
        this->CreateDWIDisplayWidget();
        this->VolumeDisplayWidget = this->dwiVDW;
        this->VolumeDisplayFrame = this->DWIDisplayFrame;
        }
      // update the Gradient Editor with the new node
      this->GradientFrame->EnabledOn();
      this->GradientFrame->SetAllowFrameToCollapse(1);
      vtkMRMLDiffusionWeightedVolumeNode *dwiNode = 
        vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(refNode);
      this->DiffusionEditorWidget->UpdateWidget(dwiNode);
      }
    else if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
      { 
      if (this->VolumeDisplayWidget == NULL || this->VolumeDisplayWidget != dtiVDW)
        {
        if (this->VolumeDisplayWidget != NULL)
          {
          this->VolumeDisplayWidget->TearDownWidget();
          }
        tearDown = 1;
        this->CreateDTIDisplayWidget();
        this->VolumeDisplayWidget = this->dtiVDW;
        this->VolumeDisplayFrame = this->DTIDisplayFrame;
        }
      this->GradientFrame->EnabledOn();
      this->GradientFrame->SetAllowFrameToCollapse(1);
       vtkMRMLDiffusionTensorVolumeNode *dtiNode = 
        vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(refNode);
      this->DiffusionEditorWidget->UpdateWidget(dtiNode);
      }
    else 
      {
      vtkErrorMacro ( "unknown type " << refNode->GetClassName() );
      }
    }

  if (this->VolumeDisplayWidget != NULL && tearDown)
    {
    // Repack new GUI
    this->VolumeDisplayWidget->SetVolumeNode(refNode);
    // set the mrml scene before adding observers
    this->VolumeDisplayWidget->SetMRMLScene(this->GetMRMLScene());
    this->VolumeDisplayWidget->AddWidgetObservers();
    //this->VolumeDisplayWidget->UpdateWidgetFromMRML();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -before %s",
      this->VolumeDisplayFrame->GetWidgetName(), oldFrame->GetWidgetName());

    // Unpack old VolumeDisplayWidget
    this->Script("pack forget %s", oldFrame->GetWidgetName());
    }

  // Update Volume Header and Display Widget
  // TODO: this may not be needed once the parts above are doing the right things
  if (refNode != NULL)
    {
    this->VolumeHeaderWidget->SetVolumeNode(refNode);
    this->VolumeHeaderWidget->UpdateWidgetFromMRML();
    if (this->VolumeDisplayWidget != NULL)
      {
      this->VolumeDisplayWidget->SetVolumeNode(refNode);
      // TODO: this call shouldn't be necessary, the set volume node should
      // trigger a modified event that triggers update widget from mrml
      this->VolumeDisplayWidget->UpdateWidgetFromMRML();
      }
    const char * currentVolName = this->NameEntry->GetWidget()->GetValue();
    const char * volName = refNode->GetName();
    if (currentVolName && volName)
      {
      if (strcmp(currentVolName, volName) != 0)
        {
        this->NameEntry->GetWidget()->SetValue(volName);
        }       
      }
    }

  if(refNode == NULL)
    {
    // Deactivate GradientsEditor, as it should only enabled when ActiveVolumeNode is a DWI
    this->GradientFrame->EnabledOff();
    this->GradientFrame->SetAllowFrameToCollapse(0);
    this->GradientFrame->CollapseFrame();
    //delete nameEntry
    this->NameEntry->GetWidget()->SetValue(""); 
    }

  // update the image origin
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::TearDownGUI ( )
  {
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::BuildGUI ( )
  {

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Volumes", "Volumes", NULL );

  // Define your help text and build the help frame here.
  const char *help = "The Volumes Module loads, saves and adjusts display parameters of volume data. \nThe Diffusion Editor allows modifying parameters (gradients, bValues, measurement frame) of DWI data and provides a quick way to interpret them. For that it estimates a tensor and shows glyphs and tracts for visual exploration. Help for Diffusion Editor: <a>http://www.slicer.org/slicerWiki/index.php/Modules:Volumes:Diffusion_Editor-Documentation</a>";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. \n\nThe Diffusion Editor was developed by Kerstin Kessel.";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Volumes" );
  this->BuildHelpAndAboutFrame ( page, help, about );

  this->NACLabel = vtkKWLabel::New();
  this->NACLabel->SetParent ( this->GetLogoFrame() );
  this->NACLabel->Create();
  this->NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

  this->NAMICLabel = vtkKWLabel::New();
  this->NAMICLabel->SetParent ( this->GetLogoFrame() );
  this->NAMICLabel->Create();
  this->NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

  this->NCIGTLabel = vtkKWLabel::New();
  this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
  this->NCIGTLabel->Create();
  this->NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );

  this->BIRNLabel = vtkKWLabel::New();
  this->BIRNLabel->SetParent ( this->GetLogoFrame() );
  this->BIRNLabel->Create();
  this->BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );
  app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w",  this->NAMICLabel->GetWidgetName());
  app->Script ( "grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  

  // ---
  // LOAD FRAME            
  LoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->LoadFrame->SetParent(page);
  this->LoadFrame->Create();
  this->LoadFrame->SetLabelText("Load");
  this->LoadFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    this->LoadFrame->GetWidgetName(), page->GetWidgetName());

  // add a file browser 
  this->LoadVolumeButton = vtkKWLoadSaveButtonWithLabel::New ( );
  this->LoadVolumeButton->SetParent ( this->LoadFrame->GetFrame() );
  this->LoadVolumeButton->Create ( );
  this->LoadVolumeButton->SetWidth(20);
  this->LoadVolumeButton->GetWidget()->SetText ("Select Volume File");
  this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open Volume File");
  this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {Volume} {*} }");
  this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
  app->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->LoadVolumeButton->GetWidgetName());

  // volume name
  this->NameEntry = vtkKWEntryWithLabel::New();
  this->NameEntry->SetParent(this->LoadFrame->GetFrame());
  this->NameEntry->Create();
  this->NameEntry->SetWidth(20);
  this->NameEntry->SetLabelWidth(12);
  this->NameEntry->SetLabelText("Volume Name:");
  this->NameEntry->GetWidget()->SetValue ( "" );
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->NameEntry->GetWidgetName());

  // center image button
  this->CenterImageMenu = vtkKWMenuButtonWithLabel::New();
  this->CenterImageMenu->SetParent(this->LoadFrame->GetFrame());
  this->CenterImageMenu->Create();
  this->CenterImageMenu->SetWidth(20);
  this->CenterImageMenu->SetLabelWidth(12);
  this->CenterImageMenu->SetLabelText("Image Origin:");
  this->CenterImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "Centered");
  this->CenterImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "From File");
  this->CenterImageMenu->GetWidget()->SetValue ( "From File" );
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->CenterImageMenu->GetWidgetName());

  // label map button (is this a label map?)
  this->LabelMapCheckButton = vtkKWCheckButton::New();
  this->LabelMapCheckButton->SetParent(this->LoadFrame->GetFrame());
  this->LabelMapCheckButton->Create();
  this->LabelMapCheckButton->SelectedStateOff();
  this->LabelMapCheckButton->SetText("Label Map");
  this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
    this->LabelMapCheckButton->GetWidgetName());

  // Single File button (Do we load a file or a series?)
  this->SingleFileCheckButton = vtkKWCheckButton::New();
  this->SingleFileCheckButton->SetParent(this->LoadFrame->GetFrame());
  this->SingleFileCheckButton->Create();
  this->SingleFileCheckButton->SelectedStateOff();
  this->SingleFileCheckButton->SetText("Single File");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->SingleFileCheckButton->GetWidgetName());

  // Apply button
  this->ApplyButton = vtkKWPushButton::New();
  this->ApplyButton->SetParent(this->LoadFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth(20);
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->ApplyButton->GetWidgetName());

  //  Volume to select
  this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->VolumeSelectorWidget->SetParent(page);
  this->VolumeSelectorWidget->Create();
  this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
  this->VolumeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->VolumeSelectorWidget->SetBorderWidth(2);
  this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->VolumeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->VolumeSelectorWidget->SetLabelText( "Active Volume: ");
  this->VolumeSelectorWidget->SetBalloonHelpString("Select a volume from the current scene.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    this->VolumeSelectorWidget->GetWidgetName());

  // ---
  // DISPLAY FRAME            
  this->DisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
  this->DisplayFrame->SetParent ( page );
  this->DisplayFrame->Create ( );
  this->DisplayFrame->SetLabelText ("Display");
  this->DisplayFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    this->DisplayFrame->GetWidgetName(), page->GetWidgetName());

  // Create frame for each Volume type

  this->ScalarDisplayFrame = vtkKWFrame::New();
  this->ScalarDisplayFrame->SetParent( this->DisplayFrame->GetFrame() );
  this->ScalarDisplayFrame->Create( );
  this->Script( "pack %s -in %s",
    this->ScalarDisplayFrame->GetWidgetName(),this->DisplayFrame->GetFrame()->GetWidgetName());

  this->LabelMapDisplayFrame = vtkKWFrame::New();
  this->LabelMapDisplayFrame->SetParent( this->DisplayFrame->GetFrame() );
  this->LabelMapDisplayFrame->Create( );
  this->Script( "pack %s -in %s",
    this->LabelMapDisplayFrame->GetWidgetName(),this->DisplayFrame->GetFrame()->GetWidgetName());

  this->DWIDisplayFrame = vtkKWFrame::New();
  this->DWIDisplayFrame->SetParent( this->DisplayFrame->GetFrame() );
  this->DWIDisplayFrame->Create( );
  //this->Script( "pack %s -in %s",
  //             this->DWIDisplayFrame->GetWidgetName(),this->DisplayFrame->GetFrame()->GetWidgetName());

  this->DTIDisplayFrame = vtkKWFrame::New();
  this->DTIDisplayFrame->SetParent( this->DisplayFrame->GetFrame() );
  this->DTIDisplayFrame->Create( );

  // Assign a scalar display widget by default.
  this->CreateScalarDisplayWidget();
  this->VolumeDisplayFrame = this->ScalarDisplayFrame;
  this->VolumeDisplayWidget = this->scalarVDW;

  // ---
  // GradientEditor FRAME
  this->GradientFrame = vtkSlicerModuleCollapsibleFrame::New();
  this->GradientFrame->SetParent(page);
  this->GradientFrame->Create();
  this->GradientFrame->SetLabelText("Diffusion Editor");
  this->GradientFrame->CollapseFrame();
  this->GradientFrame->EnabledOff();
  this->GradientFrame->SetAllowFrameToCollapse(0);
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    this->GradientFrame->GetWidgetName(), page->GetWidgetName());

  this->DiffusionEditorWidget = vtkSlicerDiffusionEditorWidget::New();
  this->DiffusionEditorWidget->SetApplication((vtkSlicerApplication *)this->GetApplication());
  this->DiffusionEditorWidget->SetParent(this->GradientFrame->GetFrame());
  this->DiffusionEditorWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
  this->DiffusionEditorWidget->Create();
  this->DiffusionEditorWidget->AddWidgetObservers();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s", 
    this->DiffusionEditorWidget->GetWidgetName(), this->GradientFrame->GetFrame()->GetWidgetName());

  // ---
  // Info FRAME            
  InfoFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->InfoFrame->SetParent ( page );
  this->InfoFrame->Create ( );
  this->InfoFrame->SetLabelText ("Info");
  this->InfoFrame->CollapseFrame ( );
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s", 
    this->InfoFrame->GetWidgetName(), page->GetWidgetName());

  this->VolumeHeaderWidget = vtkSlicerVolumeHeaderWidget::New();
  this->VolumeHeaderWidget->AddNodeSelectorWidgetOff();
  this->VolumeHeaderWidget->SetMRMLScene(this->GetMRMLScene());
  this->VolumeHeaderWidget->SetParent(this->InfoFrame->GetFrame());
  this->VolumeHeaderWidget->Create();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    this->VolumeHeaderWidget->GetWidgetName(), this->InfoFrame->GetFrame()->GetWidgetName());

  // ---
  // OPTION FRAME
  OptionFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->OptionFrame->SetParent ( page );
  this->OptionFrame->Create ( );
  this->OptionFrame->SetLabelText ("Option");
  this->OptionFrame->CollapseFrame ( );

  // ---
  // Save FRAME            
  SaveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->SaveFrame->SetParent ( page );
  this->SaveFrame->Create ( );
  this->SaveFrame->SetLabelText ("Save");
  this->SaveFrame->CollapseFrame ( );
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    this->SaveFrame->GetWidgetName(), page->GetWidgetName());

  this->UseCompressionCheckButton = vtkKWCheckButton::New();
  this->UseCompressionCheckButton->SetParent(this->SaveFrame->GetFrame());
  this->UseCompressionCheckButton->Create();
  this->UseCompressionCheckButton->SelectedStateOn();
  this->UseCompressionCheckButton->SetText("Use Compression");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->UseCompressionCheckButton->GetWidgetName());

  this->SaveVolumeButton = vtkKWLoadSaveButton::New ( );
  this->SaveVolumeButton->SetParent ( this->SaveFrame->GetFrame() );
  this->SaveVolumeButton->Create();
  this->SaveVolumeButton->SetText("Save Volume");
  this->SaveVolumeButton->GetLoadSaveDialog()->SaveDialogOn();
  this->SaveVolumeButton->GetLoadSaveDialog()->SetFileTypes("{ {volume} {*.*} }");
  this->SaveVolumeButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
  app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
    this->SaveVolumeButton->GetWidgetName());

  this->VolumeFileHeaderWidget = vtkSlicerVolumeFileHeaderWidget::New();
  this->VolumeFileHeaderWidget->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow());
  this->VolumeFileHeaderWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
  this->VolumeFileHeaderWidget->AddObserver ( vtkSlicerVolumeFileHeaderWidget::FileHeaderOKEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->VolumeFileHeaderWidget->Create();  
  this->VolumeFileHeaderWidget->SetInfo("The file format does not match standard volume formats.\n If you want to read it as binary data, fill the header and press Read.\n Otherwise, press Cancel");

  this->ProcessGUIEvents (this->VolumeSelectorWidget,
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );

  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateScalarDisplayWidget ( )
  {
  if (this->scalarVDW == NULL)
    {
    this->scalarVDW = vtkSlicerScalarVolumeDisplayWidget::New ( );
    this->scalarVDW->SetParent( this->ScalarDisplayFrame );
    this->scalarVDW->SetMRMLScene(this->GetMRMLScene());
    this->scalarVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      scalarVDW->GetWidgetName(), this->ScalarDisplayFrame->GetWidgetName());
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateVectorDisplayWidget ( )
  {
  // TODO fill in
  }
//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateLabelMapDisplayWidget ( )
  {
  if (this->labelVDW == NULL)
    {
    this->labelVDW = vtkSlicerLabelMapVolumeDisplayWidget::New ( );
    this->labelVDW->SetParent( this->LabelMapDisplayFrame );
    this->labelVDW->SetMRMLScene(this->GetMRMLScene());
    this->labelVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      labelVDW->GetWidgetName(), this->LabelMapDisplayFrame->GetWidgetName());
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateDWIDisplayWidget ( )
  {
  if (this->dwiVDW == NULL)
    {
    this->dwiVDW = vtkSlicerDiffusionWeightedVolumeDisplayWidget::New( );
    this->dwiVDW->SetParent( this->DWIDisplayFrame );
    this->dwiVDW->SetMRMLScene(this->GetMRMLScene());
    this->dwiVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      dwiVDW->GetWidgetName(), this->DWIDisplayFrame->GetWidgetName());
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateDTIDisplayWidget ( )
  {
  if (this->dtiVDW == NULL)
    {
    this->dtiVDW = vtkSlicerDiffusionTensorVolumeDisplayWidget::New( );
    this->dtiVDW->SetParent( this->DTIDisplayFrame );
    this->dtiVDW->SetMRMLScene(this->GetMRMLScene());
    this->dtiVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      dtiVDW->GetWidgetName(), this->DTIDisplayFrame->GetWidgetName());    
    }
  }

