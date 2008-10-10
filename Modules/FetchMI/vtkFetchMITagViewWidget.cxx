#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMITagViewWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMITagViewWidget );
vtkCxxRevisionMacro ( vtkFetchMITagViewWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMITagViewWidget::vtkFetchMITagViewWidget ( )
{

  this->CloseButton = NULL;
  this->TagViewWindow = NULL;
  this->NodeLabel = NULL;
  this->TagTable = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMITagViewWidget::~vtkFetchMITagViewWidget ( )
{
  if ( this->CloseButton )
    {
    this->CloseButton->SetParent ( NULL );
    this->CloseButton->Delete();
    this->CloseButton = NULL;
    }
  if ( this->TagViewWindow )
    {
    this->TagViewWindow->SetParent ( NULL );
    this->TagViewWindow->Delete();
    this->TagViewWindow = NULL;
    }
  if ( this->NodeLabel )
    {
    this->NodeLabel->SetParent ( NULL );
    this->NodeLabel->Delete();
    this->NodeLabel = NULL;
    }
  if ( this->TagTable )
    {
    this->TagTable->SetParent ( NULL );
    this->TagTable->Delete();
    this->TagTable = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMITagViewWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "CloseButton: " << this->GetCloseButton() << "\n";
    os << indent << "TagViewWindow: " << this->GetTagViewWindow() << "\n";
    os << indent << "NodeLabel: " << this->GetNodeLabel() << "\n";
    os << indent << "TagTable: " << this->GetTagTable () << "\n";

}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::Bind ( )
{
  if ( !this->IsCreated() )
    {
    return;
    }
  this->CloseButton->SetBinding ( "<ButtonPress>", this, "DestroyTagViewWindow" );
}

//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::UnBind ( )
{
  if (!this->IsCreated() )
    {
    return;
    }
  if ( this->CloseButton && this->CloseButton->IsCreated() )
    {
    this->CloseButton->RemoveBinding ( "<ButtonPress>" );
    }
}

//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::DestroyTagViewWindow ( )
{

  if ( !this->IsCreated() )
    {
    return;
    }
  this->TagViewWindow->Withdraw();
  this->UnBind();
  this->Delete();
}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::DisplayTagViewWindow ( )
{
  
  if ( ! this->IsCreated() )
    {
    return;
    }
  this->TagViewWindow->DeIconify();
  this->TagViewWindow->Raise();
  this->Script ( "puts \"raising toplevel\""); 
}




//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());

    this->TagViewWindow = vtkKWTopLevel::New();
    this->TagViewWindow->SetMasterWindow ( this->GetParent ( ) );
    this->TagViewWindow->SetApplication ( app );
    this->TagViewWindow->Create();
    this->TagViewWindow->SetBorderWidth ( 2 );
    this->TagViewWindow->SetReliefToFlat ( );
    this->TagViewWindow->SetDisplayPositionToPointer();
    this->TagViewWindow->SetTitle("Tag Viewer");
    this->TagViewWindow->SetSize ( 300, 350 );
    this->TagViewWindow->Withdraw();
    this->TagViewWindow->SetDeleteWindowProtocolCommand ( this, "DestroyTagViewWindow" );

    this->CloseButton = vtkKWPushButton::New();
    this->CloseButton->SetParent ( this->TagViewWindow );
    this->CloseButton->Create();
    this->CloseButton->SetText ( "close" );
  
    this->NodeLabel = vtkKWLabel::New();
    this->NodeLabel->SetParent ( this->TagViewWindow );
    this->NodeLabel->Create();
    this->NodeLabel->SetJustificationToCenter();
    this->NodeLabel->SetHeight ( 2 );    

    this->TagTable = vtkKWMultiColumnListWithScrollbars::New ( );
    this->TagTable->SetParent ( this->TagViewWindow );
    this->TagTable->Create ( );
    this->TagTable->GetWidget()->SetWidth(0);
    this->TagTable->GetWidget()->SetHeight(3);
    this->TagTable->GetWidget()->SetSelectionTypeToRow ( );
    this->TagTable->GetWidget()->SetSelectionModeToMultiple ( );
    this->TagTable->GetWidget()->MovableRowsOff ( );
    this->TagTable->GetWidget()->MovableColumnsOff ( );

    // pack all but HelpButton, which gets packed into another widget.
    this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 2", this->NodeLabel->GetWidgetName() );
    this->Script ("pack %s -side top -expand y -anchor c -padx 2 -pady 4", this->TagTable->GetWidgetName() );
    this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 2", this->CloseButton->GetWidgetName() );
    this->Bind();
}



