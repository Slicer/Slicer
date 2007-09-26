#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkQueryAtlasSearchTermWidget.h"

#include "vtkSlicerApplication.h"

#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkQueryAtlasIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasSearchTermWidget );
vtkCxxRevisionMacro ( vtkQueryAtlasSearchTermWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkQueryAtlasSearchTermWidget::vtkQueryAtlasSearchTermWidget ( )
{

    this->MultiColumnList = NULL;
    this->AddNewButton = NULL;
    this->DeselectAllButton = NULL;
    this->SelectAllButton = NULL;
    this->ReserveTermsButton = NULL;
    this->ClearAllButton = NULL;
    this->ClearSelectedButton = NULL;
    this->QueryAtlasIcons = NULL;
    this->ContainerFrame = NULL;
    this->NumberOfColumns = 3;

}


//---------------------------------------------------------------------------
vtkQueryAtlasSearchTermWidget::~vtkQueryAtlasSearchTermWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if ( this->MultiColumnList )
    {
    this->MultiColumnList->SetParent ( NULL );
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;    
    }
  if ( this->AddNewButton )
    {
    this->AddNewButton->SetParent ( NULL );
    this->AddNewButton->Delete();
    this->AddNewButton = NULL;    
    }
  if ( this->ReserveTermsButton )
    {
    this->ReserveTermsButton->SetParent ( NULL );
    this->ReserveTermsButton->Delete();
    this->ReserveTermsButton = NULL;
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
void vtkQueryAtlasSearchTermWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkQueryAtlasSearchTermWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "MultiColumnList: " << this->GetMultiColumnList() << "\n";
    os << indent << "AddNewButton: " << this->GetAddNewButton() << "\n";
    os << indent << "ClearSelectedButton: " << this->GetClearSelectedButton() << "\n";
    os << indent << "ClearAllButton: " << this->GetClearAllButton() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";
    os << indent << "ReserveTermsButton: " << this->GetReserveTermsButton() << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::GetAllSearchTerms ( )
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::AddTerm ( const char *term )
{
  
    // change to this script to get good string filtering in tcl
    this->Script ( "QueryAtlasAddEntryTermToSavedTerms \"%s\"", term );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  vtkKWMultiColumnList *ml = vtkKWMultiColumnList::SafeDownCast ( caller );
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller);
  int row[100];
  int numRows;
  
  if ( ( b == this->AddNewButton ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    // add a new search term.
    this->AddTerm ("<new>");
    }
  else if ( ( b == this->ClearSelectedButton ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    numRows = this->MultiColumnList->GetWidget()->GetSelectedRows ( row );
    while (numRows != 0 )
      {
      this->GetMultiColumnList()->GetWidget()->DeleteRow ( row[0] );    
      numRows = this->MultiColumnList->GetWidget()->GetSelectedRows ( row );
      }
    }
  else if ( ( b == this->SelectAllButton ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    numRows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for ( int i=0; i < numRows; i++ )
      {
      this->GetMultiColumnList()->GetWidget()->SelectCell ( i,0 );    
      }
    }
  else if ( ( b == this->DeselectAllButton ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    this->MultiColumnList->GetWidget()->ClearSelection();
    }
  else if ( ( b == this->ClearAllButton ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    this->MultiColumnList->GetWidget()->DeleteAllRows();
    }
  else if ( ( b == this->ReserveTermsButton ) && (event == vtkKWPushButton::InvokedEvent ))
    {
    this->ReserveTerms ( );
    }

  this->UpdateMRML();
} 



//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // nothing; handle in parent.
}

//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::RemoveMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::UpdateWidget()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::AddWidgetObservers ( ) {
  // in case these havn't been removed elsewhere...
  this->AddNewButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearSelectedButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeselectAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ReserveTermsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::RemoveWidgetObservers ( ) {

  // in case these havn't been removed elsewhere...
  this->AddNewButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearSelectedButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearAllButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeselectAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ReserveTermsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::CreateWidget ( )
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
  app->Script ( "pack %s -side top -fill both -expand true", this->ContainerFrame->GetWidgetName() );
  
  // create the icons
  this->QueryAtlasIcons = vtkQueryAtlasIcons::New();

  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( this->ContainerFrame );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->GetWidget()->SetWidth(0);
  this->MultiColumnList->GetWidget()->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell ( );
  this->MultiColumnList->GetWidget()->SetSelectionModeToMultiple( );
  this->MultiColumnList->GetWidget()->MovableRowsOff ( );
  this->MultiColumnList->GetWidget()->MovableColumnsOff ( );

  this->MultiColumnList->GetWidget()->AddColumn ( "Search terms" );

  this->MultiColumnList->GetWidget()->ColumnEditableOff ( 0 );
  this->MultiColumnList->GetWidget()->SetColumnWidth (0, 42);
  this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->MultiColumnList->GetWidget()->ColumnResizableOff ( 0 );
  this->MultiColumnList->GetWidget()->ColumnStretchableOn ( 0 );
  app->Script ( "pack %s -side top -fill x -expand true", this->MultiColumnList->GetWidgetName() );

  // frame for the buttons
  vtkKWFrame *bFrame = vtkKWFrame::New();
  bFrame->SetParent ( this->ContainerFrame );
  bFrame->Create();
  app->Script ("pack %s -side top -fill none -expand n -padx 2 -pady 2 -anchor c", bFrame->GetWidgetName() );

  this->AddNewButton = vtkKWPushButton::New();
  this->AddNewButton->SetParent (bFrame);
  this->AddNewButton->Create();
  this->AddNewButton->SetBorderWidth ( 0 );
  this->AddNewButton->SetReliefToFlat();  
  this->AddNewButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
  this->AddNewButton->SetBalloonHelpString ( "Add new search term" );

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
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all" );

  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent (bFrame);
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->QueryAtlasIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select all" );

  this->ReserveTermsButton = vtkKWPushButton::New();
  this->ReserveTermsButton->SetParent (bFrame);
  this->ReserveTermsButton->Create();
  this->ReserveTermsButton->SetBorderWidth ( 0 );
  this->ReserveTermsButton->SetReliefToFlat();  
  this->ReserveTermsButton->SetImageToIcon ( this->QueryAtlasIcons->GetReserveURIsIcon() );
  this->ReserveTermsButton->SetBalloonHelpString ( "Save selected terms as structure search terms" );

  app->Script ("pack %s %s %s %s %s %s -side left -anchor c -expand n -padx 2 -pady 2",
               this->AddNewButton->GetWidgetName(),
               this->DeselectAllButton->GetWidgetName(),
               this->SelectAllButton->GetWidgetName(),
               this->ClearSelectedButton->GetWidgetName(),
               this->ClearAllButton->GetWidgetName(),
               this->ReserveTermsButton->GetWidgetName() );

  bFrame->Delete();
}


//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::ReserveTerms ( )
{

  // reserve all the terms
  const char *term;
  
  this->reservedTerms.clear();
  int numRows = this->MultiColumnList->GetWidget()->GetNumberOfRows ();
  for ( int i=0; i<numRows; i++ )
    {
    term = this->GetMultiColumnList()->GetWidget()->GetCellText ( i, 0 );
    this->reservedTerms.push_back ( std::string(term) );
    }
  this->InvokeEvent ( vtkQueryAtlasSearchTermWidget::ReservedTermsEvent );
}

