#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerColorDisplayWidget.h"

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

#include "vtkMRMLColorNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkColorTransferFunction.h"

#include "vtkScalarBarActor.h"
#include "vtkScalarBarWidget.h"
#include "vtkKWScalarBarAnnotation.h"
#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerPopUpHelpWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerColorDisplayWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerColorDisplayWidget::vtkSlicerColorDisplayWidget ( )
{

    this->ColorNodeID = NULL;
    this->ColorNode = NULL; 

    this->ColorSelectorWidget = NULL;
    this->NodeHelpWidget = NULL;

    this->ColorNodeTypeLabel = NULL;

    this->MultiColumnList = NULL;
    this->NumberOfColumns = 3;

    this->NumberOfColorsLabel = NULL;

    this->ShowOnlyNamedColorsCheckButton = NULL;

    this->MinRangeEntry = NULL;
    this->MaxRangeEntry = NULL;

    this->ScalarBarAnnotation = NULL;
    this->ScalarBarWidget = NULL;

    this->SelectedColorLabel = NULL;

    this->MultiSelectModeOff();

    this->ViewerWidget = NULL;
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
  if (this->NodeHelpWidget)
    {
    this->NodeHelpWidget->SetParent(NULL);
    this->NodeHelpWidget->Delete();
    this->NodeHelpWidget = NULL;
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

  if (this->ShowOnlyNamedColorsCheckButton)
    {
    this->ShowOnlyNamedColorsCheckButton->SetParent(NULL);
    this->ShowOnlyNamedColorsCheckButton->Delete();
    this->ShowOnlyNamedColorsCheckButton = NULL;
    }

  if (this->MinRangeEntry)
    {
    this->MinRangeEntry->SetParent(NULL);
    this->MinRangeEntry->Delete();
    this->MinRangeEntry = NULL;
    }

  if (this->MaxRangeEntry)
    {
    this->MaxRangeEntry->SetParent(NULL);
    this->MaxRangeEntry->Delete();
    this->MaxRangeEntry = NULL;
    }

  if (this->ScalarBarAnnotation)
    {
    this->ScalarBarAnnotation->SetParent(NULL);
    this->ScalarBarAnnotation->Delete();
    this->ScalarBarAnnotation = NULL;
    }
  if (this->SelectedColorLabel)
    {
    this->SelectedColorLabel->SetParent(NULL);
    this->SelectedColorLabel->Delete();
    this->SelectedColorLabel = NULL;
    }

  if (this->ScalarBarWidget)
    {
    this->ScalarBarWidget->SetInteractor(NULL);
    this->ScalarBarWidget->Delete();
    this->ScalarBarWidget = NULL;
    }

  this->SetViewerWidget(NULL);
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
void vtkSlicerColorDisplayWidget::ProcessWidgetEvents (vtkObject *caller,
                                                       unsigned long event,
                                                       void *vtkNotUsed(callData))
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

  if ( (this->MinRangeEntry->GetWidget() == vtkKWEntry::SafeDownCast(caller) ||
        this->MaxRangeEntry->GetWidget() == vtkKWEntry::SafeDownCast(caller) ) &&
       event == vtkKWEntry::EntryValueChangedEvent)
    {
    this->UpdateMRML();
    }
} 



//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::ProcessMRMLEvents(vtkObject *caller,
                                                    unsigned long event,
                                                    void *vtkNotUsed(callData))
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
    // special case if it's a freesurfer procedural node
    bool isFSProcedural = false;
    // if the colour node is defined procedurally, set this flag to true
    bool isProcedural = false;
    
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
    double *range = NULL;
    if (vtkMRMLColorTableNode::SafeDownCast(colorNode) != NULL)
      {
      numColours = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetNumberOfColors();
      range =  vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetLookupTable()->GetRange();
      }
    else if (vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode) != NULL &&
             vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable() != NULL)
      {
      //numColours = vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable()->GetNumberOfColors();
      range = vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable()->GetRange();
      if (range)
        {
        numColours = (int)floor(range[1] - range[0]);
        if (range[0] < 0 && range[1] >= 0)
          {
          // add one for zero
          numColours++;
          }
        }
      isFSProcedural = true;
      }
    else if ( vtkMRMLProceduralColorNode::SafeDownCast(colorNode) != NULL &&
              vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction() != NULL)
      {
      range = vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction()->GetRange();
      if (range)
        {
        numColours = (int)floor(range[1] - range[0]);
        if (range[0] < 0 && range[1] >= 0)
          {
          // add one for zero
          numColours++;
          }
        }
      isProcedural = true;
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
    // check if no showing only named colours before deleting rows one by one
    // if there are too many, as the next test will delete everything
    if (!showOnlyNamedColors &&
        numColours < this->MultiColumnList->GetWidget()->GetNumberOfRows())
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
    

    double colour[3];
    const char *name = NULL;
    // keep track of where to add the current colour into the table
    int thisRow = 0;
    double index = 0.0;
    if ((isFSProcedural || isProcedural) && (range != NULL))
      {
      index = range[0];
      }
    

    // for each colour
    for (int row = 0; row < numColours; row++)
      {
      // get the colour
      if (isFSProcedural)
        {
        if (index <= range[1])
          {
          vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable()->GetColor(index, colour);
          index += 1.0;
          }
        }      
      else if (isProcedural)
        {
        if (index <= range[1])
          {
          vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction()->GetColor(index, colour);
          index += 1.0;
          }
        }
      else if (colorNode->GetLookupTable() != NULL)
        {
        //colorNode->GetLookupTable()->GetColor((double)row, colour);
        // GetTableValue returns the alpha as well, use GetColor to just get rgb (except it returns the wrong black value on the first call if a user edited table starts from > 0)
        double tableValue[4];
        vtkIdType r = (vtkIdType)row;
        colorNode->GetLookupTable()->GetTableValue(r, tableValue);
        colour[0] = tableValue[0];
        colour[1] = tableValue[1];
        colour[2] = tableValue[2];
        }
//      if (colour == NULL)
//        {
//        vtkErrorMacro ("SetGUIFromNode: at " << row << "th colour, got a null pointer" << endl);
//        }
      // get the colour label
      name = colorNode->GetColorName(row);
      if (name == NULL)
        {
        name = "(none)";
        }
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
          // also set it to be the selected cell background color so that the
          // color box is not lost when the row is selected
          this->MultiColumnList->GetWidget()->SetCellSelectionBackgroundColor(thisRow, this->ColourColumn, colour);
          }
        else
          {
          this->MultiColumnList->GetWidget()->ClearCellBackgroundColor(thisRow, this->ColourColumn);
          }
        thisRow++;
        }
      }

    // update the range
    if (range)
      {
      this->MinRangeEntry->GetWidget()->SetValueAsDouble(range[0]);
      this->MaxRangeEntry->GetWidget()->SetValueAsDouble(range[1]);
      // this can't be edited if it's a procedural node with just a colour
      // transfer function rather than a look up table
      if (isProcedural || isFSProcedural)
        {
        this->MinRangeEntry->GetWidget()->ReadOnlyOn();
        this->MaxRangeEntry->GetWidget()->ReadOnlyOn();
        }
      else
        {
        this->MinRangeEntry->GetWidget()->ReadOnlyOff();
        this->MaxRangeEntry->GetWidget()->ReadOnlyOff();
        }
      }
    
    // update the scalar bar
    if (this->ScalarBarWidget)
      {
      if (colorNode->GetLookupTable())
        {
        this->ScalarBarWidget->GetScalarBarActor()->SetLookupTable(colorNode->GetLookupTable());
        }
      else
        {
        //try getting a transfer function
        vtkMRMLProceduralColorNode *procColorNode = vtkMRMLProceduralColorNode::SafeDownCast(colorNode);
        if (procColorNode && procColorNode->GetColorTransferFunction())
          {
          this->ScalarBarWidget->GetScalarBarActor()->SetLookupTable((vtkScalarsToColors*)(procColorNode->GetColorTransferFunction()));
          }
        else
          {
          vtkWarningMacro("This color node " << colorNode->GetName() << " doesn't have a look up table or a color transfer function.");
//          this->ScalarBarWidget->GetScalarBarActor()->SetLookupTable(NULL);
          }
        }
      } else { vtkWarningMacro("Updatewidget: no scalar bar widget"); }

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

    if(selectedColor==-2)
      {
      ss<<"Multiple colors selected";
      }
    else if (selectedColor != -1)
      {
      ss << selectedColor;
      }
    else
      {
      ss<<"No color selected";
      }
    this->SelectedColorLabel->SetText(ss.str().c_str());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::UpdateMRML()
{
  // get the currently displayed list 
  vtkMRMLColorNode *activeColorNode = (vtkMRMLColorNode *)this->MRMLScene->GetNodeByID(this->GetColorNodeID());

  if (activeColorNode == NULL)
    {
    vtkErrorMacro("ERROR: No Color!\n");
    return;
    }
  // allow setting the range on the node's colour lut
  double min = this->MinRangeEntry->GetWidget()->GetValueAsDouble();
  double max = this->MaxRangeEntry->GetWidget()->GetValueAsDouble();
  bool requestRender = false;

  vtkMRMLProceduralColorNode *procColorNode = vtkMRMLProceduralColorNode::SafeDownCast(activeColorNode);
  
  if (activeColorNode->GetLookupTable())
    {
    activeColorNode->GetLookupTable()->SetRange(min, max);
    // reset the scalar bar actor
    // BUG: the mapper is resetting the range, and it ignores the new range
    // set on the table
    // this->ScalarBarWidget->GetScalarBarActor()->SetTitle("mm");
//    this->ScalarBarWidget->GetScalarBarActor()->SetLookupTable(activeColorNode->GetLookupTable());
    requestRender = true;
    }
  // SetRange is not implemented on ColorTransferFunctions, so just set it on
  // the node
  else if (procColorNode != NULL &&
           procColorNode->GetLookupTable() != NULL)
           // && vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(activeColorNode)->GetColorTransferFunction())
    {
    procColorNode->GetLookupTable()->SetRange(min, max);
    requestRender = true;
    }
  if (requestRender && this->ViewerWidget)
    {
    this->ViewerWidget->RequestRender();
    }
  
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkSlicerColorDisplayWidget::RemoveWidgetObservers ( ) {
  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowOnlyNamedColorsCheckButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,
                                                        (vtkCommand *)this->GUICallbackCommand);
  this->MultiColumnList->GetWidget()->RemoveObservers(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  this->RemoveObservers (vtkSlicerColorDisplayWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MinRangeEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);
  this->MaxRangeEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);

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

  // make a little frame for the node selector and help widget
  vtkKWFrame *selFrame = vtkKWFrame::New();
  selFrame->SetParent(displayFrame);
  selFrame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 selFrame->GetWidgetName() );
  
  // node selector
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New();
  this->ColorSelectorWidget->SetParent(selFrame);
  this->ColorSelectorWidget->Create();
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  this->ColorSelectorWidget->AddExcludedChildClass("vtkMRMLDiffusionTensorDisplayPropertiesNode");
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
  
  // info icon to give more info about the colour nodes
  this->NodeHelpWidget = vtkSlicerPopUpHelpWidget::New();
  this->NodeHelpWidget->SetParent(selFrame);
  this->NodeHelpWidget->Create();
  this->NodeHelpWidget->SetWidth(3);
  this->NodeHelpWidget->SetHelpTitle("Color Node Descriptions");
  this->NodeHelpWidget->SetBalloonHelpString("Bring up help window");
  
  // build up a list of descriptions of the nodes
  std::string nodeNamesDescriptions = std::string("Node names and descriptions:\n");
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
  this->NodeHelpWidget->SetHelpText(nodeNamesDescriptions.c_str());

  this->Script ("pack %s -side left -anchor w -padx 2 -pady 2 -in %s",
                this->NodeHelpWidget->GetWidgetName(),
                selFrame->GetWidgetName());
  this->Script ("pack %s -side left -anchor w -fill x -expand true -padx 2 -pady 2 -in %s",
                this->ColorSelectorWidget->GetWidgetName(),
                selFrame->GetWidgetName());  
  
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
  this->MultiColumnList->GetWidget()->SetSelectionTypeToRow();
  this->MultiColumnList->GetWidget()->MovableRowsOff();
  this->MultiColumnList->GetWidget()->MovableColumnsOff();
  // set up the columns of data for each table entry
  // refer to the header file for the order
  this->MultiColumnList->GetWidget()->AddColumn("Entry");
  this->MultiColumnList->GetWidget()->AddColumn("Name");
  this->MultiColumnList->GetWidget()->AddColumn("Color");

  
  if(this->MultiSelectMode)
  {
      this->MultiColumnList->GetWidget()->SetSelectionModeToMultiple();
  }

  
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
  
  // a checkbutton to only show the valid colours (useful for the SPL atlas
  // table which skips ranges)
  this->ShowOnlyNamedColorsCheckButton = vtkKWCheckButton::New();
  this->ShowOnlyNamedColorsCheckButton->SetParent ( buttonFrame );
  this->ShowOnlyNamedColorsCheckButton->Create();
  this->ShowOnlyNamedColorsCheckButton->SelectedStateOn();
  this->ShowOnlyNamedColorsCheckButton->SetText("Show Only Named Colors");

  // pack the checkbutton
  app->Script("pack %s -side top -anchor w -padx 4 -pady 2 -in %s",
              this->ShowOnlyNamedColorsCheckButton->GetWidgetName(),
              buttonFrame->GetWidgetName());

  // range frame
  vtkKWFrame *rangeFrame = vtkKWFrame::New();
  rangeFrame->SetParent ( displayFrame );
  rangeFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               rangeFrame->GetWidgetName(),
               displayFrame->GetWidgetName());

  // entries to show/change the range
  this->MinRangeEntry = vtkKWEntryWithLabel::New();
  this->MinRangeEntry->SetParent( rangeFrame );
  this->MinRangeEntry->Create();
  this->MinRangeEntry->SetLabelText("Lookup Table Range Min");
  this->MinRangeEntry->SetBalloonHelpString ("Only changes display range, not the values in the table. Not valid on Procedural nodes (FreeSurfer etc.) that only define a color transfer function and not a look up table.");
  this->MinRangeEntry->GetWidget()->SetWidth ( 6 );
  //this->MinRangeEntry->GetWidget()->SetValueAsDouble(-200);
  //this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
  //             this->MinRangeEntry->GetWidgetName());

  this->MaxRangeEntry = vtkKWEntryWithLabel::New();
  this->MaxRangeEntry->SetParent( rangeFrame );
  this->MaxRangeEntry->Create();
  this->MaxRangeEntry->SetLabelText("Max");
  this->MaxRangeEntry->SetBalloonHelpString ("Only changes display range, not the values in the table. Not valid on Procedural nodes (FreeSurfer etc.) that only define a color transfer function and not a look up table.");
  this->MaxRangeEntry->GetWidget()->SetWidth ( 6 );
  //this->MaxRangeEntry->GetWidget()->SetValueAsDouble(200);
  this->Script("pack %s %s -side left -anchor e -padx 4 -pady 2", 
               this->MinRangeEntry->GetWidgetName(), this->MaxRangeEntry->GetWidgetName());

  // scalar bar frame
  vtkKWFrame *scalarBarFrame = vtkKWFrame::New();
  scalarBarFrame->SetParent ( displayFrame );
  scalarBarFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               scalarBarFrame->GetWidgetName(),
               displayFrame->GetWidgetName());

  
  // scalar bar
  this->ScalarBarWidget = vtkScalarBarWidget::New();
  this->ScalarBarWidget->GetScalarBarActor()->SetOrientationToVertical();
  this->ScalarBarWidget->GetScalarBarActor()->SetNumberOfLabels(11);
  this->ScalarBarWidget->GetScalarBarActor()->SetTitle("(mm)");
  this->ScalarBarWidget->GetScalarBarActor()->SetLabelFormat(" %#8.3f");
  
  // it's a 2d actor, position it in screen space by percentages
  this->ScalarBarWidget->GetScalarBarActor()->SetPosition(0.1, 0.1);
  this->ScalarBarWidget->GetScalarBarActor()->SetWidth(0.1);
  this->ScalarBarWidget->GetScalarBarActor()->SetHeight(0.8);
  if (this->GetViewerWidget() &&
      this->GetViewerWidget()->GetMainViewer() &&
      this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()
      )
    {
    // set up the interactor
    this->ScalarBarWidget->SetInteractor(this->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor());
    }
  else
    {
    vtkWarningMacro("No ViewerWidget set, scalar bar widget interactor not set");
    this->ScalarBarWidget->SetInteractor(NULL);
    }
  this->ScalarBarAnnotation = vtkKWScalarBarAnnotation::New();
  this->ScalarBarAnnotation->SetParent ( scalarBarFrame );
  this->ScalarBarAnnotation->Create();
  this->ScalarBarAnnotation->SetBalloonHelpString("Control parameters on a 2d scalar bar showing the colors in the 3D view");
  this->ScalarBarAnnotation->SetScalarBarWidget(this->ScalarBarWidget);
  
  // pack the scalar bar annotation
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
              this->ScalarBarAnnotation->GetWidgetName(),
              scalarBarFrame->GetWidgetName());

  // keep the scalar bar annotation closed by default
  this->ScalarBarAnnotation->GetFrame()->CollapseFrame();
  
  // deleting frame widgets
  selFrame->Delete();
  buttonFrame->Delete();
  scalarBarFrame->Delete();
  rangeFrame->Delete();
  
  // add observers
  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->ShowOnlyNamedColorsCheckButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AddObserver(vtkSlicerColorDisplayWidget::ColorIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MultiColumnList->GetWidget()->AddObserver(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  this->MinRangeEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);
  this->MaxRangeEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand);

  // clean up
  displayFrame->Delete();
  
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

  //Here we need different behaviors for MultiSelection and Single Selection
  if(MultiSelectMode)
  {
    return -2;
 
  
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
    vtkDebugMacro("Error in selection: " << numRows << " selected, select just one and try again.");
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


void vtkSlicerColorDisplayWidget::UpdateEnableState(void)
{
    this->PropagateEnableState(this->ColorSelectorWidget);
    this->PropagateEnableState(this->ColorNodeTypeLabel);
    this->PropagateEnableState(this->NumberOfColorsLabel);
    this->PropagateEnableState(this->SelectedColorLabel);
    this->PropagateEnableState(this->MultiColumnList);
    this->PropagateEnableState(this->ShowOnlyNamedColorsCheckButton);
    this->PropagateEnableState(this->ScalarBarAnnotation);
    this->PropagateEnableState(this->MinRangeEntry);
    this->PropagateEnableState(this->MaxRangeEntry);
}

void vtkSlicerColorDisplayWidget::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  this->ViewerWidget = viewerWidget;
}
