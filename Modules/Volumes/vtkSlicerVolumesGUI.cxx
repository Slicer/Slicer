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

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>
#include <vtkDirectory.h>

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
  this->VolumeHeaderWidget = NULL;
  this->VolumeDisplayWidget = NULL;
  this->scalarVDW = NULL;
  this->vectorVDW = NULL;
  this->labelVDW = NULL;
  this->dwiVDW = NULL;
  this->dtiVDW = NULL;

  this->HelpFrame = NULL;
  this->LoadFrame = NULL;
  this->DisplayFrame = NULL;
  this->InfoFrame = NULL;
  this->OptionFrame = NULL;
  this->GradientFrame = NULL;

  this->ScalarDisplayFrame = NULL;
  this->LabelMapDisplayFrame = NULL;
  this->VectorDisplayFrame = NULL;
  this->DWIDisplayFrame = NULL;
  this->DTIDisplayFrame = NULL;
  this->VolumeDisplayFrame = NULL;

  this->NameEntry = NULL;
  this->CenterImageMenu = NULL;
  this->OrientImageMenu = NULL;
  this->LabelMapCheckButton = NULL;
  this->SingleFileCheckButton = NULL;
  this->KeepAllCheckButton = NULL;
  this->ApplyButton=NULL;
  this->LoadPreviousButton = NULL;
  this->LoadNextButton = NULL;
  this->CineButton = NULL;

  this->VolumeFileHeaderWidget = NULL;
  this->DiffusionEditorWidget = NULL;

  // this->KeepAll = false;
  // this->LoadingOptions = 0;
  this->AllFileNames.clear();
  this->IndexCurrentFile = 0;

  NACLabel = NULL;
  NAMICLabel = NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;

  // Try to load supporting libraries dynamically.  This is needed
  // since the toplevel is a loadable module but the other libraries
  // didn't get loaded
  Tcl_Interp* interp = this->GetApplication()->GetMainInterp();
  if (interp)
    {
    Vtkslicervolumesmodulelogic_Init(interp);
    }
  else
    {
    vtkErrorMacro("Failed to obtain reference to application TCL interpreter");
    }
}

//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::~vtkSlicerVolumesGUI ( )
{
  this->RemoveGUIObservers();

  vtkSetAndObserveMRMLNodeMacro(this->VolumeNode, NULL);

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
  if (this->OrientImageMenu)
    {
    this->OrientImageMenu->SetParent(NULL );
    this->OrientImageMenu->Delete ( );
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
  if (this->KeepAllCheckButton)
    {
    this->KeepAllCheckButton->SetParent(NULL );
    this->KeepAllCheckButton->Delete ( );
    }
  if (this->ApplyButton)
    {
    this->ApplyButton->SetParent(NULL );
    this->ApplyButton->Delete ( );
    }
  if (this->LoadPreviousButton)
    {
    this->LoadPreviousButton->SetParent(NULL );
    this->LoadPreviousButton->Delete ( );
    }
  if (this->LoadNextButton)
    {
    this->LoadNextButton->SetParent(NULL );
    this->LoadNextButton->Delete ( );
    }
//   if (this->CineButton)
//     {
//     this->CineButton->SetParent(NULL );
//     this->CineButton->Delete ( );
//     }
  if (this->NameEntry)
    {
    this->NameEntry->SetParent(NULL );
    this->NameEntry->Delete ( );
    }
  if (this->labelVDW)
    {
    this->labelVDW->RemoveMRMLObservers();
    this->labelVDW->SetParent(NULL );
    this->labelVDW->Delete ( );
    }
  if (this->vectorVDW)
    {
    this->vectorVDW->RemoveMRMLObservers();
    this->vectorVDW->SetParent(NULL );
    this->vectorVDW->Delete ( );
    }
  if (this->scalarVDW)
    {
    this->scalarVDW->RemoveMRMLObservers();
    this->scalarVDW->SetParent(NULL );
    this->scalarVDW->Delete ( );
    }
  if (this->dwiVDW)
    {
    this->dwiVDW->RemoveMRMLObservers();
    this->dwiVDW->SetParent(NULL );
    this->dwiVDW->Delete ( );
    }
  if (this->dtiVDW) 
    {
    this->dtiVDW->RemoveMRMLObservers();
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
  if (this->VectorDisplayFrame)
    {
    this->VectorDisplayFrame->SetParent(NULL );
    this->VectorDisplayFrame->Delete( );
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
  this->SetLogic ( vtkObjectPointer (&this->Logic), NULL ); 
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

  if ( this->GetNumberOfItemsInDictionary() > 0 )
    {
    os << indent << "Items in meta data: " << this->GetNumberOfItemsInDictionary() << "\n"; 
    for (unsigned int k = 0; k < this->GetNumberOfItemsInDictionary(); k++)
      {
      os << indent << indent << k << ": Key: " << this->GetNthKey(k); 
      os << " -> Value: " << this->GetNthValue(k) << "\n"; 
      }
    }
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
  if (this->ApplyButton)
    {
    this->ApplyButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }  
  if (this->LoadPreviousButton)
    {
    this->LoadPreviousButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }  
  if (this->LoadNextButton)
    {
    this->LoadNextButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }  
//   if (this->CineButton)
//     {
//     this->CineButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//     }  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddGUIObservers ( )
{
  // Fill in
  // observer load volume button    
  this->VolumeSelectorWidget->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadPreviousButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadNextButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//   this->CineButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  int LoadingOptions = 0;
  bool KeepAll = false;

  if (this->VolumeFileHeaderWidget == vtkSlicerVolumeFileHeaderWidget::SafeDownCast(caller) && 
      event == vtkSlicerVolumeFileHeaderWidget::FileHeaderOKEvent )
    {
    const char *fileName = this->LoadVolumeButton->GetWidget()->GetFileName();

    this->CheckLoadingOptions( LoadingOptions, KeepAll );

    vtkSlicerVolumesLogic* volumeLogic = this->Logic;
    vtkMRMLVolumeHeaderlessStorageNode* snode = this->VolumeFileHeaderWidget->GetVolumeHeaderlessStorageNode();

    this->VolumeNode = volumeLogic->AddHeaderVolume( fileName, this->NameEntry->GetWidget()->GetValue(), 
                                                     snode, LoadingOptions );
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
      // get dicom header (or meta data) from the selected file
      vtkITKArchetypeImageSeriesReader* reader = vtkITKArchetypeImageSeriesReader::New();
      reader->SetSingleFile( 1 );
      reader->SetArchetype( filename );
      try 
        {
        reader->Update();
        this->CopyTagAndValues( reader );
        }
      catch (vtkstd::exception &e) 
        {
        e=e; // dummy access to avoid warning
        }
      reader->Delete();
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

      this->CheckLoadingOptions( LoadingOptions, KeepAll );
      std::string fileString(fileName);
      for (unsigned int i = 0; i < fileString.length(); i++)
        {
        if (fileString[i] == '\\')
          {
          fileString[i] = '/';
          }
        }

      // let's see how many 'similar' images are there in the directory
      this->AllFileNames.clear();
      this->IndexCurrentFile = 0;

      std::string pathString =  vtksys::SystemTools::GetFilenamePath( fileString );
      std::string extName = vtksys::SystemTools::GetFilenameLastExtension( fileString );
            
      vtkDirectory* directory = vtkDirectory::New();
      directory->Open( pathString.c_str() );
      unsigned int numFiles = directory->GetNumberOfFiles();
      for (unsigned int i = 0; i < numFiles; i++)
        {
          std::string aFilename = pathString;
          aFilename += '/';
          aFilename += directory->GetFile( i );

          std::string ext = vtksys::SystemTools::GetFilenameLastExtension( aFilename );
          if (ext == extName)
            {
            this->AllFileNames.push_back( aFilename );
            }
        }
      
      for (unsigned int i = 0; i < this->AllFileNames.size(); i++)
        {
        if ( this->AllFileNames[i] == fileString )
          {
          this->IndexCurrentFile = i;
          }
        }
      directory->Delete();

      vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      volumeLogic->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

      vtkMRMLVolumeNode *volumeNode = NULL;
      std::string archetype( this->NameEntry->GetWidget()->GetValue() );
      volumeNode = volumeLogic->AddArchetypeVolume( fileString.c_str(), archetype.c_str(), LoadingOptions );
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
        if ( LoadingOptions & 1 )   // volume loaded as a label map
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
  else if (this->LoadPreviousButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    // If there is at least one file to load
    if ( this->AllFileNames.size() != 0 ) 
      {
      this->CheckLoadingOptions( LoadingOptions, KeepAll );
      this->IndexCurrentFile -= 1;
      if ( this->IndexCurrentFile == -1 )
        {
        this->IndexCurrentFile = this->AllFileNames.size()-1;
        }

      vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      volumeLogic->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

      vtkSmartPointer<vtkMRMLVolumeNode> volumeNode = 0;
      // delete current node if told to do so
      if ( !KeepAll )
        {
        volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
        // this->GetMRMLScene()->RemoveNode(volumeNode);
        std::ostringstream idle;
        idle << "after idle [list after 0 \"$::slicer3::MRMLScene RemoveNode [$::slicer3::MRMLScene GetNodeByID " << volumeNode->GetID() << "]\"]";
        //std::cout << idle.str() << std::endl;
        this->Script(idle.str().c_str());
        volumeNode = vtkSmartPointer<vtkMRMLVolumeNode>::New(); // so we don't leak this new instance
        }

      std::string currentFileName = this->AllFileNames[ this->IndexCurrentFile ];
      std::string archetype = vtksys::SystemTools::GetFilenameName ( currentFileName );

      volumeNode = volumeLogic->AddArchetypeVolume( currentFileName.c_str(), archetype.c_str(), LoadingOptions );
      if ( volumeNode == NULL ) 
        {
        this->VolumeNode = NULL;
        this->VolumeFileHeaderWidget->Invoke();  

        if (this->VolumeNode == NULL) 
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->LoadFrame->GetFrame() );
          dialog->SetStyleToMessage();
          std::string msg = std::string("Unable to read volume file ") + std::string(currentFileName);
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
        if ( LoadingOptions & 1 )   // volume loaded as a label map
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
  else if (this->LoadNextButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    // If there is at least one file to load
    if ( this->AllFileNames.size() != 0 ) 
      {
      this->CheckLoadingOptions( LoadingOptions, KeepAll );
      this->IndexCurrentFile += 1;
      if ( this->IndexCurrentFile == static_cast<int>(this->AllFileNames.size()) )
        {
        this->IndexCurrentFile = 0;
        }

      vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      volumeLogic->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

      vtkSmartPointer<vtkMRMLVolumeNode> volumeNode = 0;
      // delete current node if told to do so
      if ( !KeepAll )
        {
        volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
        // this->GetMRMLScene()->RemoveNode(volumeNode);
        std::ostringstream idle;
        idle << "after idle [list after 0 \"$::slicer3::MRMLScene RemoveNode [$::slicer3::MRMLScene GetNodeByID " << volumeNode->GetID() << "]\"]";
        //std::cout << idle.str() << std::endl;
        this->Script(idle.str().c_str());
        volumeNode = vtkSmartPointer<vtkMRMLVolumeNode>::New(); // so we don't leak this new instance
        }

      std::string currentFileName = this->AllFileNames[ this->IndexCurrentFile ];
      std::string archetype = vtksys::SystemTools::GetFilenameName ( currentFileName );
 
      volumeNode = volumeLogic->AddArchetypeVolume( currentFileName.c_str(), archetype.c_str(), LoadingOptions );
      if ( volumeNode == NULL ) 
        {
        this->VolumeNode = NULL;
        this->VolumeFileHeaderWidget->Invoke();  

        if (this->VolumeNode == NULL) 
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->LoadFrame->GetFrame() );
          dialog->SetStyleToMessage();
          std::string msg = std::string("Unable to read volume file ") + std::string(currentFileName);
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
        if ( LoadingOptions & 1 )   // volume loaded as a label map
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

    vtkSetAndObserveMRMLNodeMacro(this->VolumeNode, volume);

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
} 


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessLogicEvents(vtkObject *vtkNotUsed(caller),
                                             unsigned long event,
                                             void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    double progress = *((double *)callData);
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100*progress);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                            unsigned long vtkNotUsed(event),
                                            void *vtkNotUsed(callData))
{
  this->GradientFrame->EnabledOff();
  this->GradientFrame->SetAllowFrameToCollapse(0);
  this->GradientFrame->CollapseFrame();

  this->UpdateFramesFromMRML();

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
void vtkSlicerVolumesGUI::Enter ( vtkMRMLNode *node )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }

  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  if ( volumeNode )
    {
    this->VolumeSelectorWidget->UpdateMenu();
    this->VolumeSelectorWidget->SetSelected( volumeNode );
    }

  // Populates menu and adds color icons that match existing LUTS.
  if ( this->VolumeDisplayWidget )
    {
    this->VolumeDisplayWidget->UpdateWidgetFromMRML();
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

  vtkMRMLVolumeNode *refNode = 
    vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());


  // Update Display Widget according to the selected Volume Node
  vtkKWFrame *oldFrame = this->VolumeDisplayFrame;

  int tearDown = 0;
  if (refNode != NULL)
    {
    if ( !strcmp(refNode->GetClassName(), "vtkMRMLScalarVolumeNode") || !strcmp(refNode->GetClassName(), "vtkMRMLVectorVolumeNode"))
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

    if ( !strcmp(refNode->GetClassName(), "vtkMRMLScalarVolumeNode"))
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
    else if ( !strcmp(refNode->GetClassName(), "vtkMRMLVectorVolumeNode"))
      {
      if (this->VolumeDisplayWidget == NULL || this->VolumeDisplayWidget != vectorVDW)
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
      }
    else if ( !strcmp(refNode->GetClassName(), "vtkMRMLDiffusionWeightedVolumeNode"))
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
    else if ( !strcmp(refNode->GetClassName(), "vtkMRMLDiffusionTensorVolumeNode") )
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
    //this->VolumeDisplayWidget->UpdateWidgetFromMRML();
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

  if (this->Built)
    {
    return;
    }

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Volumes", "Volumes", NULL );

  // Define your help text and build the help frame here.
  const char *help = "The Volumes Module loads and adjusts display parameters of volume data. \n<a>http://slicer.org/slicerWiki/index.php/Documentation-3.6</a>\n\nThe Diffusion Editor allows modifying parameters (gradients, bValues, measurement frame) of DWI data and provides a quick way to interpret them. For that it estimates a tensor and shows glyphs and tracts for visual exploration. Help for Diffusion Editor: <a>http://www.slicer.org/slicerWiki/index.php/Modules:Volumes:Diffusion_Editor-Documentation</a>";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\nThe Volumes module was contributed by Alex Yarmarkovich, Isomics Inc. (Steve Pieper) with help from others at SPL, BWH (Ron Kikinis). \n\nThe Diffusion Editor was developed by Kerstin Kessel.";
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
  this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->SetMasterWindow ( this->GetApplicationGUI()->GetMainSlicerWindow() );
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
  this->NameEntry->SetLabelWidth(15);
  this->NameEntry->SetLabelText("Volume Name:");
  this->NameEntry->GetWidget()->SetValue ( "" );
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->NameEntry->GetWidgetName());

  // center image button
  this->CenterImageMenu = vtkKWMenuButtonWithLabel::New();
  this->CenterImageMenu->SetParent(this->LoadFrame->GetFrame());
  this->CenterImageMenu->Create();
  this->CenterImageMenu->SetWidth(20);
  this->CenterImageMenu->SetLabelWidth(15);
  this->CenterImageMenu->SetLabelText("Image Origin:");
  this->CenterImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "Centered");
  this->CenterImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "From File");
  this->CenterImageMenu->GetWidget()->SetValue ( "From File" );
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->CenterImageMenu->GetWidgetName());

  // UseOrientationFromFile
  this->OrientImageMenu = vtkKWMenuButtonWithLabel::New();
  this->OrientImageMenu->SetParent(this->LoadFrame->GetFrame());
  this->OrientImageMenu->Create();
  this->OrientImageMenu->SetWidth(20);
  this->OrientImageMenu->SetLabelWidth(15);
  this->OrientImageMenu->SetLabelText("Image Orientation:");
  this->OrientImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "Use IJK");
  this->OrientImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "From File");
  this->OrientImageMenu->GetWidget()->SetValue ( "From File" );
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->OrientImageMenu->GetWidgetName());

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

  // keep all images in memory when we do previous/next ?
  this->KeepAllCheckButton = vtkKWCheckButton::New();
  this->KeepAllCheckButton->SetParent(this->LoadFrame->GetFrame());
  this->KeepAllCheckButton->Create();
  this->KeepAllCheckButton->SelectedStateOff();
  this->KeepAllCheckButton->SetText("Keep all");
  this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->KeepAllCheckButton->GetWidgetName());

  // Apply button
  this->ApplyButton = vtkKWPushButton::New();
  this->ApplyButton->SetParent(this->LoadFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth(10);
  this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->ApplyButton->GetWidgetName());

   // Previous button
  this->LoadPreviousButton = vtkKWPushButton::New();
  this->LoadPreviousButton->SetParent(this->LoadFrame->GetFrame());
  this->LoadPreviousButton->Create();
  this->LoadPreviousButton->SetText("Previous");
  this->LoadPreviousButton->SetWidth(10);
  this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->LoadPreviousButton->GetWidgetName());

  // Next button
  this->LoadNextButton = vtkKWPushButton::New();
  this->LoadNextButton->SetParent(this->LoadFrame->GetFrame());
  this->LoadNextButton->Create();
  this->LoadNextButton->SetText("Next");
  this->LoadNextButton->SetWidth(10);
  this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->LoadNextButton->GetWidgetName());

  // Cine button
  //   this->CineButton = vtkKWPushButton::New();
  //   this->CineButton->SetParent(this->LoadFrame->GetFrame());
  //   this->CineButton->Create();
  //   this->CineButton->SetText("Cine");
  //   this->CineButton->SetWidth(10);
  //   this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
  //                this->CineButton->GetWidgetName());

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

  this->VectorDisplayFrame = vtkKWFrame::New();
  this->VectorDisplayFrame->SetParent( this->DisplayFrame->GetFrame() );
  this->VectorDisplayFrame->Create( );
  this->Script( "pack %s -in %s",
                this->VectorDisplayFrame->GetWidgetName(),this->DisplayFrame->GetFrame()->GetWidgetName());

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

  this->VolumeFileHeaderWidget = vtkSlicerVolumeFileHeaderWidget::New();
  this->VolumeFileHeaderWidget->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow());
  this->VolumeFileHeaderWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
  this->VolumeFileHeaderWidget->AddObserver ( vtkSlicerVolumeFileHeaderWidget::FileHeaderOKEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->VolumeFileHeaderWidget->Create();  
  this->VolumeFileHeaderWidget->SetInfo("The file format does not match standard volume formats.\n If you want to read it as binary data, fill the header and press Read.\n Otherwise, press Cancel");

  this->ProcessGUIEvents (this->VolumeSelectorWidget,
                          vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );

  this->Built = true;

}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateScalarDisplayWidget ( )
{
  if (this->scalarVDW == NULL)
    {
    this->scalarVDW = vtkSlicerScalarVolumeDisplayWidget::New ( );
    this->scalarVDW->SetParent( this->ScalarDisplayFrame );
    this->scalarVDW->SetMRMLScene(this->GetMRMLScene());
    this->scalarVDW->AddMRMLObservers();
    this->scalarVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                   scalarVDW->GetWidgetName(), this->ScalarDisplayFrame->GetWidgetName());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateVectorDisplayWidget ( )
{
  if (this->vectorVDW == NULL)
    {
    this->vectorVDW = vtkSlicerVectorVolumeDisplayWidget::New( );
    this->vectorVDW->SetParent( this->VectorDisplayFrame );
    this->vectorVDW->SetMRMLScene(this->GetMRMLScene());
    this->vectorVDW->AddMRMLObservers();
    this->vectorVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                   vectorVDW->GetWidgetName(), this->VectorDisplayFrame->GetWidgetName());
    }
}
//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::CreateLabelMapDisplayWidget ( )
{
  if (this->labelVDW == NULL)
    {
    this->labelVDW = vtkSlicerLabelMapVolumeDisplayWidget::New ( );
    this->labelVDW->SetParent( this->LabelMapDisplayFrame );
    this->labelVDW->SetMRMLScene(this->GetMRMLScene());
    this->labelVDW->AddMRMLObservers();
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
    this->dwiVDW->AddMRMLObservers();
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
    this->dtiVDW->AddMRMLObservers();
    this->dtiVDW->Create();
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                   dtiVDW->GetWidgetName(), this->DTIDisplayFrame->GetWidgetName());    
    }
}


void vtkSlicerVolumesGUI::CopyTagAndValues( vtkITKArchetypeImageSeriesReader* reader )
{
  unsigned int nItems = reader->GetNumberOfItemsInDictionary();
  this->Tags.resize(0);
  this->TagValues.resize(0);

  if (nItems > 0)
    {
    for (unsigned int k = 0; k < nItems; k++)
      {
      this->Tags.push_back( reader->GetNthKey(k) );
      this->TagValues.push_back( reader->GetNthValue(k) );
      }
    }

  return;
}


unsigned int vtkSlicerVolumesGUI::GetNumberOfItemsInDictionary()
{
  return this->Tags.size();
}

bool vtkSlicerVolumesGUI::HasKey( char* tag )
{
  std::string tagstr( tag );
  for (unsigned int k = 0; k < this->GetNumberOfItemsInDictionary(); k++)
    {
    std::string nthTag = this->GetNthKey( k );
    if (nthTag == tagstr)
      {
      return true;
      }
    }
  return false;
}

const char* vtkSlicerVolumesGUI::GetNthKey( unsigned int n )
{
  if (n >= this->Tags.size())
    {
    return NULL;
    }
  return this->Tags[n].c_str();
}

const char* vtkSlicerVolumesGUI::GetNthValue( unsigned int n )
{
  if (n >= this->TagValues.size())
    {
    return NULL;
    }
  return this->TagValues[n].c_str();
}

const char* vtkSlicerVolumesGUI::GetTagValue( char* tag )
{
  std::string tagstr (tag);
  for (unsigned int k = 0; k < this->Tags.size(); k++)
    {
    if (this->Tags[k] == tagstr)
      {
      return this->TagValues[k].c_str();
      }
    }
  return NULL;
}

void vtkSlicerVolumesGUI::CheckLoadingOptions( int &LoadingOptions, bool &KeepAll )
{
  LoadingOptions = 0;
  KeepAll = false;

  if ( this->LabelMapCheckButton->GetSelectedState() )
    {
      LoadingOptions += 1;
    }

  vtkKWMenuButton *mb = this->CenterImageMenu->GetWidget();
  if ( !strcmp (mb->GetValue(), "Centered") )
    {
      LoadingOptions += 2;
    }

  if ( this->SingleFileCheckButton->GetSelectedState() )
    {
      LoadingOptions += 4;
    }

  vtkKWMenuButton *orientMB = this->OrientImageMenu->GetWidget();
  if ( !strcmp (orientMB->GetValue(), "Use IJK") )
    {
      LoadingOptions += 16;
    }

  if ( this->KeepAllCheckButton->GetSelectedState() )
    {
      KeepAll = true;
    }
  else
    {
      KeepAll = false;
    }

  return;
}
