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
  this->LineGlyphEigenvectorMenu = NULL;
  this->TubeGlyphEigenvectorMenu = NULL;

  this->GlyphScale = NULL;
  this->GlyphResolutionScale = NULL;

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
  if (this->LineGlyphEigenvectorMenu)
    {
    this->LineGlyphEigenvectorMenu->SetParent(NULL);
    this->LineGlyphEigenvectorMenu->Delete();
    this->LineGlyphEigenvectorMenu = NULL;
    }
  if (this->TubeGlyphEigenvectorMenu)
    {
    this->TubeGlyphEigenvectorMenu->SetParent(NULL);
    this->TubeGlyphEigenvectorMenu->Delete();
    this->TubeGlyphEigenvectorMenu = NULL;
    }
  if (this->GlyphScale)
    {
    this->GlyphScale->SetParent(NULL);
    this->GlyphScale->Delete();
    this->GlyphScale = NULL;
    }

  if (this->GlyphResolutionScale)
    {
    this->GlyphResolutionScale->SetParent(NULL);
    this->GlyphResolutionScale->Delete();
    this->GlyphResolutionScale = NULL;
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
  if (node == NULL)
    {
    this->AddMRMLObservers();
    return;
    }
  vtkDebugWithObjectMacro(this,"Setting DTI Display props node" <<  node->GetID());

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

  vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
    vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
    this->DiffusionTensorDisplayPropertiesNodeID));
  if (displayNode == NULL)
    {
    vtkDebugWithObjectMacro(this,"Process WIDGET... Events, NO display node!");
    return;
    }

  // process glyph geometry menu events
  vtkKWMenu *glyphMenuButton = vtkKWMenu::SafeDownCast(caller);
  if (glyphMenuButton == this->GlyphGeometryMenu->GetWidget()->GetMenu())
    vtkDebugWithObjectMacro(this,"Process WIDGET... Events, glyph menu event!" << event);

  if (glyphMenuButton == this->GlyphGeometryMenu->GetWidget()->GetMenu() && 
    event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetGlyphGeometry(this->GlyphGeometryMap[std::string(this->GlyphGeometryMenu->GetWidget()->GetValue())]);
    return;
    }

  // process glyph scale events
  if (vtkKWScale::SafeDownCast(caller) == this->GlyphScale->GetWidget() && 
    event == vtkKWScale::ScaleValueChangedEvent)
    {
    displayNode->SetGlyphScaleFactor(this->GlyphScale->GetWidget()->GetValue());
    return;
    }

  // process glyph resolution events
  if ( vtkKWScale::SafeDownCast(caller) == this->GlyphResolutionScale->GetWidget() && 
    event == vtkKWScale::ScaleValueChangedEvent)
    {
    displayNode->SetLineGlyphResolution((int)(this->GlyphResolutionScale->GetWidget()->GetValue()));
    return;
    }


  // advanced-> line glyph frame
  // process eigenvector menu events
  vtkKWMenu *lineEigMenuButton = 
    vtkKWMenu::SafeDownCast(caller);
  if (lineEigMenuButton == this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu())
    vtkDebugWithObjectMacro(this,"Process WIDGET... Events, line eig menu event!" << event);

  if (lineEigMenuButton == this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu() && 
    event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetGlyphEigenvector(this->GlyphEigenvectorMap[std::string(this->LineGlyphEigenvectorMenu->GetWidget()->GetValue())]);
    vtkDebugWithObjectMacro(this,"Process WIDGET... Events, display node glyph set!");
    return;
    }

  // advanced -> tube glyph frame
  // process eigenvector menu events
  vtkKWMenu *tubeEigMenuButton = 
    vtkKWMenu::SafeDownCast(caller);
  if (tubeEigMenuButton == this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu() && 
    event == vtkKWMenu::MenuItemInvokedEvent)
    {
    displayNode->SetGlyphEigenvector(this->GlyphEigenvectorMap[std::string(this->TubeGlyphEigenvectorMenu->GetWidget()->GetValue())]);
    return;
    }
  // process number of sides scale events
  vtkKWScale *tubeSidesScale = vtkKWScale::SafeDownCast(caller);

  if (tubeSidesScale == this->TubeNumberOfSidesScale->GetWidget() && 
    event == vtkKWScale::ScaleValueChangedEvent)
    {
    displayNode->SetTubeGlyphNumberOfSides((int) this->TubeNumberOfSidesScale->GetWidget()->GetValue());
    return;
    }
  }


//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                                    unsigned long event, void *callData )
  {

  if (!this->DiffusionTensorDisplayPropertiesNodeID )
    {
    vtkDebugWithObjectMacro(this,"Process MRML Events null display node ");
    return;
    }

  vtkMRMLDiffusionTensorDisplayPropertiesNode *node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(caller);

  // if this event comes from our node, it is not null, and has been modified
  if (node == this->MRMLScene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) && 
    node != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkDebugWithObjectMacro(this,"Process MRML Events updating widget ");

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


  if ( !this->DiffusionTensorDisplayPropertiesNodeID )
    {
    return;
    }
  //vtkDebugWithObjectMacro(this,"Add mrml observers ;) " << this->DiffusionTensorDisplayPropertiesNodeID);

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

  //vtkDebugWithObjectMacro(this,"Remove mrml observers ;) " << this->DiffusionTensorDisplayPropertiesNodeID);

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

  if ( this->DiffusionTensorDisplayPropertiesNodeID )
    {

    vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
      this->DiffusionTensorDisplayPropertiesNodeID));
    if (displayNode != NULL) 
      {
      //this->VisibilityButton->GetWidget()->SetSelectedState(displayNode->GetVisibility());
      this->GlyphGeometryMenu->GetWidget()->SetValue(displayNode->GetGlyphGeometryAsString());
      this->LineGlyphEigenvectorMenu->GetWidget()->SetValue(displayNode->GetGlyphEigenvectorAsString());

      this->TubeGlyphEigenvectorMenu->GetWidget()->SetValue(displayNode->GetGlyphEigenvectorAsString());

      this->GlyphScale->GetWidget()->SetValue(displayNode->GetGlyphScaleFactor());

      this->GlyphResolutionScale->GetWidget()->SetValue(displayNode->GetLineGlyphResolution());

      this->TubeNumberOfSidesScale->GetWidget()->SetValue(displayNode->GetTubeGlyphNumberOfSides());

      } 
    else 
      {
      vtkDebugWithObjectMacro(this,"Update widget null display node  ;) " << this->DiffusionTensorDisplayPropertiesNodeID);
      }

    return;
    }
  }

// TO DO: is this used?
//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::UpdateMRML()
  {

  if ( this->DiffusionTensorDisplayPropertiesNodeID )
    {
    vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
      this->DiffusionTensorDisplayPropertiesNodeID));
    if (displayNode != NULL) 
      {
      //displayNode->SetVisibility(this->VisibilityButton->GetWidget()->GetSelectedState());
      displayNode->SetGlyphGeometry(this->GlyphGeometryMap[std::string(this->GlyphGeometryMenu->GetWidget()->GetValue())]);
      displayNode->SetGlyphEigenvector(this->GlyphEigenvectorMap[std::string(this->LineGlyphEigenvectorMenu->GetWidget()->GetValue())]);
      displayNode->SetGlyphScaleFactor(this->GlyphScale->GetWidget()->GetValue());
      displayNode->SetLineGlyphResolution((int)(this->GlyphResolutionScale->GetWidget()->GetValue()));
      displayNode->SetTubeGlyphNumberOfSides((int) this->TubeNumberOfSidesScale->GetWidget()->GetValue());
      }
    else 
      {
      vtkDebugWithObjectMacro(this,"Update mrml null display node  ;) " << this->DiffusionTensorDisplayPropertiesNodeID);
      }

    return;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::AddWidgetObservers ( ) {

  //this->Superclass::AddWidgetObservers();

  this->GlyphGeometryMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  //this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->GlyphScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->GlyphScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphResolutionScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TubeNumberOfSidesScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorGlyphDisplayWidget::RemoveWidgetObservers ( ) {

  //this->Superclass::RemoveWidgetObservers();

  this->GlyphGeometryMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LineGlyphEigenvectorMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeGlyphEigenvectorMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  //this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  //this->GlyphScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->GlyphScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphResolutionScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

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

  this->GlyphScale = vtkKWScaleWithLabel::New();
  this->GlyphScale->SetParent ( frame->GetFrame() );
  this->GlyphScale->Create ( );
  this->GlyphScale->SetLabelText("Scale Factor");
  this->GlyphScale->GetWidget()->SetRange(0,200);
  this->GlyphScale->GetWidget()->SetResolution(1);
  this->GlyphScale->SetBalloonHelpString("set glyph scale value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->GlyphScale->GetWidgetName() );

  this->GlyphResolutionScale = vtkKWScaleWithLabel::New();
  this->GlyphResolutionScale->SetParent ( frame->GetFrame() );
  this->GlyphResolutionScale->Create ( );
  this->GlyphResolutionScale->SetLabelText("Resolution");
  this->GlyphResolutionScale->GetWidget()->SetRange(1,50);
  this->GlyphResolutionScale->GetWidget()->SetResolution(1);
  this->GlyphResolutionScale->SetBalloonHelpString("skip step for glyphs.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->GlyphResolutionScale->GetWidgetName() );

  // ---
  // ADVANCED FRAME            
  vtkKWFrameWithLabel *advFrame = vtkKWFrameWithLabel::New ( );
  advFrame->SetParent ( frame->GetFrame() );
  advFrame->Create ( );
  advFrame->SetLabelText ( "Advanced" );
  advFrame->CollapseFrame ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    advFrame->GetWidgetName() );

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

void vtkSlicerDiffusionTensorGlyphDisplayWidget::SetGlyphResolution(int value)
  {
   if ( this->DiffusionTensorDisplayPropertiesNodeID )
    {
    vtkMRMLDiffusionTensorDisplayPropertiesNode *displayNode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(this->MRMLScene->GetNodeByID(
      this->DiffusionTensorDisplayPropertiesNodeID));
    if (displayNode != NULL) 
      {
  this->GlyphResolutionScale->GetWidget()->SetValue(value);
  displayNode->SetLineGlyphResolution(value);
      }
     }
  }

