#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"

#include "vtkFetchMIQueryTermWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerWindow.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWTopLevel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWCheckButton.h"
#include "vtkKWComboBox.h"
#include "vtkKWTkUtilities.h"
#include "vtkFetchMIIcons.h"
#include "vtkFetchMITagViewWidget.h"
#include "vtkFetchMIGUI.h"

#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIQueryTermWidget );
vtkCxxRevisionMacro ( vtkFetchMIQueryTermWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIQueryTermWidget::vtkFetchMIQueryTermWidget ( )
{
    this->SelectAllButton = NULL;
    this->RefreshButton = NULL;
    this->DeselectAllButton = NULL;
    this->HelpButton = NULL;
    this->ClearAllButton = NULL;
    this->ClearSelectedButton = NULL;
    this->SearchButton = NULL;
    this->FetchMIIcons = NULL;
    this->NumberOfColumns = 3;
    this->InPopulateWidget = 0;
//    this->DebugOn();
    this->Logic = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIQueryTermWidget::~vtkFetchMIQueryTermWidget ( )
{
  this->InPopulateWidget = 0;
  this->RemoveMRMLObservers();
  this->SetLogic ( NULL );

  if ( this->SearchButton )
    {
    this->SearchButton->SetParent ( NULL );
    this->SearchButton->Delete();
    this->SearchButton = NULL;
    }
  if ( this->HelpButton )
    {
    this->HelpButton->SetParent ( NULL );
    this->HelpButton->Delete();
    this->HelpButton = NULL;
    }
    if ( this->RefreshButton )
      {
      this->RefreshButton->SetParent ( NULL );
      this->RefreshButton->Delete();
      this->RefreshButton = NULL;
      }
  if ( this->SelectAllButton )
    {
    this->SelectAllButton->SetParent ( NULL );
    this->SelectAllButton->Delete();
    this->SelectAllButton = NULL;    
    }
  if ( this->DeselectAllButton )
    {
    this->DeselectAllButton->SetParent ( NULL );
    this->DeselectAllButton->Delete();
    this->DeselectAllButton = NULL;
    }
  if ( this->ClearAllButton )
    {
    this->ClearAllButton->SetParent ( NULL );
    this->ClearAllButton->Delete();
    this->ClearAllButton = NULL;    
    }
  if ( this->ClearSelectedButton )
    {
    this->ClearSelectedButton->SetParent ( NULL );
    this->ClearSelectedButton->Delete();
    this->ClearSelectedButton = NULL;    
    }
  if ( this->FetchMIIcons )
    {
    this->FetchMIIcons->Delete();
    this->FetchMIIcons = NULL;
    }
  this->SetMRMLScene ( NULL );
}



//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMIQueryTermWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ClearSelectedButton: " << this->GetClearSelectedButton() << "\n";
    os << indent << "ClearAllButton: " << this->GetClearAllButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";
}




//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::RaiseHelpWindow()
{
  int px, py;
  vtkFetchMITagViewWidget *win = vtkFetchMITagViewWidget::New();
  win->SetParent ( this->HelpButton );
  win->Create();
  vtkKWTkUtilities::GetWidgetCoordinates(this->HelpButton, &px, &py);
  win->GetTagViewWindow()->SetPosition ( px + 10, py + 10) ;
  win->SetTagTitle ("Tips on querying for data");
  std::stringstream ss;
  ss << "When a server is selected, this panel is automatically populated with all the metadata the server knows about. All attributes are listed in the **Attribute** column, and a pull-down list of defined values for that attribute are listed in the **Value** column. Select attribute value pairs to query for data tagged with them on the server, and choose the **Search** icon (binoculars) to query for matching datasets. Query results will be displayed in the ~~Browse Query Results & Download~~.panel below.";
  win->SetTagText ( ss.str().c_str() );
  win->DisplayTagViewWindow();
}



//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMultiColumnList *l = vtkKWMultiColumnList::SafeDownCast( caller );
  
  if ( this->IsCreated() )
    {
    if ( (l == this->GetMultiColumnList()->GetWidget()) && (event == vtkKWMultiColumnList::CellUpdatedEvent) )
      {
      if ( !this->GetInPopulateWidget() )
        {
        this->InvokeEvent (vtkFetchMIQueryTermWidget::TagChangedEvent );
        }
      }
    if ( (b == this->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllItems( );
      }
    else if ( (b == this->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedItems ();
      }
    else if ( (b == this->GetHelpButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->RaiseHelpWindow();
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllItems ( );
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllItems ( );
      }
    else if ( b == this->RefreshButton )
      {
      if ( this->Logic->GetFetchMINode() != NULL )
        {
        //--- check the node's selected server for tags.
        if ( (this->Logic->GetFetchMINode()->GetSelectedServer() != NULL) )
          {
            this->Logic->SetRestoreTagSelectionState(1);
            //--- this queries server for tags
            vtkDebugMacro ("--------------------GUI event refreshing Query.");
            this->SetStatusText ("Querying selected server for metadata..." );
            this->Logic->QueryServerForTags();
            this->SetStatusText ( "Querying selected server for metadata......." );
            this->Logic->QueryServerForTagValues( );
            this->SetStatusText ( "Querying selected server for metadata.......done." );
            // TODO: temporary fix for HID which we are
            // not yet querying for available tags. Just
            // repopulate from default tags in FetchMINode
            this->SetStatusText ( "");
          }
        }
      }
    else if ( (b == this->GetSearchButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent ( vtkFetchMIQueryTermWidget::QuerySubmittedEvent );
      this->SetStatusText ("Querying selected server for resources...");
      this->Logic->QueryServerForResources();
      this->SetStatusText ("Querying selected server for resources...done.");
      this->SetStatusText ("");
      }

    }
  this->UpdateMRML();
} 


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::SetStatusText (const char *txt)
{
  if ( this->GetApplication() )
    {
    if ( (vtkSlicerApplication::SafeDownCast(this->GetApplication()))->GetApplicationGUI() )
      {
      if ( (vtkSlicerApplication::SafeDownCast(this->GetApplication()))->GetApplicationGUI()->GetMainSlicerWindow() )
        {
        (vtkSlicerApplication::SafeDownCast(this->GetApplication()))->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::SelectRow( int i)
{
    this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 1 );
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
}



//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::SelectAllItems()
{
  int numrows, i;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
    this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 1 );
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::DeselectAllItems()
{
  int numrows, i;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
    this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 0 );
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // nothing; handle in parent.
}

//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::RemoveMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::UpdateWidget()
{
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::RemoveWidgetObservers ( ) {

  this->GetClearAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetHelpButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSearchButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetRefreshButton()->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetMultiColumnList()->GetWidget()->RemoveObservers(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::AddWidgetObservers ( ) {

  this->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetHelpButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSelectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSearchButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetRefreshButton()->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->GetMultiColumnList()->GetWidget()->AddObserver(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand );
}






//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
  
  // create the icons
  this->FetchMIIcons = vtkFetchMIIcons::New();

  // configure the multicolumn list
  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Use" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(0);
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToCheckButton ( 0);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Attribute              " );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 1, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry ( 1 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Value                   " );
  
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 2, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 2 );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 2 ); 
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(2);  

  this->Script ( "pack %s -side top -fill x -expand n", this->GetMultiColumnList()->GetWidgetName() );

  // frame for the buttons
  vtkKWFrame *bFrame = vtkKWFrame::New();
  bFrame->SetParent ( this->ContainerFrame );
  bFrame->Create();
  this->Script ("pack %s -side top -anchor c -expand y -fill x -padx 2 -pady 2", bFrame->GetWidgetName() );


  vtkKWLabel *spacer = vtkKWLabel::New();
  spacer->SetParent ( bFrame );
  spacer->Create();
  spacer->SetText ("             " );

  this->RefreshButton = vtkKWPushButton::New();
  this->RefreshButton->SetParent (bFrame);
  this->RefreshButton->Create();
  this->RefreshButton->SetBorderWidth ( 0 );
  this->RefreshButton->SetReliefToFlat();  
  this->RefreshButton->SetImageToIcon ( this->FetchMIIcons->GetRefreshServerIcon() );
  this->RefreshButton->SetBalloonHelpString ( "Re-query current server for new tags" );

  this->ClearSelectedButton = vtkKWPushButton::New();
  this->ClearSelectedButton->SetParent (bFrame);
  this->ClearSelectedButton->Create();
  this->ClearSelectedButton->SetBorderWidth ( 0 );
  this->ClearSelectedButton->SetReliefToFlat ( );  
  this->ClearSelectedButton->SetImageToIcon ( this->FetchMIIcons->GetDeleteSelectedIcon() );
  this->ClearSelectedButton->SetBalloonHelpString ( "Clear selected (non-essential) terms from list" );

  this->ClearAllButton = vtkKWPushButton::New();
  this->ClearAllButton->SetParent (bFrame);
  this->ClearAllButton->Create();
  this->ClearAllButton->SetBorderWidth ( 0 );
  this->ClearAllButton->SetReliefToFlat();  
  this->ClearAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeleteAllIcon() );
  this->ClearAllButton->SetBalloonHelpString ( "Clear all (non-essential) terms from list" );


  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent (bFrame);
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->FetchMIIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select all terms in list to use in query" );
  
  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent (bFrame);
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all terms in list" );

  this->HelpButton = vtkKWPushButton::New();
  this->HelpButton->SetParent (bFrame);
  this->HelpButton->Create();
  this->HelpButton->SetBorderWidth ( 0 );
  this->HelpButton->SetReliefToFlat();  
  this->HelpButton->SetImageToIcon ( this->FetchMIIcons->GetHelpIcon() );
  this->HelpButton->SetBalloonHelpString ( "Tips on using this panel's interface." );

  this->SearchButton = vtkKWPushButton::New();
  this->SearchButton->SetParent (bFrame);
  this->SearchButton->Create();
  this->SearchButton->SetBorderWidth ( 0 );
  this->SearchButton->SetReliefToFlat();  
  this->SearchButton->SetImageToIcon ( this->FetchMIIcons->GetSearchIcon() );
  this->SearchButton->SetBalloonHelpString ( "Query server for resources matching selected tags." );

  this->Script ("pack %s -side left -anchor w -expand n -padx 2 -pady 2",
                this->SelectAllButton->GetWidgetName() );
  this->Script ("pack %s -side left -anchor w -expand n -padx 2 -pady 2",
                this->DeselectAllButton->GetWidgetName() );
  this->Script ("pack %s -side left -anchor w -expand n -padx 2 -pady 2",
                this->HelpButton->GetWidgetName() );
  

  this->Script ("pack %s %s %s %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                spacer->GetWidgetName(),
                this->ClearSelectedButton->GetWidgetName(),
                this->ClearAllButton->GetWidgetName(),
                this->SearchButton->GetWidgetName(),
                this->RefreshButton->GetWidgetName() );
  spacer->Delete();

  bFrame->Delete();
}



//--- no longer used.
//----------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::RightClickListCallback(int row, int col, int x, int y)
{
    vtkKWMultiColumnList *list =
        this->GetMultiColumnList()->GetWidget();
    list->EditCell(row, col);
}





//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::PopulateFromServer ( )
{

  vtkDebugMacro ("--------------------Populating QueryWidget...");
  //--- block any update events until done.
  this->DeleteAllItems( );
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->Logic->AllValuesForAllTagsOnServer.begin();
        iter != this->Logic->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    this->AddNewTagForQuery ( iter->first.c_str(), iter->second );
    }


}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::AddNewItem ( const char *keyword, const char *val )
{
  // not used.
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::AddNewTagForQuery ( const char *keyword, std::vector<std::string> values )
{
  
  if ( !strcmp (keyword, "") )
    {
    return;
    }

  // check to see if term is unique before adding it
  // if NOT unique, keep its index.
  int unique = 1;
  int n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( int tagindex=0; tagindex<n; tagindex++ )
    {
    if ( !strcmp (this->GetMultiColumnList()->GetWidget()->GetCellText(tagindex, 1), keyword ) )
      {
      unique = 0;
      break;
      }
    }

  //--- if unique, add the new tag.
  //--- if not unique, just update the list of values
  vtkStringArray *tagValues = vtkStringArray::New();
  // fill up tagValues with values for this keyword
  unsigned int i=0;
  if ( values.size() <= i )
    {
    //--- When selected, this will cause
    //--- a pop-up entry to capture new value
    tagValues->InsertValue (0, "no values found" );
    }
  else
    {
      {
      for (i=0; i < values.size(); i++ )
        {
        tagValues->InsertValue (i, values[i].c_str() );
        }
      }
    }


  //--- TODO: condition the string before adding it.
  //--- spaces, quotes, special characters....
    
  
  vtkKWComboBox *cb;
  std::string cellText;
  if ( tagValues->GetNumberOfValues() > 0 )
    {
    if ( unique )
      {
      vtkDebugMacro ("--------------------Adding unique tag and values for "<< keyword << "\n");
      // now add the new row: checkbox column, the keyword column, and the values column.
      n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      this->GetMultiColumnList()->GetWidget()->AddRow();
      this->GetMultiColumnList()->GetWidget()->RowSelectableOff(n);
      this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(n, 0);
      this->GetMultiColumnList()->GetWidget()->SetCellText (n, 1, keyword );
      this->GetMultiColumnList()->GetWidget()->SetCellEditWindowToEntry (n, 1);
      this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToComboBoxWithValuesAsArray(n, 2, tagValues );
    
      //TODO: check the XNDTagTable for what's selected -- and select this if in the list.
      //--- for now, set the value in the combobox with 0th element.
      cb = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsComboBox(n, 2);
      if ( cb )
        {
        cellText = cb->GetValueFromIndex(0);
        if ( cellText.c_str() != NULL )
          {
          cb->SetValue (cellText.c_str() );
          }
        }
      this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (n, 0, 1.0, 1.0, 1.0);
      this->GetMultiColumnList()->GetWidget()->SetRowSelectionBackgroundColor ( n,
                                                                                this->GetMultiColumnList()->GetWidget()->GetRowBackgroundColor(n) );
      this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor (n, 0, 1.0, 1.0, 1.0);
      }
    else
      {
      // update the old row.
      vtkDebugMacro ("--------------------Updating tag and values for " << keyword << "\n");
      n = tagindex;
      this->GetMultiColumnList()->GetWidget()->RowSelectableOff(n);
      // comment out while changing the widget to use a combo box.
      //TODO: check the XNDTagTable for what's selected -- and select this if in the list.
      this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToComboBoxWithValuesAsArray(n, 2, tagValues );
      cb = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsComboBox(n, 2);
      if ( cb )
        {
        cellText = cb->GetValueFromIndex(0);
        cb->SetValue (cellText.c_str());
        //        this->GetMultiColumnList()->GetWidget()->SetCellText(n,2,cellText.c_str());
        }
      }
    }
  tagValues->Delete();

}


//---------------------------------------------------------------------------
int  vtkFetchMIQueryTermWidget::GetRowForAttribute ( const char *attribute )
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  vtksys_stl::string att;
  vtksys_stl::string lowatt;
  vtksys_stl::string target = attribute;
  vtksys_stl::string lowtarg;
  lowtarg = vtksys::SystemTools::LowerCase(target);

  for ( int i=0; i<r; i++ )
    {
    att = this->GetMultiColumnList()->GetWidget()->GetCellText (i,1);
    lowatt = vtksys::SystemTools::LowerCase(att);
    if ( !strcmp ( lowatt.c_str(), lowtarg.c_str() ))
      {
      return i;
      }
    }
  return -1;
  
}



//---------------------------------------------------------------------------
int vtkFetchMIQueryTermWidget::IsItemSelected(int i)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >=0 && i < r )
    {
    int sel = this->GetMultiColumnList()->GetWidget()->GetCellTextAsInt (i,0);
    if ( sel == 1 )
      {
      return (1);
      }
    }
  return 0;
}




//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::DeleteSelectedItems()
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      this->GetMultiColumnList()->GetWidget()->DeleteRow(i);
      }
    }
}



//---------------------------------------------------------------------------
int vtkFetchMIQueryTermWidget::GetNumberOfSelectedItems()
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();

  int numSelected = 0;
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      numSelected++;
      }
    }
  return ( numSelected );
}


//---------------------------------------------------------------------------
const char* vtkFetchMIQueryTermWidget::GetAttributeOfItem(int i )
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >=0 && i < r )
    {
    return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,1)  );
    }
  return NULL;
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::SelectValueOfItem(int i, const char *val )
{
  vtkKWComboBox *cb;
  std::string cellText;
  int numValues;
  
  //--- get combo box from row i.
  cb = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsComboBox(i, 2);
  if ( cb )
    {
    //--- make sure value is in the list.
    numValues = cb->GetNumberOfValues();
    for ( int i=0; i < numValues; i++ )
      {
      cellText = cb->GetValueFromIndex(i);
      if ( !(strcmp (cellText.c_str(), val) ))
        {
        cb->SetValue (cellText.c_str() );
        break;
        }
      }
    }
}



//---------------------------------------------------------------------------
const char* vtkFetchMIQueryTermWidget::GetValueOfItem(int i )
{
  vtkKWComboBox *cb;

  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >=0 && i < r )
    {
      cb = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsComboBox(i, 2);
      if ( cb )
        {
        return ( cb->GetValue () );
        }
    }
  return NULL;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIQueryTermWidget::GetNthSelectedAttribute(int n)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;

  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,1)  );
        }
      counter++;
      }
    }
  return NULL;

}



//---------------------------------------------------------------------------
const char* vtkFetchMIQueryTermWidget::GetNthSelectedValue(int n)
{
  vtkKWComboBox *cb;
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;

  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        cb = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsComboBox(i, 2);
        if ( cb )
          {
          return ( cb->GetValue() );
          }
        }
      counter++;
      }
    }
  return NULL;
}





