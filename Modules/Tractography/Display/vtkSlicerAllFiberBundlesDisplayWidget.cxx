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

  this->ColorMode = ColorModeSolid;

  this->LineVisibilityButton = NULL;
  this->TubeVisibilityButton = NULL;
  this->GlyphVisibilityButton = NULL;
  
  this->SurfaceMaterialPropertyWidget = NULL;
  
  this->DiffusionTensorIcons = vtkSlicerDiffusionTensorIcons::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerAllFiberBundlesDisplayWidget::~vtkSlicerAllFiberBundlesDisplayWidget ( )
{
  this->RemoveWidgetObservers();


  if ( this->ColorModeRadioButtons )
    {
    this->ColorModeRadioButtons->SetParent ( NULL );
    this->ColorModeRadioButtons->Delete ( );
    this->ColorModeRadioButtons = NULL;      
    }

  // Delete Icons
  if ( this->DiffusionTensorIcons )
    {
    this->DiffusionTensorIcons->Delete ( );
    this->DiffusionTensorIcons = NULL;
    }

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

  // handle input from the radio buttons for colors 
  vtkKWRadioButton *radiob = vtkKWRadioButton::SafeDownCast ( caller );

  if ( ( radiob == this->ColorModeRadioButtons->GetWidget ( ColorModeSolid ) ||
         radiob == this->ColorModeRadioButtons->GetWidget ( ColorModeFA ) ||
         radiob == this->ColorModeRadioButtons->GetWidget ( ColorModeCL ) ||
         radiob == this->ColorModeRadioButtons->GetWidget ( ColorModeTrace ) )
       && event == vtkKWRadioButton::SelectedStateChangedEvent )
    {

    this->ColorMode = radiob->GetVariableValueAsInt();
    vtkDebugMacro("Set ColorMode" << this->ColorMode);
    }

  // Update any changes into MRML display nodes
  this->UpdateMRML();


  vtkDebugMacro("Done Process Widget Events");
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::UpdateMRML()
{
  vtkDebugMacro("UpdateMRML");
  /*** TODO Rewrite to work with muliple display nodes
  // Traverse all fiber bundle nodes and set properties according to the widget
  // event

  if ( this->MRMLScene == NULL )
    {
    vtkWarningMacro( "Can't Update MRML, MRMLScene not set");
    return;
    }

  vtkMRMLFiberBundleNode *node;
  vtkMRMLFiberBundleDisplayNode *displayNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *lineDisplay;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *tubeDisplay;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDisplay;


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

        displayNode->SetVisibility(this->LineVisibilityButton->GetWidget()->GetSelectedState());

        displayNode->SetFiberTubeVisibility(this->TubeVisibilityButton->GetWidget()->GetSelectedState());

        displayNode->SetFiberGlyphVisibility(this->GlyphVisibilityButton->GetWidget()->GetSelectedState());
        
        displayNode->SetAmbient(this->SurfaceMaterialPropertyWidget->GetProperty()->GetAmbient());

        displayNode->SetDiffuse(this->SurfaceMaterialPropertyWidget->GetProperty()->GetDiffuse());

        displayNode->SetSpecular(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecular());

        displayNode->SetPower(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecularPower());
        

        // get the diffusion tensor display props/colors nodes for each type of graphics
        lineDisplay = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(
                      displayNode->GetFiberLineDiffusionTensorDisplayPropertiesNode ( ) );

        tubeDisplay = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(
                      displayNode->GetFiberTubeDiffusionTensorDisplayPropertiesNode ( ) );

        glyphDisplay = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(
                      displayNode->GetFiberGlyphDiffusionTensorDisplayPropertiesNode ( ) );

        if ( ( lineDisplay != NULL ) && ( tubeDisplay != NULL )  && ( glyphDisplay != NULL ) )
          {
          
          switch ( this->ColorMode )
            {
            
            case ColorModeSolid:
              {
              displayNode->SetColorModeForFiberLinesToSolid();
              displayNode->SetColorModeForFiberTubesToSolid();
              displayNode->SetColorModeForFiberGlyphsToSolid();
              }
              break;
              
            case ColorModeFA:
              {
              displayNode->SetColorModeForFiberLinesToScalar();
              displayNode->SetColorModeForFiberTubesToScalar();
              displayNode->SetColorModeForFiberGlyphsToScalar();
              lineDisplay->SetScalarInvariantToFractionalAnisotropy( );
              glyphDisplay->ColorGlyphByFractionalAnisotropy( );
              tubeDisplay->SetScalarInvariantToFractionalAnisotropy( );
              }
              break;
              
            case ColorModeCL:
              {
              displayNode->SetColorModeForFiberLinesToScalar();
              displayNode->SetColorModeForFiberTubesToScalar();
              displayNode->SetColorModeForFiberGlyphsToScalar();
              lineDisplay->SetScalarInvariantToLinearMeasure( );
              glyphDisplay->ColorGlyphByLinearMeasure( );
              tubeDisplay->SetScalarInvariantToLinearMeasure( );
              }
              break;
              
            case ColorModeTrace:
              {
              displayNode->SetColorModeForFiberLinesToScalar();
              displayNode->SetColorModeForFiberTubesToScalar();
              displayNode->SetColorModeForFiberGlyphsToScalar();
              lineDisplay->SetScalarInvariantToTrace( );
              glyphDisplay->ColorGlyphByTrace( );
              tubeDisplay->SetScalarInvariantToTrace( );
              }
              break;
              
            } //end switch on color mode

          vtkErrorMacro("glyph display scalar invariant " << glyphDisplay->GetScalarInvariant() << " colorMode GUI: " << this->ColorMode);

          } // end if diffusion display node

        }  // end if display node

      } // end if fiber bundle node not null

    } // end loop over f b nodes

  vtkDebugMacro("Done UpdateMRML");
***/
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::RemoveWidgetObservers ( ) {

  this->LineVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );
  


  // color mode radio buttons
  this->ColorModeRadioButtons->GetWidget( ColorModeSolid )->RemoveObservers ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ColorModeRadioButtons->GetWidget( ColorModeFA )->RemoveObservers ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ColorModeRadioButtons->GetWidget( ColorModeCL )->RemoveObservers ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ColorModeRadioButtons->GetWidget( ColorModeTrace )->RemoveObservers ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerAllFiberBundlesDisplayWidget::AddWidgetObservers ( )
{

  this->LineVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );


  // color mode radio buttons
  this->ColorModeRadioButtons->GetWidget( ColorModeSolid )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ColorModeRadioButtons->GetWidget( ColorModeFA )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ColorModeRadioButtons->GetWidget( ColorModeCL )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ColorModeRadioButtons->GetWidget( ColorModeTrace )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  //this->LineVisibilityButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  
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

  // ---- color select radio buttons -----
  this->ColorModeRadioButtons = vtkKWRadioButtonSet::New ( );

  this->ColorModeRadioButtons->SetParent ( fiberBundleDisplayFrame );
  this->ColorModeRadioButtons->Create ( );
  this->ColorModeRadioButtons->PackHorizontallyOn();

  // solid color button
  vtkKWRadioButton *radiob = this->ColorModeRadioButtons->AddWidget ( ColorModeSolid );
  radiob->SetReliefToFlat ( );
  radiob->SetOffReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->DiffusionTensorIcons->GetLineIcon ( ) );
  radiob->SetSelectImageToIcon ( this->DiffusionTensorIcons->GetGlyphIcon ( ) );
  radiob->IndicatorVisibilityOff();
  radiob->SetHighlightThickness ( 0 );
  radiob->SetBorderWidth ( 0 );
  //radiob->SetSelectColor ( app->GetSlicerTheme()->GetSlicerColors()->White );
  radiob->SetBalloonHelpString ( "Color fibers by solid color" );
  if ( this->ColorMode == ColorModeSolid )
    {
    radiob->SelectedStateOn ( );
    }
  else
    {
    radiob->SelectedStateOff ( );
    }


  radiob = this->ColorModeRadioButtons->AddWidget ( ColorModeFA );
  radiob->SetReliefToFlat ( );
  radiob->SetOffReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->DiffusionTensorIcons->GetLineIcon ( ) );
  radiob->SetSelectImageToIcon ( this->DiffusionTensorIcons->GetGlyphIcon ( ) );
  radiob->IndicatorVisibilityOff();
  radiob->SetHighlightThickness ( 0 );
  radiob->SetBorderWidth ( 0 );
  //radiob->SetSelectColor ( app->GetSlicerTheme()->GetSlicerColors()->White );
  radiob->SetBalloonHelpString ( "Color fibers by FA" );
  if ( this->ColorMode == ColorModeFA )
    {
    radiob->SelectedStateOn ( );
    }
  else
    {
    radiob->SelectedStateOff ( );
    }


  radiob = this->ColorModeRadioButtons->AddWidget ( ColorModeCL );
  radiob->SetReliefToFlat ( );
  radiob->SetOffReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->DiffusionTensorIcons->GetLineIcon ( ) );
  radiob->SetSelectImageToIcon ( this->DiffusionTensorIcons->GetGlyphIcon ( ) );
  radiob->IndicatorVisibilityOff();
  radiob->SetHighlightThickness ( 0 );
  radiob->SetBorderWidth ( 0 );
  //radiob->SetSelectColor ( app->GetSlicerTheme()->GetSlicerColors()->White );
  radiob->SetBalloonHelpString ( "Color fibers by CL (Westin's linear measure)" );
  if ( this->ColorMode == ColorModeCL )
    {
    radiob->SelectedStateOn ( );
    }
  else
    {
    radiob->SelectedStateOff ( );
    }


  radiob = this->ColorModeRadioButtons->AddWidget ( ColorModeTrace );
  radiob->SetReliefToFlat ( );
  radiob->SetOffReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->DiffusionTensorIcons->GetLineIcon ( ) );
  radiob->SetSelectImageToIcon ( this->DiffusionTensorIcons->GetGlyphIcon ( ) );
  radiob->IndicatorVisibilityOff();
  radiob->SetHighlightThickness ( 0 );
  radiob->SetBorderWidth ( 0 );
  //radiob->SetSelectColor ( app->GetSlicerTheme()->GetSlicerColors()->White );
  radiob->SetBalloonHelpString ( "Color fibers by Trace" );
  if ( this->ColorMode == ColorModeTrace )
    {
    radiob->SelectedStateOn ( );
    }
  else
    {
    radiob->SelectedStateOff ( );
    }

  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ColorModeRadioButtons->GetWidgetName() );

  // ---- END color select radio buttons -----

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
  this->AddWidgetObservers();


  fiberBundleDisplayFrame->Delete();
    
}
