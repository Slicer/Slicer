
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

#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
#include "vtkDistanceWidget.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkPolyDataPointPlacer.h"

#include "vtkAngleWidget.h"
#include "vtkAngleRepresentation2D.h"
#include "vtkAngleRepresentation3D.h"
#endif

#include "vtkProperty.h"
#include "vtkAxisActor2D.h"

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

#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  this->RulerModel1PointPlacer = vtkPolyDataPointPlacer::New();

  this->DistanceHandleRepresentation = vtkPointHandleRepresentation3D::New();
  this->DistanceHandleRepresentation->GetProperty()->SetColor(1, 0, 0);

  this->DistanceRepresentation = vtkDistanceRepresentation2D::New();
  this->DistanceRepresentation->SetHandleRepresentation(this->DistanceHandleRepresentation);
  this->DistanceRepresentation->GetAxis()->SetNumberOfMinorTicks(4);
  this->DistanceRepresentation->GetAxis()->SetTickLength(9);
  this->DistanceRepresentation->GetAxis()->SetTitlePosition(0.2);

  this->DistanceWidget = vtkDistanceWidget::New();
  this->DistanceWidget->CreateDefaultRepresentation();
  this->DistanceWidget->SetRepresentation(this->DistanceRepresentation);

  this->AngleHandleRepresentation = vtkPointHandleRepresentation3D::New();
  this->AngleHandleRepresentation->GetProperty()->SetColor(1, 0, 0);

  this->AngleRepresentation = vtkAngleRepresentation3D::New();
  this->AngleRepresentation->SetHandleRepresentation(this->AngleHandleRepresentation);
  double textscale[3] = {1.0, 1.0, 1.0};
  this->AngleRepresentation->SetTextActorScale(textscale);

  this->AngleWidget = vtkAngleWidget::New();
  this->AngleWidget->CreateDefaultRepresentation();
  this->AngleWidget->SetRepresentation(this->AngleRepresentation);
#endif
  this->RulerCheckButton = NULL;
  this->RulerModel1SelectorWidget = NULL;
  this->AngleCheckButton = NULL;
}

//----------------------------------------------------------------------------
vtkMeasurementsGUI::~vtkMeasurementsGUI()
{
//    this->RemoveMRMLNodeObservers ( );
//    this->RemoveLogicObservers ( );
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  if (this->RulerModel1PointPlacer)
    {
    this->RulerModel1PointPlacer->Delete();
    this->RulerModel1PointPlacer = NULL;
    }
  if (this->DistanceHandleRepresentation)
    {
    this->DistanceHandleRepresentation->Delete();
    this->DistanceHandleRepresentation = NULL;
    }
  if (this->DistanceRepresentation)
    {
    this->DistanceRepresentation->SetHandleRepresentation(NULL);
    this->DistanceRepresentation->Delete();
    this->DistanceRepresentation = NULL;
    }
  if (this->DistanceWidget)
    {
    this->DistanceWidget->SetRepresentation(NULL);
    this->DistanceWidget->Delete();
    this->DistanceWidget = NULL;
    }

  if (this->AngleHandleRepresentation)
    {
    this->AngleHandleRepresentation->Delete();
    this->AngleHandleRepresentation = NULL;
    }
  if (this->AngleRepresentation)
    {
    this->AngleRepresentation->SetHandleRepresentation(NULL);
    this->AngleRepresentation->Delete();
    this->AngleRepresentation = NULL;
    }
  if (this->AngleWidget)
    {
    this->AngleWidget->SetRepresentation(NULL);
    this->AngleWidget->Delete();
    this->AngleWidget = NULL;
    }
#endif
  if (this->RulerCheckButton)
    {
    this->RulerCheckButton->SetParent(NULL);
    this->RulerCheckButton->Delete();
    this->RulerCheckButton = NULL;
    }
  if (this->RulerModel1SelectorWidget)
    {
    this->RulerModel1SelectorWidget->SetParent(NULL);
    this->RulerModel1SelectorWidget->Delete();
    this->RulerModel1SelectorWidget = NULL;
    }
  if ( this->AngleCheckButton )
    {
    this->AngleCheckButton->SetParent(NULL);
    this->AngleCheckButton->Delete();
    this->AngleCheckButton = NULL;
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
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  if (this->DistanceWidget)
    {
    os << indent << "Distance Widget:\n";
    this->DistanceWidget->PrintSelf(os, indent);
    }
  if (this->AngleWidget)
    {
    os << indent << "Angle Widget:\n";
    this->AngleWidget->PrintSelf(os, indent);
    }
#else
  os << indent << "Measurements GUI: incompatible VTK version " <<  VTK_MAJOR_VERSION << "." << VTK_MINOR_VERSION << ", these widgets are only available in VTK 5.4 or later\n";
#endif
}


//---------------------------------------------------------------------------
void vtkMeasurementsGUI::AddGUIObservers ( ) 
{
  if (this->RulerCheckButton)
    {
    this->RulerCheckButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerModel1SelectorWidget)
    {
    this->RulerModel1SelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->AngleCheckButton )
    {
    this->AngleCheckButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

}



//---------------------------------------------------------------------------
void vtkMeasurementsGUI::RemoveGUIObservers ( )
{
  if (this->RulerCheckButton)
    {
    this->RulerCheckButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RulerModel1SelectorWidget)
    {
    this->RulerModel1SelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->AngleCheckButton )
    {
    this->AngleCheckButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkMeasurementsGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
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

  if (b == this->RulerCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent ) 
    {
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
    if ( this->RulerCheckButton->GetSelectedState())
      {
      if (this->DistanceWidget->GetInteractor() == NULL)
        {
        this->DistanceWidget->SetInteractor(appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
        double p1[3] = {0.5, 0.0, 0.0};
        double p2[3] = {-0.5, 0.0, 0.0};
        this->DistanceRepresentation->SetPoint1WorldPosition(p1);
        this->DistanceRepresentation->SetPoint2WorldPosition(p2);
        }
      this->DistanceWidget->On();
      }
    else
      {
      this->DistanceWidget->Off();
      }
#else
    vtkWarningMacro("Measurement widgets not available in this version of VTK, needs 5.4 or later");
#endif
    }
  if (b == this->AngleCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent ) 
    {
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
    if (this->AngleCheckButton->GetSelectedState())
      {
      if (this->AngleWidget->GetInteractor() == NULL)
        {
        this->AngleWidget->SetInteractor(appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
        }
      this->AngleWidget->On();
      }
    else
      {
      this->AngleWidget->Off();
      }
#else
    vtkWarningMacro("Angle widget only availalbe with VTK version 5.4 or higher");
#endif
    }
  
  if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->RulerModel1SelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
    vtkMRMLModelNode *model = 
      vtkMRMLModelNode::SafeDownCast(this->RulerModel1SelectorWidget->GetSelected());
    if (model != NULL  && model->GetDisplayNode() != NULL)
      {
      this->RulerModel1PointPlacer->AddProp(appGUI->GetViewerWidget()->GetActorByID(model->GetDisplayNode()->GetID()));
      this->DistanceHandleRepresentation->ConstrainedOn();
      this->DistanceHandleRepresentation->SetPointPlacer(this->RulerModel1PointPlacer);
      }
    else
      {
      this->RulerModel1PointPlacer->RemoveAllProps();
      this->DistanceHandleRepresentation->ConstrainedOff();
      this->DistanceHandleRepresentation->SetPointPlacer(NULL);
      }
#else
    vtkWarningMacro("Distance widget only available with VTK 5.4 and later");
#endif
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
  const char* help_text = "The Measurements module allows you to add a 3d ruler and a 3d angle widget to the 3D window. The first time you toggle one of the widgets on, you have to click twice for the ruler and three times for the angle widget in order to set the line end points. After that, you can move the points around by dragging the handles.\n\nOnly available when Slicer3 is compiled with VTK version 5.4 or higher.";
  const char* ack_text = "Measurements was developed by Nicole Aucoin with help from Kitware, Inc.";
  this->BuildHelpAndAboutFrame(page, help_text, ack_text);

  // Ruler
  this->RulerCheckButton = vtkKWCheckButton::New();
  this->RulerCheckButton->SetParent( page );
  this->RulerCheckButton->Create();
  this->RulerCheckButton->SetText("Toggle Ruler Widget");
  this->RulerCheckButton->SelectedStateOff();
  this->RulerCheckButton->IndicatorVisibilityOff();
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
              this->RulerCheckButton->GetWidgetName());

  this->RulerModel1SelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->RulerModel1SelectorWidget->SetParent ( page );
  this->RulerModel1SelectorWidget->Create ( );
  this->RulerModel1SelectorWidget->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->RulerModel1SelectorWidget->SetChildClassesEnabled(0);
  this->RulerModel1SelectorWidget->NoneEnabledOn();
  this->RulerModel1SelectorWidget->SetShowHidden(1);
  this->RulerModel1SelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->RulerModel1SelectorWidget->SetBorderWidth(2);
  this->RulerModel1SelectorWidget->SetPadX(2);
  this->RulerModel1SelectorWidget->SetPadY(2);
  this->RulerModel1SelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->RulerModel1SelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->RulerModel1SelectorWidget->SetLabelText( "Select Ruler Model 1: ");
  this->RulerModel1SelectorWidget->SetBalloonHelpString("Select a model on which to anchor the first end of the ruler.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->RulerModel1SelectorWidget->GetWidgetName());


  // Angle 
  this->AngleCheckButton = vtkKWCheckButton::New();
  this->AngleCheckButton->SetParent( page );
  this->AngleCheckButton->Create();
  this->AngleCheckButton->SetText("Toggle Angle Widget");
  this->AngleCheckButton->SelectedStateOff();
  this->AngleCheckButton->IndicatorVisibilityOff();
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
              this->AngleCheckButton->GetWidgetName());
}

//---------------------------------------------------------------------------
void vtkMeasurementsGUI::Init ( )
{
}
