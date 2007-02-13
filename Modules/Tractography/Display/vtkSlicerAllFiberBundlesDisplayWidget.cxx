#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerAllFiberBundlesDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerAllFiberBundlesDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerAllFiberBundlesDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerAllFiberBundlesDisplayWidget::vtkSlicerAllFiberBundlesDisplayWidget ( )
{
    this->LineVisibilityButton = NULL;
    this->TubeVisibilityButton = NULL;
    this->GlyphVisibilityButton = NULL;

    this->SurfaceMaterialPropertyWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerAllFiberBundlesDisplayWidget::~vtkSlicerAllFiberBundlesDisplayWidget ( )
{
  this->RemoveWidgetObservers();


  if (this->SurfaceMaterialPropertyWidget)
    {
    this->SurfaceMaterialPropertyWidget->SetParent(NULL);
    this->SurfaceMaterialPropertyWidget->Delete();
    this->SurfaceMaterialPropertyWidget = NULL;
    }

  if (this->LineVisibilityButton)
    {
    this->LineVisibilityButton->SetParent(NULL);
    this->LineVisibilityButton->Delete();
    this->LineVisibilityButton = NULL;
    }
  if (this->TubeVisibilityButton)
    {
    this->TubeVisibilityButton->SetParent(NULL);
    this->TubeVisibilityButton->Delete();
    this->TubeVisibilityButton = NULL;
    }
  if (this->GlyphVisibilityButton)
    {
    this->GlyphVisibilityButton->SetParent(NULL);
    this->GlyphVisibilityButton->Delete();
    this->GlyphVisibilityButton = NULL;
    }


  this->SetMRMLScene ( NULL );
  
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerAllFiberBundlesDisplayWidget: " << this->GetClassName ( ) << "\n";

    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkDebugMacro("Process Widget Events");
  vtkErrorMacro("Process Widget Events");


  this->UpdateMRML();


  vtkErrorMacro("Done Process Widget Events");
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::UpdateMRML()
{
  vtkDebugMacro("UpdateMRML");
  vtkErrorMacro("UpdateMRML ====================================");

  // Traverse all fiber bundle nodes and set properties according to the widget
  // event

  if ( this->MRMLScene == NULL )
    {
    vtkWarningMacro( "Can't Update MRML, MRMLScene not set");
    return;
    }

  vtkMRMLFiberBundleNode *node;
  vtkMRMLFiberBundleDisplayNode *displayNode;

  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiberBundleNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiberBundleNode"));
    if (node != NULL )
      {

      displayNode = vtkMRMLFiberBundleDisplayNode::SafeDownCast( 
             this->MRMLScene->GetNodeByID (node->GetDisplayNodeID() ));

      if (displayNode != NULL )
        {

        vtkErrorMacro("Update display node 1===============" );

        displayNode->SetFiberLineVisibility(this->LineVisibilityButton->GetWidget()->GetSelectedState());

        vtkErrorMacro("Update display node 2===============" );

        displayNode->SetFiberTubeVisibility(this->TubeVisibilityButton->GetWidget()->GetSelectedState());

        vtkErrorMacro("Update display node 3===============" );

        displayNode->SetFiberGlyphVisibility(this->GlyphVisibilityButton->GetWidget()->GetSelectedState());
        
        
        vtkErrorMacro("Update display node 4===============" );
        displayNode->SetAmbient(this->SurfaceMaterialPropertyWidget->GetProperty()->GetAmbient());

        vtkErrorMacro("Update display node 5===============" );
        displayNode->SetDiffuse(this->SurfaceMaterialPropertyWidget->GetProperty()->GetDiffuse());

        vtkErrorMacro("Update display node 6===============" );
        displayNode->SetSpecular(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecular());

        vtkErrorMacro("Update display node 7===============" );
        displayNode->SetPower(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecularPower());
        
        }

      }

    }

  vtkErrorMacro("Done UpdateMRML ====================================");
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::RemoveWidgetObservers ( ) {

  this->LineVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );
  

}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::CreateWidget ( )
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
  vtkKWFrame *fiberBundleDisplayFrame = vtkKWFrame::New ( );
  fiberBundleDisplayFrame->SetParent ( this->GetParent() );
  fiberBundleDisplayFrame->Create ( );
/*
  fiberBundleDisplayFrame->SetLabelText ("Display");
  fiberBundleDisplayFrame->CollapseFrame ( );
*/
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 fiberBundleDisplayFrame->GetWidgetName() );


  this->LineVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->LineVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->LineVisibilityButton->Create ( );
  this->LineVisibilityButton->SetLabelText("Line Visibility");
  this->LineVisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->LineVisibilityButton->GetWidgetName() );
  this->TubeVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->TubeVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->TubeVisibilityButton->Create ( );
  this->TubeVisibilityButton->SetLabelText("Tube Visibility");
  this->TubeVisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->TubeVisibilityButton->GetWidgetName() );
  this->GlyphVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->GlyphVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->GlyphVisibilityButton->Create ( );
  this->GlyphVisibilityButton->SetLabelText("Glyph Visibility");
  this->GlyphVisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->GlyphVisibilityButton->GetWidgetName() );

  this->SurfaceMaterialPropertyWidget = vtkKWSurfaceMaterialPropertyWidget::New();
  this->SurfaceMaterialPropertyWidget->SetParent ( fiberBundleDisplayFrame );
  this->SurfaceMaterialPropertyWidget->Create ( );
  this->SurfaceMaterialPropertyWidget->SetBalloonHelpString("set fiberBundle opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SurfaceMaterialPropertyWidget->GetWidgetName() );

  if (this->SurfaceMaterialPropertyWidget->GetProperty() == NULL)
    {
    vtkProperty *prop = vtkProperty::New();
    this->SurfaceMaterialPropertyWidget->SetProperty(prop);
    prop->Delete();
    }
  
  // add observers

  this->LineVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );



  fiberBundleDisplayFrame->Delete();
    
}
