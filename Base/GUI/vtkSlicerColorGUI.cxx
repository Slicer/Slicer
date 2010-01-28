#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWMessage.h"

#include "vtkKWTopLevel.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkSlicerColorDisplayWidget.h"
#include "vtkSlicerColorEditWidget.h"

#include "vtkKWMessageDialog.h"

#include "vtkSlicerColorGUI.h"

#include "vtkKWPushButton.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"

#include "vtkKWColorPickerDialog.h"
#include "vtkKWColorPickerWidget.h"
#include "vtkKWColorSwatchesWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorGUI );
vtkCxxRevisionMacro ( vtkSlicerColorGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerColorGUI::vtkSlicerColorGUI ( )
{
  this->Logic = NULL;
  NACLabel = NULL;
  NAMICLabel = NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;
  this->ColorDisplayWidget = NULL;
  this->ColorEditWidget = NULL;
  this->LoadColorFileButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerColorGUI::~vtkSlicerColorGUI ( )
{

  this->SetModuleLogic (static_cast<vtkSlicerColorLogic*>(0));
  
  if (this->ColorDisplayWidget ) 
    {
    this->ColorDisplayWidget->SetParent(NULL);
    this->ColorDisplayWidget->Delete ( );
    }

  if (this->ColorEditWidget ) 
    {
    this->ColorEditWidget->SetParent(NULL);
    this->ColorEditWidget->Delete ( );
    }

  if ( this->NACLabel )
    {
    this->NACLabel->SetParent ( NULL );
    this->NACLabel->Delete();
    this->NACLabel = NULL;
    }
  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  if ( this->NCIGTLabel )
    {
    this->NCIGTLabel->SetParent ( NULL );
    this->NCIGTLabel->Delete();
    this->NCIGTLabel = NULL;
    }
  if ( this->BIRNLabel )
    {
    this->BIRNLabel->SetParent ( NULL );
    this->BIRNLabel->Delete();
    this->BIRNLabel = NULL;
    }
  if (this->LoadColorFileButton )
    {
    this->LoadColorFileButton->SetParent(NULL );
    this->LoadColorFileButton->Delete ( );
    }
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
  if (this->LoadColorFileButton)
    {
    this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  /*
  if (this->GetApplication())
    {
    this->GetApplication()->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  */
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: AddGUIObservers\n");
  this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent , (vtkCommand *)this->GUICallbackCommand );

  /*
  if (this->GetApplication() != NULL)
    {
    // watch the application for modified events as they may signal the user set
    // color file paths changing
    this->GetApplication()->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  */
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void * vtkNotUsed(callData) )
{
  if (this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event ==  vtkKWTopLevel::WithdrawEvent )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadColorFileButton->GetWidget()->GetFileName();
    if ( fileName )
      {
      vtkMRMLColorNode *node = this->Logic->LoadColorFile(fileName);
      if (!node)
        {
        vtkErrorMacro("ProcessGUIEvents: unable to read file " << fileName);
        }
      else
        {
        this->ColorDisplayWidget->SetColorNode(node);
        node->Delete();
        }
      this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      }
    return;
    }
  vtkSlicerApplication *app  = vtkSlicerApplication::SafeDownCast(caller);
  if (this->GetApplication() == app && app != NULL &&
      event == vtkCommand::ModifiedEvent)
    {
    // the application was modified, get the color files directories
    // pass it on to the color logic
    if (this->GetLogic() != NULL)
      {
      std::cout << "ProcessGUIEvents: passing possibly modified color file paths to color logic\n";
      this->GetLogic()->SetUserColorFilePaths(app->GetColorFilePaths());
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessLogicEvents ( vtkObject * vtkNotUsed(caller),
                                              unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
{
  // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessMRMLEvents ( vtkObject * vtkNotUsed(caller),
                                             unsigned long event, void * vtkNotUsed(callData) )
{    
  vtkDebugMacro("vtkSlicerColorGUI::ProcessMRMLEvents: event = " << event << ".\n");
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ReleaseModuleEventBindings ( )
{
  
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::Enter ( vtkMRMLNode *node )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  
  if (node == NULL)
    {
    return;
    }
  // otherwise try to select it
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(node);
  if (colorNode)
    {
    this->GetApplication()->ProcessPendingEvents();

    this->ColorDisplayWidget->GetColorSelectorWidget()->UpdateMenu();
    this->ColorDisplayWidget->GetColorSelectorWidget()->SetSelected(colorNode);
    this->ColorEditWidget->GetCopyNodeSelectorWidget()->UpdateMenu();
    this->ColorEditWidget->GetCopyNodeSelectorWidget()->SetSelected(colorNode);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::BuildGUI ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  // ---
  // MODULE GUI FRAME 
  // configure a page for a model loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Color", "Color", NULL );
  
  // Define your help text and build the help frame here.
  // build up a list of descriptions of the nodes
  std::string nodeNamesDescriptions = std::string("**Display:**\nNode names and descriptions:\n");
  if (this->GetMRMLScene())
    {
      int numColorNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLColorNode");
      for (int n = 0; n < numColorNodes; n++)
        {
        vtkMRMLColorNode *cnode = vtkMRMLColorNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLColorNode"));
        if (cnode)
          {
          std::string name = std::string("");
          if (cnode->GetName())
            {
            name = std::string("**") + std::string(cnode->GetName()) + std::string("**");
            }
          std::string desc = std::string("");
          if (cnode->GetDescription())
            {
            desc = std::string(cnode->GetDescription());
            }
          nodeNamesDescriptions = nodeNamesDescriptions + name + std::string(": ") + desc + std::string("\n");
        }
      }
    }
  std::string helpString = std::string("The Color Module manages color look up tables.\n\nTables are used by mappers to translate between an integer and a colour value for display of models and volumes.\nSlicer supports three kinds of tables:\n1. Continuous scales, like the greyscale table.\n2. Parametric tables, defined by an equation, such as the FMRIPA table.\n3. Discrete tables, such as those read in from a file.\n\n\n**Load:**\nYou can specify a directory from which to read color files using the View -> Application Settings window, Module Settings frame, in teh User defined color file paths section. The color file format is a plain text file with the .txt or .ctbl extension. Each line in the file has:\nlabel\tname\tR\tG\tB\tA\nlabel is an integer, name a string, and RGBA are 0-255.\n\nUsers are only allowed to edit User type tables. Use the Edit frame to create a new color table, and save it to a file using the File, Save interface.\n\n") + nodeNamesDescriptions;
  const char *help = helpString.c_str();
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Color" );
  this->BuildHelpAndAboutFrame ( page, help, about );

  this->NACLabel = vtkKWLabel::New();
  this->NACLabel->SetParent ( this->GetLogoFrame() );
  this->NACLabel->Create();
  this->NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

  this->NAMICLabel = vtkKWLabel::New();
  this->NAMICLabel->SetParent ( this->GetLogoFrame() );
  this->NAMICLabel->Create();
  this->NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

  this->NCIGTLabel = vtkKWLabel::New();
  this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
  this->NCIGTLabel->Create();
  this->NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
  this->BIRNLabel = vtkKWLabel::New();
  this->BIRNLabel->SetParent ( this->GetLogoFrame() );
  this->BIRNLabel->Create();
  this->BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );
  app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());
  app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  

  // --
  // LOAD FRAME
  vtkSlicerModuleCollapsibleFrame *loadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  loadFrame->SetParent (page);
  loadFrame->Create();
  loadFrame->SetLabelText("Load");
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
              loadFrame->GetWidgetName(),
              this->UIPanel->GetPageWidget("Color")->GetWidgetName());

  // add a file browser 
  this->LoadColorFileButton = vtkKWLoadSaveButtonWithLabel::New ( );
  this->LoadColorFileButton->SetParent ( loadFrame->GetFrame() );
  this->LoadColorFileButton->Create ( );
  this->LoadColorFileButton->SetWidth(20);
  this->LoadColorFileButton->GetWidget()->SetText ("Select Color File");
  this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open Color File");
  this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                                         "{ {Color text} {*.txt} }");
  this->LoadColorFileButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
                                                                                         "OpenPath");
  app->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
              this->LoadColorFileButton->GetWidgetName());
  
  // ---
  // DISPLAY FRAME            
  vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  displayFrame->SetParent ( page );
  displayFrame->Create ( );
  displayFrame->SetLabelText ("Display");
  displayFrame->ExpandFrame ( );
  //displayFrame->CollapseFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                displayFrame->GetWidgetName(),
                this->UIPanel->GetPageWidget("Color")->GetWidgetName());
  
  // color display widget
  this->ColorDisplayWidget = vtkSlicerColorDisplayWidget::New ( );
  this->ColorDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
  if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
  this->ColorDisplayWidget->SetViewerWidget(this->GetApplicationGUI()->GetActiveViewerWidget());
    }
  else { vtkWarningMacro("Unable to pass the viewer widget to the col display widget"); }
  this->ColorDisplayWidget->SetParent ( displayFrame->GetFrame() );
  this->ColorDisplayWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ColorDisplayWidget->GetWidgetName(), 
                displayFrame->GetFrame()->GetWidgetName());

  // --
  // EDIT FRAME
  vtkSlicerModuleCollapsibleFrame *editFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  editFrame->SetParent (page);
  editFrame->Create();
  editFrame->SetLabelText("Edit");
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
              editFrame->GetWidgetName(),
              this->UIPanel->GetPageWidget("Color")->GetWidgetName());
  editFrame->CollapseFrame();
  
  // color editing widget
  this->ColorEditWidget = vtkSlicerColorEditWidget::New ( );
  this->ColorEditWidget->SetMRMLScene(this->GetMRMLScene() );
  this->ColorEditWidget->SetParent ( editFrame->GetFrame() );
  this->ColorEditWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ColorEditWidget->GetWidgetName(), 
                editFrame->GetFrame()->GetWidgetName());

  editFrame->Delete();
  displayFrame->Delete ( );
  loadFrame->Delete();
}

//---------------------------------------------------------------------------
int vtkSlicerColorGUI::AddLUTsToColorDialog()
{
  if (!this->GetApplication())
    {
      vtkErrorMacro("Can't add LUTs to color dialog, no application");
      return 1;
    }
  vtkKWColorPickerDialog *picker = this->GetApplication()->GetColorPickerDialog();
  if (!picker) 
    {
      vtkErrorMacro("No color picker dialog yet");
      return 1;
    }
  vtkKWColorPickerWidget *widget = picker->GetColorPickerWidget();
  if (!widget)
    {
      return 1;
    }
  vtkKWColorSwatchesWidget *swatches = widget->GetColorSwatchesWidget();
  if (!swatches)
    {
      return 1;
    }
  if (!this->GetMRMLScene())
    {
      vtkErrorMacro("AddLUTsToPicker: mrml scene not set yet");
      return 1;
    }
  // remove the collections that are there now
  swatches->RemoveAllCollections();
  // loop over all the colour table nodes in the scene and add them as collections
  int numNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLColorTableNode");
  for (int n = 0; n < numNodes; n++)
    {
      vtkMRMLColorTableNode *colorNode = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLColorTableNode"));
      if (colorNode)
      {
      int collectionID = swatches->AddCollection(colorNode->GetName());
      int numColours = colorNode->GetNumberOfColors();
      double rgb[4];
      for (int c = 0; c < numColours; c++)
        {      
          const char *colorName = colorNode->GetColorName(c);
          if (strcmp(colorName, "(none)"))
            {
            colorNode->GetLookupTable()->GetColor(c, rgb);
            swatches->AddRGBSwatch(collectionID, colorName, rgb);
            }
        }
      }
      else
        {
        vtkErrorMacro("Can't find " << n << "th colour node");
        }
    }
  return 0;
}
