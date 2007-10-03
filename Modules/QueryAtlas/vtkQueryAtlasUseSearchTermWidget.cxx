#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkQueryAtlasUseSearchTermWidget.h"

#include "vtkSlicerApplication.h"

#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkQueryAtlasIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasUseSearchTermWidget );
vtkCxxRevisionMacro ( vtkQueryAtlasUseSearchTermWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkQueryAtlasUseSearchTermWidget::vtkQueryAtlasUseSearchTermWidget ( )
{

    this->MultiColumnList = NULL;
    this->AddNewButton = NULL;
    this->SelectAllButton = NULL;
    this->DeselectAllButton = NULL;
    this->ClearAllButton = NULL;
    this->ClearSelectedButton = NULL;
    this->ToggleQuotesButton = NULL;
    this->QueryAtlasIcons = NULL;
    this->ContainerFrame = NULL;
    this->NumberOfColumns = 3;

}


//---------------------------------------------------------------------------
vtkQueryAtlasUseSearchTermWidget::~vtkQueryAtlasUseSearchTermWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if ( this->MultiColumnList )
    {
    this->MultiColumnList->SetParent ( NULL );
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;    
    }
  if ( this->ToggleQuotesButton )
    {
    this->ToggleQuotesButton->SetParent ( NULL );
    this->ToggleQuotesButton->Delete();
    this->ToggleQuotesButton = NULL;    
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
  if ( this->QueryAtlasIcons )
    {
    this->QueryAtlasIcons->Delete();
    this->QueryAtlasIcons = NULL;
    }
  if ( this->ContainerFrame )
    {
    this->ContainerFrame->SetParent ( NULL );
    this->ContainerFrame->Delete();
    this->ContainerFrame = NULL;
    }
  this->SetMRMLScene ( NULL );

}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::GetAllSearchTerms ( )
{
}
//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::GetSearchTermsToUse ( )
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkQueryAtlasUseSearchTermWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "MultiColumnList: " << this->GetMultiColumnList() << "\n";
    os << indent << "AddNewButton: " << this->GetAddNewButton() << "\n";
    os << indent << "ToggleQuotesButton: " << this->GetToggleQuotesButton() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ClearSelectedButton: " << this->GetClearSelectedButton() << "\n";
    os << indent << "ClearAllButton: " << this->GetClearAllButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";
    // print widgets?


}



//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  
  if ( this->IsCreated() )
    {
    if ( (b == this->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllSearchTerms( );
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllSearchTerms ( );
      }
    else if ( (b == this->GetAddNewButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->AddNewSearchTerm( "" );
      }
    else if ( (b == this->GetToggleQuotesButton() ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->ToggleQuotesOnSelectedSearchTerms ( );
      }
    else if ( (b == this->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedSearchTerms ();
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllSearchTerms ( );
      }
    }
  this->UpdateMRML();
} 



//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // nothing; handle in parent.
}

//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::RemoveMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::UpdateWidget()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::RemoveWidgetObservers ( ) {

  this->GetClearAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetAddNewButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetToggleQuotesButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::AddWidgetObservers ( ) {
  this->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetAddNewButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetToggleQuotesButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::CreateWidget ( )
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
  
  // frame for all the component widgets
  this->ContainerFrame = vtkKWFrame::New();
  this->ContainerFrame->SetParent ( this->GetParent() );
  this->ContainerFrame->Create();
  app->Script ( "pack %s -side top -fill both -expand n", this->ContainerFrame->GetWidgetName() );
  
  // create the icons
  this->QueryAtlasIcons = vtkQueryAtlasIcons::New();

  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( this->ContainerFrame );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->GetWidget()->SetWidth(0);
  this->MultiColumnList->GetWidget()->SetHeight(3);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToRow ( );
  this->MultiColumnList->GetWidget()->SetSelectionModeToMultiple ( );
  this->MultiColumnList->GetWidget()->MovableRowsOff ( );
  this->MultiColumnList->GetWidget()->MovableColumnsOff ( );

  this->MultiColumnList->GetWidget()->AddColumn ( "Search terms (only selected terms will be used)" );

  this->MultiColumnList->GetWidget()->ColumnEditableOn ( 0 );
  this->MultiColumnList->GetWidget()->SetColumnWidth (0, 50);
  this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->MultiColumnList->GetWidget()->ColumnResizableOn ( 0 );
  this->MultiColumnList->GetWidget()->ColumnStretchableOn ( 0 );
  app->Script ( "pack %s -side top -fill x -expand n", this->MultiColumnList->GetWidgetName() );

  // frame for the buttons
  vtkKWFrame *bFrame = vtkKWFrame::New();
  bFrame->SetParent ( this->ContainerFrame );
  bFrame->Create();
  app->Script ("pack %s -side top -fill none -anchor c -expand n -padx 2 -pady 2", bFrame->GetWidgetName() );

  this->AddNewButton = vtkKWPushButton::New();
  this->AddNewButton->SetParent (bFrame);
  this->AddNewButton->Create();
  this->AddNewButton->SetBorderWidth ( 0 );
  this->AddNewButton->SetReliefToFlat();  
  this->AddNewButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
  this->AddNewButton->SetBalloonHelpString ( "Add new search term" );

  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent (bFrame);
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->QueryAtlasIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select (and use) all terms in list" );

  this->ToggleQuotesButton = vtkKWPushButton::New();
  this->ToggleQuotesButton->SetParent (bFrame);
  this->ToggleQuotesButton->Create();
  this->ToggleQuotesButton->SetBorderWidth ( 0 );
  this->ToggleQuotesButton->SetReliefToFlat();  
  this->ToggleQuotesButton->SetImageToIcon ( this->QueryAtlasIcons->GetToggleQuotesIcon() );
  this->ToggleQuotesButton->SetBalloonHelpString ( "Toggle quotes around selected terms in list" );

  this->ClearSelectedButton = vtkKWPushButton::New();
  this->ClearSelectedButton->SetParent (bFrame);
  this->ClearSelectedButton->Create();
  this->ClearSelectedButton->SetBorderWidth ( 0 );
  this->ClearSelectedButton->SetReliefToFlat ( );  
  this->ClearSelectedButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearSelectedIcon() );
  this->ClearSelectedButton->SetBalloonHelpString ( "Delete selected terms from list" );

  this->ClearAllButton = vtkKWPushButton::New();
  this->ClearAllButton->SetParent (bFrame);
  this->ClearAllButton->Create();
  this->ClearAllButton->SetBorderWidth ( 0 );
  this->ClearAllButton->SetReliefToFlat();  
  this->ClearAllButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearAllIcon() );
  this->ClearAllButton->SetBalloonHelpString ( "Delete all terms in list" );

  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent (bFrame);
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->QueryAtlasIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all terms in list" );

  app->Script ("pack %s %s %s %s %s %s -side right -anchor c -expand n -padx 3 -pady 2",
               this->ClearAllButton->GetWidgetName() ,
               this->ClearSelectedButton->GetWidgetName(),
               this->ToggleQuotesButton->GetWidgetName(),
               this->SelectAllButton->GetWidgetName(),
               this->DeselectAllButton->GetWidgetName(),
               this->AddNewButton->GetWidgetName());
  bFrame->Delete();

}



//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::SelectAllSearchTerms ( )
{
  int i;
  int numrows;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
    this->GetMultiColumnList()->GetWidget()->SelectCell ( i, 0 );
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::DeselectAllSearchTerms ( )
{

  this->GetMultiColumnList()->GetWidget()->ClearSelection();

}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::DeleteAllSearchTerms ( )
{
  this->GetMultiColumnList()->GetWidget()->DeleteAllRows();
}




//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::AddNewSearchTerm ( const char *term )
{
  // default search terms in list
  int i, unique;

  unique = 1;
  // check to see if term is unique before adding it
  int n = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for ( i=0; i<n; i++ )
    {
    if ( !strcmp (this->MultiColumnList->GetWidget()->GetCellText(i, 0), term ) )
      {
      unique = 0;
      }
    }
  if ( !strcmp (term, "") )
    {
    term = "<new>";
    }
  if ( unique )
    {
    i = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    this->GetMultiColumnList()->GetWidget()->InsertCellText (i, 0, term );
    this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry (0);
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::DeleteSelectedSearchTerms ( )
{
  int numRows;
  int row[100];
  // get the row that was last selected and remove by index

  numRows = this->MultiColumnList->GetWidget()->GetSelectedRows ( row );
  while (numRows != 0 )
    {
    this->GetMultiColumnList()->GetWidget()->DeleteRow ( row[0] );    
    numRows = this->MultiColumnList->GetWidget()->GetSelectedRows ( row );
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasUseSearchTermWidget::ToggleQuotesOnSelectedSearchTerms ( )
{
  int numRows;
  // get the row that was last selected and remove by index

  const char *term;
  numRows = this->MultiColumnList->GetWidget()->GetNumberOfRows();

  for ( int i=0; i < numRows; i++ )
    {
    if ( this->GetMultiColumnList()->GetWidget()->IsRowSelected( i ) )
      {
      term = this->MultiColumnList->GetWidget()->GetCellText ( i, 0 );
      std::string str(term);
      int len = str.length();
      if ( len != 0 )
        {
        if ( str[0] == '"' && str[len-1] == '"' )
          {
          // get rid of quotes already there
          str = str.substr (1, len-1);
          str = str.substr (0, len-2);
          } else {
          // put quotes in.
          str = "\"" + str + "\"";
          }
        this->MultiColumnList->GetWidget()->SetCellText ( i, 0, str.c_str() );
        }
      }
    }
}



