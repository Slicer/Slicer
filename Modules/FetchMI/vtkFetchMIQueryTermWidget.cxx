#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMIQueryTermWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWCheckButton.h"
#include "vtkKWComboBox.h"
#include "vtkFetchMIIcons.h"

#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIQueryTermWidget );
vtkCxxRevisionMacro ( vtkFetchMIQueryTermWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIQueryTermWidget::vtkFetchMIQueryTermWidget ( )
{
    this->AddNewButton = NULL;
    this->SelectAllButton = NULL;
    this->DeselectAllButton = NULL;
    this->ClearAllButton = NULL;
    this->ClearSelectedButton = NULL;
    this->SearchButton = NULL;
    this->FetchMIIcons = NULL;
    this->NumberOfColumns = 3;
    this->Logic = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIQueryTermWidget::~vtkFetchMIQueryTermWidget ( )
{
  this->RemoveMRMLObservers();
  this->SetLogic ( NULL );

  if ( this->SearchButton )
    {
    this->SearchButton->SetParent ( NULL );
    this->SearchButton->Delete();
    this->SearchButton = NULL;
    }
  if ( this->AddNewButton )
    {
    this->AddNewButton->SetParent ( NULL );
    this->AddNewButton->Delete();
    this->AddNewButton = NULL;    
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
    os << indent << "AddNewButton: " << this->GetAddNewButton() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ClearSelectedButton: " << this->GetClearSelectedButton() << "\n";
    os << indent << "ClearAllButton: " << this->GetClearAllButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";
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
      this->InvokeEvent (vtkFetchMIQueryTermWidget::TagChangedEvent );
      }
    if ( (b == this->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllItems( );
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllItems ( );
      }
    else if ( (b == this->GetAddNewButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->AddNewItem( "", "" );
      }
    else if ( (b == this->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedItems ();
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllItems ( );
      }
    else if ( (b == this->GetSearchButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent ( vtkFetchMIQueryTermWidget::QuerySubmittedEvent );
      this->Logic->QueryServerForResources();
      }

    }
  this->UpdateMRML();
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
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetAddNewButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSearchButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetMultiColumnList()->GetWidget()->RemoveObservers(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::AddWidgetObservers ( ) {
  this->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetAddNewButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSearchButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  this->GetMultiColumnList()->GetWidget()->AddColumn ( "use" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(0);
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToCheckButton ( 0);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "attribute            " );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 1, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry ( 1 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "value                 " );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 2 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 2, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 2 );
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry ( 2 );
/*  
  this->GetMultiColumnList()->GetWidget()->AddColumn ( "help" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 3 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 3, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii (3 );
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry ( 3 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (3 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 3 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 3 );
  this->GetMultiColumnList()->GetWidget()->SetSelectionCommand (this, "ShowHelpCallback");
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(3);
*/
  
  // some problems with editing, so add a call back
  this->GetMultiColumnList()->GetWidget()->SetRightClickCommand(this, "RightClickListCallback");

  this->Script ( "pack %s -side top -fill x -expand n", this->GetMultiColumnList()->GetWidgetName() );

  // frame for the buttons
  vtkKWFrame *bFrame = vtkKWFrame::New();
  bFrame->SetParent ( this->ContainerFrame );
  bFrame->Create();
  this->Script ("pack %s -side top -anchor c -expand y -fill x -padx 2 -pady 2", bFrame->GetWidgetName() );

  vtkKWLabel *spacer = vtkKWLabel::New();
  spacer->SetParent ( bFrame );
  spacer->Create();
  spacer->SetText ("                 " );

  this->AddNewButton = vtkKWPushButton::New();
  this->AddNewButton->SetParent (bFrame);
  this->AddNewButton->Create();
  this->AddNewButton->SetBorderWidth ( 0 );
  this->AddNewButton->SetReliefToFlat();  
  this->AddNewButton->SetImageToIcon ( this->FetchMIIcons->GetAddNewIcon() );
  this->AddNewButton->SetBalloonHelpString ( "Add new query term" );

  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent (bFrame);
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->FetchMIIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select (and use) all terms in list" );

  this->ClearSelectedButton = vtkKWPushButton::New();
  this->ClearSelectedButton->SetParent (bFrame);
  this->ClearSelectedButton->Create();
  this->ClearSelectedButton->SetBorderWidth ( 0 );
  this->ClearSelectedButton->SetReliefToFlat ( );  
  this->ClearSelectedButton->SetImageToIcon ( this->FetchMIIcons->GetDeleteSelectedIcon() );
  this->ClearSelectedButton->SetBalloonHelpString ( "Delete selected (non-essential) terms from list" );

  this->ClearAllButton = vtkKWPushButton::New();
  this->ClearAllButton->SetParent (bFrame);
  this->ClearAllButton->Create();
  this->ClearAllButton->SetBorderWidth ( 0 );
  this->ClearAllButton->SetReliefToFlat();  
  this->ClearAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeleteAllIcon() );
  this->ClearAllButton->SetBalloonHelpString ( "Delete all (non-essential) terms in list" );

  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent (bFrame);
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all terms in list" );

  this->SearchButton = vtkKWPushButton::New();
  this->SearchButton->SetParent (bFrame);
  this->SearchButton->Create();
  this->SearchButton->SetBorderWidth ( 0 );
  this->SearchButton->SetReliefToFlat();  
  this->SearchButton->SetImageToIcon ( this->FetchMIIcons->GetSearchIcon() );
  this->SearchButton->SetBalloonHelpString ( "Query server for matching resources" );

  this->Script ("pack %s -side left -anchor w -expand n -padx 0 -pady 2",
                this->SelectAllButton->GetWidgetName() );
  this->Script ("pack %s -side left -anchor w -expand n -padx 4 -pady 2",
                this->DeselectAllButton->GetWidgetName() );
  this->Script ("pack %s %s %s %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                spacer->GetWidgetName(),
                this->ClearSelectedButton->GetWidgetName(),
                this->ClearAllButton->GetWidgetName(),
                this->AddNewButton->GetWidgetName(),
                this->SearchButton->GetWidgetName() );

  spacer->Delete();
  bFrame->Delete();
}



//----------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::RightClickListCallback(int row, int col, int x, int y)
{
    vtkKWMultiColumnList *list =
        this->GetMultiColumnList()->GetWidget();
    list->EditCell(row, col);
}


//---------------------------------------------------------------------------
void vtkFetchMIQueryTermWidget::AddNewItem ( const char *keyword, const char *value )
{
  // default query terms in list
  int i, unique;

  unique = 1;
  // check to see if term is unique before adding it
  int n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i=0; i<n; i++ )
    {
    if ( !strcmp (this->GetMultiColumnList()->GetWidget()->GetCellText(i, 1), keyword ) )
      {
      unique = 0;
      }
    }
  if ( !strcmp (keyword, "") )
    {
    keyword = "new";
    }
  if ( !strcmp (value, "") )
    {
    value = "none";
    }

  if ( unique )
    {
    i = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    this->GetMultiColumnList()->GetWidget()->AddRow();
    this->GetMultiColumnList()->GetWidget()->RowSelectableOff(i);
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 1, keyword );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 2, value );
    this->GetMultiColumnList()->GetWidget()->SetCellEditWindowToEntry (i, 1);
    this->GetMultiColumnList()->GetWidget()->SetCellEditWindowToEntry (i, 2);

    this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetRowSelectionBackgroundColor ( i,
                                                                              this->GetMultiColumnList()->GetWidget()->GetRowBackgroundColor(i) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    }
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
    for ( int n=0; n < r; n++)
      {
      if ( n == i )
        {
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,1)  );
        }
      }
    }
  return NULL;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIQueryTermWidget::GetValueOfItem(int i )
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >=0 && i < r )
    {
    for ( int n=0; n < r; n++)
      {
      if ( n == i )
        {
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,2)  );
        }
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
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;

  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return ( this->GetMultiColumnList()->GetWidget()->GetCellText (i,2) );
        }
      counter++;
      }
    }
  return NULL;
}





