#include "vtkObjectFactory.h"

#include "vtkSlicerSecondaryViewerWindow.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"

#include "vtkRenderer.h"

#include "vtkCornerAnnotation.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerFiducialListWidget.h"
#include "vtkSlicerROIViewerWidget.h"
#include "vtkSlicerROIViewerWidget.h"
#include "vtkMRMLViewNode.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSecondaryViewerWindow);
vtkCxxRevisionMacro(vtkSlicerSecondaryViewerWindow, "$Revision: 1.0 $");
//----------------------------------------------------------------------------
vtkSlicerSecondaryViewerWindow::vtkSlicerSecondaryViewerWindow()
{
  this->MultipleMonitorsAvailable = false; 
  this->SecondaryMonitorPosition[0]=0;
  this->SecondaryMonitorPosition[1]=0;
  this->SecondaryMonitorSize[0]=0;
  this->SecondaryMonitorSize[1]=0;

  // Cannot create ViewerWidget object here, because calling New() and Delete() on a ViewerWidget crashes (you must call Create() before Delete()) 
  // To have a consistent behavior for all windows/widgets, we create all of them in the CreateWidget method.
  this->MainFrame = NULL;
  this->ViewerWidget = NULL; 
  this->FiducialListWidget = NULL;
  this->ROIViewerWidget = NULL;

  this->ApplicationGUI = NULL;
}


//----------------------------------------------------------------------------
vtkSlicerSecondaryViewerWindow::~vtkSlicerSecondaryViewerWindow()
{
  Destroy();  
}

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

#ifdef _WIN32
//----------------------------------------------------------------------------
BOOL CALLBACK StoreNonPrimaryMonitorRectEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT prc, LPARAM dwData) 
{
  MONITORINFO mi;
  mi.cbSize = sizeof(MONITORINFO);
  if (GetMonitorInfo(hMonitor, &mi))
  {
    if (!(mi.dwFlags & MONITORINFOF_PRIMARY))
    {
      RECT *rect = (RECT*)dwData;
      if (rect==NULL)
      {
        return false;
      }
      // store the rect of the non-primary monitor only
      rect->left=mi.rcMonitor.left;
      rect->right=mi.rcMonitor.right;
      rect->top=mi.rcMonitor.top;
      rect->bottom=mi.rcMonitor.bottom;     
    }
  }
  return true;
} 

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::UpdateSecondaryMonitorPosition()
{
  // Window rect
  RECT rect;    

  // Fill rect by default with primary monitor info
  rect.left=0;
  rect.right=GetSystemMetrics(SM_CXFULLSCREEN);
  rect.top=0;
  rect.bottom=GetSystemMetrics(SM_CYFULLSCREEN);    

  // if a secondary monitor is available then get its virtual screen coordinates
  int nMonitors = GetSystemMetrics(SM_CMONITORS);
  if (nMonitors>1)
  {
    this->MultipleMonitorsAvailable = true; 
    if (!EnumDisplayMonitors(NULL, NULL, StoreNonPrimaryMonitorRectEnumProc, (LPARAM) &rect))
    {
      vtkErrorMacro("DetectMonitors: EnumDisplayMonitors failed");
    }
  }
  else
  {
    this->MultipleMonitorsAvailable = false; 
  }

  this->SecondaryMonitorPosition[0]=rect.left;
  this->SecondaryMonitorPosition[1]=rect.top;
  this->SecondaryMonitorSize[0]=rect.right-rect.left;
  this->SecondaryMonitorSize[1]=rect.bottom-rect.top;
}

#else // _WIN32

void vtkSlicerSecondaryViewerWindow::UpdateSecondaryMonitorPosition()
{
  // TODO: implement monitor detection for linux
  this->MultipleMonitorsAvailable = false; 
  this->SecondaryMonitorPosition[0]=0;
  this->SecondaryMonitorPosition[1]=0;
  this->SecondaryMonitorSize[0]=1024;
  this->SecondaryMonitorSize[1]=768;
}

#endif // _WIN32

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::CreateWidget()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app==NULL)
  {
    vtkErrorMacro("CreateWindow: application is invalid");
    return;
  }

  vtkSlicerApplicationGUI* gui=GetApplicationGUI();
  if (app==NULL)
  {
    vtkErrorMacro("CreateWindow: gui is invalid");
    return;
  }

  if (this->IsCreated())
  {
    return;
  }

  vtkKWTopLevel::CreateWidget();

  this->SetApplication ( app );
  this->SetBorderWidth ( 1 );
  this->SetReliefToFlat();

  this->SetTitle ("3D Slicer -- Secondary Window");
  this->SetSize (450, 450);
  this->Withdraw();

  this->MainFrame = vtkKWFrame::New();
  this->MainFrame->SetParent ( this );
  this->MainFrame->Create();
  this->MainFrame->SetBorderWidth ( 1 );
  this->Script ( "pack %s -side top -anchor nw -fill both -expand 1 -padx 0 -pady 1", this->MainFrame->GetWidgetName() ); 
  
  // Create list of MRML events for that the widgets will observe
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  // Create view node
  vtkSmartPointer<vtkMRMLViewNode> viewNode = vtkSmartPointer<vtkMRMLViewNode>::New();
  viewNode->SetName(gui->GetMRMLScene()->GetUniqueNameByString("ProstateNavSecondaryWindow"));  
  
  // Don't show the viewer widget in the main viewer (it is important because having multiple widgets for the same view node
  // causes hang in camera clipping range computation). We will create the viewer widget for showing it in our secondary viewer window.
  viewNode->SetVisibility(false);

  // Don't show the view node for the user
  viewNode->SetHideFromEditors(true);

  gui->GetMRMLScene()->AddNode(viewNode);  

  // Create the 3D Viewer
  this->ViewerWidget = vtkSlicerViewerWidget::New();  
  this->ViewerWidget->SetApplication( app );
  this->ViewerWidget->SetParent(this->MainFrame);
  this->ViewerWidget->Create();  
  this->ViewerWidget->SetAndObserveMRMLSceneEvents (gui->GetMRMLScene(), events );
  this->ViewerWidget->SetAndObserveViewNode (viewNode);
  this->ViewerWidget->UpdateFromMRML();
  this->ViewerWidget->SetApplicationLogic ( gui->GetApplicationLogic() );

  // Add the fiducial list widget  
  this->FiducialListWidget=vtkSlicerFiducialListWidget::New();
  this->FiducialListWidget->SetApplication( app );
  this->FiducialListWidget->SetViewerWidget(this->ViewerWidget);
  this->FiducialListWidget->SetInteractorStyle(vtkSlicerViewerInteractorStyle::SafeDownCast(this->ViewerWidget->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle()));
  this->FiducialListWidget->Create();
  this->FiducialListWidget->SetAndObserveMRMLSceneEvents (gui->GetMRMLScene(), events );
  this->FiducialListWidget->UpdateFromMRML();

  // Add the roi widget
  this->ROIViewerWidget=vtkSlicerROIViewerWidget::New();
  this->ROIViewerWidget->SetApplication( app );
  this->ROIViewerWidget->SetMainViewerWidget(this->ViewerWidget);
  this->ROIViewerWidget->SetMRMLScene(gui->GetMRMLScene());
  this->ROIViewerWidget->Create();
  this->ROIViewerWidget->UpdateFromMRML();

  this->ViewerWidget->PackWidget(this->MainFrame); 
}

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::DisplayOnSecondaryMonitor()
{
  this->DeIconify();
  this->Raise();

  this->UpdateSecondaryMonitorPosition();

  this->SetPosition(this->SecondaryMonitorPosition[0], this->SecondaryMonitorPosition[1]);
  this->SetSize(this->SecondaryMonitorSize[0], this->SecondaryMonitorSize[1]);

  if (this->MultipleMonitorsAvailable)
  {
    // this->HideDecorationOn(); // always show decoration (window title bar), otherwise Slicer main window can become inaccessible
  }
}

//----------------------------------------------------------------------------
vtkSlicerApplicationGUI* vtkSlicerSecondaryViewerWindow::GetApplicationGUI()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app!=NULL)
  {
    vtkSlicerApplicationGUI* gui=app->GetApplicationGUI();
    if (gui!=NULL)
    {
      return gui;
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::Destroy()
{
  if (this->IsCreated())
  {
    this->Withdraw();
  }
  if ( this->FiducialListWidget )
  {
    this->FiducialListWidget->RemoveMRMLObservers ();
    this->FiducialListWidget->SetParent(NULL);
    this->FiducialListWidget->Delete();
    this->FiducialListWidget = NULL;
  }
  if ( this->ROIViewerWidget )
  {
    this->ROIViewerWidget->RemoveMRMLObservers ();
    this->ROIViewerWidget->SetParent(NULL);
    this->ROIViewerWidget->Delete();
    this->ROIViewerWidget = NULL;
  }
  if (this->ViewerWidget)
  {
    this->ViewerWidget->RemoveMRMLObservers();
    this->ViewerWidget->SetApplicationLogic(NULL);
    this->ViewerWidget->SetParent(NULL);
    this->ViewerWidget->Delete();
    this->ViewerWidget=NULL;
  }
  if (this->MainFrame)
  {
    this->MainFrame->RemoveAllObservers();    
    this->MainFrame->SetParent(NULL);
    this->MainFrame->Delete();
    this->MainFrame=NULL;
  } 
  //this->SetApplication(NULL);
}
