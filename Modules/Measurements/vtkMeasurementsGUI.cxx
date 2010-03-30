
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"

#include "vtkMeasurementsGUI.h"

#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>


#include "vtkKWCheckButton.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkGeneralTransform.h"

#include "vtkMeasurementsAngleWidget.h"

#include "vtkAffineWidget.h"
#include "vtkAffineRepresentation2D.h"


#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkAxisActor2D.h"

#include "vtkTransform.h"

#include "vtkMeasurementsRulerWidget.h"

class vtkAffineCallback : public vtkCommand
{
public:
  static vtkAffineCallback *New() 
    { return new vtkAffineCallback; }
  virtual void Execute(vtkObject *caller, unsigned long, void*);
  vtkAffineCallback():TransformableNode(0),AffineRep(0)
    {
    this->Transform = vtkTransform::New();
    }
  ~vtkAffineCallback()
    {
    if (this->Transform)
      {
      this->Transform->Delete();
      }
    }
  vtkMRMLTransformableNode *TransformableNode;
  vtkAffineRepresentation2D *AffineRep;
  vtkTransform *Transform;
};

void vtkAffineCallback::Execute(vtkObject*, unsigned long, void*)
{
  if (this->AffineRep)
   {
    this->AffineRep->GetTransform(this->Transform);
    if (this->TransformableNode)
      {
      if (this->TransformableNode->GetTransformNodeID() == NULL &&
          this->TransformableNode->GetScene() != NULL)
        {
        vtkMRMLLinearTransformNode *t = vtkMRMLLinearTransformNode::New();
        vtkMRMLNode *newNode = this->TransformableNode->GetScene()->AddNode(t);
        if (newNode->GetID() != NULL)
          {
          this->TransformableNode->SetAndObserveTransformNodeID(newNode->GetID());
          }
        else
          {
          std::cerr << "Unable to add new transform node to the scene!\n";
          }
        t->Delete();
        }
    if (this->TransformableNode->GetTransformNodeID() == NULL)
      {
      std::cerr << "Unable to find or create a transform node for " << this->TransformableNode->GetName() << std::endl;
      }
    else
      {
      vtkMRMLLinearTransformNode *t = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformableNode->GetScene()->GetNodeByID(this->TransformableNode->GetTransformNodeID()));
      if (t)
        {
        // now we have the node's transform node, get at the matrix to update it
        vtkMatrix4x4 *g = t->GetMatrixTransformToParent();
        if (g && this->Transform)
          {
          g->DeepCopy(this->Transform->GetMatrix());
          }
        else 
          {
          std::cerr << "ERROR: unable to set the transformable node's matrix from the affine widget's transfrom matrix\n";
          } 
        }
      }
    }
  }
  else
    {
    std::cerr << "Affine callback: affine representation is null\n";
    }
}


//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMeasurementsGUI, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkMeasurementsGUI* vtkMeasurementsGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkMeasurementsGUI");

  if(ret)
    {
      return (vtkMeasurementsGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMeasurementsGUI;
}



//----------------------------------------------------------------------------
vtkMeasurementsGUI::vtkMeasurementsGUI()
{
  this->Logic = NULL;

  // Ruler Widget set up
  this->RulerWidget = NULL;
  
  // Affine Widget set up
  this->TransformRepresentation = vtkAffineRepresentation2D::New();
  this->TransformRepresentation->SetBoxWidth(100);
  this->TransformRepresentation->SetCircleWidth(75);
  this->TransformRepresentation->SetAxesWidth(60);
  this->TransformRepresentation->DisplayTextOn();

  this->TransformWidget = vtkAffineWidget::New();
  this->TransformWidget->SetRepresentation(this->TransformRepresentation);

  this->AngleWidget = NULL;

  this->TransformCheckButton = NULL;
  this->TransformableNodeSelectorWidget = NULL;
}

//----------------------------------------------------------------------------
vtkMeasurementsGUI::~vtkMeasurementsGUI()
{
//    this->RemoveMRMLNodeObservers ( );
//    this->RemoveLogicObservers ( );


  if (this->RulerWidget)
    {
    this->RulerWidget->SetParent(NULL);
    this->RulerWidget->Delete();
    this->RulerWidget = NULL;
    }

  if (this->AngleWidget)
    {
    this->AngleWidget->SetParent(NULL);
    this->AngleWidget->Delete();
    this->AngleWidget = NULL;
    }
  if (this->TransformRepresentation)
    {
    this->TransformRepresentation->Delete();
    this->TransformRepresentation = NULL;
    }
  if (this->TransformWidget)
    {
    this->TransformWidget->SetRepresentation(NULL);
    this->TransformWidget->Delete();
    this->TransformWidget = NULL;
    }

  if (this->TransformCheckButton)
    {
    this->TransformCheckButton->SetParent(NULL);
    this->TransformCheckButton->Delete();
    this->TransformCheckButton = NULL;
    }
  if (this->TransformableNodeSelectorWidget)
    {
    this->TransformableNodeSelectorWidget->SetParent(NULL);
    this->TransformableNodeSelectorWidget->Delete();
    this->TransformableNodeSelectorWidget = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMeasurementsGUI::Enter()
{

}


//----------------------------------------------------------------------------
void vtkMeasurementsGUI::Exit ( )
{

}


//----------------------------------------------------------------------------
vtkIntArray *vtkMeasurementsGUI::NewObservableEvents()
{
 vtkIntArray *events = vtkIntArray::New();
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkMeasurementsGUI::TearDownGUI ( )
{

  this->RemoveGUIObservers ( );
  this->SetLogic ( NULL );
  this->SetAndObserveMRMLScene ( NULL );

}


//----------------------------------------------------------------------------
void vtkMeasurementsGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  if (this->TransformWidget)
    {
    os << indent << "Transform Widget:\n";
    this->TransformWidget->PrintSelf(os, indent);
    }
}


//---------------------------------------------------------------------------
void vtkMeasurementsGUI::AddGUIObservers ( ) 
{
  if (this->TransformCheckButton)
    {
    this->TransformCheckButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->TransformableNodeSelectorWidget)
    {
    this->TransformableNodeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkMeasurementsGUI::RemoveGUIObservers ( )
{
  if (this->TransformCheckButton)
    {
    this->TransformCheckButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->TransformableNodeSelectorWidget)
    {
    this->TransformableNodeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkMeasurementsGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *vtkNotUsed(callData) ) 
{

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

  
  vtkKWCheckButton *b = vtkKWCheckButton::SafeDownCast ( caller ); 

  if (b == this->TransformCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent ) 
    {
    if ( this->TransformCheckButton->GetSelectedState())
      {
      if (this->TransformWidget->GetInteractor() == NULL)
        {
        if (appGUI->GetActiveViewerWidget())
          {
          this->TransformWidget->SetInteractor(appGUI->GetActiveViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
          }
        double bounds[6] = {-50, 0, -50, 0, 0, 0};
        this->TransformRepresentation->PlaceWidget(bounds);
        }
        // reset the node if it's changed
      this->UpdateTransformableNode();
      this->TransformWidget->On();
      }
    else
      {
      this->TransformWidget->Off();
      }
    }
  if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->TransformableNodeSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
      this->UpdateTransformableNode();
    }
}


//---------------------------------------------------------------------------
void vtkMeasurementsGUI::SetStatusText(const char *txt)
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
void vtkMeasurementsGUI::BuildGUI ( ) 
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
  win->SetStatusText ( "Building Interface for Measurements Module...." );
  app->Script ( "update idletasks" );

  this->UIPanel->AddPage ( "Measurements", "Measurements", NULL );
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Measurements" );    

  // HELP
  const char* help_text = "The Measurements module allows you to add 3d rulers, 3d angle widgets, or a 2d affine transform editor widget to the 3D window.\nThe first time you toggle on the angle or affine transform widget, you have to click once to render the transform widget, and three times to place the angle widget end points. After that, you can move the widgets around by dragging the handles.\nIn order to get the end points of the ruler or the angle to move along a selected model surface, you currently need to rotate the view so that the handle is rendered on top of the model, the next click will snap it to the model surface. If you move the model or slice plane, the measurement point won't move with it, the constraint only works when moving the end point handle. If you make the model or slice invisible, the end point cannot be moved, as the constraint will always be false.\n\nOnly available when Slicer3 is compiled with VTK version 5.4 or higher.\nFor more information, see <a>http://www.slicer.org/slicerWiki/index.php/Modules:Measurements-Documentation-3.5</a>\n\n\nUNDER CONSTRUCTION";
  const char* ack_text = "Measurements was developed by Nicole Aucoin with help from Kitware, Inc.";
  this->BuildHelpAndAboutFrame(page, help_text, ack_text);

 
  //
  // Ruler
  //
  vtkSlicerModuleCollapsibleFrame *rulerFrame = vtkSlicerModuleCollapsibleFrame::New();
  rulerFrame->SetParent(page);
  rulerFrame->Create();
  rulerFrame->SetLabelText("Ruler Widget");
  rulerFrame->ExpandFrame();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", rulerFrame->GetWidgetName());
  
  this->RulerWidget = vtkMeasurementsRulerWidget::New ( );
  this->RulerWidget->SetMRMLScene(this->GetMRMLScene() );
  if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
    this->RulerWidget->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
  else { vtkWarningMacro("Unable to pass the viewer widget to the ruler widget"); }
  this->RulerWidget->AddMRMLObservers();
  this->RulerWidget->SetParent ( rulerFrame->GetFrame() );
  this->RulerWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                this->RulerWidget->GetWidgetName());

  //
  // Angle Frame
  //
  vtkSlicerModuleCollapsibleFrame *angleFrame = vtkSlicerModuleCollapsibleFrame::New();
  angleFrame->SetParent(page);
  angleFrame->Create();
  angleFrame->SetLabelText("Angle Widget");
  angleFrame->CollapseFrame();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", angleFrame->GetWidgetName());
  
  this->AngleWidget = vtkMeasurementsAngleWidget::New ( );
  this->AngleWidget->SetMRMLScene(this->GetMRMLScene() );
  if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
    this->AngleWidget->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
  else { vtkWarningMacro("Unable to pass the viewer widget to the ruler widget"); }
  this->AngleWidget->AddMRMLObservers();
  this->AngleWidget->SetParent ( angleFrame->GetFrame() );
  this->AngleWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                this->AngleWidget->GetWidgetName());

  //
  // Transform Frame
  //
  vtkSlicerModuleCollapsibleFrame *transformFrame = vtkSlicerModuleCollapsibleFrame::New();
  transformFrame->SetParent(page);
  transformFrame->Create();
  transformFrame->SetLabelText("Transform Widget");
  transformFrame->ExpandFrame();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", transformFrame->GetWidgetName());
  
  // Transform
  this->TransformCheckButton = vtkKWCheckButton::New();
  this->TransformCheckButton->SetParent( transformFrame->GetFrame() );
  this->TransformCheckButton->Create();
  this->TransformCheckButton->SetText("Toggle Transform Widget");
  this->TransformCheckButton->SelectedStateOff();
  this->TransformCheckButton->IndicatorVisibilityOff();
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
              this->TransformCheckButton->GetWidgetName());

  this->TransformableNodeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->TransformableNodeSelectorWidget->SetParent ( transformFrame->GetFrame() );
  this->TransformableNodeSelectorWidget->Create ( );
  this->TransformableNodeSelectorWidget->AddNodeClass("vtkMRMLTransformableNode", NULL, NULL, NULL);
  // show all transformable nodes
  this->TransformableNodeSelectorWidget->SetChildClassesEnabled(1);
  this->TransformableNodeSelectorWidget->NoneEnabledOn();
  // but don't show hidden ones, since we don't want to transform colour nodes
  this->TransformableNodeSelectorWidget->SetShowHidden(0);
  // and don't allow transform nodes themselves
  this->TransformableNodeSelectorWidget->AddExcludedChildClass("vtkMRMLLinearTransformNode");
  this->TransformableNodeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->TransformableNodeSelectorWidget->SetBorderWidth(2);
  this->TransformableNodeSelectorWidget->SetPadX(2);
  this->TransformableNodeSelectorWidget->SetPadY(2);
  this->TransformableNodeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->TransformableNodeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->TransformableNodeSelectorWidget->SetLabelText( "Select Data to Transform: ");
  this->TransformableNodeSelectorWidget->SetBalloonHelpString("Select a node on which to create/manipulate the transform.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->TransformableNodeSelectorWidget->GetWidgetName());

  rulerFrame->Delete();
  angleFrame->Delete();
  transformFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkMeasurementsGUI::Init ( )
{ 
}

//---------------------------------------------------------------------------
void vtkMeasurementsGUI::UpdateTransformableNode()
{
    vtkMRMLTransformableNode *transformableNode = 
      vtkMRMLTransformableNode::SafeDownCast(this->TransformableNodeSelectorWidget->GetSelected());
    if (transformableNode != NULL)
      {
      // remove current observers
      this->TransformWidget->RemoveObservers(vtkCommand::InteractionEvent);
      // set up a call back and observer
      vtkAffineCallback *acbk = vtkAffineCallback::New();
      acbk->AffineRep = this->TransformRepresentation;
      acbk->TransformableNode = transformableNode;
      this->TransformWidget->AddObserver(vtkCommand::InteractionEvent,acbk);
      acbk->Delete();
      }
}

//---------------------------------------------------------------------------
void vtkMeasurementsGUI::SetActiveViewer(vtkSlicerViewerWidget *activeViewer )
{
  if (activeViewer == NULL)
    {
    vtkDebugMacro("SetActiveViewer: active viewer is NULL, unsetting the viewer widget.");
    }
  else
    {
    vtkDebugMacro("SetActiveViewer: resetting the active view.");
    }
  // update the active viewer where needed
  if (this->RulerWidget)
    {
    this->RulerWidget->SetViewerWidget(activeViewer);
    }
  if (this->AngleWidget)
    {
    this->AngleWidget->SetViewerWidget(activeViewer);
    }
  if (this->TransformWidget)
    {
    if (activeViewer &&
        activeViewer->GetMainViewer())
      {
      this->TransformWidget->SetInteractor(activeViewer->GetMainViewer()->GetRenderWindowInteractor());
      }
    else
      {
      this->TransformWidget->SetInteractor(NULL);
      }
    }
}
