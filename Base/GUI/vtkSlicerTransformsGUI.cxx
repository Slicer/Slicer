#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerTransformsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWFrameWithLabel.h"

#include "vtkSlicerTransformLogic.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLTransformStorageNode.h"

#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWLoadSaveButton.h"

#include "vtkSmartPointer.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTransformsGUI );
vtkCxxRevisionMacro ( vtkSlicerTransformsGUI, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerTransformsGUI::vtkSlicerTransformsGUI ( )
{

    this->TransformEditorWidget = NULL;
    this->LoadButton = NULL;
    NACLabel = NULL;
    NAMICLabel = NULL;
    NCIGTLabel = NULL;
    BIRNLabel = NULL;

    // Register transform types with ITK factory
    // This code is from Applications/CLI/BRAINSTools/BRAINSCommonLib/GenericTransformImage.cxx
    // We do this in order to register ScaleVersor3DTransform, which is not done
    // in ITK 3.18. The rest of transforms need to be re-registered when a new
    // transform is added. TODO: this must be revised in 3.20, hopefully
    // ScaleVersor3DTransform will be out of Review by then      
    vtkSmartPointer<vtkSlicerTransformLogic> logic = vtkSmartPointer<vtkSlicerTransformLogic>::New();
    logic->RegisterITKTransforms();

}


//---------------------------------------------------------------------------
vtkSlicerTransformsGUI::~vtkSlicerTransformsGUI ( )
{
  //this->UIPanel->RemovePage("Transforms");

  if (this->TransformEditorWidget)
    {
    this->TransformEditorWidget->RemoveWidgetObservers ( );
    this->TransformEditorWidget->SetParent (NULL);
    this->TransformEditorWidget->Delete ( );
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
  if (this->LoadButton ) 
    {
    this->LoadButton->SetParent(NULL);
    this->LoadButton->Delete ( );
    }    

}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerTransformsGUI: " << this->GetClassName ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::RemoveGUIObservers ( )
{
  if (this->LoadButton)
    {
    this->LoadButton->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
   }
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::AddGUIObservers ( )
{
  if (this->LoadButton)
    {
    this->LoadButton->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::ProcessGUIEvents (vtkObject *caller,
                                               unsigned long event,
                                               void *vtkNotUsed(callData))
{
  vtkKWLoadSaveDialog *loadSaveDialog = vtkKWLoadSaveDialog::SafeDownCast(caller);
  if (loadSaveDialog && loadSaveDialog == this->LoadButton->GetLoadSaveDialog() &&
      event == vtkKWTopLevel::WithdrawEvent  )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadButton->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerTransformLogic *logic = vtkSlicerTransformLogic::New();
      logic->AddTransform(fileName, this->GetMRMLScene());
      logic->Delete();
      
      this->LoadButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      }
    
    // reset the file browse button text
    this->LoadButton->SetText ("Load Transform");
    }

  return;
} 

//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::ReleaseModuleEventBindings ( )
{
  
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::Enter ( vtkMRMLNode *node )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();

  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(node);
  if ( transformNode )
    {
    this->GetApplication()->ProcessPendingEvents();
    this->TransformEditorWidget->GetTransformEditSelectorWidget()->UpdateMenu();
    this->TransformEditorWidget->GetTransformEditSelectorWidget()->SetSelected( transformNode );
    this->TransformEditorWidget->UpdateMatrix();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::TearDownGUI ( )
{
  this->Exit();
  this->RemoveGUIObservers();
}



//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::BuildGUI ( )
{
  // Fill in *placeholder GUI*
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Transforms", "Transforms", NULL );
    
  // Define your help text and build the help frame here.
  const char *help = "The Transformations module creates and edits slicer Transformation nodes. Transformation nodes are used in Slicer to define spacial relationships between different nodes (such as volumes, models, fiducials, ROI's, or other Transform nodes) or between the nodes and the global RAS space. You can establish these relations by dragging the nodes under the Transformation nodes in the Data module.\n The Transforms module is used for interactive editing of Transformation nodes. Transformation nodes can be both linear (expressed as 4 by 4 matrix) or non-linear warp transformations. The transformation module allow you to edit only linear Transformation nodes. The module can e used for example for manual registering of two images of the same subject accrued at different times or using different modalities. Automatic registration modules are also available in Slicer under the Registration category.  \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:Transforms-Documentation-3.6</a>\n";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\nThe Transforms module was contributed by Alex Yarmarkovich, Isomics Inc. with help from others at SPL, BWH (Ron Kikinis)";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Transforms" );
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
    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  

    //  COMMENTED UNTIL THE CREATION LOGIC WORKS
    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *modLoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modLoadFrame->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
    modLoadFrame->Create ( );
    modLoadFrame->SetLabelText ("Load");
    modLoadFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modLoadFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());

    // add a file browser 
    this->LoadButton = vtkKWLoadSaveButton::New ( );
    this->LoadButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadButton->Create ( );
    this->LoadButton->SetText ("Load Transforms");
    this->LoadButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
    this->LoadButton->GetLoadSaveDialog()->SetFileTypes(
                                                             "{ {model} {*.*} }");
    app->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
                this->LoadButton->GetWidgetName());
   //*/

  // DISPLAY & EDIT FRAME
//  vtkSlicerModuleCollapsibleFrame *displayAndEditFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  vtkKWFrameWithLabel *displayAndEditFrame = vtkKWFrameWithLabel::New();
  displayAndEditFrame->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
  displayAndEditFrame->Create ( );
  // displayAndEditFrame->ExpandFrame ( );
  displayAndEditFrame->SetLabelText ("Display and edit");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                displayAndEditFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());

  // ---
  this->TransformEditorWidget = vtkSlicerTransformEditorWidget::New ( );
  this->TransformEditorWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
  this->TransformEditorWidget->SetParent ( displayAndEditFrame->GetFrame() );
  this->TransformEditorWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->TransformEditorWidget->GetWidgetName(), displayAndEditFrame->GetFrame()->GetWidgetName());

  displayAndEditFrame->Delete ();
  modLoadFrame->Delete();
}





