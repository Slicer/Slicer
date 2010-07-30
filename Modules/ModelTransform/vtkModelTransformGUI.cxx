
#include "vtkSlicerApplication.h"

#include "vtkModelTransformGUI.h"

// vtkSlicer includes
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkSlicerModelsLogic.h"

#include "vtkMRMLModelStorageNode.h"

// KWWidgets includes
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWEvent.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"

// STL includes
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <iterator>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"


//------------------------------------------------------------------------------
vtkModelTransformGUI* vtkModelTransformGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelTransformGUI");
  if(ret)
    {
      return (vtkModelTransformGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkModelTransformGUI;
}


//----------------------------------------------------------------------------
vtkModelTransformGUI::vtkModelTransformGUI()
{
  this->UpdatingMRML = 0;
  this->UpdatingGUI = 0;

  this->Logic = NULL;
    
  this->ModelTransformNode = NULL;
  this->ModelNode = NULL;
  this->TransformNode = NULL;

  this->SpecificationFrame = NULL;
  this->ParameterSelector = NULL;
  this->ModelSelector = NULL;
  this->ModelOutSelector = NULL;
  this->TransformSelector = NULL;

  // Try to load supporting libraries dynamically.  This is needed
  // since the toplevel is a loadable module but the other libraries
  // didn't get loaded
  Tcl_Interp* interp = this->GetApplication()->GetMainInterp();
  if (interp)
    {
    Vtkslicermodeltransformmodulelogic_Init(interp);
    }
  else
    {
    vtkErrorMacro("Failed to obtain reference to application TCL interpreter");
    }
}

//----------------------------------------------------------------------------
vtkModelTransformGUI::~vtkModelTransformGUI()
{
    this->RemoveMRMLNodeObservers ( );

    this->UpdatingMRML = 0;
    this->UpdatingGUI = 0;

    vtkSetAndObserveMRMLNodeMacro(this->ModelTransformNode, NULL);
    vtkSetAndObserveMRMLNodeMacro(this->ModelNode, NULL);
    vtkSetAndObserveMRMLNodeMacro(this->TransformNode, NULL);

    if ( this->ParameterSelector )
      {
      this->ParameterSelector->SetParent (NULL );
      this->ParameterSelector->Delete();
      this->ParameterSelector = NULL;
      }
    if ( this->ModelSelector )
      {
      this->ModelSelector->SetParent (NULL );
      this->ModelSelector->Delete();
      this->ModelSelector = NULL;
      }
    if ( this->ModelOutSelector )
      {
      this->ModelOutSelector->SetParent (NULL );
      this->ModelOutSelector->Delete();
      this->ModelOutSelector = NULL;
      }
    if ( this->TransformSelector )
      {
      this->TransformSelector->SetParent (NULL );
      this->TransformSelector->Delete();
      this->TransformSelector = NULL;
      }
    if ( this->SpecificationFrame )
      {
      this->SpecificationFrame->SetParent (NULL );
      this->SpecificationFrame->Delete();
      this->SpecificationFrame = NULL;
      }

    this->Raised = false;

    if ( this->Logic )
      {
      this->Logic->Delete();
      this->Logic = NULL;
      }

//    this->SetAndObserveMRMLScene ( NULL );    
}



//----------------------------------------------------------------------------
void vtkModelTransformGUI::Enter()
{
  
  
  //--- mark as currently being visited.
  this->Raised = true;

  //--- mark as visited at least once.
  this->Visited = true;

  //--- only build when first visited.
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->AddObserver ( vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand() );


    //--- Set up GUI observers 
    vtkIntArray *guiEvents = this->NewObservableEvents ( );
    if ( guiEvents != NULL )
      {
      this->SetAndObserveMRMLSceneEvents ( this->MRMLScene, guiEvents );
      guiEvents->Delete();
      }
    }

  this->AddGUIObservers();    
  this->CreateModuleEventBindings();

  //--- make GUI reflect current MRML state
  this->UpdateGUI();
}


//----------------------------------------------------------------------------
void vtkModelTransformGUI::Exit ( )
{

  //--- mark as no longer selected.
  this->Raised = false;

  this->RemoveGUIObservers();
  this->ReleaseModuleEventBindings();
//  this->SetAndObserveMRMLScene ( NULL );
  
}


//----------------------------------------------------------------------------
vtkIntArray *vtkModelTransformGUI::NewObservableEvents()
{
  if ( !this->Visited )
    {
    return NULL;
    }
  
 vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkModelTransformGUI::TearDownGUI ( )
{
  if ( !this->Built )
    {
    return;
    }

  this->RemoveObservers ( vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand() );
  this->RemoveGUIObservers ( );
  this->ParameterSelector->SetMRMLScene ( NULL );
  this->ModelSelector->SetMRMLScene ( NULL );
  this->ModelOutSelector->SetMRMLScene ( NULL );
  this->TransformSelector->SetMRMLScene ( NULL );
  this->ReleaseModuleEventBindings();
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkModelTransformGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


//---------------------------------------------------------------------------
void vtkModelTransformGUI::AddGUIObservers ( ) 
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

  if ( this->ParameterSelector )
    {
    if  (this->MRMLScene != NULL )
      {
      this->ParameterSelector->SetMRMLScene ( this->MRMLScene );
      }
    this->ParameterSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                             ( vtkCommand *) this->GUICallbackCommand );
    this->ParameterSelector->UpdateMenu();
    }

  if ( this->ModelSelector )
    {
    if  (this->MRMLScene != NULL )
      {
      this->ModelSelector->SetMRMLScene ( this->MRMLScene );
      }
    this->ModelSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                             ( vtkCommand *) this->GUICallbackCommand );
    this->ModelSelector->UpdateMenu();
    }

  if ( this->ModelOutSelector )
    {
    if  (this->MRMLScene != NULL )
      {
      this->ModelOutSelector->SetMRMLScene ( this->MRMLScene );
      }
    this->ModelOutSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                             ( vtkCommand *) this->GUICallbackCommand );
    this->ModelOutSelector->UpdateMenu();
    }

  if ( this->TransformSelector )
    {
    if  (this->MRMLScene != NULL )
      {
      this->TransformSelector->SetMRMLScene ( this->MRMLScene );
      }
    this->TransformSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                             ( vtkCommand *) this->GUICallbackCommand );
    this->TransformSelector->UpdateMenu();
    }

}



//---------------------------------------------------------------------------
void vtkModelTransformGUI::RemoveGUIObservers ( )
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
  if ( this->ParameterSelector  )
    {
    this->ParameterSelector->SetMRMLScene ( NULL );
    this->ParameterSelector ->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                            ( vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ModelSelector  )
    {
    this->ModelSelector->SetMRMLScene ( NULL );
    this->ModelSelector ->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                            ( vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ModelOutSelector  )
    {
    this->ModelOutSelector->SetMRMLScene ( NULL );
    this->ModelOutSelector ->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                            ( vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->TransformSelector  )
    {
    this->TransformSelector->SetMRMLScene ( NULL );
    this->TransformSelector ->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                            ( vtkCommand *)this->GUICallbackCommand );
    }
}




//---------------------------------------------------------------------------
void vtkModelTransformGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in.
}


//---------------------------------------------------------------------------
void vtkModelTransformGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *vtkNotUsed(callData))
{

  if ( !this->Built )
    {
    return;
    }
  if (this->ModelTransformNode == NULL)
    {
    this->CreateParameterNode();
    this->UpdateParameterNode();
    }
  if ( caller == this->ParameterSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    vtkMRMLModelTransformNode *model = vtkMRMLModelTransformNode::SafeDownCast ( this->ParameterSelector->GetSelected() );
    vtkSetAndObserveMRMLNodeMacro(this->ModelTransformNode, model);
    }
  else if ( caller == this->ModelSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast ( this->ModelSelector->GetSelected() );
    vtkSetAndObserveMRMLNodeMacro(this->ModelNode, model);
    if (this->ModelTransformNode) 
      {
      this->ModelTransformNode->SetInputModelID(model->GetID());
      }
    }
  else if ( caller == this->ModelOutSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast ( this->ModelOutSelector->GetSelected() );
    if (this->ModelTransformNode) 
      {
      this->ModelTransformNode->SetOutputModelID(model->GetID());
      }
    }
  else if ( caller == this->TransformSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    vtkMRMLTransformNode *model = vtkMRMLTransformNode::SafeDownCast ( this->TransformSelector->GetSelected() );
    vtkSetAndObserveMRMLNodeMacro(this->TransformNode, model);
    if (this->ModelTransformNode) 
      {
      this->ModelTransformNode->SetTransformNodeID(model->GetID());
      }
    }
}





//---------------------------------------------------------------------------
void vtkModelTransformGUI::ProcessMRMLEvents(vtkObject *caller,
                                          unsigned long event,
                                          void *vtkNotUsed(callData))
{
  if ( !this->Raised )
    {
    return;
    }
  if ( !this->Visited )
    {
    return;
    }

  if ( this->ApplicationGUI == NULL)
    {
    vtkErrorMacro ("ProcessMRMLEvents: ApplicationGUI is null");
    return;    
    }

  if (this->ModelTransformNode == NULL)
    {
    this->CreateParameterNode();
    this->UpdateParameterNode();
    }

  if (event == vtkCommand::ModifiedEvent &&
      this->ModelTransformNode)
    {
    this->DoTransformModel();
    }
  //vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
}

//---------------------------------------------------------------------------
void vtkModelTransformGUI::DoTransformModel ()
{

  if (this->GetMRMLScene() && this->ModelTransformNode)
    {
    vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(
                                          this->GetMRMLScene()->GetNodeByID(
                                          this->ModelTransformNode->GetTransformNodeID()));
    vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(
                                          this->GetMRMLScene()->GetNodeByID(
                                          this->ModelTransformNode->GetInputModelID()));
    vtkMRMLModelNode *modelOut = vtkMRMLModelNode::SafeDownCast(
                                          this->GetMRMLScene()->GetNodeByID(
                                          this->ModelTransformNode->GetOutputModelID()));

    vtkSlicerModelsLogic::TransformModel(tnode, modelNode, 
                                        this->ModelTransformNode->GetTransformNormals(), modelOut);

    if (modelOut && modelOut->GetDisplayNode() == NULL) 
      {
      vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
      displayNode->SetVisibility(1);
      this->GetMRMLScene()->AddNodeNoNotify(displayNode);
      modelOut->SetAndObserveDisplayNodeID(displayNode->GetID());  
      displayNode->SetPolyData( modelOut->GetPolyData() );
      if (this->ModelTransformNode->GetTransformNormals())
        {
        displayNode->SetBackfaceCulling(0);
        }
      displayNode->Delete();
      }
    if (modelOut && modelOut->GetStorageNode() == NULL)
      {
      vtkMRMLModelStorageNode *storageNode = vtkMRMLModelStorageNode::New();
      this->GetMRMLScene()->AddNodeNoNotify(storageNode);  
      modelOut->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
     }
   }
}

//---------------------------------------------------------------------------
void vtkModelTransformGUI::UpdateGUI ()
{
  if ( !this->Built )
    {
    return;
    }
  
  // update from MRML
  if ( this->UpdatingMRML )
    {
    return;
    }
  if ( this->UpdatingGUI )
    {
    return;
    }
  
  this->UpdatingGUI = 1;


  this->UpdatingGUI = 0;
}






//---------------------------------------------------------------------------
void vtkModelTransformGUI::BuildGUI ( ) 
{
  //--- include this to enable lazy building
  if ( !this->Visited )
    {
    return;
    }
  
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
  win->SetStatusText ( "Building Interface for ModelTransform Module...." );
  app->Script ( "update idletasks" );


  if ( this->MRMLScene != NULL )
    {
    vtkMRMLModelTransformNode* m = vtkMRMLModelTransformNode::New();
    this->MRMLScene->RegisterNodeClass(m);
    m->Delete();
    }
  else
    {
    vtkErrorMacro("GUI is being built before MRML Scene is set");
    return;
    }
      
  this->UIPanel->AddPage ( "ModelTransform", "ModelTransform", NULL );

  // HELP FRAME
  const char* about = "ModelTransform was developed by Alex Yarmarkovich. This work was supported by NA-MIC, NAC, BIRN, NCIGT, Harvard CTSC, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\n";
  
  const char *help = "**ModelTransform** is a module for creating a new model which is a transformed version of the input polygonal model.  \n\n **Usage:** To use this module, select an existing model from the scene, select a transform node, select or create the resulting model. The new model will be created, added to the scene under the parent of the transform node, and displayed in the 3D viewer.\n\n";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ModelTransform" );
  this->BuildHelpAndAboutFrame ( page, help, about );
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


  app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky e", NAMICLabel->GetWidgetName());
  app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky e",  NACLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky e",  BIRNLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky e",  NCIGTLabel->GetWidgetName());                  
  app->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 2 -sticky w",  CTSCLabel->GetWidgetName());                  
  app->Script ( "grid columnconfigure %s 0 -weight 0", this->GetLogoFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 0", this->GetLogoFrame()->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 2 -weight 1", this->GetLogoFrame()->GetWidgetName() );

  NACLabel->Delete();
  NAMICLabel->Delete();
  NCIGTLabel->Delete();
  BIRNLabel->Delete();
  CTSCLabel->Delete();  

  // MAIN UI FRAME
  this->SpecificationFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  SpecificationFrame->SetParent ( this->UIPanel->GetPageWidget ( "ModelTransform" ) );
  SpecificationFrame->Create ( );
  SpecificationFrame->ExpandFrame ( );
  SpecificationFrame->SetLabelText ("Transform Surface Models");
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2 -in %s",
                  SpecificationFrame->GetWidgetName(), this->UIPanel->GetPageWidget("ModelTransform")->GetWidgetName());

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( this->SpecificationFrame->GetFrame() );
  f->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",  f->GetWidgetName() );

  this->ParameterSelector = vtkSlicerNodeSelectorWidget::New();
  this->ParameterSelector->SetParent( f );
  this->ParameterSelector->Create();
  this->ParameterSelector->AddNodeClass("vtkMRMLModelTransformNode", NULL, NULL, NULL);
  this->ParameterSelector->SetChildClassesEnabled(0);
  this->ParameterSelector->SetNewNodeEnabled(1);
  this->ParameterSelector->SetShowHidden (1);
  this->ParameterSelector->SetMRMLScene(this->GetMRMLScene());
  this->ParameterSelector->GetWidget()->GetWidget()->SetWidth (24 );
  this->ParameterSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ParameterSelector->SetBorderWidth(2);
  this->ParameterSelector->SetPadX(2);
  this->ParameterSelector->SetPadY(2);
  this->ParameterSelector->SetLabelText( "Parameters ");
  this->ParameterSelector->SetBalloonHelpString("Select or create module parameters.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ParameterSelector->GetWidgetName());

  this->ModelSelector = vtkSlicerNodeSelectorWidget::New();
  this->ModelSelector->SetParent( f );
  this->ModelSelector->Create();
  this->ModelSelector->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->ModelSelector->SetChildClassesEnabled(1);
  this->ModelSelector->SetShowHidden (1);
  this->ModelSelector->SetMRMLScene(this->GetMRMLScene());
  this->ModelSelector->GetWidget()->GetWidget()->SetWidth (24 );
  this->ModelSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ModelSelector->SetBorderWidth(2);
  this->ModelSelector->SetPadX(2);
  this->ModelSelector->SetPadY(2);
  this->ModelSelector->SetLabelText( "Input Model ");
  this->ModelSelector->SetBalloonHelpString("Select a model to transform.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ModelSelector->GetWidgetName());

  this->TransformSelector = vtkSlicerNodeSelectorWidget::New();
  this->TransformSelector->SetParent( f );
  this->TransformSelector->Create();
  this->TransformSelector->AddNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, NULL);
  this->TransformSelector->SetChildClassesEnabled(1);
  this->TransformSelector->SetShowHidden (1);
  this->TransformSelector->SetMRMLScene(this->GetMRMLScene());
  this->TransformSelector->GetWidget()->GetWidget()->SetWidth (24 );
  this->TransformSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->TransformSelector->SetBorderWidth(2);
  this->TransformSelector->SetPadX(2);
  this->TransformSelector->SetPadY(2);
  this->TransformSelector->SetLabelText( "Linear transform ");
  this->TransformSelector->SetBalloonHelpString("Select a linear transform.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->TransformSelector->GetWidgetName());

  this->ModelOutSelector = vtkSlicerNodeSelectorWidget::New();
  this->ModelOutSelector->SetParent( f );
  this->ModelOutSelector->Create();
  this->ModelOutSelector->AddNodeClass("vtkMRMLModelNode", NULL, NULL, "ModelTransformed");
  this->ModelOutSelector->SetChildClassesEnabled(1);
  this->ModelOutSelector->SetNoneEnabled(1);
  this->ModelOutSelector->SetShowHidden (1);
  this->ModelOutSelector->SetNewNodeEnabled(1);
  this->ModelOutSelector->SetMRMLScene(this->GetMRMLScene());
  this->ModelOutSelector->GetWidget()->GetWidget()->SetWidth (24 );
  this->ModelOutSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ModelOutSelector->SetBorderWidth(2);
  this->ModelOutSelector->SetPadX(2);
  this->ModelOutSelector->SetPadY(2);
  this->ModelOutSelector->SetLabelText( "Output Model");
  this->ModelOutSelector->SetBalloonHelpString("Select an output model.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ModelOutSelector->GetWidgetName());


  f->Delete();

  this->Init();
  this->Built = true;
}


//---------------------------------------------------------------------------
void vtkModelTransformGUI::Init ( )
{
  this->CreateParameterNode();
  this->UpdateParameterNode();
}

//---------------------------------------------------------------------------
void vtkModelTransformGUI::SetSlicerText(const char *txt)
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
void vtkModelTransformGUI::CreateParameterNode ( )
{
  if (this->GetMRMLScene() == NULL)
  {
    return;
  }

  vtkMRMLModelTransformNode *param = NULL;

  param = vtkMRMLModelTransformNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLModelTransformNode"));
  if (param == NULL)
    {
    param = vtkMRMLModelTransformNode::New();
    this->GetMRMLScene()->AddNodeNoNotify(param);
    param->Delete();
    }
  vtkSetAndObserveMRMLNodeMacro(this->ModelTransformNode, param);

}

//---------------------------------------------------------------------------
void vtkModelTransformGUI::UpdateParameterNode ( )
{
  if (this->ModelSelector == NULL || 
      this->ModelOutSelector == NULL ||
      this->TransformSelector == NULL)
    {
    return;
    }

  vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast ( this->ModelSelector->GetSelected() );
  vtkSetAndObserveMRMLNodeMacro(this->ModelNode, model);
  if (this->ModelTransformNode && model) 
    {
    this->ModelTransformNode->SetInputModelID(model->GetID());
    }

  model = vtkMRMLModelNode::SafeDownCast ( this->ModelOutSelector->GetSelected() );
  if (this->ModelTransformNode && model) 
    {
    this->ModelTransformNode->SetOutputModelID(model->GetID());
    }
 
  vtkMRMLTransformNode *xform = vtkMRMLTransformNode::SafeDownCast ( this->TransformSelector->GetSelected() );
  vtkSetAndObserveMRMLNodeMacro(this->TransformNode, xform);
  if (this->ModelTransformNode && xform) 
    {
    this->ModelTransformNode->SetTransformNodeID(xform->GetID());
    }
}
