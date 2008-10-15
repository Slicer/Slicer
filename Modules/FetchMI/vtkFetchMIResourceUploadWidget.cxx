#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMIResourceUploadWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkFetchMIIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIResourceUploadWidget );
vtkCxxRevisionMacro ( vtkFetchMIResourceUploadWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIResourceUploadWidget::vtkFetchMIResourceUploadWidget ( )
{

    this->UploadSelectedButton = NULL;
    this->SelectAllButton = NULL;
    this->DeselectAllButton = NULL;
    this->ApplyTagsButton = NULL;
    this->ShowTagsButton = NULL;
    this->FetchMIIcons = NULL;
    this->NumberOfColumns = 3;
    this->Logic = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIResourceUploadWidget::~vtkFetchMIResourceUploadWidget ( )
{
  this->RemoveMRMLObservers();
  this->SetLogic ( NULL );

  if ( this->UploadSelectedButton )
    {
    this->UploadSelectedButton->SetParent ( NULL );
    this->UploadSelectedButton->Delete();
    this->UploadSelectedButton = NULL;    
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
  if ( this->ApplyTagsButton )
    {
    this->ApplyTagsButton->SetParent ( NULL );
    this->ApplyTagsButton->Delete();
    this->ApplyTagsButton = NULL;    
    }
  if ( this->ShowTagsButton )
    {
    this->ShowTagsButton->SetParent ( NULL );
    this->ShowTagsButton->Delete();
    this->ShowTagsButton = NULL;    
    }
  if ( this->FetchMIIcons )
    {
    this->FetchMIIcons->Delete();
    this->FetchMIIcons = NULL;
    }
  this->SetMRMLScene ( NULL );

}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMIResourceUploadWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "UploadSelectedButton: " << this->GetUploadSelectedButton() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ShowTagsButton: " << this->GetShowTagsButton() << "\n";
    os << indent << "ApplyTagsButton: " << this->GetApplyTagsButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMultiColumnList *l = vtkKWMultiColumnList::SafeDownCast( caller );
  
  if ( this->IsCreated() )
    {
    if ( (l == this->GetMultiColumnList()->GetWidget()) && (event == vtkKWMultiColumnList::CellUpdatedEvent) )
      {
      this->UpdateSelectedStorableNodes();
      }
    if ( (b == this->GetApplyTagsButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent (vtkFetchMIResourceUploadWidget::TagSelectedDataEvent);
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllItems ( );
      this->GetMultiColumnList()->GetWidget()->InvokeEvent ( vtkKWMultiColumnList::CellUpdatedEvent);
      }
    else if ( (b == this->GetUploadSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent ( vtkFetchMIResourceUploadWidget::UploadRequestedEvent );
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllItems ( );
      this->GetMultiColumnList()->GetWidget()->InvokeEvent ( vtkKWMultiColumnList::CellUpdatedEvent);
      }
    else if ( (b == this->GetShowTagsButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent (vtkFetchMIResourceUploadWidget::ShowAllTagViewEvent );
      }
    }
  this->UpdateMRML();
} 


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::SelectRow( int i)
{
    this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 1 );
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::DeselectRow( int i)
{
    this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 0 );
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::SelectAllItems()
{
  int numrows, i;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
    this->SelectRow(i);
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::DeselectAllItems()
{
  int numrows, i;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
    this->DeselectRow(i);
    }
}




//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::UpdateSelectedStorableNodes()
{
  int numrows, i;
  
  //--- mark which nodes are selected and whether the scene is selected
  //--- in the logic class.
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
    if ( this->IsItemSelected(i) )
      {
      if ( !(strcmp( (this->GetMultiColumnList()->GetWidget()->GetCellText (i, 3)), "Scene description")))
        {
        this->Logic->SelectScene();
        }
      else
        {
        this->Logic->AddSelectedStorableNode ( this->GetMultiColumnList()->GetWidget()->GetCellText(i,3) );
        }
      }
    else
      {
      if ( !(strcmp( (this->GetMultiColumnList()->GetWidget()->GetCellText (i, 3)), "Scene description")))
        {
        this->Logic->DeselectScene();
        }
      else
        {
        this->Logic->RemoveSelectedStorableNode ( this->GetMultiColumnList()->GetWidget()->GetCellText(i,3) );
        }
      }
    }
}



//---------------------------------------------------------------------------
int vtkFetchMIResourceUploadWidget::IsItemSelected(int i)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >=0 && i < r )
    {
    int sel = this->GetMultiColumnList()->GetWidget()->GetCellTextAsInt (i,0);
    if ( sel == 0 || sel == 1 )
      {
      return (sel);
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::DeleteSelectedItems()
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
int vtkFetchMIResourceUploadWidget::GetNumberOfSelectedItems()
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
int vtkFetchMIResourceUploadWidget::GetNumberOfItems()
{
  return (this->GetMultiColumnList()->GetWidget()->GetNumberOfRows() );
}


//---------------------------------------------------------------------------
const char* vtkFetchMIResourceUploadWidget::GetNthDataTarget(int i )
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >= 0 && i < r )
    {
    return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,3) );
    }
  return NULL;

}


//---------------------------------------------------------------------------
const char* vtkFetchMIResourceUploadWidget::GetNthSlicerDataType(int i)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();

  if ( i >= 0 && i < r )
    {
    return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,2) );
    }
  return NULL;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIResourceUploadWidget::GetNthSelectedSlicerDataType(int n)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;

  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,2) );
        }
      counter++;
      }
    }
  return NULL;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIResourceUploadWidget::GetNthSelectedDataTarget(int n)
{
 int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;

  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return ( this->GetMultiColumnList()->GetWidget()->GetCellText (i,3) );
        }
      counter++;
      }
    }
  return NULL;
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // nothing; handle in parent.
}

//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::RemoveMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::UpdateWidget()
{
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::RemoveWidgetObservers ( ) {

  this->GetShowTagsButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetApplyTagsButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetUploadSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetMultiColumnList()->GetWidget()->RemoveObservers(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::AddWidgetObservers ( ) {


  this->GetShowTagsButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetApplyTagsButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetUploadSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSelectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetMultiColumnList()->GetWidget()->AddObserver(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::CreateWidget ( )
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

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "tag" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(0);
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToCheckButton ( 0);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "show" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (1 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetSelectionCommand (this, "ShowTagViewCallback");
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(1);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Slicer data type" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 2 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 2, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 2 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "scene/data" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 3 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 3, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 3 );

  // some problems with editing, so add a call back
  this->GetMultiColumnList()->GetWidget()->SetRightClickCommand(this, "RightClickListCallback");

  this->Script ( "pack %s -side top -fill x -expand n", this->GetMultiColumnList()->GetWidgetName() );

  // frame for the buttons
  vtkKWFrame *bFrame = vtkKWFrame::New();
  bFrame->SetParent ( this->ContainerFrame );
  bFrame->Create();
  this->Script ("pack %s -side top -fill x -anchor c -expand n -padx 2 -pady 2", bFrame->GetWidgetName() );

 vtkKWLabel *spacer = vtkKWLabel::New();
  spacer->SetParent ( bFrame );
  spacer->Create();
  spacer->SetText ("                  " );
  
  this->UploadSelectedButton = vtkKWPushButton::New();
  this->UploadSelectedButton->SetParent (bFrame);
  this->UploadSelectedButton->Create();
  this->UploadSelectedButton->SetBorderWidth ( 0 );
  this->UploadSelectedButton->SetReliefToFlat();  
  this->UploadSelectedButton->SetImageToIcon ( this->FetchMIIcons->GetUploadIcon() );
  this->UploadSelectedButton->SetBalloonHelpString ( "Upload tagged scene and data" );

  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent (bFrame);
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->FetchMIIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select all datasets in list" );

  this->ShowTagsButton = vtkKWPushButton::New();
  this->ShowTagsButton->SetParent (bFrame);
  this->ShowTagsButton->Create();
  this->ShowTagsButton->SetBorderWidth ( 0 );
  this->ShowTagsButton->SetReliefToFlat();  
  this->ShowTagsButton->SetImageToIcon ( this->FetchMIIcons->GetShowDataTagsIcon() );
  this->ShowTagsButton->SetBalloonHelpString ( "Pop-up a tag-view of scene and data" );

  this->ApplyTagsButton = vtkKWPushButton::New();
  this->ApplyTagsButton->SetParent (bFrame);
  this->ApplyTagsButton->Create();
  this->ApplyTagsButton->SetBorderWidth ( 0 );
  this->ApplyTagsButton->SetReliefToFlat();  
  this->ApplyTagsButton->SetImageToIcon ( this->FetchMIIcons->GetApplyTagsIcon() );
  this->ApplyTagsButton->SetBalloonHelpString ( "Apply selected tags to selected datasets" );


  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent (bFrame);
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all datasets in list" );

  this->Script ("pack %s -side left -anchor w -expand n -padx 0 -pady 2",
                this->SelectAllButton->GetWidgetName() );
  this->Script ("pack %s -side left -anchor w -expand n -padx 4 -pady 2",
                this->DeselectAllButton->GetWidgetName() );
  this->Script ("pack %s %s %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                spacer->GetWidgetName(),
                this->ApplyTagsButton->GetWidgetName(),
                this->ShowTagsButton->GetWidgetName(),
                this->UploadSelectedButton->GetWidgetName());

  spacer->Delete();
  bFrame->Delete();

}

//----------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::RightClickListCallback(int row, int col, int x, int y)
{
    vtkKWMultiColumnList *list =
        this->GetMultiColumnList()->GetWidget();
    list->EditCell(row, col);
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::AddNewItem ( const char *dataset, const char *dtype )
{
  // default query terms in list
  int i, unique;

  unique = 1;
  // check to see if dataset is unique before adding it
  int n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i=0; i<n; i++ )
    {
    if ( !strcmp (this->GetMultiColumnList()->GetWidget()->GetCellText(i, 3), dataset ) )
      {
      unique = 0;
      }
    }
  if ( !strcmp (dataset, "") )
    {
    dataset = "<unknown_data>";
    }
  if ( !strcmp (dtype, "") )
    {
    dtype = "<unknown>";
    }
  if ( unique )
    {
    //this->GetMultiColumnList()->GetWidget()->SetSelectionTypeToRow();
    i = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    this->GetMultiColumnList()->GetWidget()->AddRow();
    this->GetMultiColumnList()->GetWidget()->RowSelectableOff(i);
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    this->GetMultiColumnList()->GetWidget()->SetCellImageToIcon(i, 1, this->FetchMIIcons->GetShowDataTagsIcon() );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 2, dtype );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 3, dataset );
    this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 1,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 1) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 2,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 2) );
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ShowTagViewCallback()
{

  vtkKWMultiColumnList *l = this->GetMultiColumnList()->GetWidget();
  int numRows = l->GetNumberOfRows();
  int s;

  if ( l )
    {
    for ( int i=0; i<numRows; i++ )
      {
      s = l->IsCellSelected(i, 0 );
      if ( s )
        {
        //--- TODO: Create a TagViewWindow,
        //--- populate the display
        //--- and raise it.
        l->DeselectCell (i, 0);
        //--- lower when done and delete contents.
        break;
        }
      }
    }
}



