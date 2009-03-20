#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include <itksys/SystemTools.hxx> 
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModelDisplayWidget.h"
#include "vtkSlicerModelHierarchyWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerModelInfoWidget.h"

// for pick events
//#include "vtkSlicerViewerWidget.h"
//#include "vtkSlicerViewerInteractorStyle.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWTopLevel.h"

// for scalars
#include "vtkPointData.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelsGUI );
vtkCxxRevisionMacro ( vtkSlicerModelsGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::vtkSlicerModelsGUI ( )
{

  // classes not yet defined!
  this->Logic = NULL;
  this->ModelHierarchyLogic = NULL;

  //this->ModelNode = NULL;
  this->LoadModelButton = NULL;
  this->LoadModelDirectoryButton = NULL;
  this->ModelDisplayWidget = NULL;
  this->ClipModelsWidget = NULL;
  this->LoadScalarsButton = NULL;
  this->ModelDisplaySelectorWidget = NULL;
  this->ModelHierarchyWidget = NULL;
  this->ModelDisplayFrame = NULL;
  this->ModelInfoWidget = NULL;

  NACLabel = NULL;
  NAMICLabel =NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;

  // for picking
//  this->ViewerWidget = NULL;
//  this->InteractorStyle = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::~vtkSlicerModelsGUI ( )
{
  this->RemoveGUIObservers();

  this->SetModuleLogic ( NULL );
  this->SetModelHierarchyLogic ( NULL );

  if (this->ModelDisplaySelectorWidget)
    {
    this->ModelDisplaySelectorWidget->SetParent(NULL);
    this->ModelDisplaySelectorWidget->Delete();
    this->ModelDisplaySelectorWidget = NULL;
    }

  if (this->ModelInfoWidget)
    {
    this->ModelInfoWidget->SetParent(NULL);
    this->ModelInfoWidget->Delete();
    this->ModelInfoWidget = NULL;
    }

  if (this->ModelHierarchyWidget)
    {
    this->ModelHierarchyWidget->SetParent(NULL);
    this->ModelHierarchyWidget->Delete();
    this->ModelHierarchyWidget = NULL;
    }

  if (this->LoadModelButton ) 
    {
    this->LoadModelButton->SetParent(NULL);
    this->LoadModelButton->Delete ( );
    }    
  if (this->LoadModelDirectoryButton ) 
    {
    this->LoadModelDirectoryButton->SetParent(NULL);
    this->LoadModelDirectoryButton->Delete ( );
    }
  if (this->ModelDisplayWidget ) 
    {
    this->ModelDisplayWidget->SetParent(NULL);
    this->ModelDisplayWidget->Delete ( );
    }
  if (this->ClipModelsWidget ) 
    {
    this->ClipModelsWidget->SetParent(NULL);
    this->ClipModelsWidget->Delete ( );
    }
  if (this->LoadScalarsButton )
    {
    this->LoadScalarsButton->SetParent(NULL);
    this->LoadScalarsButton->Delete ( );
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
  if (this->ModelDisplayFrame)
    {
    this->ModelDisplayFrame->SetParent ( NULL );
    this->ModelDisplayFrame->Delete();
    }
//  this->SetViewerWidget(NULL);   
//  this->SetInteractorStyle(NULL);
  this->Built = false;
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerModelsGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "ModelNode: " << this->GetModelNode ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::RemoveGUIObservers ( )
{
  if (this->LoadModelButton)
    {
    this->LoadModelButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );    
    }
  if (this->LoadModelDirectoryButton)
    {
    this->LoadModelDirectoryButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );    
    }
  if (this->LoadScalarsButton)
    {
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );    
    }
  if (this->ModelDisplaySelectorWidget)
    {
    this->ModelDisplaySelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ModelHierarchyWidget)
    { 
    this->ModelHierarchyWidget->RemoveObservers(vtkSlicerModelHierarchyWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::AddGUIObservers ( )
{
  this->LoadModelButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadModelDirectoryButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->ModelDisplaySelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ModelHierarchyWidget->AddObserver(vtkSlicerModelHierarchyWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{

  if (vtkSlicerModelHierarchyWidget::SafeDownCast(caller) == this->ModelHierarchyWidget && 
      event == vtkSlicerModelHierarchyWidget::SelectedEvent)
    {
    vtkMRMLModelNode *model = reinterpret_cast<vtkMRMLModelNode *>(callData);
    if (model != NULL && model->GetDisplayNode() != NULL)
      {
      //this->ModelDisplaySelectorWidget->SetSelected(model);
      if (this->ModelDisplayFrame)
        {
        this->ModelDisplayFrame->ExpandFrame();
        this->ModelDisplayFrame->Raise();
        }
      //this->ModelDisplayWidget->SetModelDisplayNode(model->GetDisplayNode());
      //this->ModelDisplayWidget->SetModelNode(model);
      }
    return;
    }
/**
  if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->ModelDisplaySelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLModelNode *model = 
        vtkMRMLModelNode::SafeDownCast(this->ModelDisplaySelectorWidget->GetSelected());

    if (model != NULL && model->GetDisplayNode() != NULL)
      {
      this->ModelDisplayWidget->SetModelDisplayNode(model->GetModelDisplayNode());
      this->ModelDisplayWidget->SetModelNode(model);
      }
    return;
    }
**/
  if (this->LoadModelButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadModelButton->GetWidget()->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerModelsLogic* modelLogic = this->Logic;
      
      vtkMRMLModelNode *modelNode = modelLogic->AddModel( fileName );
      if ( modelNode == NULL ) 
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
        dialog->SetStyleToMessage();
        std::string msg = std::string("Unable to read model file ") + std::string(fileName);
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();

        vtkErrorMacro("Unable to read model file " << fileName);
        // reset the file browse button text
        this->LoadModelButton->GetWidget()->SetText ("None");
        }
      else
        {
         this->LoadModelButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
         const itksys_stl::string fname(fileName);
         itksys_stl::string name = itksys::SystemTools::GetFilenameName(fname);
         this->LoadModelButton->GetWidget()->SetText (name.c_str());

         // set it to be the active model
         // set the display model
         this->ModelHierarchyWidget->GetModelDisplaySelectorWidget()->SetSelected(modelNode);
        }
      }
    return;
    }
    else if (this->LoadModelDirectoryButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadModelDirectoryButton->GetWidget()->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerModelsLogic* modelLogic = this->Logic;

      vtkKWMessageDialog *dialog0 = vtkKWMessageDialog::New();
      dialog0->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
      dialog0->SetStyleToMessage();
      std::string msg0 = std::string("Reading *.vtk from models directory ") + std::string(fileName);
      dialog0->SetText(msg0.c_str());
      dialog0->Create ( );
      dialog0->Invoke();
      dialog0->Delete();
      
      if (modelLogic->AddModels( fileName, ".vtk") == 0)
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
        dialog->SetStyleToMessage();
        std::string msg = std::string("Unable to read models directory ") + std::string(fileName);
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();
        }
      else
        {
        this->LoadModelDirectoryButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
        dialog->SetStyleToMessage();
        dialog->SetText("Done reading models...");
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();
        }
      }
    this->LoadModelDirectoryButton->GetWidget()->SetText ("None");
    return;
    }
  else if (this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
    {
    // If a scalar file has been selected for loading...
    const char *fileName = this->LoadScalarsButton->GetWidget()->GetFileName();
    if ( fileName ) 
      {
      // get the model from the display widget rather than this gui's save
      // model selector
      vtkMRMLModelNode *modelNode = NULL;
      if (this->ModelHierarchyWidget != NULL &&
          this->ModelHierarchyWidget->GetModelDisplaySelectorWidget() != NULL)
        {
        modelNode = vtkMRMLModelNode::SafeDownCast(this->ModelHierarchyWidget->GetModelDisplaySelectorWidget()->GetSelected());
        }
      else { vtkErrorMacro("Model hierarchy widget or it's model display selector widget is null, can't get the model node."); }
      if (modelNode != NULL)
        {
        vtkDebugMacro("vtkSlicerModelsGUI: loading scalar for model " << modelNode->GetName());
        // load the scalars
        vtkSlicerModelsLogic* modelLogic = this->Logic;
        if (!modelLogic->AddScalar(fileName, modelNode))
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
          dialog->SetStyleToMessage();
          std::string msg = std::string("Unable to read scalars file ") + std::string(fileName);
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          
          vtkErrorMacro("Error loading scalar overlay file " << fileName);
          this->LoadScalarsButton->GetWidget()->SetText ("None");
          }
        else
          {
          this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
          // set the active scalar in the display node to this one
          // - is done in the model storage node         
          }
        }
      else
        {
        vtkErrorMacro("Unable to get the model on which to load " << fileName );
        }
      }
    return;
    }
  /*
  else if (event == vtkSlicerViewerInteractorStyle::SelectRegionEvent &&
           vtkSlicerViewerInteractorStyle::SafeDownCast(caller) != NULL &&
           callData != NULL)
    {
    vtkDebugMacro("ProcessGUIEvents: Pick Manipulate event!\n");
    // do the pick
    int x = ((int *)callData)[0];
    int y = ((int *)callData)[1];
    if (this->GetViewerWidget() &&
        this->GetViewerWidget()->Pick(x,y) != 0)
      {
      // get the node name, this returns the model display node's id
      const char *dispNodeID = this->GetViewerWidget()->GetPickedNodeName();
      if (strcmp(dispNodeID, "") != 0)
        {
        vtkDebugMacro("ProcessGUIEvents: got picked node " << dispNodeID);
        vtkMRMLModelNode *modelNode = NULL;
        vtkMRMLModelDisplayNode *modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(dispNodeID));
        if (modelDisplayNode != NULL)
          {
          int selectedFlag = !(modelDisplayNode->GetSelected());
          vtkDebugMacro("Got a display node with name " << modelDisplayNode->GetName() << ", toggling selected to " << selectedFlag);
          // find the model node with this display node
          std::vector<vtkMRMLNode *> modelNodes;
          int numModelNodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLModelNode", modelNodes);
          for (unsigned int n=0; n<modelNodes.size() && modelNode == NULL; n++)
            {
            vtkMRMLModelNode *modelNodeToTest = vtkMRMLModelNode::SafeDownCast(modelNodes[n]);
            if (modelNodeToTest != NULL)
              {
              int numDispNodes = modelNodeToTest->GetNumberOfDisplayNodes();
              for (int d = 0; d < numDispNodes && modelNode == NULL; d++)
                {
                vtkMRMLModelDisplayNode *mDN = vtkMRMLModelDisplayNode::SafeDownCast(modelNodeToTest->GetNthDisplayNode(d));
                if (modelDisplayNode != NULL &&
                    strcmp(mDN->GetID(), modelDisplayNode->GetID()) == 0)
                  {
                  vtkDebugMacro("Found display node on model node with id " << modelNodeToTest->GetID());
                  modelNode = modelNodeToTest;
                  }
                }
              }
            }
          if (modelNode != NULL)
            {
            vtkDebugMacro("Got the modelnode, setting it active: " << modelNode->GetID());
            // set it to be the selected/active model
            // set the display model
            this->ModelHierarchyWidget->GetModelDisplaySelectorWidget()->SetSelected(modelNode);
            vtkDebugMacro("Setting selected to " << selectedFlag << " on the model node " << modelNode->GetName());
            
            modelNode->SetSelected(selectedFlag);
            if (modelDisplayNode)
              {
              vtkDebugMacro("Setting selected to " << selectedFlag << " on the model display node " << modelDisplayNode->GetName());
              modelDisplayNode->SetSelected(selectedFlag);
              } 
            }
          } else { vtkDebugMacro("Couldn't find a display node with id " << dispNodeID); }
        // swallow the pick
        if (this->GUICallbackCommand != NULL)
          {
          vtkDebugMacro("ProcessGUIEvents: swallowing the pick");
          this->GUICallbackCommand->SetAbortFlag(1);
          }
        else
          {
          vtkErrorMacro("Unable to get the gui call back command that calls process widget events, event = " << event << " is not swallowed here");
          }
        }
      else { vtkDebugMacro("Unable to get the name of the object that was picked."); }
      } else { vtkDebugMacro("ProcessGUIEvents: invalid pick"); }
    return;
    }
  */

}    

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ReleaseModuleEventBindings ( )
{
  
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::Enter ( vtkMRMLNode *node )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  if (node)
    {
    this->ModelHierarchyWidget->UpdateTreeFromMRML();
    this->ModelHierarchyWidget->GetModelDisplaySelectorWidget()->UnconditionalUpdateMenu();
    this->ModelHierarchyWidget->SelectNode(node);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Models", "Models", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Models Module loads and adjusts display parameters of models.\n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:Models-Documentation</a>\nSave models via the File menu, Save button.\nThe Load Model button will allow you to load any model that Slicer can read, Load Model Directory will load all the VTK models in a directory. Load FreeSurfer Overlay will load a scalar file and associate it with the currently active model.\nYou can adjust the display properties of the models in the Display pane. Select the model you wish to work on from the model selector drop down menu. Scalar overlays are loaded with a default colour look up table, but can be reassigned manually. Once a new scalar overlay is chosen, currently the old color map is still used, so that must be adjusted in conjunction with the overlay.\nClipping is turned on for a model in the Display pane, and the slice planes that will clip the model are selected in the Clipping pane.\nThe Model Hierarchy pane allows you to group models together and set the group's properties.";
    const char *about = "This module was contributed by Nicole Aucoin, SPL, BWH (Ron Kikinis), and Alex Yarmarkovich, Isomics Inc. (Steve Pieper).\nThis work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Models" );
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

    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *modLoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modLoadFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    modLoadFrame->Create ( );
    modLoadFrame->SetLabelText ("Load");
    modLoadFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modLoadFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    // add a file browser 
    this->LoadModelButton = vtkKWLoadSaveButtonWithLabel::New ( );
    this->LoadModelButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadModelButton->Create ( );
    this->LoadModelButton->SetLabelText ("Load Model: ");
    this->LoadModelButton->GetWidget()->SetText ("None");
    this->LoadModelButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open Model");
    this->LoadModelButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
    this->LoadModelButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                             "{ {model} {*.*} }");
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -ipadx 0 -ipady 0", 
                this->LoadModelButton->GetWidgetName());

   // add a file browser 
    this->LoadModelDirectoryButton = vtkKWLoadSaveButtonWithLabel::New ( );
    this->LoadModelDirectoryButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadModelDirectoryButton->Create ( );
    this->LoadModelDirectoryButton->SetLabelText ("Load Model Directory: ");
    this->LoadModelDirectoryButton->GetWidget()->SetText ("None");
    this->LoadModelDirectoryButton->SetBalloonHelpString("Load *.vtk surface files from a directory");
    this->LoadModelDirectoryButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -ipadx 0 -ipady 0", 
                this->LoadModelDirectoryButton->GetWidgetName());


    this->LoadScalarsButton = vtkKWLoadSaveButtonWithLabel::New();
    this->LoadScalarsButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadScalarsButton->Create ( );
    this->LoadScalarsButton->SetLabelText ("Load FreeSurfer Overlay: ");
    this->LoadScalarsButton->SetBalloonHelpString("Load scalar values and assign them to the currently active model.");
    this->LoadScalarsButton->GetWidget()->SetText ("None");
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open FreeSurfer Overlay");
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {All} {.*} } { {Thickness} {.thickness} } { {Curve} {.curv} } { {Average Curve} {.avg_curv} } { {Sulc} {.sulc} } { {Area} {.area} } { {W} {.w} } { {Parcellation Annotation} {.annot} } { {Volume} {.mgz .mgh} } { {Label} {.label} }");
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -ipadx 0 -ipady 0", 
                this->LoadScalarsButton->GetWidgetName());
    
    // DISPLAY FRAME            
    this->ModelDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->ModelDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    this->ModelDisplayFrame->Create ( );
    this->ModelDisplayFrame->SetLabelText ("Hierarchy & Display");
    this->ModelDisplayFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

 
    this->ModelHierarchyWidget = vtkSlicerModelHierarchyWidget::New ( );
    this->ModelHierarchyWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->ModelHierarchyWidget->SetModelHierarchyLogic(this->GetModelHierarchyLogic());
    this->ModelHierarchyWidget->SetParent ( this->ModelDisplayFrame->GetFrame() );
    this->ModelHierarchyWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelHierarchyWidget->GetWidgetName(), 
                  this->ModelDisplayFrame->GetFrame()->GetWidgetName());


    // Clip FRAME  
    vtkSlicerModuleCollapsibleFrame *clipFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    clipFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    clipFrame->Create ( );
    clipFrame->SetLabelText ("Clipping");
    clipFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  clipFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->ClipModelsWidget = vtkSlicerClipModelsWidget::New ( );
    this->ClipModelsWidget->SetMRMLScene(this->GetMRMLScene() );
    this->ClipModelsWidget->SetParent ( clipFrame->GetFrame() );
    this->ClipModelsWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ClipModelsWidget->GetWidgetName(), 
                  clipFrame->GetFrame()->GetWidgetName());

    // Info FRAME  
    vtkSlicerModuleCollapsibleFrame *infoFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    infoFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    infoFrame->Create ( );
    infoFrame->SetLabelText ("Info");
    infoFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  infoFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->ModelInfoWidget = vtkSlicerModelInfoWidget::New ( );
    this->ModelInfoWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->ModelInfoWidget->SetParent ( infoFrame->GetFrame() );
    this->ModelInfoWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelInfoWidget->GetWidgetName(), 
                  infoFrame->GetFrame()->GetWidgetName());

   //this->ProcessGUIEvents (this->ModelDisplaySelectorWidget,
                          //vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );

    modLoadFrame->Delete ( );
    clipFrame->Delete ( );    
    infoFrame->Delete ( );

    // set up picking
    this->Init();
}

/*
//----------------------------------------------------------------------------
void vtkSlicerModelsGUI::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  this->ViewerWidget = viewerWidget;
}

//----------------------------------------------------------------------------
void vtkSlicerModelsGUI::SetInteractorStyle( vtkSlicerViewerInteractorStyle *interactorStyle )
{
  // note: currently the GUICallbackCommand calls ProcessGUIEvents
  // remove observers
  if (this->InteractorStyle != NULL &&
      this->InteractorStyle->HasObserver(vtkSlicerViewerInteractorStyle::SelectRegionEvent, this->GUICallbackCommand) == 1)
    {
    this->InteractorStyle->RemoveObservers(vtkSlicerViewerInteractorStyle::SelectRegionEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  
  this->InteractorStyle = interactorStyle;

  // add observers
  if (this->InteractorStyle)
    {
    vtkDebugMacro("SetInteractorStyle: Adding observer on interactor style");
    this->InteractorStyle->AddObserver(vtkSlicerViewerInteractorStyle::SelectRegionEvent, (vtkCommand *)this->GUICallbackCommand);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModelsGUI::Init(void)
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (appGUI == NULL)
    {
    return;
    }
  
  // get the viewer widget
  this->SetViewerWidget(appGUI->GetViewerWidget());

  // get the interactor style, to set up plotting events
  if (appGUI->GetViewerWidget() != NULL &&
      appGUI->GetViewerWidget()->GetMainViewer() != NULL &&
      appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() != NULL &&
      appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle() != NULL)
    {
    this->SetInteractorStyle(vtkSlicerViewerInteractorStyle::SafeDownCast(appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle()));
    }
  else
    {
    vtkErrorMacro("Init: unable to get the interactor style, picking will not work.");
    }
}
*/
