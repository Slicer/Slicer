#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMIResourceUploadWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerWindow.h"

#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWTkUtilities.h"
#include "vtkFetchMIIcons.h"
#include "vtkFetchMITagViewWidget.h"
#include "vtkKWTopLevel.h"
#include "vtkKWEntry.h"
#include "vtkKWMessageDialog.h"
#include "vtkMRMLFetchMINode.h"
#include "vtkXNDHandler.h"
#include "vtkXNDTagTable.h"
#include "vtkHIDTagTable.h"
#include "vtkTagTable.h"

#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIResourceUploadWidget );
vtkCxxRevisionMacro ( vtkFetchMIResourceUploadWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIResourceUploadWidget::vtkFetchMIResourceUploadWidget ( )
{

  this->UploadButton = NULL;
  this->SelectAllButton = NULL;
  this->DeselectAllButton = NULL;
  this->ApplyTagsButton = NULL;
  this->RemoveTagsButton = NULL;
  this->ShowTagsForAllButton = NULL;
  this->CurrentTagLabel = NULL;
  this->FetchMIIcons = NULL;
  this->NumberOfColumns = 3;
  this->TaggingHelpButton = NULL;
  this->SelectTagMenuButton = NULL;

  this->NewUserTag = NULL;
  this->NewUserValue = NULL;
  this->NewTagWindow = NULL;
  this->AddNewTagEntry = NULL;
  this->AddNewTagLabel = NULL;
  this->AddNewTagButton = NULL;
  this->AddNewValueEntry = NULL;
  this->AddNewValueButton = NULL;
  this->CloseNewTagWindowButton = NULL;

  this->NewUserTag = NULL;
  this->NewUserValue = NULL;

  this->Logic = NULL;
}





//---------------------------------------------------------------------------
vtkFetchMIResourceUploadWidget::~vtkFetchMIResourceUploadWidget ( )
{
  this->RemoveMRMLObservers();
  this->SetLogic ( NULL );

  this->DestroyNewTagWindow();

  if ( this->UploadButton )
    {
    this->UploadButton->SetParent ( NULL );
    this->UploadButton->Delete();
    this->UploadButton = NULL;    
    }
  if ( this->CurrentTagLabel )
    {
    this->CurrentTagLabel->SetParent ( NULL );
    this->CurrentTagLabel->Delete();
    this->CurrentTagLabel = NULL;    
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
  if ( this->RemoveTagsButton )
    {
    this->RemoveTagsButton->SetParent ( NULL );
    this->RemoveTagsButton->Delete();
    this->RemoveTagsButton = NULL;    
    }
  if ( this->ShowTagsForAllButton )
    {
    this->ShowTagsForAllButton->SetParent ( NULL );
    this->ShowTagsForAllButton->Delete();
    this->ShowTagsForAllButton = NULL;    
    }
    if ( this->TaggingHelpButton )
      {
      this->TaggingHelpButton->SetParent ( NULL );
      this->TaggingHelpButton->Delete();
      this->TaggingHelpButton = NULL;
      }
    if ( this->SelectTagMenuButton )
      {
      this->SelectTagMenuButton->SetParent ( NULL );
      this->SelectTagMenuButton->Delete();
      this->SelectTagMenuButton = NULL;      
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
    os << indent << "UploadButton: " << this->GetUploadButton() << "\n";
    os << indent << "CurrentTagLabel: " << this->GetCurrentTagLabel() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ShowTagsForAllButton: " << this->GetShowTagsForAllButton() << "\n";
    os << indent << "ApplyTagsButton: " << this->GetApplyTagsButton() << "\n";
    os << indent << "RemoveTagsButton: " << this->GetRemoveTagsButton() << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::SetStatusText (const char *txt)
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
void vtkFetchMIResourceUploadWidget::DestroyNewTagWindow( )
{
  if ( !this->NewTagWindow )
    {
    return;
    }
  if ( !this->NewTagWindow->IsCreated() )
    {
    vtkErrorMacro ("DestroyNewTagWindow: NewTagWindow is not created.");
    return;
    }

  this->WithdrawNewTagWindow ( );

  if ( this->AddNewTagEntry )
    {
    this->AddNewTagEntry->SetParent ( NULL );
    this->AddNewTagEntry->Delete();
    this->AddNewTagEntry = NULL;
    }
  if ( this->AddNewValueEntry )
    {
    this->AddNewValueEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent,  (vtkCommand *)this->GUICallbackCommand);
    this->AddNewValueEntry->SetParent ( NULL );
    this->AddNewValueEntry->Delete();
    this->AddNewValueEntry = NULL;
    }
  if ( this->AddNewValueButton )
    {
    this->AddNewValueButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand);
    this->AddNewValueButton->SetParent ( NULL );
    this->AddNewValueButton->Delete();    
    this->AddNewValueButton = NULL;
    }
  if ( this->AddNewTagLabel )
    {
    this->AddNewTagLabel->SetParent ( NULL );
    this->AddNewTagLabel->Delete();
    this->AddNewTagLabel = NULL;
    }
  if ( this->CloseNewTagWindowButton )
    {
    this->CloseNewTagWindowButton->SetParent ( NULL );
    this->CloseNewTagWindowButton->Delete();    
    this->CloseNewTagWindowButton = NULL;
    }

  this->NewTagWindow->Delete();
  this->NewTagWindow = NULL;
}




//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::WithdrawNewTagWindow ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    app->Script ( "grab release %s", this->NewTagWindow->GetWidgetName() );
    }
  this->NewTagWindow->Withdraw();
}




//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::RaiseNewTagWindow(const char *att)
{

  if ( this->Logic == NULL )
    {
    vtkErrorMacro ("RaiseNewTagWindow: Got NULL Logic.");
    return;
    }
  if ( this->Logic->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("RaiseNewTagWindow: Got NULL FetchMINode");
    return;
    }

  if ( this->NewTagWindow == NULL )
    {
    int px, py;
    //--- top level container.
    this->NewTagWindow = vtkKWTopLevel::New();
    this->NewTagWindow->SetMasterWindow (this->GetSelectTagMenuButton() );
    this->NewTagWindow->SetApplication ( this->GetParent()->GetApplication() );
    this->NewTagWindow->Create();
    vtkKWTkUtilities::GetWidgetCoordinates(this->GetApplyTagsButton(), &px, &py);
    this->NewTagWindow->SetPosition ( px + 10, py + 10) ;
    this->NewTagWindow->SetBorderWidth ( 1 );
    this->NewTagWindow->SetReliefToFlat();
    this->NewTagWindow->SetTitle ("Add a new tag");
    this->NewTagWindow->SetSize (450, 100);
    this->NewTagWindow->Withdraw();
    this->NewTagWindow->SetDeleteWindowProtocolCommand ( this, "DestroyNewTagWindow");

    vtkKWFrame *f1 = vtkKWFrame::New();
    f1->SetParent ( this->NewTagWindow );
    f1->Create();
    f1->SetBorderWidth ( 1 );
    this->Script ( "pack %s -side top -anchor nw -fill x -expand n -padx 0 -pady 1", f1->GetWidgetName() );

    //--- new tag entry
    vtkKWLabel *l1 = vtkKWLabel::New();
    l1->SetParent (f1);
    l1->Create();
    l1->SetText ( "Tag:" );
    l1->SetWidth ( 12 );
    this->AddNewTagEntry = vtkKWEntry::New();
    this->AddNewTagEntry->SetParent ( f1 );
    this->AddNewTagEntry->Create();
    this->AddNewTagEntry->SetWidth(20);
    //--- new value entry
    vtkKWLabel *l3 = vtkKWLabel::New();
    l3->SetParent (f1);
    l3->Create();
    l3->SetText ( "Value:" );
    l3->SetWidth ( 12 );
    this->AddNewValueEntry = vtkKWEntry::New();
    this->AddNewValueEntry->SetParent ( f1 );
    this->AddNewValueEntry->Create();
    this->AddNewValueEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent,  (vtkCommand *)this->GUICallbackCommand);
    //--- add new value button
    this->AddNewValueButton = vtkKWPushButton::New();
    this->AddNewValueButton->SetParent ( f1 );
    this->AddNewValueButton->Create();
    this->AddNewValueButton->SetBorderWidth ( 0 );
    this->AddNewValueButton->SetReliefToFlat();
    this->AddNewValueButton->SetImageToIcon ( this->FetchMIIcons->GetAddNewIcon() );
    this->AddNewValueButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand);

    this->AddNewTagLabel  = vtkKWLabel::New();
    this->AddNewTagLabel->SetParent ( f1 );
    this->AddNewTagLabel->Create();
    this->AddNewTagLabel->SetText ( "" );    
    this->AddNewTagLabel->SetBackgroundColor ( 0.881378431373, 0.88137254902, 0.98294117647);

    this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", l1->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 2", this->AddNewTagEntry->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l3->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 1 -sticky ew -padx 2 -pady 2", this->AddNewValueEntry->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 2 -sticky ew -padx 2 -pady 2", this->AddNewValueButton->GetWidgetName() );
    this->Script ( "grid %s -row 2 -column 1 -columnspan 2 -sticky ew -padx 2 -pady 2", this->AddNewTagLabel->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 0 -weight 0", f1->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 1", f1->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 2 -weight 0", f1->GetWidgetName() );

    f1->Delete();
    l1->Delete();
    l3->Delete();

    }
  
    //--- initialize entry
    if ( att != NULL && (strcmp(att, "" )) )
      {
      this->AddNewTagEntry->SetValue ( att );
      }

  //-- display
  this->NewTagWindow->DeIconify();
  this->NewTagWindow->Raise();
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    app->Script ( "grab %s", this->NewTagWindow->GetWidgetName() );
    app->ProcessIdleTasks();
    }
}





//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::UpdateNewUserTag( const char *att, const char *val)
{


  if ( this->Logic == NULL )
    {
    vtkErrorMacro ("UpdateNewUserTag: Got NULL Logic.");
    this->ResetCurrentTagLabel();
    return;
    }
  if ( this->Logic->GetFetchMINode() == NULL) 
    {
    vtkErrorMacro ( "vtkFetchMIResourceUploadWidget: FetchMINode is NULL.");
    this->ResetCurrentTagLabel();
    return;
    }

  this->SetNewUserTag ( att );
  if ( this->GetNewUserTag() == NULL )
    {
    vtkErrorMacro ("UpdateNewUserTag: Can't add a new tag with NULL tag name.");
    this->ResetCurrentTagLabel();
    return;    
    }
  this->SetNewUserValue ( val );
  if ( this->GetNewUserValue() == NULL )
    {
    vtkErrorMacro ("UpdateNewUserTag: Can't add a new tag with NULL value.");
    this->ResetCurrentTagLabel();
    return;
    }
  //--- check for empty strings.
  if ( !(strcmp( val, "" )) || !(strcmp(att, "")) )
    {
    vtkWarningMacro ("UpdateNewUserTag: Can't add a new tag with empty tagname or value.");
    this->ResetCurrentTagLabel();
    return;
    }

  this->Logic->AddUniqueTag(att );
  this->Logic->AddUniqueValueForTag(att, val);
  this->SetCurrentTagAttribute  (att);
  this->SetCurrentTagValue ( val );


  std::stringstream ss;
  if ( this->CurrentTagLabel )
    {
    ss << this->CurrentTagAttribute;
    ss << " = ";
    ss << this->CurrentTagValue;
    this->CurrentTagLabel->SetForegroundColor ( 0.0, 0.0, 0.0);
    this->CurrentTagLabel->SetText ( ss.str().c_str() );
    }

}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ProcessWidgetEvents(vtkObject *caller,
                                                         unsigned long event,
                                                         void *vtkNotUsed(callData) )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMultiColumnList *l = vtkKWMultiColumnList::SafeDownCast( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  
  if ( this->IsCreated() )
    {
    if ( this->SelectTagMenuButton != NULL )
      {
      if ( this->SelectTagMenuButton->GetMenu() != NULL )
        {
        if ( (m != NULL ) && (event == vtkKWMenu::MenuItemInvokedEvent) )
          {
          if ( m == this->SelectTagMenuButton->GetMenu() )
            {
            if ( m->GetItemSelectedState ("Add new tag" )== 1)
              {
              this->RaiseNewTagWindow ( "" );
              }
            }
          else
            {
            //-- still a menu calling, but maybe a cascade from the SelectTagMenuButton's menu...
            //-- here, we'd be adding a value for existing tag, or selecting a tag and value.
            for (int i=0; i < this->SelectTagMenuButton->GetMenu()->GetNumberOfItems(); i++ )
              {
              vtkKWMenu *c = this->SelectTagMenuButton->GetMenu()->GetItemCascade(i);
              if ( m == c)
                {
                //--- will drop in here if a tag and a tag value are selected, or if
                //--- a tag and Add new value are selected. If the selected item
                //--- in the cascade's value = "Add new value", then we want to 
                //--- clear the current tag value for now.
                //---
                //--- i is the index of the tag; j will be index of the value.
                this->SetCurrentTagAttribute ( this->SelectTagMenuButton->GetMenu()->GetItemLabel(i) );
                // and get the index of the selected value -- if user is
                // choosing to add a new value, no item will be selected.
                for ( int j=0; j < c->GetNumberOfItems(); j++ )
                  {
                  if ( c->GetItemSelectedState (j) == 1 )
                    {
                    if ( !(strcmp (c->GetItemSelectedValue(j), "Add new value")))
                      {
                      this->SetCurrentTagValue ( NULL );                      
                      this->RaiseNewTagWindow ( this->GetCurrentTagAttribute() );
                      }
                    else
                      {
                      this->SetCurrentTagValue ( c->GetItemSelectedValue(j) );
                      this->UpdateNewUserTag( this->GetCurrentTagAttribute(), this->GetCurrentTagValue());
                      }
                    break;
                    }
                  }
                }
              }

            }
          //--- reset text of menu.
          this->SelectTagMenuButton->SetValue ( "Select a tag or create a new one");
          }
        }
      }

    if ( (l == this->GetMultiColumnList()->GetWidget()) && (event == vtkKWMultiColumnList::CellUpdatedEvent) )
      {
      this->UpdateSelectedStorableNodes();
      }
    if ( (b == this->GetApplyTagsButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent (vtkFetchMIResourceUploadWidget::TagSelectedDataEvent);
      this->InvokeEvent (vtkFetchMIResourceUploadWidget::ShowAllTagViewEvent );      
      }
    else if ( (b == this->GetRemoveTagsButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent (vtkFetchMIResourceUploadWidget::RemoveTagSelectedDataEvent);
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllItems ( );
      this->GetMultiColumnList()->GetWidget()->InvokeEvent ( vtkKWMultiColumnList::CellUpdatedEvent);
      }
    else if ( (b == this->GetUploadButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent ( vtkFetchMIResourceUploadWidget::UploadRequestedEvent );
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllItems ( );
      this->GetMultiColumnList()->GetWidget()->InvokeEvent ( vtkKWMultiColumnList::CellUpdatedEvent);
      }
    else if ( (b == this->GetShowTagsForAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->InvokeEvent (vtkFetchMIResourceUploadWidget::ShowAllTagViewEvent );
      }
    else if (b == this->TaggingHelpButton )
      {
      this->RaiseTaggingHelpWindow();
      }
    else if ( b == this->AddNewValueButton && this->AddNewValueButton != NULL )
      {
      int hideWinFlag = 0;
      if ( this->AddNewValueEntry != NULL && this->AddNewTagEntry != NULL )
        {
        //--- make sure they're up and contain info
        if ( ( this->AddNewTagEntry->GetValue() != NULL) && (this->AddNewValueEntry->GetValue() != NULL ) &&
             ( strcmp ("", this->AddNewTagEntry->GetValue() )) && ( strcmp ( "", this->AddNewValueEntry->GetValue() ) ) )
          {
          hideWinFlag = 1;
          this->UpdateNewUserTag ( this->AddNewTagEntry->GetValue(), this->AddNewValueEntry->GetValue());
          if ( this->AddNewTagLabel )
            {
            std::stringstream ss;
            ss << "Added: ";
            ss << this->AddNewTagEntry->GetValue();
            ss << " = ";
            ss << this->AddNewValueEntry->GetValue();
            std::string s = ss.str();
            this->AddNewTagLabel->SetText ( s.c_str() );
            }
          }
        this->AddNewValueEntry->SetValue ("");
        this->AddNewTagEntry->SetValue ("");
        }
      if ( hideWinFlag )
        {
        this->WithdrawNewTagWindow();
        }
      }
    if ( e == this->AddNewValueEntry && event == vtkKWEntry::EntryValueChangedEvent )
      {
      //--- make sure they're up and contain info
      int hideWinFlag = 0;
      if ( this->AddNewValueEntry && this->AddNewTagEntry )
        {
        if ( ( this->AddNewTagEntry->GetValue() != NULL) && (this->AddNewValueEntry->GetValue() != NULL ) &&
             ( strcmp ("", this->AddNewTagEntry->GetValue() )) && ( strcmp ( "", this->AddNewValueEntry->GetValue() ) ) )
          {
          hideWinFlag = 1;
          this->UpdateNewUserTag ( this->AddNewTagEntry->GetValue(), this->AddNewValueEntry->GetValue());
          if ( this->AddNewTagLabel )
            {
            std::stringstream ss;
            ss << "Added: ";
            ss << this->AddNewTagEntry->GetValue();
            ss << " = ";
            ss << this->AddNewValueEntry->GetValue();
            std::string s = ss.str();
            this->AddNewTagLabel->SetText ( s.c_str() );
            }
          }
        this->AddNewValueEntry->SetValue ( "" );
        }
      if ( hideWinFlag )
        {
        this->WithdrawNewTagWindow();
        }
      }
    }
  this->UpdateMRML();
}

//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::RaiseTaggingHelpWindow()
{
  int px, py;
  vtkFetchMITagViewWidget *win = vtkFetchMITagViewWidget::New();
  win->SetParent ( this->TaggingHelpButton);
  win->Create();
  vtkKWTkUtilities::GetWidgetCoordinates(this->TaggingHelpButton, &px, &py);
  win->GetTagViewWindow()->SetPosition ( px + 10, py + 10) ;
  win->SetTagTitle ("Tagging Slicer's scene and data for upload:");
  std::stringstream ss;
  ss << "**Tags** are used to describe your data. A tag is comprised of a ~~keyword~~ and a ~~value~~. This descriptive pair is useful for organizing, querying for, and retrieving datasets stored on a searchable repository. Examples of tags might be: **Date = 10-10-2006**, or ** SlicerDataType = MRML**, or **Project = AbdominalAtlas**. Once data tagged with this description is saved in a database, it can later be retrieved by querying for one or more of those tags. Using this interface, new attributes and values can also be specified: **Important note:** currently special characters are **not supported** and can cause errors. Do not include special characters in new tags.";
  win->SetTagText ( ss.str().c_str() );
  win->DisplayTagViewWindow();
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
      if ( !(strcmp( (this->GetMultiColumnList()->GetWidget()->GetCellText (i, 4)), "Scene description")))
        {
        this->Logic->SelectScene();
        }
      else
        {
        this->Logic->AddSelectedStorableNode ( this->GetMultiColumnList()->GetWidget()->GetCellText(i,4) );
        }
      }
    else
      {
      if ( !(strcmp( (this->GetMultiColumnList()->GetWidget()->GetCellText (i, 4)), "Scene description")))
        {
        this->Logic->DeselectScene();
        }
      else
        {
        this->Logic->RemoveSelectedStorableNode ( this->GetMultiColumnList()->GetWidget()->GetCellText(i,4) );
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
      vtkKWFrame *f = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsFrame(r, 1);
      f->RemoveBinding ( "<Button-1>");
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
    return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,4) );
    }
  return NULL;

}


//---------------------------------------------------------------------------
const char* vtkFetchMIResourceUploadWidget::GetNthSlicerDataType(int i)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();

  if ( i >= 0 && i < r )
    {
    return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,3) );
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
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,3) );
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
        return ( this->GetMultiColumnList()->GetWidget()->GetCellText (i,4) );
        }
      counter++;
      }
    }
  return NULL;
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                                       unsigned long vtkNotUsed(event),
                                                       void *vtkNotUsed(callData))
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
void vtkFetchMIResourceUploadWidget::RemoveWidgetObservers ( )
{
  this->TaggingHelpButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SelectTagMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GetShowTagsForAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetApplyTagsButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetRemoveTagsButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetUploadButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetMultiColumnList()->GetWidget()->RemoveObservers(vtkKWMultiColumnList::CellUpdatedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::AddWidgetObservers ( )
{
  this->TaggingHelpButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SelectTagMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GetShowTagsForAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetApplyTagsButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetRemoveTagsButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetUploadButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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

  // frame for tag selection
  vtkKWFrame *topFrame = vtkKWFrame::New();
  topFrame->SetParent ( this->ContainerFrame );
  topFrame->Create();
  this->Script ("pack %s -side top -fill x -anchor nw -expand y -padx 2 -pady 6", topFrame->GetWidgetName() );

  this->SelectTagMenuButton = vtkKWMenuButton::New();
  this->SelectTagMenuButton->SetParent (topFrame);
  this->SelectTagMenuButton->Create();
  this->SelectTagMenuButton->IndicatorVisibilityOn();
  this->SelectTagMenuButton->SetBalloonHelpString (  "Select or create new tag." );  
  this->SelectTagMenuButton->SetValue ("Select a tag or create a new one");
  this->SelectTagMenuButton->SetBinding ("<Button-1>", this, "PopulateTagMenuButtonCallback" );

  this->CurrentTagLabel = vtkKWLabel::New();
  this->CurrentTagLabel->SetParent ( topFrame);
  this->CurrentTagLabel->Create();
  this->CurrentTagLabel->SetAnchorToCenter();
  this->CurrentTagLabel->SetBackgroundColor ( 0.881378431373, 0.88137254902, 0.98294117647);
  this->ResetCurrentTagLabel();

  this->Script ( "grid %s -row 0 -column 0  -sticky ew -padx 2 -pady 2", this->SelectTagMenuButton->GetWidgetName() );
  this->Script ( "grid %s -row 1 -column 0 -sticky ew -padx 2 -pady 2", this->CurrentTagLabel->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 0 -weight 1", topFrame->GetWidgetName() );

  // frames for the buttons
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( this->ContainerFrame );
  f->Create();
  vtkKWFrame *f1 = vtkKWFrame::New();
  f1->SetParent ( f );
  f1->Create();
  vtkKWFrame *f2 = vtkKWFrame::New();
  f2->SetParent ( f );
  f2->Create();
  vtkKWFrame *f3 = vtkKWFrame::New();
  f3->SetParent ( f );
  f3->Create();
  vtkKWFrame *f4 = vtkKWFrame::New();
  f4->SetParent ( f );
  f4->Create();
  
  this->Script ("pack %s -side top -anchor w -expand y -fill x -padx 2 -pady 6", f->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 0 -sticky w -padx 2 -pady 0", f1->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 1 -sticky w -padx 20 -pady 0", f2->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 2 -sticky ew -padx 2 -pady 0", f3->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 3 -sticky e -padx 2 -pady 0", f4->GetWidgetName() );
  this->Script ("grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  this->Script ("grid columnconfigure %s 1 -weight 0", f->GetWidgetName() );
  this->Script ("grid columnconfigure %s 2 -weight 1", f->GetWidgetName() );
  this->Script ("grid columnconfigure %s 3 -weight 0", f->GetWidgetName() );
  
  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent ( f1 );
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->FetchMIIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select all datasets in list" );

  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent ( f1 );
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->FetchMIIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all datasets in list" );

  this->ShowTagsForAllButton = vtkKWPushButton::New();
  this->ShowTagsForAllButton->SetParent ( f2 );
  this->ShowTagsForAllButton->Create();
  this->ShowTagsForAllButton->SetBorderWidth ( 0 );
  this->ShowTagsForAllButton->SetReliefToFlat();  
  this->ShowTagsForAllButton->SetImageToIcon ( this->FetchMIIcons->GetShowDataTagsIcon() );
  this->ShowTagsForAllButton->SetBalloonHelpString ( "Show all tags describing scene and data" );

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f2 );
  l1->Create();
  l1->SetText ( "  Apply, Remove & View Tags:" );
  this->ApplyTagsButton = vtkKWPushButton::New();
  this->ApplyTagsButton->SetParent ( f2 );
  this->ApplyTagsButton->Create();
  this->ApplyTagsButton->SetBorderWidth ( 0 );
  this->ApplyTagsButton->SetReliefToFlat();  
  this->ApplyTagsButton->SetImageToIcon ( this->FetchMIIcons->GetApplyTagsIcon() );
  this->ApplyTagsButton->SetBalloonHelpString ( "Apply this tag to selected datasets" );

  this->RemoveTagsButton = vtkKWPushButton::New();
  this->RemoveTagsButton->SetParent ( f2 );
  this->RemoveTagsButton->Create();
  this->RemoveTagsButton->SetBorderWidth ( 0 );
  this->RemoveTagsButton->SetReliefToFlat();  
  this->RemoveTagsButton->SetImageToIcon ( this->FetchMIIcons->GetRemoveTagsIcon() );
  this->RemoveTagsButton->SetBalloonHelpString ( "Remove this tag from selected datasets" );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f2 );
  l2->Create();
  l2->SetText ( "  Upload:" );
  this->UploadButton = vtkKWPushButton::New();
  this->UploadButton->SetParent ( f3 );
  this->UploadButton->Create();
  this->UploadButton->SetBorderWidth ( 0 );
  this->UploadButton->SetReliefToFlat();
  this->UploadButton->SetBalloonHelpString ( "Once scene and data are tagged, upload Scene and all data to the selected server." );
  this->UploadButton->SetBorderWidth ( 0 );
  this->UploadButton->SetReliefToFlat();
  this->UploadButton->SetImageToIcon ( this->FetchMIIcons->GetUploadIcon() );

  this->TaggingHelpButton = vtkKWPushButton::New();
  this->TaggingHelpButton->SetParent ( f4 );
  this->TaggingHelpButton->Create();
  this->TaggingHelpButton->SetBorderWidth ( 0 );
  this->TaggingHelpButton->SetReliefToFlat();  
  this->TaggingHelpButton->SetImageToIcon ( this->FetchMIIcons->GetHelpIcon() );
  this->TaggingHelpButton->SetBalloonHelpString ( "See more information about describing datasets with tags." );

  this->Script ("pack %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                this->SelectAllButton->GetWidgetName(),
                this->DeselectAllButton->GetWidgetName() );
  this->Script ("pack %s %s %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                l1->GetWidgetName(),
                this->ApplyTagsButton->GetWidgetName(),
                this->RemoveTagsButton->GetWidgetName(),
                this->ShowTagsForAllButton->GetWidgetName() );
  this->Script ("pack %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                l2->GetWidgetName(),
                this->UploadButton->GetWidgetName() );
  this->Script ("pack %s -side right -anchor w -expand n -padx 2 -pady 2",
                this->TaggingHelpButton->GetWidgetName() );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Tag" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(0);
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToCheckButton ( 0);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Show" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (1 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(1);
  //--- turn off for now until we implement it.
  this->GetMultiColumnList()->GetWidget()->ColumnVisibilityOff ( 1 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Filename      " );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 2 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 2, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 2 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Slicer Data Type" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 3 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 3, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 3 );
//  this->GetMultiColumnList()->GetWidget()->ColumnVisibilityOff ( 3 );
  
  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Scene/Data" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 4 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 4, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 4 );

  this->GetMultiColumnList()->GetWidget()->SetHeight ( 22);
  
  this->Script ( "pack %s -side top -fill x -pady 0 -expand n", this->GetMultiColumnList()->GetWidgetName() );

  topFrame->Delete();
  l1->Delete();
  l2->Delete();
  f1->Delete();
  f2->Delete();
  f3->Delete();
  f4->Delete();
  f->Delete();
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::PopulateTagMenuButtonCallback ()
{
  //--- populate this menu button from logic's structure.
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ("vtkFetchMIResourceUploadWidget: PopulateTagMenuButtonCallback got NULL Logic.");
    return;
    }
  if ( this->Logic->GetFetchMINode() == NULL)
    {
    vtkErrorMacro ("vtkFetchMIResourceUploadWidget: PopulateTagMenuButtonCallback got NULL FetchMINode.");
    return;
    }
  if ( this->Logic->GetCurrentWebService() == NULL )
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (this->GetParent() );
    dialog->SetStyleToMessage();
    std::string msg = "A server must be selected before tags can be assigned to the scene and data.";
    dialog->SetText ( msg.c_str() );
    dialog->Create();
    dialog->Invoke();
    dialog->Delete();
    return;
    }
  
  const char *svr = this->Logic->GetCurrentWebService()->GetName();
  int attIndex;
  int valIndex;

  if ( this->SelectTagMenuButton )
    {
    vtkKWMenu *m =   this->SelectTagMenuButton->GetMenu();
    if ( m )
      {
      //--- before deleting previous menu, remove observers on all cascade menus.
      vtkKWMenu *c;
      for ( int j=0; j < m->GetNumberOfItems(); j++ )
        {
        c= m->GetItemCascade ( j );
        if (c)
          {
          c->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
          c->DeleteAllItems();
          }
        }
      //--- clear out previous menu
      m->DeleteAllItems();

      //--- Now repopulate.

      this->SetCurrentTagAttribute ( NULL );
      this->SetCurrentTagValue ( NULL );

      // TODO: What if there are no tags, either because a server hasn't
      // been selected, or it has but no tags were found for it.
      // (unlikely i think...) but whatever.
      // For now, try populating from the node's tagtable rather than
      // the logic's table. 
      if ( this->Logic->CurrentWebServiceMetadata.size() <= 0 )
        {
        // check to see if there's a server selected.
        if ( svr == NULL || !(strcmp(svr, "" )) )
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent (this->GetParent() );
          dialog->SetStyleToMessage();
          std::string msg = "A server must be selected before tags can be assigned to the scene and data.";
          dialog->SetText ( msg.c_str() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
          return;
          }
        }
      // populate with tags, and options to add new ones.
      std::map<std::string, std::vector<std::string> >::iterator iter;
      int cascadeCount = 0;
      for (  iter = this->Logic->CurrentWebServiceMetadata.begin();
            iter != this->Logic->CurrentWebServiceMetadata.end();
            iter++ )
        {
        if ( iter->first.c_str() == NULL )
          {
          return;
          }
        vtkKWMenu *vm = vtkKWMenu::New();
        vm->SetParent ( this->SelectTagMenuButton->GetMenu() );
        vm->Create();
        for ( unsigned int i=0; i < iter->second.size(); i++ )
          {
          if ( iter->second[i].c_str() != NULL )
            {
            valIndex = vm->AddRadioButton ( iter->second[i].c_str() );
            if ( valIndex >= 0 )
              {
              vm->SetItemSelectedValue ( valIndex, iter->second[i].c_str() );
//              vm->SetItemDeselectedValue ( valIndex, "" );
              }
            }
          }

        vm->AddSeparator();
        //--- the window should raise with the corresponding tag selected in the menu.
        //--- that tag gets set in ProcessGUIEvents, on the TaggedDataList->CurrentTagAttribute.
        vm->AddRadioButton ( "Add new value" );
//        vm->AddCommand ( "Add new value", this, "RaiseNewTagWindow" );

        attIndex = m->AddCascade ( iter->first.c_str(), vm );
        if ( attIndex >= 0 )
          {          
          m->GetItemCascade(cascadeCount)->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
          m->SetItemSelectedValue ( attIndex, iter->first.c_str() );
//          m->SetItemDeselectedValue ( attIndex, "" );
          }

        vm->Delete();
        cascadeCount++;
        }

      m->AddSeparator();
      //--- the window will come up with no selected tags or values.
      m->AddRadioButton ( "Add new tag" );
//      m->AddCommand ( "Add new tag", this, "RaiseNewTagWindow");
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::RightClickListCallback(int row, int col,
                                                            int vtkNotUsed(x), int vtkNotUsed(y))
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
  std::string fileName;
  std::string filePath;
  vtkMRMLStorableNode *node;
  vtkMRMLStorageNode *snode;
  
  if  (this->MRMLScene == NULL )
    {
    vtkErrorMacro ("vtkFetchMIResourceUploadWidget::AddNewItem: Got NULL MRMLScene." );
    return;
    }

  if ( !(strcmp(dataset, "Scene description" )))
    {
    //--- what is the filename for the scene???
    filePath = this->MRMLScene->GetURL();
    }
  else
    {
    //--- what is the filename for the node???
    node = vtkMRMLStorableNode::SafeDownCast (this->MRMLScene->GetNodeByID(dataset ));
    if ( node != NULL )
      {
      snode = node->GetStorageNode();
      if ( snode != NULL && snode->GetFileName())
        {
        filePath = snode->GetFileName();
        }
      }
    }
  
  //--- pull out filename from path, if possible.
  if ( strcmp (filePath.c_str(), "") )
    {
    size_t found=filePath.find_last_of("/\\");
    if ( found != std::string::npos )
      {
      fileName = filePath.substr(found+1);
      }
    else
      {
      fileName = filePath.c_str();
      }
    }

  if ( !strcmp(fileName.c_str(), "")  )
    {
    fileName.clear();
    fileName = "unknown";
    }

  unique = 1;
  // check to see if dataset is unique before adding it
  int n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i=0; i<n; i++ )
    {
    if ( !strcmp (this->GetMultiColumnList()->GetWidget()->GetCellText(i, 4), dataset ) )
      {
      unique = 0;
      }
    }
  if ( !strcmp (dataset, "") )
    {
    dataset = "unknown";
    }
  if ( !strcmp (dtype, "") )
    {
    dtype = "unknown";
    }
  if ( unique )
    {
    //this->GetMultiColumnList()->GetWidget()->SetSelectionTypeToRow();
    i = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    this->GetMultiColumnList()->GetWidget()->AddRow();
    this->GetMultiColumnList()->GetWidget()->RowSelectableOff(i);
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    //--- select each new addition.
    this->GetMultiColumnList()->GetWidget()->SelectCell ( i, 0 );
    this->GetMultiColumnList()->GetWidget()->SetCellImageToIcon(i, 1, this->FetchMIIcons->GetShowDataTagsIcon() );
//    vtkKWFrame *f = this->GetMultiColumnList()->GetWidget()->GetCellWindowAsFrame(i, 1);
//    f->SetBinding ( "<Button-1>", this, "ShowTagViewCallback");
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 2, fileName.c_str() );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 3, dtype );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 4, dataset );
    this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 1,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 1) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 2,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 2) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 3,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 3) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 4,
                                                                              this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 4) );
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ShowTagsForSelection (  )
{
  vtkWarningMacro ("ShowTagsForSelection.");
  this->InvokeEvent ( vtkFetchMIResourceUploadWidget::ShowSelectionTagViewEvent );
}



//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ShowTagViewCallback( )
{
  vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
  dialog->SetParent (this->GetParent() );
  dialog->SetStyleToMessage();
  std::string msg = "Tag viewer for individual datasets not yet implemented; use the view all tags button at panel bottom.";
  dialog->SetText ( msg.c_str() );
  dialog->Create();
  dialog->Invoke();
  dialog->Delete();
  return;
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::ResetCurrentTagLabel ( )
{
  this->CurrentTagLabel->SetForegroundColor ( 0.5, 0.5, 0.5 );
  this->CurrentTagLabel->SetText ( "Selected Tag: (none)" );
}



//---------------------------------------------------------------------------
const char *vtkFetchMIResourceUploadWidget::GetCurrentTagAttribute()
{
  return ( this->CurrentTagAttribute.c_str() );
}

//---------------------------------------------------------------------------
const char *vtkFetchMIResourceUploadWidget::GetCurrentTagValue()
{
  return ( this->CurrentTagValue.c_str() );
}


//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::SetCurrentTagAttribute(const char* att)
{
  if ( att != NULL )
    {
    this->CurrentTagAttribute = att;
    }
}

//---------------------------------------------------------------------------
void vtkFetchMIResourceUploadWidget::SetCurrentTagValue(const char* val)
{
  std::stringstream ss;
  const char *att = this->GetCurrentTagAttribute();
  if ( val != NULL && att != NULL)
    {
    if ( (strcmp (val, "" )) && (strcmp(att, "")) )
      {
      this->CurrentTagValue = val;
      }
    }
  else
    {
    this->ResetCurrentTagLabel();
    }
}

