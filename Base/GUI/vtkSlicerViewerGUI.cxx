#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerViewerGUI.h"

#include "vtkKWWidget.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewerGUI );
vtkCxxRevisionMacro ( vtkSlicerViewerGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerViewerGUI::vtkSlicerViewerGUI ( )
{
  this->MainViewer = vtkKWRenderWidget::New ( );
  //this->Logic = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerViewerGUI::~vtkSlicerViewerGUI ( )
{
  if ( this->MainViewer ) 
    {
    this->MainViewer->RemoveAllViewProps ( );
    this->MainViewer->Delete ( );
    this->MainViewer = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  
  os << indent << "vtkSlicerViewerGUI: " << this->GetClassName ( ) << "\n";
  //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
  // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::RemoveGUIObservers ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::AddGUIObservers ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
  this->UpdateFromMRML();
  
}

//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::Exit ( )
{
    // Fill in
}




//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::BuildGUI ( )
{
  if ( this->MainViewer != NULL ) 
    {
    if (this->MainSlicerWindow != NULL)
      {
      this->MainViewer->SetParent (this->MainSlicerWindow->GetViewFrame ( ) );
      }
    this->MainViewer->Create ( );

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication()); 
    app->Script  ("pack %s -side top -fill both -expand y -padx 0 -pady 0",
                  this->MainViewer->GetWidgetName ( ) );
    this->MainViewer->SetRendererBackgroundColor ( 
      app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
    
    this->MainViewer->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();
    
    // set up antialiasing
    this->MainViewer->GetRenderWindow()->LineSmoothingOn();
    this->MainViewer->GetRenderWindow()->PolygonSmoothingOn ( );
    this->MainViewer->GetRenderWindow()->PointSmoothingOn();
    // this->MainViewer->SetMultiSamples ( 4 );
    
    MainViewer->ResetCamera ( );
    
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerGUI::UpdateFromMRML ( )
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  
  this->MainViewer->RemoveAllViewProps ( );
  
  scene->InitTraversal();
  while (node=scene->GetNextNodeByClass("vtkMRMLModelNode"))
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(node);
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::New ();
    mapper->SetInput ( model->GetPolyData() );
    vtkActor *actor = vtkActor::New ( );
    actor->SetMapper ( mapper );
    // don't add the actor, so we can see the interactor
    this->MainViewer->AddViewProp ( actor );

    actor->Delete();
    mapper->Delete();
    }
}

