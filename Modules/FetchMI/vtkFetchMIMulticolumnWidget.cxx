#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMIMulticolumnWidget.h"
#include "vtkSlicerApplication.h"

#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIMulticolumnWidget );
vtkCxxRevisionMacro ( vtkFetchMIMulticolumnWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIMulticolumnWidget::vtkFetchMIMulticolumnWidget ( )
{
    this->MultiColumnList = NULL;
    this->ContainerFrame = NULL;
    this->NumberOfColumns = 1;
}


//---------------------------------------------------------------------------
vtkFetchMIMulticolumnWidget::~vtkFetchMIMulticolumnWidget ( )
{
  this->SetMRMLScene ( NULL );
  if ( this->MultiColumnList )
    {
    this->MultiColumnList->SetParent ( NULL );
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;    
    }
  if ( this->ContainerFrame )
    {
    this->ContainerFrame->SetParent ( NULL );
    this->ContainerFrame->Delete();
    this->ContainerFrame = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMIMulticolumnWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "ContainerFrame: " << this->GetContainerFrame() << "\n";
    os << indent << "MultiColumnList: " << this->GetMultiColumnList() << "\n";

}




//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::CreateWidget ( )
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
  
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( this->ContainerFrame );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->GetWidget()->SetWidth(0);
  this->MultiColumnList->GetWidget()->SetHeight(5);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToRow ( );
  this->MultiColumnList->GetWidget()->SetSelectionModeToMultiple ( );
  this->MultiColumnList->GetWidget()->MovableRowsOff ( );
  this->MultiColumnList->GetWidget()->MovableColumnsOff ( );
}



//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::GetAllItems ( )
{
}
//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::GetItemsToUse ( )
{
}


//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::SelectAllItems ( )
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
void vtkFetchMIMulticolumnWidget::DeselectAllItems ( )
{
  this->GetMultiColumnList()->GetWidget()->ClearSelection();
}


//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::DeleteAllItems ( )
{
  this->GetMultiColumnList()->GetWidget()->DeleteAllRows();
}



//---------------------------------------------------------------------------
void vtkFetchMIMulticolumnWidget::DeleteSelectedItems ( )
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








