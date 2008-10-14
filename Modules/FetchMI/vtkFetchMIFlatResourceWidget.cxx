#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMIFlatResourceWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkFetchMIIcons.h"
#include "vtkKWMessageDialog.h"

#include <vtksys/SystemTools.hxx>
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIFlatResourceWidget );
vtkCxxRevisionMacro ( vtkFetchMIFlatResourceWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIFlatResourceWidget::vtkFetchMIFlatResourceWidget ( )
{

    this->DownloadSelectedButton = NULL;
    this->SelectAllButton = NULL;
    this->DeselectAllButton = NULL;
    this->ClearAllButton = NULL;
    this->ClearSelectedButton = NULL;
    this->FetchMIIcons = NULL;
    //--- initial (select & result)
    this->NumberOfColumns = 2;
    this->Logic = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIFlatResourceWidget::~vtkFetchMIFlatResourceWidget ( )
{
  this->RemoveMRMLObservers();
  this->SetLogic ( NULL );

  if ( this->DownloadSelectedButton )
    {
    this->DownloadSelectedButton->SetParent ( NULL );
    this->DownloadSelectedButton->Delete();
    this->DownloadSelectedButton = NULL;    
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
void vtkFetchMIFlatResourceWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMIFlatResourceWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "DownloadSelectedButton: " << this->GetDownloadSelectedButton() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ClearSelectedButton: " << this->GetClearSelectedButton() << "\n";
    os << indent << "ClearAllButton: " << this->GetClearAllButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";

}



//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  
  if ( this->IsCreated() )
    {
    if ( (b == this->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllItems( );
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllItems ( );
      }
    else if ( (b == this->GetDownloadSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      //--- Request download of each selected resource
      int num = this->GetNumberOfSelectedItems();
      std::string dtype;
      std::string uri;
      int retval;
      int dlFlag = 0;
      int dtFlag = 0;
      std::string msg;
      for ( int n=0; n <num; n++)
        {
        dtype = this->GetNthSelectedSlicerDataType (n);
        uri = this->GetNthSelectedURI (n);
        retval = this->Logic->RequestResourceDownload (uri.c_str(), dtype.c_str());
        if ( retval == 0 )
          {
          // unknown data type
          dtFlag = 1;
          }
        if ( retval < 0 )
          {
          // null pointer problem with download request
          dlFlag = 1;
          }
        }
      //--- report to user if any downloads failed.
      if ( dtFlag == 1 )
        {
          //--- pop up error message: unknown dtype.
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetParent() );
          dialog->SetStyleToMessage();
          msg = "The SlicerDataType of one or more of the requested resources has an unknown value. Known values are: MRML, Volume, ScalarVolume, LabelMap, VTKModel, FreeSurferModel, DTIVolume, DWIVolume, and UnstructuredGrid. Tagging the data with an incorrect SlicerDataType can cause the scene to load improperly.";
          dialog->SetText ( msg.c_str() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
        }
      if ( dlFlag == 1 )
        {
        //--- popup error message.
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetParent() );
          dialog->SetStyleToMessage();
          msg = "Download of one or more requested resources failed.";
          dialog->SetText ( msg.c_str() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
        }
      }
    else if ( (b == this->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedItems ();
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllItems ( );
      }
    }
  this->UpdateMRML();
} 


//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::SelectRow( int i)
{
    this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 1 );
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
}


//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::SelectAllItems()
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
void vtkFetchMIFlatResourceWidget::DeselectAllItems()
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
int vtkFetchMIFlatResourceWidget::IsItemSelected(int i)
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
void vtkFetchMIFlatResourceWidget::DeleteSelectedItems()
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
int vtkFetchMIFlatResourceWidget::GetNumberOfSelectedItems()
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
const char* vtkFetchMIFlatResourceWidget::GetNthSelectedSlicerDataType(int n)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;
  
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,1) );
        }
      counter++;
      }
    }
  return NULL;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIFlatResourceWidget::GetNthSelectedURI (int n)
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




//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // nothing; handle in parent.
}

//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::RemoveMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::UpdateWidget()
{
}


//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::RemoveWidgetObservers ( ) {

  this->GetClearAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDownloadSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::AddWidgetObservers ( ) {
  this->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetDownloadSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkFetchMIFlatResourceWidget::CreateWidget ( )
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

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "select" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(0);
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToCheckButton(0);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Slicer data type" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 1, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 1 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "query result" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 2 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 2, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 2 );
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
  
  this->DownloadSelectedButton = vtkKWPushButton::New();
  this->DownloadSelectedButton->SetParent (bFrame);
  this->DownloadSelectedButton->Create();
  this->DownloadSelectedButton->SetBorderWidth ( 0 );
  this->DownloadSelectedButton->SetReliefToFlat();  
  this->DownloadSelectedButton->SetImageToIcon ( this->FetchMIIcons->GetDownloadIcon() );
  this->DownloadSelectedButton->SetBalloonHelpString ( "Download selected resources" );

  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent (bFrame);
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->FetchMIIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select all resources for downloading" );

  this->ClearSelectedButton = vtkKWPushButton::New();
  this->ClearSelectedButton->SetParent (bFrame);
  this->ClearSelectedButton->Create();
  this->ClearSelectedButton->SetBorderWidth ( 0 );
  this->ClearSelectedButton->SetReliefToFlat ( );  
  this->ClearSelectedButton->SetImageToIcon ( this->FetchMIIcons->GetDeleteSelectedIcon() );
  this->ClearSelectedButton->SetBalloonHelpString ( "Remove selected resources from list" );

  this->ClearAllButton = vtkKWPushButton::New();
  this->ClearAllButton->SetParent (bFrame);
  this->ClearAllButton->Create();
  this->ClearAllButton->SetBorderWidth ( 0 );
  this->ClearAllButton->SetReliefToFlat();  
  this->ClearAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeleteAllIcon() );
  this->ClearAllButton->SetBalloonHelpString ( "Remove all resources from list" );

  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent (bFrame);
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all resources in list" );

  this->Script ("pack %s -side left -anchor w -expand n -padx 0 -pady 2",
                this->SelectAllButton->GetWidgetName() );
  this->Script ("pack %s -side left -anchor w -expand n -padx 4 -pady 2",
                this->DeselectAllButton->GetWidgetName() );

  this->Script ("pack %s %s %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                spacer->GetWidgetName(),
                this->ClearSelectedButton->GetWidgetName(),
                this->ClearAllButton->GetWidgetName(),
                this->DownloadSelectedButton->GetWidgetName());

  spacer->Delete();
  bFrame->Delete();

}





//---------------------------------------------------------------------------
int  vtkFetchMIFlatResourceWidget::GetRowForAttribute ( const char *attribute )
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
void vtkFetchMIFlatResourceWidget::AddNewItem ( const char *term, const char *dtype )
{
  // default query terms in list
  int i, unique;

  unique = 1;
  // check to see if term is unique before adding it
  int n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i=0; i<n; i++ )
    {
    if ( !strcmp (this->GetMultiColumnList()->GetWidget()->GetCellText(i, 2), term ) )
      {
      unique = 0;
      }
    }
  if ( !strcmp (term, "") )
    {
    term = "<new>";
    }
  if ( !strcmp (dtype, "") )
    {
    term = "<unknown>";
    }
  if ( unique )
    {
    i = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    this->GetMultiColumnList()->GetWidget()->AddRow();
    this->GetMultiColumnList()->GetWidget()->RowSelectableOff(i);
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 1, dtype );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 2, term );
    this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 1,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 1) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 2,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 2) );


    }
}







