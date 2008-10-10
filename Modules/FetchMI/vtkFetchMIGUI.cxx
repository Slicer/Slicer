#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkFetchMIGUI.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkFetchMIIcons.h"
#include "vtkFetchMIQueryTermWidget.h"
#include "vtkFetchMIFlatResourceWidget.h"
#include "vtkFetchMIResourceUploadWidget.h"

#include "vtkXNDTagTable.h"
#include "vtkHIDTagTable.h"
#include "vtkTagTable.h"
#include <map>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
vtkFetchMIGUI* vtkFetchMIGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFetchMIGUI");
  if(ret)
    {
      return (vtkFetchMIGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFetchMIGUI;
}


//----------------------------------------------------------------------------
vtkFetchMIGUI::vtkFetchMIGUI()
{
  this->Logic = NULL;
  this->FetchMINode = NULL;
  this->QueryList = NULL;
  this->ResourceList = NULL;
  this->TaggedDataList = NULL;
  this->AddServerButton = NULL;
  this->ServerMenuButton = NULL;
  this->AddServerEntry = NULL;
  this->FetchMIIcons = NULL;
  this->QueryTagsButton = NULL;
  this->FetchMINode = NULL;
  this->UpdatingGUI = 0;
  this->UpdatingMRML = 0;
}

//----------------------------------------------------------------------------
vtkFetchMIGUI::~vtkFetchMIGUI()
{
    this->RemoveMRMLNodeObservers ( );
    this->RemoveLogicObservers ( );
    
    if ( this->QueryTagsButton )
      {
      this->QueryTagsButton->SetParent ( NULL );
      this->QueryTagsButton->Delete();
      this->QueryTagsButton = NULL;
      }
    if ( this->QueryList )
      {
      this->QueryList->SetParent ( NULL );
      this->QueryList->Delete();
      this->QueryList = NULL;
      }
    if ( this->ResourceList )
      {
      this->ResourceList->SetParent ( NULL );
      this->ResourceList->Delete();
      this->ResourceList = NULL;
      }
    if ( this->TaggedDataList )
      {
      this->TaggedDataList->SetParent ( NULL );
      this->TaggedDataList->Delete();
      this->TaggedDataList = NULL;
      }
    
    if ( this->AddServerButton )
      {
      this->AddServerButton->SetParent ( NULL );
      this->AddServerButton->Delete();
      this->AddServerButton = NULL;
      }
    if ( this->AddServerEntry )
      {
      this->AddServerEntry->SetParent ( NULL );
      this->AddServerEntry->Delete();
      this->AddServerEntry = NULL;
      }
    if ( this->ServerMenuButton )
      {
      this->ServerMenuButton->SetParent ( NULL );
      this->ServerMenuButton->Delete();
      this->ServerMenuButton = NULL;
      }
    if ( this->FetchMIIcons )
      {
      this->FetchMIIcons->Delete();
      this->FetchMIIcons = NULL;
      }

    this->UpdatingMRML = 0;
    this->UpdatingGUI = 0;

    this->Logic = NULL;
    vtkSetAndObserveMRMLNodeMacro( this->FetchMINode, NULL );
}

//----------------------------------------------------------------------------
void vtkFetchMIGUI::TearDownGUI ( )
{
  if ( this->FetchMINode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->FetchMINode, NULL );
    }
  this->QueryList->RemoveWidgetObservers();
  this->ResourceList->RemoveWidgetObservers();
  this->TaggedDataList->RemoveWidgetObservers();
  this->RemoveGUIObservers ( );
  this->Logic->SetFetchMINode ( NULL );
  this->SetLogic ( NULL );
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkFetchMIGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::AddGUIObservers ( ) 
{
  this->QueryList->AddWidgetObservers();
  this->ResourceList->AddWidgetObservers();
  this->TaggedDataList->AddWidgetObservers();

  this->AddServerEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->QueryTagsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddServerButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ServerMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkFetchMIGUI::RemoveGUIObservers ( )
{
  this->QueryList->RemoveWidgetObservers();
  this->ResourceList->RemoveWidgetObservers();
  this->TaggedDataList->RemoveWidgetObservers();

  this->AddServerEntry->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->QueryTagsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddServerButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ServerMenuButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
vtkIntArray* vtkFetchMIGUI::NewObservableEvents()
{
  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  return events;
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in.
}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::RemoveLogicObservers ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  if ( this->FetchMINode == NULL )
    {
    return;
    }

  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );

  if ( b && event == vtkKWPushButton::InvokedEvent )
    {
    if ( b == this->AddServerButton )
      {
      if ( this->GetAddServerEntry()->GetValue() != NULL )
        {
        this->FetchMINode->AddNewServer (this->GetAddServerEntry()->GetValue() );
        }
      }
    else if ( b == this->QueryTagsButton )
      {
      this->Logic->QueryServerForTags();
      }
    }

  if ( e && event == vtkKWEntry::EntryValueChangedEvent )
    {
    if (e == this->AddServerEntry )
      {
      if ( e->GetValue() != NULL )
        {
        this->FetchMINode->AddNewServer(e->GetValue());
        }
      }
    }

  if ( m && event == vtkKWMenu::MenuItemInvokedEvent )
    {
    if ( this->ServerMenuButton != NULL )
      {
      if ( m == this->ServerMenuButton->GetMenu() )
        {
        if ( this->ServerMenuButton->GetValue() != NULL )
          {
          this->FetchMINode->SetServer ( this->ServerMenuButton->GetValue() );
          }
        }
      }
    }


  
}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateResourceTableFromMRML ( )
{
  if ( this->GetFetchMINode() != NULL )
    {
    //--- update the Resource Table.
    vtkTagTable *t = this->FetchMINode->GetResourceDescription();
    if ( t != NULL )
      {
      //--- see if we get this far ok.
      const char *att;
      const char *val;
      int i, row;
      for (i=0; i < t->GetNumberOfTags(); i++ )
        {
        att = t->GetTagAttribute(i);
        val = t->GetTagValue(i);
        this->ResourceList->AddNewItem (att, val);
        row = this->ResourceList->GetRowForAttribute ( att );
        if ( row >= 0 && (t->IsTagSelected(att)) )
          {
          this->ResourceList->SelectRow(row);              
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateTagTableFromMRML ( )
{

  if ( this->GetFetchMINode() != NULL )
    {
    const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
    if (svctype == NULL)
      {
      return;
      }
    
    //--- update the Tags Table.
    if ( !strcmp ( "XND", svctype ))
      {
      if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ) != NULL)
        {
        vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
        if ( t != NULL )
          {
          //--- see if we get this far ok.
          const char *att;
          const char *val;
          int i, row;
          for (i=0; i < t->GetNumberOfTags(); i++ )
            {
            att = t->GetTagAttribute(i);
            val = t->GetTagValue(i);
            this->QueryList->AddNewItem (att, val );
            row = this->QueryList->GetRowForAttribute ( att );
            if ( row >= 0 && (t->IsTagSelected(att)) )
              {
              this->QueryList->SelectRow(row);              
              }
            }
          }
        }
      }

    else if ( !strcmp ( "HID", svctype))
      {
      if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ) != NULL)
        {
        vtkHIDTagTable *t = vtkHIDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
        if ( t != NULL )
          {
          std::map<std::string, std::string> tt = t->TagTable;
          std::map<std::string, int> st= t->TagSelectionTable;
          }
        }
      }
    }

}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLFetchMINode* node = vtkMRMLFetchMINode::SafeDownCast(caller);
  if (node != NULL && this->GetFetchMINode() == node) 
    {
    if (event == vtkMRMLFetchMINode::TagResponseReadyEvent )
      {
      this->UpdateTagTableFromMRML();
      }
    if (event == vtkMRMLFetchMINode::ResourceResponseReadyEvent )
      {
      this->UpdateResourceTableFromMRML();
      }
    else
      {
      this->UpdateGUI();
      }
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateMRML ()
{
  // update from GUI
  if ( this->UpdatingGUI )
    {
    return;
    }
  this->UpdatingMRML = 1;
  
  vtkMRMLFetchMINode* n = this->GetFetchMINode();
  if (n == NULL)
    {
    n = vtkMRMLFetchMINode::New();
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue ( vtkMRMLFetchMINode::KnownServersModifiedEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::SelectedServerModifiedEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::TagsModifiedEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::SaveSelectionEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::RemoteIOErrorChoiceEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::ResourceResponseReadyEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::TagResponseReadyEvent );    
    vtkSetAndObserveMRMLNodeEventsMacro ( this->FetchMINode, n, events );
    if ( this->Logic->GetFetchMINode() != NULL )
      {
      this->Logic->SetFetchMINode ( NULL );
      this->Logic->SetFetchMINode( n );
      }
    events->Delete();
  }

  // save node parameters for Undo
  //this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);
//  n->Delete();
  this->UpdatingMRML = 0;
}



//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateGUI ()
{
  // update from MRML
  if ( this->UpdatingMRML )
    {
    return;
    }
  this->UpdatingGUI = 1;
  
  vtkMRMLFetchMINode* n = this->GetFetchMINode();
  if (n == NULL )
    {
    n = vtkMRMLFetchMINode::New();
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue ( vtkMRMLFetchMINode::KnownServersModifiedEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::SelectedServerModifiedEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::TagsModifiedEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::SaveSelectionEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::RemoteIOErrorChoiceEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::ResourceResponseReadyEvent );
    events->InsertNextValue ( vtkMRMLFetchMINode::TagResponseReadyEvent );    
    vtkSetAndObserveMRMLNodeEventsMacro ( this->FetchMINode, n, events );
    if ( this->Logic->GetFetchMINode() != NULL )
      {
      this->Logic->SetFetchMINode ( NULL );
      this->Logic->SetFetchMINode( n );
      }
    events->Delete();
    }
  
  if (n != NULL)
    {
    //---  update the list of known servers in the
    if ( this->ServerMenuButton != NULL )
      {
      this->ServerMenuButton->GetMenu()->DeleteAllItems();
      std::string s;
      int l = this->FetchMINode->KnownServers.size();
      for (int i=0; i < l; i ++ )
        {
        s = this->FetchMINode->KnownServers[i];
        this->ServerMenuButton->GetMenu()->AddRadioButton ( s.c_str() );      
        }

      //--- select active server in the ServerMenuButton
      if ( this->FetchMINode->GetSelectedServer() != NULL )
        {
        this->ServerMenuButton->SetValue ( this->FetchMINode->GetSelectedServer() );
        }
      else
        {
        this->ServerMenuButton->SetValue ( "<none>" );
        }
      }
    
    }
  else
    {
    vtkErrorMacro ("FetchMIGUI: UpdateGUI has a NULL FetchMINode." );
    }
  // n->Delete();
  this->UpdateTagTableFromMRML();
  this->UpdatingGUI = 0;
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkMRMLFetchMINode* n = vtkMRMLFetchMINode::New();
  if ( this->Logic != NULL )
    {
    this->Logic->GetMRMLScene()->RegisterNodeClass(n);
    if ( this->Logic->GetFetchMINode() == NULL )
      {
      this->Logic->SetFetchMINode ( NULL );
      this->Logic->SetFetchMINode( n );
      }
    }
  n->Delete();

  this->UIPanel->AddPage ( "FetchMI", "FetchMI", NULL );
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
    
  // HELP FRAME
  // Define your help text and build the help frame here.
  const char *help = "FetchMI (Medical Informatics) help.\n\n *** Select a server\n\n  *** Query the server for tags. If server has any defined,  it'll fill up the top listbox\n\n *** You can add attributes for tags in the top listbox\n\n *** Select the tags you want to use in your query\n\n *** Click the spyglass to search the server for matching resources.\n\n *** Get a big blob of resources back (because something's wrong with my query string) and listed in the second listbox.\n\n *** Select a MRML scene file from that list\n\n *** And click download (and nothing happens yet.)\n\n more logic to come....\n\n ***The bottom listbox should initialize with all data in scene (and scene file) just like the savedatawidget.\n\n ***Each time a node added or deleted event occurs, it updates\n\n ***user selects tags in top box, and applies them to selected datasets in bottom box using the Apply tags button. Tags are preserved in node. (tagtable will be added to the scene file so scene tags can be preserved too).\n\n ***user can click tag-view icon to show all tags on any individual dataset  or scene in a popup widget.\n\n *** user can upload selected data or scene to selected server. (in our first pass, just scene).";

  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. \n\n";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "FetchMI" );
  this->BuildHelpAndAboutFrame ( page, help, about );

  // create icons
  this->FetchMIIcons = vtkFetchMIIcons::New();

  // server panel
  vtkKWFrame *serverFrame = vtkKWFrame::New();
  serverFrame->SetParent ( page );
  serverFrame->Create();
  this->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2 -in %s",
                 serverFrame->GetWidgetName(), page->GetWidgetName() );

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( serverFrame );
  l1->Create();
  l1->SetText ( "Choose a server:" );
  this->ServerMenuButton = vtkKWMenuButton::New();
  this->ServerMenuButton->SetParent (serverFrame );
  this->ServerMenuButton->Create();
  this->ServerMenuButton->SetValue ( "<none>" );
  this->QueryTagsButton = vtkKWPushButton::New();
  this->QueryTagsButton->SetParent ( serverFrame );
  this->QueryTagsButton->Create();
  this->QueryTagsButton->SetBorderWidth ( 0 );
  this->QueryTagsButton->SetReliefToFlat();
  this->QueryTagsButton->SetImageToIcon ( this->FetchMIIcons->GetQueryTagsIcon() );
  this->QueryTagsButton->SetBalloonHelpString ( "Query for tags that the selected web service supports.");  
  
  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( serverFrame );
  l2->Create();
  l2->SetText ( "Add new (XND) server:" );
  this->AddServerEntry = vtkKWEntry::New();
  this->AddServerEntry->SetParent ( serverFrame );
  this->AddServerEntry->Create ();
  this->AddServerEntry->SetValue ( "" );
  this->AddServerButton = vtkKWPushButton::New();
  this->AddServerButton->SetParent ( serverFrame );
  this->AddServerButton->Create();
  this->AddServerButton->SetBorderWidth ( 0 );
  this->AddServerButton->SetReliefToFlat();  
  this->AddServerButton->SetImageToIcon ( this->FetchMIIcons->GetAddNewIcon() );
  this->AddServerButton->SetBalloonHelpString ( "Add a new XNAT Desktop server to the menu" );

  this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", l2->GetWidgetName() );
  this->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 2", this->AddServerEntry->GetWidgetName() );
  this->Script ( "grid %s -row 0 -column 2 -sticky w -padx 2 -pady 2", this->AddServerButton->GetWidgetName() );
  this->Script ( "grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", l1->GetWidgetName() );
  this->Script ( "grid %s -row 1 -column 1 -sticky ew -padx 2 -pady 2", this->ServerMenuButton->GetWidgetName() );
  this->Script ( "grid %s -row 1 -column 2 -sticky w -padx 2 -pady 2", this->QueryTagsButton->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 0 -weight 0", serverFrame->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 1 -weight 1", serverFrame->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 2 -weight 0", serverFrame->GetWidgetName() );

  // Query Frame
  vtkSlicerModuleCollapsibleFrame *queryFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  queryFrame->SetParent(page);
  queryFrame->Create();
  queryFrame->SetLabelText("Query Webservices");
  queryFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    queryFrame->GetWidgetName(), page->GetWidgetName());

  // Download Frame
  vtkSlicerModuleCollapsibleFrame *resourceFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  resourceFrame->SetParent(page);
  resourceFrame->Create();
  resourceFrame->SetLabelText("Browse Results & Download");
  resourceFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    resourceFrame->GetWidgetName(), page->GetWidgetName());

  // Tag & Upload Frame
  vtkSlicerModuleCollapsibleFrame *descriptionFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  descriptionFrame->SetParent(page);
  descriptionFrame->Create();
  descriptionFrame->SetLabelText("Describe Data & Upload");
  descriptionFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    descriptionFrame->GetWidgetName(), page->GetWidgetName());

  this->QueryList = vtkFetchMIQueryTermWidget::New();
  this->QueryList->SetParent ( queryFrame->GetFrame() );
  this->QueryList->Create();
  this->QueryList->SetLogic ( this->Logic );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->QueryList->GetWidgetName() );

  this->ResourceList = vtkFetchMIFlatResourceWidget::New();
  this->ResourceList->SetParent ( resourceFrame->GetFrame() );
  this->ResourceList->Create();
  this->ResourceList->SetLogic ( this->Logic );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->ResourceList->GetWidgetName() );

  this->TaggedDataList = vtkFetchMIResourceUploadWidget::New();
  this->TaggedDataList->SetParent ( descriptionFrame->GetFrame() );
  this->TaggedDataList->Create();
  this->TaggedDataList->SetLogic ( this->Logic );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->TaggedDataList->GetWidgetName() );

  // Clean up.
  l1->Delete();
  l2->Delete();
  serverFrame->Delete();
  queryFrame->Delete();
  resourceFrame->Delete();
  descriptionFrame->Delete();

  this->UpdateGUI();
  this->Logic->CreateTemporaryFiles();
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::Init ( )
{
}
