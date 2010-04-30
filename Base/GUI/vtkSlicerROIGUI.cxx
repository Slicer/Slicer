#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWMessage.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkKWFrameWithLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkSlicerROIGUI.h"
#include "vtkSlicerROIDisplayWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerROIGUI );
vtkCxxRevisionMacro ( vtkSlicerROIGUI, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerROIGUI::vtkSlicerROIGUI ( )
{
  this->Logic = NULL;
  this->ROISelectorWidget = NULL;
  this->ROIDisplayWidget = NULL;
}

//---------------------------------------------------------------------------
vtkSlicerROIGUI::~vtkSlicerROIGUI ( )
{
  this->SetModuleLogic (static_cast<vtkSlicerROILogic*>(0));

  if (this->ROISelectorWidget)
    {
    this->ROISelectorWidget->SetParent(NULL);
    this->ROISelectorWidget->Delete();
    this->ROISelectorWidget = NULL;
    }
  if (this->ROIDisplayWidget) {
    this->ROIDisplayWidget->SetParent(NULL);
    this->ROIDisplayWidget->Delete();
    this->ROIDisplayWidget = NULL;
    }
 
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerROIGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "SlicerROIGUI: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerROIGUI::RemoveGUIObservers ( )
{
  this->ROISelectorWidget->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->GUICallbackCommand);
}

//---------------------------------------------------------------------------
void vtkSlicerROIGUI::AddGUIObservers ( )
{

  this->ROISelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  // observe the scene for node deleted events
  if (this->MRMLScene)
    {
    if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->GUICallbackCommand) != 1)
      {
      this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->GUICallbackCommand);
      }
    else
      {
      vtkDebugMacro("MRML scene already has the node removed event being watched by the roi gui");
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::ProcessGUIEvents ( vtkObject *caller,
                                        unsigned long event, void * vtkNotUsed(callData) )
  {
  // process ROI list node selector events
  vtkSlicerNodeSelectorWidget *roiSelector = 
    vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (roiSelector == this->ROISelectorWidget &&
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    vtkMRMLROINode *roi =vtkMRMLROINode::SafeDownCast(this->ROISelectorWidget->GetSelected());
    if (roi!= NULL)
      {
      this->ROIDisplayWidget->SetROINode(roi);
      }
    return;
    }

  return;
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::ProcessLogicEvents (vtkObject *vtkNotUsed(caller),
                                          unsigned long vtkNotUsed(event),
                                          void * vtkNotUsed(callData))
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::ProcessMRMLEvents ( vtkObject *caller,
                                         unsigned long event, void * vtkNotUsed(callData) )
{
}

//---------------------------------------------------------------------------
void vtkSlicerROIGUI::UpdateGUI()
{
// Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::CreateModuleEventBindings ( )
{
// Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerROIGUI::ReleaseModuleEventBindings ( )
{
// Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::Enter ( )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  this->UpdateGUI();
}

//---------------------------------------------------------------------------
void vtkSlicerROIGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerROIGUI::BuildGUI ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  // create a page
  this->UIPanel->AddPage ( "ROI", "ROI", NULL );

  // Define your help text and build the help frame here.
  const char *help = "The ROI module creates and edits Region Of Interest (ROI). An ROI is a rectangle region in the RAS space. It is defined by the location of it's center and three extents of it's sides. The ROI can be rotated and/or translated relative to the RAS space by placing it inside the Transformation node in the Data module. The ROI module allows user to create new ROI's, change their visibility, and also change the size and location of ROI's.\nRegion Of Interest (ROI) is used in other modules such as ClipModels, VolumeRendering, etc. \n For more information see <a>http://wiki.slicer.org/slicerWiki/index.php/Modules:ROIModule-Documentation-3.6</a>\n";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\nThe Volumes module was contributed by Alex Yarmarkovich, Isomics Inc. with help from others at SPL, BWH (Ron Kikinis)";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ROI" );
  this->BuildHelpAndAboutFrame ( page, help, about );

  // ---
  // ROI FRAME
  vtkSlicerModuleCollapsibleFrame *ROIFrame = vtkSlicerModuleCollapsibleFrame::New();
  ROIFrame->SetParent( page );
  ROIFrame->Create();
  ROIFrame->SetLabelText("ROI Display");
  ROIFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    ROIFrame->GetWidgetName(), this->UIPanel->GetPageWidget("ROI")->GetWidgetName());

  // node selector
  this->ROISelectorWidget = vtkSlicerNodeSelectorWidget::New();
  this->ROISelectorWidget->SetParent(ROIFrame->GetFrame());
  this->ROISelectorWidget->Create();
  this->ROISelectorWidget->SetNodeClass("vtkMRMLROINode", NULL, NULL, NULL);
  this->ROISelectorWidget->NewNodeEnabledOn();
  this->ROISelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ROISelectorWidget->SetBorderWidth(2);
  this->ROISelectorWidget->SetPadX(2);
  this->ROISelectorWidget->SetPadY(2);
  this->ROISelectorWidget->SetShowHidden(1);
  //this->ROISelectorWidget->GetWidget()->IndicatorVisibilityOff();
  this->ROISelectorWidget->GetWidget()->SetWidth(24);
  this->ROISelectorWidget->SetLabelText( "ROI Select: ");
  this->ROISelectorWidget->SetBalloonHelpString("Select a ROI from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ROISelectorWidget->GetWidgetName(),
                ROIFrame->GetFrame()->GetWidgetName() );

  // display widgwt
  this->ROIDisplayWidget = vtkSlicerROIDisplayWidget::New();
  this->ROIDisplayWidget->SetParent ( ROIFrame->GetFrame() );
  this->ROIDisplayWidget->Create();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s", 
               this->ROIDisplayWidget->GetWidgetName(),
               ROIFrame->GetFrame()->GetWidgetName());



 

  ROIFrame->Delete ();
  return;
}

