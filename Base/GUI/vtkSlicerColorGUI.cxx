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

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkSlicerColorGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorGUI );
vtkCxxRevisionMacro ( vtkSlicerColorGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerColorGUI::vtkSlicerColorGUI ( )
{
    this->Logic = NULL;
    this->ColorSelectorWidget = NULL;
    this->ColorNodeID = NULL; // "(none)";
    this->ColorNode = NULL; // "(none)";

    this->ColorNodeTypeScale = NULL;
    
    //this->DebugOn();
}


//---------------------------------------------------------------------------
vtkSlicerColorGUI::~vtkSlicerColorGUI ( )
{

    this->SetModuleLogic ( NULL );

    if (this->ColorSelectorWidget)
    {
        this->ColorSelectorWidget->SetParent(NULL);
        this->ColorSelectorWidget->Delete();
        this->ColorSelectorWidget = NULL;
    }

    if (this->ColorNodeTypeScale) {
        this->ColorNodeTypeScale->SetParent(NULL);
        this->ColorNodeTypeScale->Delete();
        this->ColorNodeTypeScale = NULL;
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
    this->ColorSelectorWidget->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ColorNodeTypeScale->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->RemoveObservers (vtkSlicerColorGUI::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::AddGUIObservers ( )
{
    vtkDebugMacro("vtkSlicerColorGUI: AddGUIObservers\n");
    this->ColorSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

    this->ColorNodeTypeScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    
    this->AddObserver(vtkSlicerColorGUI::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // process  color node selector events
    vtkSlicerNodeSelectorWidget *colorSelector = 
        vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    if (colorSelector == this->ColorSelectorWidget &&
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
        vtkDebugMacro("vtkSlicerColorGUI: ProcessGUIEvent Node Selector Event: " << event << ".\n");
        std::cout << "vtkSlicerColorGUI: ProcessGUIEvent Node Selector Event: " << event << ".\n";
        
        vtkMRMLColorNode *colorNode =
            vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
        if (colorNode != NULL)
        {
            this->SetColorNode(colorNode);
        }
        return;
    }
    // process id changed events
    if (event == vtkSlicerColorGUI::ColorIDModifiedEvent)
      {
      std::cout << "vtkSlicerColorGUI::ProcessGUIEvents : got a colour id modified event.\n";     
      }
        
    vtkMRMLColorNode *colorNode =
        vtkMRMLColorNode::SafeDownCast(caller);
    if (colorNode== this->MRMLScene->GetNodeByID(this->GetColorNodeID()) &&
        event == vtkCommand::ModifiedEvent)
    {
        return;
    }
    
  // get the currently displayed list
  
  // is there one list?
  vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());
  if (activeColorNode == NULL)
  {
      std::cerr << "ERROR: No Color, adding one first!\n";
      this->ColorSelectorWidget->ProcessNewNodeCommand("vtkMRMLColorNode",
                                                              this->MRMLScene->GetTagByClassName("vtkMRMLColorNode"));

      // now get the newly active node 
      activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());
      if (activeColorNode == NULL)
      {
          std::cerr << "ERROR adding a new color for the point...\n";
          return;
      }
  }

  // color node type scale
  vtkKWScale *scale = vtkKWScale::SafeDownCast(caller);
  if (scale == this->ColorNodeTypeScale && event == vtkKWScale::ScaleValueChangedEvent)
    {
    std::cout << "vtkSlicerColorGUI: color node type changed...\n";
    activeColorNode->SetType((int)(this->ColorNodeTypeScale->GetValue()));
    // update the label text too
    std::string newLabel = std::string("Node Type: ") + std::string(activeColorNode->GetTypeAsString());
    this->ColorNodeTypeScale->SetLabelText(newLabel.c_str());
    }
  
  // save state for undo
  this->MRMLScene->SaveStateForUndo(activeColorNode);

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
    if (node == vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()) && event == vtkCommand::ModifiedEvent)
    {
        vtkDebugMacro("\tmodified event on the color selected node.\n");
        if (activeColorNode !=  vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()))
          {
          // select it  and update the gui
          std::cout << "vtkSlicerColorGUI::ProcessMRMLEvents: modified event on the color selected node, setting the color node\n";
          this->SetColorNode(vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()));
          }
        return;        
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
    
    int scale = activeColorNode->GetType();
    if (this->ColorNodeTypeScale != NULL &&
        scale != this->ColorNodeTypeScale->GetValue())
      {
      this->ColorNodeTypeScale->SetValue(scale);
      std::string newLabel = std::string("Node Type: ") + std::string(activeColorNode->GetTypeAsString());
      this->ColorNodeTypeScale->SetLabelText(newLabel.c_str());
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
  const char *help = "**Color Module:** Manages color look up tables.\nTables are used by mappers to translate between an integer and a colour value for display of models and volumes.\nSlicer supports three kinds of tables:\n1. Continuous scales, like the greyscale table.\n2. Parametric tables, defined by an equation, such as the FMRIPA table. 3. Discreet tables, such as those read in from a file.";

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
  
    // node selector
    this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    this->ColorSelectorWidget->SetParent(displayFrame->GetFrame());
    this->ColorSelectorWidget->Create();
    this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
    this->ColorSelectorWidget->NewNodeEnabledOn();
    this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->ColorSelectorWidget->SetBorderWidth(2);
    this->ColorSelectorWidget->SetPadX(2);
    this->ColorSelectorWidget->SetPadY(2);
    //this->ColorSelectorWidget->GetWidget()->IndicatorVisibilityOff();
    this->ColorSelectorWidget->GetWidget()->SetWidth(24);
    this->ColorSelectorWidget->SetLabelText( "Color Select: ");
    this->ColorSelectorWidget->SetBalloonHelpString("Select a color from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->ColorSelectorWidget->GetWidgetName());
    

    // type
    this->ColorNodeTypeScale = vtkKWScale::New();
    this->ColorNodeTypeScale->SetParent( displayFrame );
    this->ColorNodeTypeScale->Create();
    this->ColorNodeTypeScale->SetLabelText("Node Type:             ");
    this->ColorNodeTypeScale->SetLength(200);
    this->ColorNodeTypeScale->SetBalloonHelpString ( "Set the type of the color node.");
    vtkMRMLColorNode *basicNode = vtkMRMLColorNode::New();
    this->ColorNodeTypeScale->SetRange(basicNode->GetFirstType(), basicNode->GetLastType());
    basicNode->Delete();
    this->ColorNodeTypeScale->SetOrientationToHorizontal ();
    this->ColorNodeTypeScale->SetResolution(1.0);
    app->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
                this->ColorNodeTypeScale->GetWidgetName());
    
    // deleting frame widgets
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
