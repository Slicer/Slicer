#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <itksys/SystemTools.hxx> 
#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>

#include "vtkKWCheckBoxSelectionDialog.h"

#include "vtkKWTopLevel.h"
#include "vtkKWDialog.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWCheckButton.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkKWCheckBoxSelectionDialog );
vtkCxxRevisionMacro ( vtkKWCheckBoxSelectionDialog, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkKWCheckBoxSelectionDialog::vtkKWCheckBoxSelectionDialog ( )
{
  this->Dialog = NULL;
  this->OkButton = NULL;
  this->CancelButton = NULL;
  this->MultiColumnList = NULL;

  this->Title=NULL;
  this->EntryColomnName=NULL;
  this->BoxColomnName=NULL;


  this->SetTitle("");
  this->SetEntryColomnName("");
  this->SetBoxColomnName("");

  this->SelectedLabels = vtkStringArray::New();
}


//---------------------------------------------------------------------------
vtkKWCheckBoxSelectionDialog::~vtkKWCheckBoxSelectionDialog ( )
{
  this->RemoveWidgetObservers();

  if (this->MultiColumnList)
    {
    this->MultiColumnList->SetParent(NULL);
    this->MultiColumnList->Delete();
    }
  if (this->OkButton)
    {
    this->OkButton->SetParent(NULL);
    this->OkButton->Delete();
    }
  if (this->CancelButton)
    {
    this->CancelButton->SetParent(NULL);
    this->CancelButton->Delete();
    }
  if (this->Dialog)
    {
    this->Dialog->SetParent(NULL);
    this->Dialog->Delete();
    }
  this->SetTitle(NULL);
  this->SetEntryColomnName(NULL);
  this->SetBoxColomnName(NULL);

  this->SelectedLabels->Delete();
}

//---------------------------------------------------------------------------
void vtkKWCheckBoxSelectionDialog::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  
  os << indent << "vtkKWCheckBoxSelectionDialog: " << this->GetClassName ( ) << "\n";
  // print widgets?
}


//---------------------------------------------------------------------------
void vtkKWCheckBoxSelectionDialog::AddEntry(char *label, int selected)
{
  this->Create();

  this->MultiColumnList->GetWidget()->AddRow();
  int row = this->MultiColumnList->GetWidget()->GetNumberOfRows() - 1;
  this->MultiColumnList->GetWidget()->SetCellText(row,0,label);
  this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,1,selected);
  this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row, 1);
}

//---------------------------------------------------------------------------
void vtkKWCheckBoxSelectionDialog::RemoveAllEntries()
{
  if (this->MultiColumnList && this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    this->MultiColumnList->GetWidget()->DeleteAllRows ();
    }
}

//---------------------------------------------------------------------------
vtkStringArray* vtkKWCheckBoxSelectionDialog::GetSelectedEntries()
{
  this->SelectedLabels->Reset();

  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, 1))
      {
      std::string label (this->MultiColumnList->GetWidget()->GetCellText(row, 0));
      this->SelectedLabels->InsertNextValue(label.c_str());
      }
    }
  return SelectedLabels;
}

//---------------------------------------------------------------------------
vtkStringArray* vtkKWCheckBoxSelectionDialog::GetUnselectedEntries()
{
  this->SelectedLabels->Reset();

  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (!this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, 1))
      {
      std::string label (this->MultiColumnList->GetWidget()->GetCellText(row, 0));
      this->SelectedLabels->InsertNextValue(label.c_str());
      }
    }
  return SelectedLabels;
}

//---------------------------------------------------------------------------
void vtkKWCheckBoxSelectionDialog::ProcessWidgetEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  if (this->OkButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    this->MultiColumnList->GetWidget()->FinishEditing();
    this->Cancel = 0;
    this->Dialog->OK();
//  this->InvokeEvent(vtkKWCheckBoxSelectionDialog::DataSavedEvent);
    }
  else if (this->CancelButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    { 
    this->Cancel = 1;
    this->Dialog->Cancel();
    }
} 





//---------------------------------------------------------------------------
void vtkKWCheckBoxSelectionDialog::RemoveWidgetObservers ( ) 
{
  if (this->OkButton)
    {
    this->OkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->CancelButton)
    {
    this->CancelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkKWCheckBoxSelectionDialog::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->Dialog = vtkKWDialog::New();
  this->Dialog->SetMasterWindow ( this->GetParent());
  this->Dialog->SetParent ( this->GetParent());
  this->Dialog->SetTitle(this->Title);
  this->Dialog->SetSize(200, 400);
  this->Dialog->Create ( );

  // Data Frame
  vtkKWFrameWithLabel *dataFrame = vtkKWFrameWithLabel::New ( );
  dataFrame->SetParent ( this->Dialog );
  dataFrame->Create ( );
  dataFrame->SetLabelText (this->Title);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 dataFrame->GetWidgetName() );
  
  // add the multicolumn list to show the points
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( dataFrame->GetFrame() );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
    
  // set up the columns of data for each point
  // name, x, y, z, orientation w, x, y, z, selected
  this->MultiColumnList->GetWidget()->AddColumn(this->GetEntryColomnName());
  this->MultiColumnList->GetWidget()->ColumnEditableOff(0);
  this->MultiColumnList->GetWidget()->SetColumnWidth(0, 40);

  this->MultiColumnList->GetWidget()->AddColumn(this->GetBoxColomnName());
  this->MultiColumnList->GetWidget()->SetColumnWidth(1, 6);
  this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(1);
  this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput(1);
  this->MultiColumnList->GetWidget()->ColumnEditableOn(1);

  // make the save column editable by checkbox
  // now set the attributes that are equal across the columns
  int col;
  for (col = 0; col < 2; col++)
    {        
    this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
    }
  this->Script ( "pack %s -fill both -expand true",
                     this->MultiColumnList->GetWidgetName());
 

  // this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");
    
  // save Frame
  vtkKWFrame *saveFrame = vtkKWFrame::New ( );
  saveFrame->SetParent ( this->Dialog );
  saveFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 saveFrame->GetWidgetName() );

  // add OK button
  this->OkButton = vtkKWPushButton::New ( );
  this->OkButton->SetParent ( saveFrame );
  this->OkButton->Create ( );
  this->OkButton->SetText ("Save");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
              this->OkButton->GetWidgetName());

  // add Cancel button
  this->CancelButton = vtkKWPushButton::New ( );
  this->CancelButton->SetParent ( saveFrame );
  this->CancelButton->Create ( );
  this->CancelButton->SetText ("Cancel");
  this->Script("pack %s -side left -anchor w -padx 36 -pady 4", 
              this->CancelButton->GetWidgetName());

  // add observers
  this->OkButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  
  this->MultiColumnList->SetEnabled(1);
  this->OkButton->SetEnabled(1);
  dataFrame->Delete();
  saveFrame->Delete();
  
}

void vtkKWCheckBoxSelectionDialog::Invoke ( )
{
  if (this->Dialog)
    {
    this->Dialog->Invoke ( );
    }
}
