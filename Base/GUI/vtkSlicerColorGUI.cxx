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

#include "vtkKWColorPickerDialog.h"
#include "vtkKWColorPickerWidget.h"
#include "vtkKWColorSwatchesWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorGUI );
vtkCxxRevisionMacro ( vtkSlicerColorGUI, "$Revision$");


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
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: AddGUIObservers\n");
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void * vtkNotUsed(callData) )
{
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
    this->ColorDisplayWidget->SetColorNodeID(colorNode->GetID());
    
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
  std::string helpString = std::string("The Color Module manages color look up tables.\n\nFor more information see <a>http://www.slicer.org/slicerWiki/index.php/Modules:Colors-Documentation-3.6</a>\n\nTables are used by mappers to translate between an integer and a colour value for display of models and volumes.\nSlicer supports three kinds of tables:\n1. Continuous scales, like the greyscale table.\n2. Parametric tables, defined by an equation, such as the FMRIPA table.\n3. Discrete tables, such as those read in from a file.\n\n\nYou can specify a directory from which to read color files using the View -> Application Settings window, Module Settings frame, in teh User defined color file paths section. The color file format is a plain text file with the .txt or .ctbl extension. Each line in the file has:\nlabel\tname\tR\tG\tB\tA\nlabel is an integer, name a string, and RGBA are 0-255.\n\nUsers are only allowed to edit User type tables. Use the Edit frame to create a new color table (you can copy from an existing one), and save it to a file using the File, Save interface.\n\n\n") + nodeNamesDescriptions;
  const char *help = helpString.c_str();
  const char *about = "This module was developed by Nicole Aucoin, SPL, BWH (Ron Kikinis). This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
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
  // update the viewer widget
  this->SetActiveViewer(this->GetApplicationGUI()->GetActiveViewerWidget());
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

  // select the default label map by default
  if (this->MRMLScene)
    {
    vtkSmartPointer<vtkSlicerColorLogic> colorLogic = vtkSmartPointer<vtkSlicerColorLogic>::New();
    char *defaultID = const_cast<char *>(colorLogic->GetDefaultLabelMapColorNodeID());
    vtkMRMLColorNode *defaultNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(defaultID));
    if (defaultNode)
      {
      if (this->ColorDisplayWidget->GetColorSelectorWidget())
        {
        this->ColorDisplayWidget->GetColorSelectorWidget()->UpdateMenu();
        this->ColorDisplayWidget->GetColorSelectorWidget()->SetSelected(defaultNode);
        }
      this->ColorDisplayWidget->SetColorNodeID(defaultID);
      if (this->ColorEditWidget->GetCopyNodeSelectorWidget())
        {
        this->ColorEditWidget->GetCopyNodeSelectorWidget()->UpdateMenu();
        this->ColorEditWidget->GetCopyNodeSelectorWidget()->SetSelected(defaultNode);
        }
      }
    else
      {
      vtkDebugMacro("CreateWidget: no default node, skipping prefilling the table");
      }
    }
  else
    {
    vtkDebugMacro("CreateWidget: no mrml scene, can't set the default label map color node");
    }
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
      // check the names, don't want to add all of them
      // this will add default labels, freesurfer labels, color files that
      // have Color in the name and ones that start with Slicer3
      std::string nodeName = colorNode->GetName();
      if (nodeName.find("Labels") != std::string::npos ||
          nodeName.find("Color") != std::string::npos ||
          nodeName.find("Slicer3") != std::string::npos)
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
      }
      else
        {
        vtkErrorMacro("Can't find " << n << "th colour node");
        }
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::SetActiveViewer(vtkSlicerViewerWidget *activeViewer )
{
  if (this->ColorDisplayWidget)
    {
    this->ColorDisplayWidget->SetViewerWidget(activeViewer);
    }
}
