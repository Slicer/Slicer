#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerLabelMapVolumeDisplayWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerLabelMapVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerLabelMapVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerLabelMapVolumeDisplayWidget::vtkSlicerLabelMapVolumeDisplayWidget ( )
{
    this->ColorSelectorWidget = NULL;
    this->UpdatingMRML = 0;
    this->UpdatingWidget = 0;
}


//---------------------------------------------------------------------------
vtkSlicerLabelMapVolumeDisplayWidget::~vtkSlicerLabelMapVolumeDisplayWidget ( )
{
  if (this->IsCreated())
    {
    this->RemoveWidgetObservers();
    }

  if (this->ColorSelectorWidget)
    {
    this->ColorSelectorWidget->SetParent(NULL);
    this->ColorSelectorWidget->Delete();
    this->ColorSelectorWidget = NULL;
    }
  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerLabelMapVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingWidget = 1;

  this->Superclass::ProcessWidgetEvents(caller, event, callData);
  
  //
  // process color selector events
  //
  vtkSlicerNodeSelectorWidget *colSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (colSelector == this->ColorSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLColorNode *color =
      vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color != NULL)
      {
      // get the volume display node
      vtkMRMLVolumeDisplayNode *displayNode = this->GetVolumeDisplayNode();
      if (displayNode != NULL && displayNode->GetColorNodeID() != NULL)
        {
        // set and observe it's colour node id
        if (strcmp(displayNode->GetColorNodeID(), color->GetID()) != 0)
          {
          // there's a change, set it
          displayNode->SetAndObserveColorNodeID(color->GetID());
          }
        }
      }
    this->UpdatingWidget = 0;
    return;
    }
}



//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingMRML = 1;

  vtkMRMLVolumeNode *curVolumeNode = this->GetVolumeNode();
  if (curVolumeNode  == NULL)
    {
    this->UpdatingMRML = 0;
    return;
    }
  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidgetFromMRML();
    this->UpdatingMRML = 0;
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::UpdateWidgetFromMRML ()
{
  vtkDebugMacro("UpdateWidgetFromMRML");
  
  vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();

  // check to see if the color selector widget has it's mrml scene set (it
  // could have been set to null)
  if ( this->ColorSelectorWidget )
    {
    if (this->GetMRMLScene() != NULL &&
        this->ColorSelectorWidget->GetMRMLScene() == NULL)
      {
      vtkDebugMacro("UpdateWidgetFromMRML: resetting the color selector's mrml scene");
      this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
      }
    }  
  vtkMRMLLabelMapVolumeDisplayNode *displayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
  
  if (displayNode != NULL)
    {
    // set the color node selector to reflect the volume's color node
    this->ColorSelectorWidget->SetSelected(displayNode->GetColorNode());
    }
  
  return;

}

//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::AddWidgetObservers ( )
{
  this->Superclass::AddWidgetObservers();
  
   this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

}

//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::RemoveWidgetObservers ( ) 
{
  this->Superclass::RemoveWidgetObservers();

  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerLabelMapVolumeDisplayWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

    // ---
    // DISPLAY FRAME            
    //vtkKWFrame *volDisplayFrame = vtkKWFrame::New ( );
    //volDisplayFrame->SetParent ( this->GetParent() );
    //volDisplayFrame->Create ( );
    //this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    //              volDisplayFrame->GetWidgetName() );

  vtkKWWidget *volDisplayFrame = this->GetParent();
  // a selector to change the color node associated with this display
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ColorSelectorWidget->SetParent ( volDisplayFrame );
  this->ColorSelectorWidget->Create ( );
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  this->ColorSelectorWidget->ShowHiddenOn();
  this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ColorSelectorWidget->SetBorderWidth(2);
  // this->ColorSelectorWidget->SetReliefToGroove();
  this->ColorSelectorWidget->SetPadX(2);
  this->ColorSelectorWidget->SetPadY(2);
  this->ColorSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ColorSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->ColorSelectorWidget->SetLabelText( "Color Select: ");
  this->ColorSelectorWidget->SetBalloonHelpString("select a color from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ColorSelectorWidget->GetWidgetName());
  
  this->AddWidgetObservers();
  if (this->MRMLScene != NULL)
    {
    this->SetAndObserveMRMLScene(this->MRMLScene);
    }

}
