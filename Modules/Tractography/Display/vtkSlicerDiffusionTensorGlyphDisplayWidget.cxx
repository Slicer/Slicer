#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerDiffusionTensorGlyphDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDiffusionTensorGlyphDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerDiffusionTensorGlyphDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDiffusionTensorGlyphDisplayWidget::vtkSlicerDiffusionTensorGlyphDisplayWidget ( )
{

    this->DiffusionTensorDisplayPropertiesNodeID = NULL;


    //this->VisibilityButton = NULL;
    this->GlyphGeometryMenu = NULL;
    this->GlyphColorMenu = NULL;
    this->LineGlyphEigenvectorMenu = NULL;

    this->GlyphScale = NULL;

    this->TubeNumberOfSidesScale = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerDiffusionTensorGlyphDisplayWidget::~vtkSlicerDiffusionTensorGlyphDisplayWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

//   if (this->VisibilityButton)
//     {
//     this->VisibilityButton->SetParent(NULL);
//     this->VisibilityButton->Delete();
//     this->VisibilityButton = NULL;
//     }
  if (this->GlyphGeometryMenu)
    {
    this->GlyphGeometryMenu->SetParent(NULL);
    this->GlyphGeometryMenu->Delete();
    this->GlyphGeometryMenu = NULL;
    }
  if (this->GlyphColorMenu)
    {
    this->GlyphColorMenu->SetParent(NULL);
    this->GlyphColorMenu->Delete();
    this->GlyphColorMenu = NULL;
    }
  if (this->LineGlyphEigenvectorMenu)
    {
    this->LineGlyphEigenvectorMenu->SetParent(NULL);
    this->LineGlyphEigenvectorMenu->Delete();
    this->LineGlyphEigenvectorMenu = NULL;
    }
  if (this->GlyphScale)
    {
    this->GlyphScale->SetParent(NULL);
    this->GlyphScale->Delete();
    this->GlyphScale = NULL;
    }

  if (this->TubeNumberOfSidesScale)
    {
    this->TubeNumberOfSidesScale->SetParent(NULL);
    this->TubeNumberOfSidesScale->Delete();
    this->TubeNumberOfSidesScale = NULL;
    }



  this->SetMRMLScene ( NULL );
  this->SetDiffusionTensorDisplayPropertiesNodeID (NULL);
  
}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerDiffusionTensorGlyphDisplayWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "DiffusionTensorDisplayPropertiesNode ID: " << this->GetDiffusionTensorDisplayPropertiesNodeID() << "\n";

    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::SetDiffusionTensorDisplayPropertiesNode ( vtkMRMLDiffusionTensorDisplayPropertiesNode *node )
{ 
  vtkDebugWithObjectMacro(this,"Setting DTI Display props node" <<  node->GetID());
  vtkErrorWithObjectMacro(this,"Setting DTI Display props node" <<  node->GetID());

  // 
  // Set the member variables and do a first process
  //
  // Stop observing the old node
  this->RemoveMRMLObservers();

  this->SetDiffusionTensorDisplayPropertiesNodeID( node->GetID() );

  // Observe the display node. It may be used for various types of actual (data) nodes.
  // Keeping track of these is handled by widgets using this widget.
  // Start observing the new node
  this->AddMRMLObservers();

  if ( node )
    {
    this->ProcessMRMLEvents(node, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkDebugWithObjectMacro(this,"Process Widget Events");
  vtkErrorWithObjectMacro(this,"Process WIDGET... Events!!!!!!!!!!!!!!!!!!!!!!!!!  ;)");

  vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
    vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                               this->DiffusionTensorDisplayPropertiesNodeID));
  if (displayNode == NULL)
    {
    vtkErrorWithObjectMacro(this,"Process WIDGET Events null display node  ;)");
    return;
    }

  vtkErrorWithObjectMacro(this,"Process WIDGET... Events, got display node!  ;)");

  // process glyph geometry menu events
  vtkKWMenu *glyphMenuButton = 
      vtkKWMenu::SafeDownCast(caller);
  if (glyphMenuButton == this->GlyphGeometryMenu->GetWidget()->GetMenu())
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, glyph menu event!  ;)" << event);

  if (glyphMenuButton == this->GlyphGeometryMenu->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetGlyphGeometry(this->GlyphGeometryMap[std::string(this->GlyphGeometryMenu->GetWidget()->GetValue())]);
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, display node glyph set!  ;)");
    return;
    }

  // process glyph color menu events
  vtkKWMenu *colorMenuButton = 
      vtkKWMenu::SafeDownCast(caller);
  if (colorMenuButton == this->GlyphColorMenu->GetWidget()->GetMenu())
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, color menu event!  ;)" << event);

  if (colorMenuButton == this->GlyphColorMenu->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetColorGlyphBy(this->GlyphColorMap[std::string(this->GlyphColorMenu->GetWidget()->GetValue())]);
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, display node glyph color set!  ;)");
    return;
    }

  // advanced frame

  // process glyph scale events
  vtkKWScale *glyphScale = vtkKWScale::SafeDownCast(caller);
  if (glyphScale == this->GlyphScale->GetWidget() )
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, glyph scale event!  ;)" << event);

  if (glyphScale == this->GlyphScale->GetWidget() && 
        event == vtkKWScale::ScaleValueChangedEvent)
    {
    displayNode->SetGlyphScaleFactor(this->GlyphScale->GetWidget()->GetValue());
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, glyph scale factor set!  ;)");
    return;
    }


  // advanced-> line glyph frame
  // process eigenvector menu events
  vtkKWMenu *lineEigMenuButton = 
      vtkKWMenu::SafeDownCast(caller);
  if (lineEigMenuButton == this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu())
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, line eig menu event!  ;)" << event);

  if (lineEigMenuButton == this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetGlyphEigenvector(this->GlyphEigenvectorMap[std::string(this->LineGlyphEigenvectorMenu->GetWidget()->GetValue())]);
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, display node glyph set!  ;)");
    return;
    }


  // advanced -> tube glyph frame
  // process eigenvector menu events
  vtkKWMenu *tubeEigMenuButton = 
      vtkKWMenu::SafeDownCast(caller);
  if (tubeEigMenuButton == this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu())
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, tube eig menu event!  ;)" << event);

  if (tubeEigMenuButton == this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetGlyphEigenvector(this->GlyphEigenvectorMap[std::string(this->TubeGlyphEigenvectorMenu->GetWidget()->GetValue())]);
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, display node glyph set!  ;)");
    return;
    }
  // process number of sides scale events
  vtkKWScale *tubeSidesScale = vtkKWScale::SafeDownCast(caller);
  if (tubeSidesScale == this->TubeNumberOfSidesScale->GetWidget() )
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, tubeSides scale event!  ;)" << event);

  if (tubeSidesScale == this->TubeNumberOfSidesScale->GetWidget() && 
        event == vtkKWScale::ScaleValueChangedEvent)
    {
    vtkErrorWithObjectMacro(this,"Process WIDGET... Events, number of tube sides set!  ;)");
    displayNode->SetTubeGlyphNumberOfSides((int) this->TubeNumberOfSidesScale->GetWidget()->GetValue());
    return;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  vtkDebugWithObjectMacro(this,"Process MRML Events");

  vtkErrorWithObjectMacro(this,"Process MRML Events!!!!!!!!!!!!!!!!!!!!!!!!!  ;)");

  if ( !this->DiffusionTensorDisplayPropertiesNodeID )
    {
    vtkErrorWithObjectMacro(this,"Process MRML Events null display node  ;)");
    return;
    }

  vtkMRMLDiffusionTensorDisplayPropertiesNode *node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(caller);
  
  // if this event comes from our node, it is not null, and has been modified
  if (node == this->MRMLScene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) && 
      node != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkErrorWithObjectMacro(this,"Process MRML Events updating widget  ;)");

    // stop observing display node
    //this->RemoveMRMLObservers();

    // set the color node selector to reflect the volume's color node
 //    if (node->GetColorNode() != NULL)
//       {
//       this->ColorSelectorWidget->SetSelected(displayNode->GetColorNode());
//       }
//     else
//       {
//       vtkWarningMacro("Slicer DiffusionTensorGlyph Display Widget cannot set the color selector widget, as the fiberBundle's display node has no color node set");
//       }


    this->UpdateWidget();
    
    // start observing display node again
    //this->AddMRMLObservers();

    }
  



}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::AddMRMLObservers ( )
{

  vtkErrorWithObjectMacro(this,"Add mrml observers ;) " << this->DiffusionTensorDisplayPropertiesNodeID);

  if ( !this->DiffusionTensorDisplayPropertiesNodeID )
    {
    return;
    }

  vtkMRMLDiffusionTensorDisplayPropertiesNode *node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID));
  
  if (node != NULL)
    {
    node->AddObserver(vtkCommand::ModifiedEvent,
                      (vtkCommand *)this->MRMLCallbackCommand );      
    }

}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::RemoveMRMLObservers ( )
{

  //vtkErrorWithObjectMacro(this,"Remove mrml observers ;) " << this->DiffusionTensorDisplayPropertiesNodeID);

  if ( !this->DiffusionTensorDisplayPropertiesNodeID )
    {
    return;
    }

  vtkMRMLDiffusionTensorDisplayPropertiesNode *node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID));
  
  if (node != NULL)
    {
    node->RemoveObservers(vtkCommand::ModifiedEvent,
                          (vtkCommand *)this->MRMLCallbackCommand );    
    }
}

// called from process MRML events
//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::UpdateWidget()
{
  vtkErrorWithObjectMacro(this,"Update widget  ;)");

  if ( this->DiffusionTensorDisplayPropertiesNodeID )
    {

    vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                              this->DiffusionTensorDisplayPropertiesNodeID));
    if (displayNode != NULL) 
      {
      //this->VisibilityButton->GetWidget()->SetSelectedState(displayNode->GetVisibility());
      this->GlyphGeometryMenu->GetWidget()->SetValue(displayNode->GetGlyphGeometryAsString());
      this->GlyphColorMenu->GetWidget()->SetValue(displayNode->GetColorGlyphByAsString());
      this->LineGlyphEigenvectorMenu->GetWidget()->SetValue(displayNode->GetGlyphEigenvectorAsString());

      this->TubeGlyphEigenvectorMenu->GetWidget()->SetValue(displayNode->GetGlyphEigenvectorAsString());

      this->GlyphScale->GetWidget()->SetValue(displayNode->GetGlyphScaleFactor());

      this->TubeNumberOfSidesScale->GetWidget()->SetValue(displayNode->GetTubeGlyphNumberOfSides());

      } 
    else 
      {
      vtkErrorWithObjectMacro(this,"Update widget null display node  ;) " << this->DiffusionTensorDisplayPropertiesNodeID);
      }
    
    return;
    }
  vtkErrorWithObjectMacro(this,"DONE Update widget  ;)");
}

// TO DO: is this used?
//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::UpdateMRML()
{

  vtkErrorWithObjectMacro(this,"Update mrml  ;) " << this->DiffusionTensorDisplayPropertiesNodeID);
  
  if ( this->DiffusionTensorDisplayPropertiesNodeID )
    {
    vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                              this->DiffusionTensorDisplayPropertiesNodeID));
    if (displayNode != NULL) 
      {
      //displayNode->SetVisibility(this->VisibilityButton->GetWidget()->GetSelectedState());
      displayNode->SetGlyphGeometry(this->GlyphGeometryMap[std::string(this->GlyphGeometryMenu->GetWidget()->GetValue())]);
      displayNode->SetColorGlyphBy(this->GlyphColorMap[std::string(this->GlyphColorMenu->GetWidget()->GetValue())]);
      displayNode->SetGlyphEigenvector(this->GlyphEigenvectorMap[std::string(this->LineGlyphEigenvectorMenu->GetWidget()->GetValue())]);
      displayNode->SetGlyphScaleFactor(this->GlyphScale->GetWidget()->GetValue());
      displayNode->SetTubeGlyphNumberOfSides((int) this->TubeNumberOfSidesScale->GetWidget()->GetValue());
      }
    else 
      {
      vtkErrorWithObjectMacro(this,"Update mrml null display node  ;) " << this->DiffusionTensorDisplayPropertiesNodeID);
      }

    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::AddWidgetObservers ( ) {

  //this->Superclass::AddWidgetObservers();

  this->GlyphGeometryMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphColorMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  //this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->GlyphScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->GlyphScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TubeNumberOfSidesScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::RemoveWidgetObservers ( ) {
  
  //this->Superclass::RemoveWidgetObservers();

  this->GlyphGeometryMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphColorMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  //this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  //this->GlyphScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->GlyphScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TubeNumberOfSidesScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();


  //Create dummy display properties node to init variables
  vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = 
    vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
  
  
  // ---
  // DISPLAY FRAME            
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ( "Glyph Display Properties" );
  //frame->CollapseFrame ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  // glyph visibility
//   this->VisibilityButton = vtkKWCheckButtonWithLabel::New();
//   this->VisibilityButton->SetParent ( frame->GetFrame() );
//   this->VisibilityButton->Create ( );
//   this->VisibilityButton->SetLabelText("Visibility");
//   this->VisibilityButton->SetBalloonHelpString("set glyph visibility.");
//   this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
//                  this->VisibilityButton->GetWidgetName() );

  // glyph geometry menu
  vtkKWMenuButtonWithLabel *glyphMenuButton = 
    vtkKWMenuButtonWithLabel::New();

  this->GlyphGeometryMenu = glyphMenuButton;
  glyphMenuButton->SetParent( frame->GetFrame() );
  glyphMenuButton->Create();

  // initialize glyph geometry menu
  int initIdx = propNode->GetFirstGlyphGeometry();
  int endIdx = propNode->GetLastGlyphGeometry();
  int currentVal = propNode->GetGlyphGeometry();
  this->GlyphGeometryMap.clear();
  int k;
  for (k=initIdx ; k<=endIdx ; k++)
    {
    propNode->SetGlyphGeometry(k);
    const char *tag = propNode->GetGlyphGeometryAsString();
    this->GlyphGeometryMap[std::string(tag)]=k;
    glyphMenuButton->GetWidget()->GetMenu()->AddRadioButton(tag);
    }
  // init to class default value
  propNode->SetGlyphGeometry(currentVal);
  glyphMenuButton->GetWidget()->SetValue(propNode->GetGlyphGeometryAsString());
  
  // pack glyph menu
  glyphMenuButton->SetLabelText("Glyph Type");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               glyphMenuButton->GetWidgetName());


  // glyph color menu
  vtkKWMenuButtonWithLabel *colorMenuButton = 
    vtkKWMenuButtonWithLabel::New();

  this->GlyphColorMenu = colorMenuButton;
  colorMenuButton->SetParent( frame->GetFrame() );
  colorMenuButton->Create();

  // initialize color menu
  initIdx = propNode->GetFirstColorGlyphBy();
  endIdx = propNode->GetLastColorGlyphBy();
  currentVal = propNode->GetColorGlyphBy();
  this->GlyphColorMap.clear();
  for (k=initIdx ; k<=endIdx ; k++)
    {
    propNode->SetColorGlyphBy(k);
    const char *tag = propNode->GetColorGlyphByAsString();
    this->GlyphColorMap[std::string(tag)]=k;
    colorMenuButton->GetWidget()->GetMenu()->AddRadioButton(tag);
    }
  // init to class default value
  propNode->SetColorGlyphBy(currentVal);
  colorMenuButton->GetWidget()->SetValue(propNode->GetColorGlyphByAsString());

  // pack color menu
  colorMenuButton->SetLabelText("Glyph Color");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               colorMenuButton->GetWidgetName());


  // ---
  // ADVANCED FRAME            
  vtkKWFrameWithLabel *advFrame = vtkKWFrameWithLabel::New ( );
  advFrame->SetParent ( frame->GetFrame() );
  advFrame->Create ( );
  advFrame->SetLabelText ( "Advanced" );
  advFrame->CollapseFrame ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 advFrame->GetWidgetName() );

  this->GlyphScale = vtkKWScaleWithLabel::New();
  this->GlyphScale->SetParent ( advFrame->GetFrame() );
  this->GlyphScale->Create ( );
  this->GlyphScale->SetLabelText("Scale Factor");
  this->GlyphScale->GetWidget()->SetRange(0,200);
  this->GlyphScale->GetWidget()->SetResolution(1);
  this->GlyphScale->SetBalloonHelpString("set glyph opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->GlyphScale->GetWidgetName() );




  // ---
  // Lines FRAME            
  vtkKWFrameWithLabel *lineFrame = vtkKWFrameWithLabel::New ( );
  lineFrame->SetParent ( advFrame->GetFrame() );
  lineFrame->Create ( );
  lineFrame->SetLabelText ( "Line Glyph Display" );
  lineFrame->CollapseFrame ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 lineFrame->GetWidgetName() );


  // line glyph eigenvector menu
  vtkKWMenuButtonWithLabel *lineEigMenuButton = 
    vtkKWMenuButtonWithLabel::New();

  this->LineGlyphEigenvectorMenu = lineEigMenuButton;
  lineEigMenuButton->SetParent( lineFrame->GetFrame() );
  lineEigMenuButton->Create();

  // initialize eigenvector menu
  initIdx = propNode->GetFirstGlyphEigenvector();
  endIdx = propNode->GetLastGlyphEigenvector();
  currentVal = propNode->GetGlyphEigenvector();
  this->GlyphEigenvectorMap.clear();
  for (k=initIdx ; k<=endIdx ; k++)
    {
    propNode->SetGlyphEigenvector(k);
    const char *tag = propNode->GetGlyphEigenvectorAsString();
    this->GlyphEigenvectorMap[std::string(tag)]=k;
    lineEigMenuButton->GetWidget()->GetMenu()->AddRadioButton(tag);
    }
  // init to class default value
  propNode->SetGlyphEigenvector(currentVal);
  lineEigMenuButton->GetWidget()->SetValue(propNode->GetGlyphEigenvectorAsString());

  // pack eigenvector menu
  lineEigMenuButton->SetLabelText("Glyph Eigenvector");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               lineEigMenuButton->GetWidgetName());



  // ---
  // Tubes FRAME            
  vtkKWFrameWithLabel *tubeFrame = vtkKWFrameWithLabel::New ( );
  tubeFrame->SetParent ( advFrame->GetFrame() );
  tubeFrame->Create ( );
  tubeFrame->SetLabelText ( "Tube Glyph Display" );
  tubeFrame->CollapseFrame ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 tubeFrame->GetWidgetName() );


  // tube glyph eigenvector menu
  vtkKWMenuButtonWithLabel *tubeEigMenuButton = 
    vtkKWMenuButtonWithLabel::New();

  this->TubeGlyphEigenvectorMenu = tubeEigMenuButton;
  tubeEigMenuButton->SetParent( tubeFrame->GetFrame() );
  tubeEigMenuButton->Create();

  // initialize eigenvector menu
  initIdx = propNode->GetFirstGlyphEigenvector();
  endIdx = propNode->GetLastGlyphEigenvector();
  currentVal = propNode->GetGlyphEigenvector();
  this->GlyphEigenvectorMap.clear();
  for (k=initIdx ; k<=endIdx ; k++)
    {
    propNode->SetGlyphEigenvector(k);
    const char *tag = propNode->GetGlyphEigenvectorAsString();
    this->GlyphEigenvectorMap[std::string(tag)]=k;
    tubeEigMenuButton->GetWidget()->GetMenu()->AddRadioButton(tag);
    }
  // init to class default value
  propNode->SetGlyphEigenvector(currentVal);
  tubeEigMenuButton->GetWidget()->SetValue(propNode->GetGlyphEigenvectorAsString());

  // pack eigenvector menu
  tubeEigMenuButton->SetLabelText("Glyph Eigenvector");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               tubeEigMenuButton->GetWidgetName());


  this->TubeNumberOfSidesScale = vtkKWScaleWithLabel::New();
  this->TubeNumberOfSidesScale->SetParent ( tubeFrame->GetFrame() );
  this->TubeNumberOfSidesScale->Create ( );
  this->TubeNumberOfSidesScale->SetLabelText("Number Of Sides");
  this->TubeNumberOfSidesScale->GetWidget()->SetRange(1,20);
  this->TubeNumberOfSidesScale->GetWidget()->SetResolution(1);
  this->TubeNumberOfSidesScale->SetBalloonHelpString("set number of sides of the tube.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->TubeNumberOfSidesScale->GetWidgetName() );


  // ---
  // Ellipsoids FRAME            
  vtkKWFrameWithLabel *ellipseFrame = vtkKWFrameWithLabel::New ( );
  ellipseFrame->SetParent ( advFrame->GetFrame() );
  ellipseFrame->Create ( );
  ellipseFrame->SetLabelText ( "Ellipsoid Glyph Display" );
  ellipseFrame->CollapseFrame ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 ellipseFrame->GetWidgetName() );


  // add observers
  this->AddWidgetObservers();


  ellipseFrame->Delete();
  tubeFrame->Delete();
  lineFrame->Delete();
  advFrame->Delete();
  frame->Delete();
  propNode->Delete();
}
