#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWMessage.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"

#include "vtkSlicerColorDisplayWidget.h"

#include "vtkKWMessageDialog.h"

#include "vtkSlicerColorGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorGUI );
vtkCxxRevisionMacro ( vtkSlicerColorGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerColorGUI::vtkSlicerColorGUI ( )
{
    this->Logic = NULL;
    this->ColorNodeID = NULL; // "(none)";
    this->ColorNode = NULL; // "(none)";

    this->ColorDisplayWidget = NULL;

    this->AddColorButton = NULL;

    //this->DebugOn();
}


//---------------------------------------------------------------------------
vtkSlicerColorGUI::~vtkSlicerColorGUI ( )
{

  this->SetModuleLogic ( NULL );
  
  if (this->ColorDisplayWidget ) 
    {
    this->ColorDisplayWidget->SetParent(NULL);
    this->ColorDisplayWidget->Delete ( );
    }
  if (this->AddColorButton)
    {
    this->AddColorButton->SetParent(NULL);
    this->AddColorButton->Delete();
    this->AddColorButton = NULL;
    }
  
  //this->SetColorNodeID("(none)");
  this->SetColorNodeID(NULL);
  vtkSetMRMLNodeMacro(this->ColorNode, NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerColorGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::RemoveGUIObservers ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: RemoveGUIObservers\n");
  this->AddColorButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  
  this->RemoveObservers (vtkSlicerColorGUI::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: AddGUIObservers\n");
  
  this->AddColorButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  
  
  this->AddObserver(vtkSlicerColorGUI::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{

  vtkDebugMacro("vtkSlicerColorGUI: ProcessGUIEvent event =  " << event << ".\n");

  // process id changed events
  if (event == vtkSlicerColorGUI::ColorIDModifiedEvent)
    {
    vtkDebugMacro("vtkSlicerColorGUI::ProcessGUIEvents : got a colour id modified event.\n");
    }
  
  vtkMRMLColorNode *colorNode =
    vtkMRMLColorNode::SafeDownCast(caller);
  if (colorNode== this->MRMLScene->GetNodeByID(this->GetColorNodeID()) &&
      event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("returning, not doing anything with the colour node id modified\n");
    return;
    }
  
  // get the currently displayed list
  
  // is there one list?
  vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());

  // save state for undo
  this->MRMLScene->SaveStateForUndo(activeColorNode);

  if (activeColorNode == NULL)
    {
    std::cerr << "ERROR: No Color!\n";
/*
    this->ColorSelectorWidget->ProcessNewNodeCommand("vtkMRMLColorNode",
                                                     this->MRMLScene->GetTagByClassName("vtkMRMLColorNode"));
    
    // now get the newly active node 
    activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());
    if (activeColorNode == NULL)
      {
      std::cerr << "ERROR adding a new color for the point...\n";
      return;
      }
    // set it to user defined
    activeColorNode->SetTypeToUser();
*/
    }

  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button == this->AddColorButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkDebugMacro("vtkSlicerColorGUI: ProcessGUIEvent: Add Color Button event: " << event << ".\n");
    // save state for undo
    this->MRMLScene->SaveStateForUndo();
    
    activeColorNode->AddColor("new", 0.0, 0.0, 0.0);
    }  
  
  
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{    
    vtkDebugMacro("vtkSlicerColorGUI::ProcessMRMLEvents: event = " << event << ".\n");
    
    if (event == vtkCommand::WidgetValueChangedEvent)
    {
        vtkDebugMacro("got a widget value changed event... the color node was changed.\n");
    }

    vtkMRMLColorNode *node = vtkMRMLColorNode::SafeDownCast(caller);
    vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());
    
    if (node == activeColorNode && event == vtkCommand::ModifiedEvent)
    {
        vtkDebugMacro("\tmodified event on the color node.\n");
        if (node == NULL)
        {
            vtkDebugMacro("\tBUT: the node is null\n");
            return;
        }
        vtkDebugMacro("\t\tUpdating the GUI\n");
        return;
    }
    if (node == activeColorNode && event == vtkMRMLColorNode::TypeModifiedEvent)
    {
        vtkDebugMacro("\ttype modified event on the active color.");
        if (node == NULL)
        {
            return;
        }
        SetGUIFromNode(activeColorNode);
        return;
    }
    
    if (node == activeColorNode && event == vtkMRMLColorNode::DisplayModifiedEvent)
    {
        vtkDebugMacro("vtkSlicerColorGUI::ProcessMRMLEvents: DisplayModified event on the color node...\n");
    }
    


    vtkDebugMacro("vtkSlicerColorGUI: Done processing mrml events...");
//    std::cerr << "vtkSlicerColorGUI::ProcessMRMLEvents  mismatched
//    caller and event (" << event << ")\n";
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::SetGUIFromNode(vtkMRMLColorNode * activeColorNode)
{
    if (activeColorNode == NULL)
    {
        return;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::Enter ( )
{
    vtkDebugMacro("vtkSlicerColorGUI: Enter\n");
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::Exit ( )
{
    vtkDebugMacro("vtkSlicerColorGUI: Exit\n");
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  // Define your help text here.
  const char *help = "**Color Module:** Manages color look up tables.\nTables are used by mappers to translate between an integer and a colour value for display of models and volumes.\nSlicer supports three kinds of tables:\n1. Continuous scales, like the greyscale table.\n2. Parametric tables, defined by an equation, such as the FMRIPA table.\n3. Discreet tables, such as those read in from a file.\nUsers are only allowed to edit User type tables. TODO: allow copy from a standard one.";

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Color", "Color", NULL );

    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Color" );
    
    // HELP FRAME
    vtkSlicerModuleCollapsibleFrame *modHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modHelpFrame->SetParent (page );
    modHelpFrame->Create ( );
    modHelpFrame->CollapseFrame ( );
    modHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Color")->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( modHelpFrame->GetFrame() );
    this->HelpText->Create ( );
    this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
    this->HelpText->SetVerticalScrollbarVisibility ( 1 );
    this->HelpText->GetWidget()->SetText ( help );
    this->HelpText->GetWidget()->SetReliefToFlat ( );
    this->HelpText->GetWidget()->SetWrapToWord ( );
    this->HelpText->GetWidget()->ReadOnlyOn ( );
    this->HelpText->GetWidget()->QuickFormattingOn ( );
    this->HelpText->GetWidget()->SetBalloonHelpString ( "" );
    app->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                  this->HelpText->GetWidgetName ( ) );

    // ---
    // DISPLAY FRAME            
    vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayFrame->SetParent ( page );
    displayFrame->Create ( );
    displayFrame->SetLabelText ("Display");
    displayFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("Color")->GetWidgetName());
  
    // color display widget
    this->ColorDisplayWidget = vtkSlicerColorDisplayWidget::New ( );
    this->ColorDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
    this->ColorDisplayWidget->SetParent ( displayFrame->GetFrame() );
    this->ColorDisplayWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ColorDisplayWidget->GetWidgetName(), 
                  displayFrame->GetFrame()->GetWidgetName());

    // button frame
    vtkKWFrame *buttonFrame = vtkKWFrame::New();
    buttonFrame->SetParent ( displayFrame->GetFrame() );
    buttonFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                 buttonFrame->GetWidgetName(),
                 displayFrame->GetFrame()->GetWidgetName());

    // a button to add a new colour
    this->AddColorButton = vtkKWPushButton::New();
    this->AddColorButton->SetParent( buttonFrame);
    this->AddColorButton->Create();
    this->AddColorButton->SetText("Add a Color");
    this->AddColorButton->SetBalloonHelpString("Add a colour to a user defined list");

    // pack the buttons
/* leave the add color button out for now, TODO: add in editing of the tables
     app->Script("pack %s -side top -anchor w -padx 4 -pady 2 -in %s", 
     this->AddColorButton->GetWidgetName(),
     buttonFrame->GetWidgetName());
*/
    // deleting frame widgets
    buttonFrame->Delete();
    displayFrame->Delete ( );
    modHelpFrame->Delete ( );
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::SetColorNode (vtkMRMLColorNode *colorNode)
{
    if (colorNode == NULL)
    {
        std::cerr << "vtkSlicerColorGUI: ERROR: SetColorNode - color node is null.\n";
        return;
    }
    // save the ID
    vtkDebugMacro("setting the color node id to " << colorNode->GetID());
    this->SetColorNodeID(colorNode->GetID());
    this->SetGUIFromNode(colorNode);
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::SetColorNodeID (char * id)
{
    if (this->GetColorNodeID() != NULL &&
        id != NULL &&
        strcmp(id,this->GetColorNodeID()) == 0)
    {
        vtkDebugMacro("no change in id, not doing anything for now: " << id << endl);
        return;
    }

    // get the old node
    vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetColorNodeID()));
       
    // set the id properly - see the vtkSetStringMacro
    this->ColorNodeID = id;

    if (id == NULL)
      {
      vtkDebugMacro("SetColorNodeID: NULL input id, removed observers and returning.\n");
      return;
      }
    
    // get the new node
    colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetColorNodeID()));
    // set up observers on the new node
    if (colorNode != NULL)
      {
      if (this->GetDebug())
        {
        colorNode->DebugOn();
        }
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      events->InsertNextValue(vtkMRMLColorNode::DisplayModifiedEvent);
      events->InsertNextValue(vtkMRMLColorNode::TypeModifiedEvent);
      vtkSetAndObserveMRMLNodeEventsMacro(this->ColorNode, colorNode, events);
      events->Delete();

      // set up the GUI
      this->SetGUIFromNode(this->ColorNode);
      }
    else
      {
        std::cerr << "ERROR: unable to get the mrml color node to observe!\n";
      }
}
