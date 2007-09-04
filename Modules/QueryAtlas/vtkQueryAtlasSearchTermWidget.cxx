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
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::GetAllSearchTerms ( )
{
}



//---------------------------------------------------------------------------
void vtkQueryAtlasSearchTermWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
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
void vtkQueryAtlasSearchTermWidget::RemoveWidgetObservers ( ) {

  // in case these havn't been removed elsewhere...
  this->MultiColumnList->GetWidget()->RemoveObservers(vtkKWMultiColumnList::SelectionChangedEvent,
                                                      (vtkCommand *)this->GUICallbackCommand);
  this->AddNewButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearSelectedButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearAllButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  this->MultiColumnList->GetWidget()->MovableRowsOff ( );
  this->MultiColumnList->GetWidget()->MovableColumnsOff ( );

  this->MultiColumnList->GetWidget()->AddColumn ( "Search terms" );

  this->MultiColumnList->GetWidget()->ColumnEditableOn ( 0 );
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

  app->Script ("pack %s %s %s -side right -anchor c -expand n -padx 2 -pady 2",
               this->ClearAllButton->GetWidgetName(),
               this->ClearSelectedButton->GetWidgetName(),
               this->AddNewButton->GetWidgetName() );

  bFrame->Delete();
}



