#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerColorDisplayWidget.h"

#include "vtkSlicerApplication.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"

#include "vtkMRMLColorNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerColorDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerColorDisplayWidget::vtkSlicerColorDisplayWidget ( )
{

    this->ColorNodeID = NULL;
    this->ColorNode = NULL; 

    this->ColorSelectorWidget = NULL;

    this->ColorNodeTypeLabel = NULL;

    this->MultiColumnList = NULL;
    this->NumberOfColumns = 3;

    this->NumberOfColorsLabel = NULL;

    this->AddColorButton = NULL;

    this->ShowOnlyNamedColorsCheckButton = NULL;

    this->SelectedColorLabel = NULL;
    
    //this->DebugOn();
}


//---------------------------------------------------------------------------
vtkSlicerColorDisplayWidget::~vtkSlicerColorDisplayWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if (this->ColorSelectorWidget)
    {
    this->ColorSelectorWidget->SetParent(NULL);
    this->ColorSelectorWidget->Delete();
    this->ColorSelectorWidget = NULL;
    }
  if (this->ColorNodeTypeLabel) 
    {
    this->ColorNodeTypeLabel->SetParent(NULL);
    this->ColorNodeTypeLabel->Delete();
    this->ColorNodeTypeLabel = NULL;
    }
  if (this->MultiColumnList) 
    {
    this->MultiColumnList->SetParent(NULL);
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;
    }
    
  if (this->NumberOfColorsLabel)
    {
    this->NumberOfColorsLabel->SetParent (NULL);
    this->NumberOfColorsLabel->Delete();
    this->NumberOfColorsLabel = NULL;
    }

  if (this->AddColorButton)
    {
    this->AddColorButton->SetParent(NULL);
    this->AddColorButton->Delete();
    this->AddColorButton = NULL;
    }

  if (this->ShowOnlyNamedColorsCheckButton)
    {
    this->ShowOnlyNamedColorsCheckButton->SetParent(NULL);
    this->ShowOnlyNamedColorsCheckButton->Delete();
    this->ShowOnlyNamedColorsCheckButton = NULL;
    }

  if (this->SelectedColorLabel)
    {
    this->SelectedColorLabel->SetParent(NULL);
    this->SelectedColorLabel->Delete();
    this->SelectedColorLabel = NULL;
    }
  
  this->SetMRMLScene ( NULL );
  this->SetColorNodeID (NULL);
  vtkSetMRMLNodeMacro(this->ColorNode, NULL);

}


//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerColorDisplayWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "ColorNode ID: " << this->GetColorNodeID() << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::SetColorNode ( vtkMRMLColorNode *colorNode )
{
  // Select this color node
  if (vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()) != colorNode)
    {
    this->ColorSelectorWidget->SetSelected(colorNode); 
    }

  // 
  // Set the member variables and do a first process
  //
  this->RemoveMRMLObservers();

  if (colorNode)
    {
    this->SetColorNodeID( colorNode->GetID() );
    }
  else
    {
    this->SetColorNodeID(NULL);
    }

  this->AddMRMLObservers();

  if ( colorNode )
    {
//    this->ProcessMRMLEvents(colorNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::SetColorNodeID (char * id)
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
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      events->InsertNextValue(vtkMRMLColorNode::TypeModifiedEvent);
      vtkSetAndObserveMRMLNodeEventsMacro(this->ColorNode, colorNode, events);
      events->Delete();

      // throw the event
      this->InvokeEvent(vtkSlicerColorDisplayWidget::ColorIDModifiedEvent);
      // set up the GUI
      this->UpdateWidget();
      }
    else
      {
      vtkErrorMacro ("ERROR: unable to get the mrml color node to observe!\n");
      }
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  // process id changed events
  if (vtkSlicerColorDisplayWidget::SafeDownCast(caller) != NULL &&
      event == vtkSlicerColorDisplayWidget::ColorIDModifiedEvent)
    {
    vtkDebugMacro("vtkSlicerColorDisplayWidget::ProcessGUIEvents : got a colour id modified event.\n");
    this->UpdateWidget();
    return;
    }

  // did the color node that we're watching get modified?
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(caller);
  if (colorNode == this->MRMLScene->GetNodeByID(this->GetColorNodeID()) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidget();
    return;
    }

  //
  // Did the selected row change?
  //
  vtkKWMultiColumnList *listBox = vtkKWMultiColumnList::SafeDownCast(caller);  
  if (listBox != NULL &&
      listBox == this->MultiColumnList->GetWidget() &&
      event == vtkKWMultiColumnList::SelectionChangedEvent)
    {
    this->UpdateSelectedColor();
    this->InvokeEvent(vtkSlicerColorDisplayWidget::SelectedColorModifiedEvent);
    return;
    }
  
  //
  // Did we change the show only named checkbutton?
  //
  vtkKWCheckButton *checkButton = vtkKWCheckButton::SafeDownCast(caller);
  if (checkButton != NULL && checkButton == this->ShowOnlyNamedColorsCheckButton &&
      event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    this->UpdateWidget();
    return;
    }
  //
  // process color selector events
  //
  vtkSlicerNodeSelectorWidget *colorSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (colorSelector == this->ColorSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLColorNode *color = 
        vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color != NULL)
      {
      this->SetColorNode(color);
      } else { vtkDebugMacro("Got a node selector event, but the color is null"); }

    return;
    }

  // get the currently displayed list 
  vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());

  if (activeColorNode == NULL)
    {
    vtkErrorMacro("ERROR: No Color!\n");
    return;
    }

  // 
  // save state for undo?
//  this->MRMLScene->SaveStateForUndo();

  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button == this->AddColorButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkDebugMacro("vtkSlicerColorDisplayWidget: ProcessGUIEvent: Add Color Button event: " << event << ".\n");
    // if it's a colour table type node
    if (vtkMRMLColorTableNode::SafeDownCast(activeColorNode) != NULL)
      {
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeColorNode);
    
      vtkMRMLColorTableNode::SafeDownCast(activeColorNode)->AddColor("new", 0.0, 0.0, 0.0);
      }
    }
  this->UpdateMRML();
} 



//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if ( !this->ColorNodeID )
    {
    return;
    }

  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(caller);
  vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());

  if (colorNode == (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID()) && 
      colorNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("vtkSlicerColorDisplayWidget::ProcessMRMLEvents color node modified, updating widget and returning\n");
    this->UpdateWidget();
    return;
    }

  if (colorNode == vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()) && 
      event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("\tmodified event on the color selected node.\n");
    if (activeColorNode !=  vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()))
      {
      // select it  and update the gui
      vtkDebugMacro("vtkSlicerColorDisplayWidget::ProcessMRMLEvents: modified event on the color selected node, setting the color node\n");
      this->SetColorNode(vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()));
      }
    return;        
    }    
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::AddMRMLObservers ( )
{
  /*
    if ( !this->ColorNodeID )
    {
    return;
    }

  vtkMRMLColorNode *colorNode =
  vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ColorNodeID));
  if (colorNode != NULL)
  {
  }
  */
  
//  this->AddObserver(vtkSlicerColorDisplayWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand );  

}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::RemoveMRMLObservers ( )
{
  //this->RemoveObservers(vtkSlicerColorDisplayWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
  /*
  if ( !this->ColorNodeID )
    {
    return;
    }

  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ColorNodeID));
  
  if (colorNode != NULL)
    {
    }
  */
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::UpdateWidget()
{
  
  if ( this->ColorNodeID )
    {
    vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ColorNodeID));
    
    if (colorNode == NULL)
      {
      return;
      }
    if (this->ColorNodeTypeLabel != NULL)
      {
      std::string newLabel = std::string("Node Type: ") + std::string(colorNode->GetTypeAsString());
      this->ColorNodeTypeLabel->SetText(newLabel.c_str());
      }
    
    int numColours = 0;
    if (vtkMRMLColorTableNode::SafeDownCast(colorNode) != NULL)
      {
      numColours = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetNumberOfColors();
      }
    else if (vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode) != NULL &&
             vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable() != NULL)
      {
      numColours = vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable()->GetNumberOfColors();
      }

    bool showOnlyNamedColors;
    if (this->ShowOnlyNamedColorsCheckButton->GetSelectedState())
      {
      showOnlyNamedColors = true;
      }
    else
      {
      showOnlyNamedColors = false;
      }
    // set the number of colours
    std::stringstream ss;
    if (showOnlyNamedColors)
      {
      ss << "Total Number of Colours in Table: ";
      }
    else
      {
      ss << "Number of Colors: ";
      }
    ss << numColours;
    std::string newColorLabel = ss.str().c_str();
    this->NumberOfColorsLabel->SetText(newColorLabel.c_str());
    
    
    bool deleteFlag = true;
    if (numColours > this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // add rows to the table
      int numToAdd = numColours - this->MultiColumnList->GetWidget()->GetNumberOfRows();
      this->MultiColumnList->GetWidget()->AddRows(numToAdd);
      }
    if (numColours < this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // delete some rows
      for (int r = this->MultiColumnList->GetWidget()->GetNumberOfRows(); r >= numColours; r--)
        {
        this->MultiColumnList->GetWidget()->DeleteRow(r);
        }
      }
    if (showOnlyNamedColors || numColours != this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // clear out the multi column list box and fill it in with the new list
      // - if only showing named colours, there might not be numColours rows
      vtkDebugMacro("Clearing out the colours MCLB, numColours = " << numColours);
      this->MultiColumnList->GetWidget()->DeleteAllRows();
      }
    else
      {
      deleteFlag = false;
      }
    
    // a row for each colour
    double *colour = NULL;
    const char *name;
    // keep track of where to add the current colour into the table
    int thisRow = 0;
    for (int row = 0; row < numColours; row++)
      {
      // get the colour
      if (colorNode->GetLookupTable() != NULL)
        {
        colour = colorNode->GetLookupTable()->GetTableValue(row);
        }
      if (colour == NULL)
        {
        vtkErrorMacro ("SetGUIFromNode: at " << row << "th colour, got a null pointer" << endl);
        }
      // get the colour label
      name = colorNode->GetColorName(row);
      if (!showOnlyNamedColors ||
          (showOnlyNamedColors && strcmp(name, colorNode->GetNoName()) != 0))
        {
        // update this colour
        if (deleteFlag)
          {
          this->MultiColumnList->GetWidget()->AddRow();
          }
        // now set the table
        // which entry is it in the colour table?
        if (thisRow == 0 || row == 0 ||
            this->MultiColumnList->GetWidget()->GetCellTextAsInt(thisRow, this->EntryColumn) != row)
          {
          vtkDebugMacro("Setting entry column #" << thisRow << " to " << row);
          this->MultiColumnList->GetWidget()->SetCellTextAsInt(thisRow, this->EntryColumn, row);
          }
        
        // what's it's name?
        if (strcmp(this->MultiColumnList->GetWidget()->GetCellText(thisRow,this->NameColumn), name) != 0)
          {
          this->MultiColumnList->GetWidget()->SetCellText(thisRow,this->NameColumn,name);
          }
        
        // what's the colour?
        if (colour != NULL)
          {
          this->MultiColumnList->GetWidget()->SetCellBackgroundColor(thisRow, this->ColourColumn, colour);
          }
        else
          {
          this->MultiColumnList->GetWidget()->ClearCellBackgroundColor(thisRow, this->ColourColumn);
          }
        thisRow++;
        }
      vtkDebugMacro("Done rebuilding table, row = " << row << ", thisRow = " << thisRow);
      }

    this->UpdateSelectedColor();
    }
  else
    {
    vtkDebugMacro("UpdateWidget: No colour node id \n");
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::UpdateSelectedColor()
{
  if (this->SelectedColorLabel)
    {
    int selectedColor = this->GetSelectedColorIndex();
    std::stringstream ss;
    ss << "Selected Color Label: ";
    if (selectedColor != -1)
      {
      ss << selectedColor;
      }
    this->SelectedColorLabel->SetText(ss.str().c_str());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::RemoveWidgetObservers ( ) {
  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddColorButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ShowOnlyNamedColorsCheckButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,
                                                        (vtkCommand *)this->GUICallbackCommand);
  this->MultiColumnList->GetWidget()->RemoveObservers(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  this->RemoveObservers (vtkSlicerColorDisplayWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
  // ---
  // DISPLAY FRAME            
  vtkKWFrame *displayFrame = vtkKWFrame::New ( );
  displayFrame->SetParent ( this->GetParent() );
  displayFrame->Create ( );
/*
  displayFrame->SetLabelText ("Display");
  displayFrame->CollapseFrame ( );
*/
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 displayFrame->GetWidgetName() );

  // node selector
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New();
  this->ColorSelectorWidget->SetParent(displayFrame);
  this->ColorSelectorWidget->Create();
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  // don't allow new nodes to be created until can edit them
//    this->ColorSelectorWidget->NewNodeEnabledOn();
  this->ColorSelectorWidget->ShowHiddenOn();
  this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ColorSelectorWidget->SetBorderWidth(2);
  this->ColorSelectorWidget->SetPadX(2);
  this->ColorSelectorWidget->SetPadY(2);
  //this->ColorSelectorWidget->GetWidget()->IndicatorVisibilityOff();
  this->ColorSelectorWidget->GetWidget()->SetWidth(24);
  this->ColorSelectorWidget->SetLabelText( "Color Select: ");
  this->ColorSelectorWidget->SetBalloonHelpString("Select a color from the current mrml scene.");
  this->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ColorSelectorWidget->GetWidgetName(),
                displayFrame->GetWidgetName());
  
  
  // type
  this->ColorNodeTypeLabel = vtkKWLabel::New();
  this->ColorNodeTypeLabel->SetParent( displayFrame );
  this->ColorNodeTypeLabel->Create();
  this->ColorNodeTypeLabel->SetText("Node Type:             ");
  this->ColorNodeTypeLabel->SetBalloonHelpString ( "The type of the color node.");
  app->Script("pack %s -side top -anchor w -padx 2 -pady 2 -in %s", 
              this->ColorNodeTypeLabel->GetWidgetName(),
              displayFrame->GetWidgetName());
  
  // how many colours in the lookup table?
  this->NumberOfColorsLabel = vtkKWLabel::New();
  this->NumberOfColorsLabel->SetParent( displayFrame );
  this->NumberOfColorsLabel->Create();
  this->NumberOfColorsLabel->SetText("Number of Colors:");
  this->Script("pack %s -side top -anchor w -padx 2 -pady 2 -in %s",
               this->NumberOfColorsLabel->GetWidgetName(), 
               displayFrame->GetWidgetName());

  // Display the currently selected colour index
  this->SelectedColorLabel = vtkKWLabel::New();
  this->SelectedColorLabel->SetParent(displayFrame);
  this->SelectedColorLabel->Create();
  this->SelectedColorLabel->SetText("Selected Color Label: ");
  app->Script("pack %s -side top -anchor w -padx 2 -pady 2 -in %s",
              this->SelectedColorLabel->GetWidgetName(),
              displayFrame->GetWidgetName());
  
  // Display the colours in the lookup table
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( displayFrame );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
  this->MultiColumnList->GetWidget()->MovableRowsOff();
  this->MultiColumnList->GetWidget()->MovableColumnsOff();
  // set up the columns of data for each table entry
  // refer to the header file for the order
  this->MultiColumnList->GetWidget()->AddColumn("Entry");
  this->MultiColumnList->GetWidget()->AddColumn("Name");
  this->MultiColumnList->GetWidget()->AddColumn("Color");

  
  // make the colour column editable by colour chooser
  //    this->MultiColumnList->GetWidget()->SetColumnEditWindowToColorButton(this->ColourColumn);
  // don't show the colour text
  //this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput (this->ColourColumn);
  
  // now set attribs that are equal across the columns
  int col;
  for (col = 0; col < this->NumberOfColumns; col++)
    {
    this->MultiColumnList->GetWidget()->SetColumnWidth(col, 6);
    this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
    this->MultiColumnList->GetWidget()->ColumnEditableOff(col);
    }
  // set the name and colour column widths to be higher
  this->MultiColumnList->GetWidget()->SetColumnWidth(this->NameColumn, 25);
  
  app->Script ( "pack %s -fill both -expand true -in %s",
                this->MultiColumnList->GetWidgetName(),
                displayFrame->GetWidgetName());
  this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");
              
  // button frame
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent ( displayFrame );
  buttonFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               buttonFrame->GetWidgetName(),
               displayFrame->GetWidgetName());
  
  // a button to add a new colour
  this->AddColorButton = vtkKWPushButton::New();
  this->AddColorButton->SetParent( buttonFrame);
  this->AddColorButton->Create();
  this->AddColorButton->SetText("Add a Color");
  this->AddColorButton->SetBalloonHelpString("Add a colour to a user defined list");

  // a checkbutton to only show the valid colours (useful for the SPL atlas
  // table which skips ranges)
  this->ShowOnlyNamedColorsCheckButton = vtkKWCheckButton::New();
  this->ShowOnlyNamedColorsCheckButton->SetParent ( buttonFrame );
  this->ShowOnlyNamedColorsCheckButton->Create();
  this->ShowOnlyNamedColorsCheckButton->SelectedStateOn();
  this->ShowOnlyNamedColorsCheckButton->SetText("Show Only Named Colors");
  
  // pack the buttons
/* leave the add color button out for now, TODO: add in editing of the tables
   app->Script("pack %s -side top -anchor w -padx 4 -pady 2 -in %s", 
   this->AddColorButton->GetWidgetName(),
   buttonFrame->GetWidgetName());
*/
  // pack the checkbutton
  app->Script("pack %s -side top -anchor w -padx 4 -pady 2 -in %s",
              this->ShowOnlyNamedColorsCheckButton->GetWidgetName(),
              buttonFrame->GetWidgetName());
  
  // deleting frame widgets
  buttonFrame->Delete();
    
  // add observers
  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddColorButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ShowOnlyNamedColorsCheckButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AddObserver(vtkSlicerColorDisplayWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MultiColumnList->GetWidget()->AddObserver(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  

  // clean up
  displayFrame->Delete();

  // TODO: somehow trigger the filling in of the table
  
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::UpdateElement(int row, int col, char * str)
{
  vtkDebugMacro("UpdateElement: row = " << row << ", col = " << col << ", str = " << str << "\n");
  
  // make sure that the row and column exists in the table
  if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->MultiColumnList->GetWidget()->GetNumberOfColumns()))
    {
    vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());
    // is there an active list?
    if (activeColorNode == NULL)
      {
      // 
      vtkErrorMacro ("UpdateElement: ERROR: No colournode, add one first!\n");
      return;
      }
    // the entry in the colour table
    int entry = this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, this->EntryColumn);
    
    // now update the requested value
    if (col == this->NameColumn)
      {
      activeColorNode->SetColorName(entry,str);
      }
    else if (col == this->ColourColumn)
      {
      double r, g, b;
      std::stringstream ss;
      ss << str;
      ss >> r;
      ss >> g;
      ss >> b;
      const char *name = activeColorNode->GetColorName(entry);
      if (vtkMRMLColorTableNode::SafeDownCast(activeColorNode) != NULL)
        {
        vtkMRMLColorTableNode::SafeDownCast(activeColorNode)->SetColor(entry, name, r, g, b);
        }
      }
    }
  else
    {
    vtkErrorMacro ("Invalid row " << row << " or column " << col <<  ", valid columns are 0-" << this->NumberOfColumns << "\n");
    }
}

//---------------------------------------------------------------------------
int vtkSlicerColorDisplayWidget::GetSelectedColorIndex()
{
  if (this->MultiColumnList == NULL)
    {
    return -1;
    }

  int numRows = this->MultiColumnList->GetWidget()->GetNumberOfSelectedRows();
  int row;  
  if (numRows == 0 && this->MultiColumnList->GetWidget()->GetNumberOfRows() > 1)
    {
    // no selection was made, set it up to select index 1 (0 is black)
    row = 1;
    this->MultiColumnList->GetWidget()->SelectSingleRow(row);
    numRows = this->MultiColumnList->GetWidget()->GetNumberOfSelectedRows();
    vtkDebugMacro("No rows were selected, forcing selection of row " << row << ", numRows = " << numRows);
    }

  if (numRows != 1)
    {
    vtkWarningMacro("Error in selection: " << numRows << " selected, select just one and try again.");
    } 
  // the table index may not match the colour index, return the value
  row = this->MultiColumnList->GetWidget()->GetIndexOfFirstSelectedRow();
  return this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, this->EntryColumn);
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::SetSelectedColorIndex(int index)
{
  if (this->MultiColumnList == NULL)
    {
    return;
    }
  this->MultiColumnList->GetWidget()->SelectSingleRow(index);
  this->UpdateSelectedColor();
}

