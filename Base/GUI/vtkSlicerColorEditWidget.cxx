#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerColorEditWidget.h"

#include "vtkSlicerApplication.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"

#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMRMLColorNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"

#include "vtkKWMessageDialog.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorEditWidget );
vtkCxxRevisionMacro ( vtkSlicerColorEditWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerColorEditWidget::vtkSlicerColorEditWidget ( )
{

    this->ColorNodeID = NULL;
    this->ColorNode = NULL; 

    this->CopyNodeSelectorWidget = NULL;

    this->MultiColumnList = NULL;
    // for now, leave out the colour text column
    this->NumberOfColumns = 3;

    this->NumberOfColorsEntry = NULL;
    this->NameEntry = NULL;
    
    this->SaveToFileButton = NULL;
    this->GenerateButton = NULL;
    this->CopyButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerColorEditWidget::~vtkSlicerColorEditWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if (this->CopyNodeSelectorWidget)
    {
    this->CopyNodeSelectorWidget->SetParent(NULL);
    this->CopyNodeSelectorWidget->Delete();
    this->CopyNodeSelectorWidget = NULL;
    }

  if (this->MultiColumnList) 
    {
    this->MultiColumnList->SetParent(NULL);
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;
    }
    
  if (this->NumberOfColorsEntry)
    {
    this->NumberOfColorsEntry->SetParent (NULL);
    this->NumberOfColorsEntry->Delete();
    this->NumberOfColorsEntry = NULL;
    }
  
  if (this->NameEntry)
    {
    this->NameEntry->SetParent (NULL);
    this->NameEntry->Delete();
    this->NameEntry = NULL;
    }

  if (this->SaveToFileButton)
    {
    this->SaveToFileButton->SetParent(NULL);
    this->SaveToFileButton->Delete();
    this->SaveToFileButton = NULL;
    }

  if (this->GenerateButton)
    {
    this->GenerateButton->SetParent(NULL);
    this->GenerateButton->Delete();
    this->GenerateButton = NULL;
    }

  if (this->CopyButton)
    {
    this->CopyButton->SetParent(NULL);
    this->CopyButton->Delete();
    this->CopyButton = NULL;
    }
  
  this->SetMRMLScene ( NULL );
  this->SetColorNodeID (NULL);
  vtkSetMRMLNodeMacro(this->ColorNode, NULL);

}


//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerColorEditWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "ColorNode ID: " << this->GetColorNodeID() << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::SetColorNode ( vtkMRMLColorNode *colorNode )
{
  /*
  // Select this color node
  if (vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()) != colorNode)
    {
    this->ColorSelectorWidget->SetSelected(colorNode); 
    }
  */
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
void vtkSlicerColorEditWidget::SetColorNodeID (char * id)
{
    if (this->GetColorNodeID() != NULL &&
        id != NULL &&
        strcmp(id,this->GetColorNodeID()) == 0)
    {
        vtkDebugMacro("no change in id, not doing anything for now: " << id << endl);
        return;
    }

    // get the old node
    //vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetColorNodeID()));
       
    // set the id properly - see the vtkSetStringMacro
    this->ColorNodeID = id;

    if (id == NULL)
      {
      vtkDebugMacro("SetColorNodeID: NULL input id, removed observers and returning.\n");
      return;
      }

    return;


    /*
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
      this->InvokeEvent(vtkSlicerColorEditWidget::ColorIDModifiedEvent);
      // set up the GUI
      this->UpdateWidget();
      }
    else
      {
      vtkErrorMacro ("ERROR: unable to get the mrml color node to observe!\n");
      }
    */
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::ProcessWidgetEvents(vtkObject *caller,
                                                   unsigned long event,
                                                   void *vtkNotUsed(callData))
{
  // process id changed events
  if (vtkSlicerColorEditWidget::SafeDownCast(caller) != NULL &&
      event == vtkSlicerColorEditWidget::ColorIDModifiedEvent)
    {
    vtkDebugMacro("vtkSlicerColorEditWidget::ProcessGUIEvents : got a colour id modified event.\n");
    //this->UpdateWidget();
    return;
    }

  vtkKWEntry *entry = vtkKWEntry::SafeDownCast(caller);

  // did the number of colors change?
  if (entry != NULL &&
      entry == this->NameEntry->GetWidget() &&
      event == vtkKWEntry::EntryValueChangedEvent)
    {
    vtkDebugMacro("ProcesWidgetEvents: name changed to " << this->NameEntry->GetWidget()->GetValue());
    if (this->GetColorNode() != NULL)
      {
      this->GetColorNode()->SetName(this->NameEntry->GetWidget()->GetValue());
      }
    }
  // update the name of the node if there is one already
  if (entry != NULL &&
      entry == this->NumberOfColorsEntry->GetWidget() &&
      event == vtkKWEntry::EntryValueChangedEvent)
    {
    vtkDebugMacro("ProcessWidgetEvents: number of colors changed to " << this->NumberOfColorsEntry->GetWidget()->GetValue());
    bool deleteFlag = true;
    int numColours = this->NumberOfColorsEntry->GetWidget()->GetValueAsInt();
    if (numColours > this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // add rows to the table
      int numToAdd = numColours - this->MultiColumnList->GetWidget()->GetNumberOfRows();
      this->MultiColumnList->GetWidget()->AddRows(numToAdd);
      // set the colour column edit 
      for (int c = 0; c < this->MultiColumnList->GetWidget()->GetNumberOfRows(); c++)
        {
        this->MultiColumnList->GetWidget()->SetCellWindowCommandToColorButton(c, this->ColourColumn);
        }
      }
    if (numColours < this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // delete some rows
      for (int r = this->MultiColumnList->GetWidget()->GetNumberOfRows(); r >= numColours; r--)
        {
        this->MultiColumnList->GetWidget()->DeleteRow(r);
        }
      }
    if (numColours != this->MultiColumnList->GetWidget()->GetNumberOfRows())
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
    }

  // did the color node that we're watching get modified?
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(caller);
  if (colorNode == this->MRMLScene->GetNodeByID(this->GetColorNodeID()) &&
      event == vtkCommand::ModifiedEvent)
    {
    //this->UpdateWidget();
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
    this->InvokeEvent(vtkSlicerColorEditWidget::SelectedColorModifiedEvent);
    return;
    }

  // 
  // save state for undo?
  // this->MRMLScene->SaveStateForUndo();

  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button == this->GenerateButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkDebugMacro("vtkSlicerColorEditWidget: ProcessGUIEvent: Generate Button event: " << event << ".\n");
    // generate a new mrml color table node
    this->GenerateNewColorTableNode();    
    }
  if (button == this->SaveToFileButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkDebugMacro("vtkSlicerColorEditWidget: ProcessGUIEvent: SaveToFile Button event: " << event << ".\n");
    // save the current color table to a file
    this->SaveColorTableNode();    
    }
  if (button == this->CopyButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkDebugMacro("vtkSlicerColorEditWidget: ProcessGUIEvent: Copy Button event: " << event << ".\n");
    // generate a new mrml color table node and copy from the selected node
    this->CopyAndEditColorTableNode();    
    }
  this->UpdateMRML();
} 



//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::ProcessMRMLEvents(vtkObject *caller,
                                                 unsigned long event,
                                                 void *vtkNotUsed(callData))
{
  if ( !this->ColorNodeID )
    {
    return;
    }

  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(caller);
//  vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());

  if (colorNode == (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID()) && 
      colorNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("vtkSlicerColorEditWidget::ProcessMRMLEvents color node modified, returning\n");
    //this->UpdateWidget();
    return;
    }

  /*
  if (colorNode == vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()) && 
      event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("\tmodified event on the color selected node.\n");
    if (activeColorNode !=  vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()))
      {
      // select it  and update the gui
      vtkDebugMacro("vtkSlicerColorEditWidget::ProcessMRMLEvents: modified event on the color selected node, setting the color node\n");
      this->SetColorNode(vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected()));
      }
    return;        
    }
  */
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::AddMRMLObservers ( )
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
  
//  this->AddObserver(vtkSlicerColorEditWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand );  

}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::RemoveMRMLObservers ( )
{
  //this->RemoveObservers(vtkSlicerColorEditWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
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
void vtkSlicerColorEditWidget::UpdateWidget()
{
  
  if ( this->ColorNodeID )
    {
    vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ColorNodeID));
    
    if (colorNode == NULL)
      {
      return;
      }
    
    int numColours = 0;
    if (vtkMRMLColorTableNode::SafeDownCast(colorNode) != NULL)
      {
      numColours = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetNumberOfColors();      
      }
    
    // set the number of colours
    // todo: dont' trigger another update
    this->NumberOfColorsEntry->GetWidget()->SetValueAsInt(numColours);

    this->MultiColumnList->GetWidget()->DeleteAllRows();
    
    // a row for each colour
    double colour[3];
    const char *name;
    // keep track of where to add the current colour into the table
    int thisRow = 0;
    for (int row = 0; row < numColours; row++)
      {
      // get the colour
      if (colorNode->GetLookupTable() != NULL)
        {
        colorNode->GetLookupTable()->GetColor((double)row, colour);
        }
      if (colour == NULL)
        {
        vtkErrorMacro ("SetGUIFromNode: at " << row << "th colour, got a null pointer" << endl);
        }
      // get the colour label
      name = colorNode->GetColorName(row);
      
      // update this colour
      this->MultiColumnList->GetWidget()->AddRow();

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
        std::stringstream ss;
        ss << colour[0];
        ss << " ";
        ss << colour[1];
        ss << " ";
        ss << colour[2];
        std::string colourStr = ss.str();
        this->MultiColumnList->GetWidget()->SetCellText(thisRow, this->ColourColumn, colourStr.c_str());
        }
      // make sure it's editable
      this->MultiColumnList->GetWidget()->SetCellWindowCommandToColorButton(thisRow, this->ColourColumn);
      thisRow++;
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
void vtkSlicerColorEditWidget::UpdateSelectedColor()
{
  // don't need to do anything for now
  //int selectedColor = this->GetSelectedColorIndex();
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::RemoveWidgetObservers ( ) {
  //this->CopyNodeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NameEntry->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NumberOfColorsEntry->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SaveToFileButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GenerateButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->CopyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MultiColumnList->GetWidget()->RemoveObservers(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  this->RemoveObservers (vtkSlicerColorEditWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::CreateWidget ( )
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
  // EDIT FRAME            
  vtkKWFrame *editFrame = vtkKWFrame::New ( );
  editFrame->SetParent ( this->GetParent() );
  editFrame->Create ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 editFrame->GetWidgetName() );

  // name of the new color table
  this->NameEntry = vtkKWEntryWithLabel::New();
  this->NameEntry->SetParent( editFrame );
  this->NameEntry->Create();
  this->NameEntry->GetLabel()->SetText("Name:");
  this->NameEntry->GetWidget()->SetBalloonHelpString ( "Set the name for the new color node");
  this->NameEntry->SetLabelWidth(5);
  this->NameEntry->GetWidget()->SetWidth(25);
  this->NameEntry->SetLabelPositionToLeft();
  this->Script("pack %s -side top -anchor w -padx 2 -pady 2 -in %s",
               this->NameEntry->GetWidgetName(), 
               editFrame->GetWidgetName());
  
  // how many colours in the lookup table?
  this->NumberOfColorsEntry = vtkKWEntryWithLabel::New();
  this->NumberOfColorsEntry->SetParent( editFrame );
  this->NumberOfColorsEntry->Create();
  this->NumberOfColorsEntry->GetLabel()->SetText("Number of Colors:");
  this->NumberOfColorsEntry->GetWidget()->SetBalloonHelpString ( "Set the number of colors in the new color node (0 to clear out the table).");
  this->NumberOfColorsEntry->SetLabelWidth(17);
  this->NumberOfColorsEntry->GetWidget()->SetWidth(4);
  this->NumberOfColorsEntry->SetLabelPositionToLeft();
  this->NumberOfColorsEntry->GetWidget()->SetRestrictValueToInteger();
  this->NumberOfColorsEntry->GetWidget()->SetValidationCommand(this, "ValidateNumberOfColors");
  this->Script("pack %s -side top -anchor w -padx 2 -pady 2 -in %s",
               this->NumberOfColorsEntry->GetWidgetName(), 
               editFrame->GetWidgetName());

  // Display the colours in the lookup table
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( editFrame );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
  this->MultiColumnList->GetWidget()->MovableRowsOff();
  this->MultiColumnList->GetWidget()->MovableColumnsOff();
  // set up the columns of data for each table entry
  // refer to the header file for the order
  this->MultiColumnList->GetWidget()->AddColumn("Label");
  this->MultiColumnList->GetWidget()->AddColumn("Name");
  this->MultiColumnList->GetWidget()->AddColumn("RGB");
  //this->MultiColumnList->GetWidget()->AddColumn("Color");

  // now set attribs that are equal across the columns
  int col;
  for (col = 0; col < this->NumberOfColumns; col++)
    {
    this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
    this->MultiColumnList->GetWidget()->ColumnEditableOn(col);
    }

  // set the column widths
  this->MultiColumnList->GetWidget()->SetColumnWidth(this->EntryColumn, 5);
  this->MultiColumnList->GetWidget()->SetColumnWidth(this->NameColumn, 17);
//  this->MultiColumnList->GetWidget()->SetColumnWidth(this->ColourTextColumn, 20);
  this->MultiColumnList->GetWidget()->SetColumnWidth(this->ColourColumn, 5);

  // make the colour column editable by colour chooser
  for (int row = 0; row < this->MultiColumnList->GetWidget()->GetNumberOfRows(); row++)
    {    
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToColorButton(row, this->ColourColumn);
    }
  // don't show the colour text
  this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput (this->ColourColumn);
  
  
  app->Script ( "pack %s -fill both -expand true -in %s",
                this->MultiColumnList->GetWidgetName(),
                editFrame->GetWidgetName());
  this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");

  // ---
  // BUTTON FRAME
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent ( editFrame );
  buttonFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               buttonFrame->GetWidgetName(),
               editFrame->GetWidgetName());

 // a button to generate a new colour table from the list box contents
  this->GenerateButton = vtkKWPushButton::New();
  this->GenerateButton->SetParent( buttonFrame);
  this->GenerateButton->Create();
  this->GenerateButton->SetText("Generate a New Color Node");
  this->GenerateButton->SetBalloonHelpString("Generate a new color table node from the contents of the editing table. Only do this once, then you can edit it. If you press this a second time, it will create a new node again. Save it through the File Save interface. Use the View, Application Settings, Module Settings, user defined color file paths to set a directory from which to load the file automatically on start up.");
  
  // a button to save a colour table to file
  this->SaveToFileButton = vtkKWPushButton::New();
  this->SaveToFileButton->SetParent( buttonFrame);
  this->SaveToFileButton->Create();
  this->SaveToFileButton->SetText("Save Node to File");
  this->SaveToFileButton->SetBalloonHelpString("Save the current color table node to file. Use the color table name as the file name. If you set the View, Application Settings, Module Settings, user defined color file paths, the table will be saved in one of them and will be loaded again on start up.");
  
  // pack the buttons
  app->Script("pack %s -side top -anchor w -padx 4 -pady 2 -in %s", 
              this->GenerateButton->GetWidgetName(),
              buttonFrame->GetWidgetName());

  // ---
  // COPY FRAME   
  vtkKWFrame *copyFrame = vtkKWFrame::New ( );
  copyFrame->SetParent ( this->GetParent() );
  copyFrame->Create ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 copyFrame->GetWidgetName() );

  // select a node to copy
  this->CopyNodeSelectorWidget = vtkSlicerNodeSelectorWidget::New();
  this->CopyNodeSelectorWidget->SetParent(copyFrame);
  this->CopyNodeSelectorWidget->Create();
  this->CopyNodeSelectorWidget->SetNodeClass("vtkMRMLColorTableNode", NULL, NULL, NULL);
  //this->CopyNodeSelectorWidget->NewNodeEnabledOn();
  this->CopyNodeSelectorWidget->ShowHiddenOn();
  this->CopyNodeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->CopyNodeSelectorWidget->SetBorderWidth(2);
  this->CopyNodeSelectorWidget->SetPadX(2);
  this->CopyNodeSelectorWidget->SetPadY(2);
  //this->CopyNodeSelectorWidget->GetWidget()->IndicatorVisibilityOff();
  this->CopyNodeSelectorWidget->GetWidget()->SetWidth(24);
  this->CopyNodeSelectorWidget->SetLabelText( "Select a Color Table to Copy: ");
  this->CopyNodeSelectorWidget->SetBalloonHelpString("Select a color from the current mrml scene, then can make a copy of it for editing.");

  // a button to trigger the copy
  this->CopyButton = vtkKWPushButton::New();
  this->CopyButton->SetParent( copyFrame);
  this->CopyButton->Create();
  this->CopyButton->SetText("Copy Node");
  this->CopyButton->SetBalloonHelpString("Copy the colour table node selected in the drop down menu.");

  this->Script ("pack %s %s -side top -anchor w -fill x -padx 2 -pady 2 -in %s",
                this->CopyNodeSelectorWidget->GetWidgetName(), this->CopyButton->GetWidgetName(),
                copyFrame->GetWidgetName());
  
  // add observers
//  this->CopyNodeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NameEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NumberOfColorsEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SaveToFileButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GenerateButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->CopyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AddObserver(vtkSlicerColorEditWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MultiColumnList->GetWidget()->AddObserver(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  

  // clean up
  editFrame->Delete();
  copyFrame->Delete();
  buttonFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::UpdateElement(int row, int col, char * str)
{
  vtkDebugMacro("UpdateElement: row = " << row << ", col = " << col << ", str = " << str << "\n");
  
  // make sure that the row and column exists in the table
  if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->MultiColumnList->GetWidget()->GetNumberOfColumns()))
    {
    // this isn't necessary until use the colour text column
    /*
    // make sure that if edit the colour text column, the colour column is
    // updated and vice versa, first checking if they're out of sync
    if (col == this->ColourColumn &&
        strcmp(str, this->MultiColumnList->GetWidget()->GetCellText(row, this->ColourTextColumn)))
      {
      // update the colour text
      this->MultiColumnList->GetWidget()->SetCellText(row, this->ColourTextColumn, str);
      }
    else if (col == this->ColourTextColumn &&
             strcmp(str, this->MultiColumnList->GetWidget()->GetCellText(row, this->ColourColumn)))
      {
      // update the colour
      this->MultiColumnList->GetWidget()->SetCellText(row, this->ColourColumn, str);
      }
    */
    }
  else
    {
    vtkErrorMacro ("Invalid row " << row << " or column " << col <<  ", valid columns are 0-" << this->NumberOfColumns << "\n");
    }
}

//---------------------------------------------------------------------------
int vtkSlicerColorEditWidget::GetSelectedColorIndex()
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
void vtkSlicerColorEditWidget::SetSelectedColorIndex(int index)
{
  if (this->MultiColumnList == NULL)
    {
    return;
    }
  this->MultiColumnList->GetWidget()->SelectSingleRow(index);
  this->UpdateSelectedColor();
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::UpdateEnableState(void)
{
    this->PropagateEnableState(this->SaveToFileButton);
    this->PropagateEnableState(this->GenerateButton);
    this->PropagateEnableState(this->CopyButton);
    this->PropagateEnableState(this->CopyNodeSelectorWidget);
    this->PropagateEnableState(this->NumberOfColorsEntry);
    this->PropagateEnableState(this->NameEntry);
    this->PropagateEnableState(this->MultiColumnList);
}

//---------------------------------------------------------------------------
int vtkSlicerColorEditWidget::ValidateNumberOfColors(const char *str)
{
  if (str == NULL)
    {
//    return 0;
    // allow an empty string for ease of editing
    return 1;
    }

  // allow an empty string for ease of editing
  if (strcmp(str, "") == 0)
    {
    return 1;
    }
  
  int num = atoi(str);
  int zeroNum = atoi((std::string("0") + std::string(str)).c_str());
  if (num >= 0 || zeroNum >= 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::GenerateNewColorTableNode()
{
  // need to have a name and number of entries to start
  int numColours = this->NumberOfColorsEntry->GetWidget()->GetValueAsInt();
  const char *tableName = this->NameEntry->GetWidget()->GetValue();
  vtkDebugMacro("GenerateNewColorTableNode: numColours = " << numColours << ", tableName = " << tableName);
  
  if (numColours <= 0 ||
      tableName == NULL)
    {
    vtkErrorMacro("You need to set a name for the table and a number of colors");
    return;
    }
  
  // create a new file node with the correct number of colour table entries
  vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::New();
  node->SetName(tableName);
  node->SetTypeToUser();
  node->SetAttribute("Category", "User Generated");
  //  node->SetFileName(filename.c_str());
  
  // get the max label value from the table
  int maxEntry = 0;
  int minEntry = 0;
  for (int rangeRow = 0; rangeRow < this->MultiColumnList->GetWidget()->GetNumberOfRows(); rangeRow++)
    {
    int label =  this->MultiColumnList->GetWidget()->GetCellTextAsInt(rangeRow, this->EntryColumn);
    if (label > maxEntry)
      {
      maxEntry = label;
      }
    if (rangeRow == 0 || label < minEntry)
      {
      minEntry = label;
      }
    }
  // make the colour table big enough to hold the max entry, plus one for zero
  int numberOfTableColours = maxEntry + 1;
  node->SetNumberOfColors(numberOfTableColours);
  node->GetLookupTable()->SetRange(minEntry, maxEntry);

  // init to black and no name
  for (int i = 0; i < numberOfTableColours; i++)
    {
    if (node->SetColor(i, "(none)", 0.0, 0.0, 0.0) == 0)
      {
      vtkWarningMacro("GenerateNewColorTableNode: unable to set color " << i << " when clearing out table, breaking loop");
      break;
      }
    }

  // populate the node
  bool errorCondition = false;
  for (int row = 0; row < this->MultiColumnList->GetWidget()->GetNumberOfRows(); row++)
    {
    int label =  this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, this->EntryColumn);
    std::string colourName =  this->MultiColumnList->GetWidget()->GetCellText(row, this->NameColumn);
    std::string colourStr = this->MultiColumnList->GetWidget()->GetCellText(row, this->ColourColumn);
    double r = 0.0, g = 0.0, b = 0.0;
    std::stringstream ss;
    ss << colourStr;
    ss >> r;
    ss >> g;
    ss >> b;
    vtkDebugMacro("row " << row << " label = " << label << ", colourName = " << colourName.c_str() << ", colour = " << colourStr << ", r = " << r << ", g = " << g << ", b = " << b);
    if (node->SetColor(label, colourName.c_str(), r, g, b) == 0)
      {
      vtkErrorMacro("Unable to set color " << label << " to name " << colourName.c_str() << ", r= " << r << ", g = " <<  g << ", b = " << b << ", breaking loop over the rows here (current row = " << row << ", total rows = " << this->MultiColumnList->GetWidget()->GetNumberOfRows() << ")");
      errorCondition = true;
      break;
      }
    
    }
  std::string msg;
  if (!errorCondition)
    {
    // let the node know that the names are intialised
    node->NamesInitialisedOn();

    // don't hide this node, so can save it in the save data widget
    node->HideFromEditorsOff();
    // set it as modified since read, since we need to save it still
    node->ModifiedSinceReadOn();
    
    // add it to the scene
    this->MRMLScene->AddNode(node);

    // make a storage node for it
    vtkSmartPointer<vtkMRMLColorTableStorageNode> colorStorageNode =  vtkSmartPointer<vtkMRMLColorTableStorageNode>::New();
    colorStorageNode->SaveWithSceneOn();
    // set up the path
    // is there a user defined colour file path?
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    if (app && app->GetColorFilePaths())
      {
      std::string appPaths = app->GetColorFilePaths();
      if (appPaths.compare("") != 0)
        {
        // take the first one
#ifdef WIN32
        const char *delim = ";";
#else
        const char *delim = ":";
#endif
        std::string::size_type pos = appPaths.find_first_of(delim);
        std::string pathString = std::string("");
        // there may not be a delimiter, in that case just go to the end of the string
        pathString = appPaths.substr(0, pos);
        vtkDebugMacro("Got first color path " << pathString);
        vtksys_stl::string dir = vtksys_stl::string(pathString);
        vtksys_stl::vector<vtksys_stl::string> filesVector;
        vtksys::SystemTools::SplitPath(dir.c_str(), filesVector);
        filesVector.push_back(vtksys_stl::string(tableName) + vtksys_stl::string(".ctbl"));
        std::string fname = vtksys::SystemTools::JoinPath(filesVector);
        vtkWarningMacro("Setting default file name to " << fname);
        colorStorageNode->SetFileName(fname.c_str());
        }
      }
    this->MRMLScene->AddNode(colorStorageNode);
    node->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    
    // set it to be active
    this->SetColorNodeID(node->GetID());

    msg = std::string("Created new colors node, ") + std::string(node->GetName()) + std::string(", category = ") + std::string(node->GetAttribute("Category"));
    }
  else
    {
    msg = std::string("Failed to create a new colors node, ") + std::string(node->GetName()) + std::string(", category = ") + std::string(node->GetAttribute("Category"));
    }
  
  // pop up some feedback
  vtkKWMessageDialog *message = vtkKWMessageDialog::New();
  message->SetParent ( this->GetParent() );
  message->SetStyleToMessage();
  
  message->SetText(msg.c_str());
  message->Create();
  message->Invoke();
  message->Delete();

  node->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::SaveColorTableNode()
{
  // get the color node
  const char *nodeID = this->GetColorNodeID();
  if (nodeID == NULL)
    {
      vtkErrorMacro("SaveColorTableNode: no node id is set...");
      return;
    }
  vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::SafeDownCast(this->MRMLScene->GetNodeByID(nodeID));
  if (node == NULL)
    {
      vtkErrorMacro("SaveColorTableNode: no node with id " << nodeID << " found in scene, cannot save.");
      return;
    }

  const char *tableName = this->NameEntry->GetWidget()->GetValue();

  // set up a storage node with file name
  std::string pathPrefix = std::string("./");
  std::string colourPaths = this->Script("$::slicer3::Application GetColorFilePaths");
  vtkDebugMacro("Colour paths = " << colourPaths.c_str());
  if (colourPaths != std::string(""))
    {
#ifdef WIN32
      const char *delim = ";";
#else
    const char *delim = ":";
#endif
    // get the first string
    char *colourPathsChar = const_cast<char *>(colourPaths.c_str());
    char *pathPtr = strtok(colourPathsChar, delim);
    if (pathPtr != NULL)
      {
      pathPrefix = std::string(pathPtr) + std::string("/");
      vtkDebugMacro("Found user defined colour dir: " << pathPrefix.c_str());
      }
    }

  vtkMRMLColorTableStorageNode *snode;
  snode = vtkMRMLColorTableStorageNode::SafeDownCast(node->GetStorageNode());
  if (snode == NULL)
    {
    snode = vtkMRMLColorTableStorageNode::SafeDownCast(node->CreateDefaultStorageNode()); //vtkMRMLColorTableStorageNode::New();
    if (snode == NULL)
      {
      vtkErrorMacro("SaveColorTableNode: unable to get a storage node set up, failed to save table " << tableName);
      return;
      }
    this->MRMLScene->AddNode(snode);
    node->SetAndObserveStorageNodeID(snode->GetID());
    snode->Delete();
    }

  // over ride the path with the table name
  std::string filename = pathPrefix + std::string(tableName) + std::string(".txt");
  snode->SetFileName(filename.c_str());
  node->SetFileName(filename.c_str());
  vtkDebugMacro("SaveTableNode: setting colour storage node file name to " << filename.c_str());

  // save the new colour table
  int retval = snode->WriteData(node);
  if (!retval)
    {
    vtkErrorMacro("Error saving colour table to file " << filename.c_str());
    }
  else
    {
    vtkKWMessageDialog *message = vtkKWMessageDialog::New();
    message->SetParent ( this->GetParent() );
    message->SetStyleToMessage();
    std::string msg = "Saved colours to file " + filename;
    message->SetText(msg.c_str());
    message->Create();
    message->Invoke();
    message->Delete();
    }
  // set it to be active
  //this->SetColorNodeID(node->GetID());
  
  // clean up
  //snode->Delete();
  node->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerColorEditWidget::CopyAndEditColorTableNode()
{
  // is there a node selected?
  vtkMRMLColorTableNode *node = 
        vtkMRMLColorTableNode::SafeDownCast(this->CopyNodeSelectorWidget->GetSelected());

  if (node == NULL)
    {
    vtkErrorMacro("CopyAndEditColorTableNode: no color table node selected, pick one first!");
    return;
    }
  
  // get the name and add Copy 
  std::string newName = std::string("Copy") + std::string(node->GetName());
  this->NameEntry->GetWidget()->SetValue(newName.c_str());


  // get the number of colours
  int numColours = node->GetNumberOfColors();
  double *range = NULL;
  range = node->GetLookupTable()->GetRange();
  if (range)
    {
    numColours = (int)floor(range[1] - range[0]);
    if (range[0] <= 0)
      {
      // add one for 0
      numColours++;
      }
    }
  // set the number of colours
  this->NumberOfColorsEntry->GetWidget()->SetValueAsInt(numColours);

  // make sure that the number of rows are correct
  if (this->MultiColumnList->GetWidget()->GetNumberOfRows() != numColours)
    {
    // clear out the multicolumn list
    this->MultiColumnList->GetWidget()->DeleteAllRows();
    // add enough rows
    this->MultiColumnList->GetWidget()->AddRows(numColours);
    }

  // then populate the multicolum list
  int thisRow = 0;
  for (int i = (int)floor(range[0]); i <= (int)floor(range[1]); i++)
    {
    // get the colour label
    const char *name = NULL;
    name = node->GetColorName(i);
    if (name == NULL)
      {
      name = "(none)";
      }
    this->MultiColumnList->GetWidget()->SetCellTextAsInt(thisRow, this->EntryColumn, i);
    this->MultiColumnList->GetWidget()->SetCellText(thisRow,this->NameColumn,name);
    double colour[3];
    node->GetLookupTable()->GetColor((double)i, colour);
    // set the text, as that's how we populate the new node
    std::stringstream ss;
    ss << colour[0];
    ss << " ";
    ss << colour[1];
    ss << " ";
    ss << colour[2];
    std::string colourStr = ss.str();
//    this->MultiColumnList->GetWidget()->SetCellText(thisRow, this->ColourTextColumn, colourStr.c_str());
    this->MultiColumnList->GetWidget()->SetCellText(thisRow, this->ColourColumn, colourStr.c_str());
    // make the colour column editable by colour chooser
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToColorButton(thisRow, this->ColourColumn);
    thisRow++;
    }

}
