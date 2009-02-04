#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkFetchMIGUI.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerPopUpHelpWidget.h"

#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkXNDHandler.h"
#include "vtkXNDTagTable.h"
#include "vtkHIDTagTable.h"
#include "vtkTagTable.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"
#include "vtkFetchMIIcons.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkFetchMIQueryTermWidget.h"
#include "vtkFetchMIFlatResourceWidget.h"
#include "vtkFetchMIResourceUploadWidget.h"
#include "vtkFetchMITagViewWidget.h"

#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


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
  this->CloseNewServerButton = NULL;
  this->NewServerWindow = NULL;
  this->ServerMenuButton = NULL;
  this->AddServerEntry = NULL;
  this->FetchMIIcons = NULL;
  this->FetchMINode = NULL;
  this->UpdatingGUI = 0;
  this->UpdatingMRML = 0;
  this->DataDirectoryName = NULL;
  this->TagViewer = NULL;
  this->ReservedURI = NULL;

  
//  this->DebugOn();
}

//----------------------------------------------------------------------------
vtkFetchMIGUI::~vtkFetchMIGUI()
{
    this->RemoveMRMLNodeObservers ( );
    this->RemoveLogicObservers ( );
    
    if ( this->QueryList )
      {
      this->QueryList->SetParent ( NULL );
      this->QueryList->SetApplication ( NULL );
      this->QueryList->Delete();
      this->QueryList = NULL;
      }
    if ( this->ResourceList )
      {
      this->ResourceList->SetParent ( NULL );
      this->ResourceList->SetApplication ( NULL );
      if ( this->ResourceList->GetMRMLScene() != NULL )
        {
        this->ResourceList->SetMRMLScene ( NULL );
        }
      this->ResourceList->Delete();
      this->ResourceList = NULL;
      }
    if ( this->TaggedDataList )
      {
      this->TaggedDataList->SetParent ( NULL );
      this->TaggedDataList->SetApplication ( NULL );
      if ( this->TaggedDataList->GetMRMLScene() != NULL )
        {
        this->TaggedDataList->SetMRMLScene ( NULL );
        }
      this->TaggedDataList->Delete();
      this->TaggedDataList = NULL;
      }
    if ( this->AddServerButton )
      {
      this->AddServerButton->SetParent ( NULL );
      this->AddServerButton->Delete();
      this->AddServerButton = NULL;
      }
    if ( this->CloseNewServerButton )
      {
      this->CloseNewServerButton->SetParent ( NULL );
      this->CloseNewServerButton->Delete();
      this->CloseNewServerButton = NULL;
      }
    if ( this->NewServerWindow)
      {
      this->UnBindNewServerWindow();
      this->NewServerWindow->SetParent ( NULL );
      this->NewServerWindow->Delete();
      this->NewServerWindow = NULL;
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
    if ( this->TagViewer)
      {
      this->TagViewer->SetParent ( NULL );
      this->TagViewer->Delete();
      this->TagViewer = NULL;
      }

    this->UpdatingMRML = 0;
    this->UpdatingGUI = 0;
    this->SetDataDirectoryName ( NULL );

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
  if ( this->TagViewer )
    {
    this->TagViewer->UnBind();
    }
  this->QueryList->RemoveWidgetObservers();
  this->ResourceList->RemoveWidgetObservers();
  this->TaggedDataList->RemoveWidgetObservers();
  this->RemoveGUIObservers ( );
  this->Logic->SetFetchMINode ( NULL );
  this->SetLogic ( NULL );
  this->ResourceList->SetMRMLScene( NULL );
  this->TaggedDataList->SetMRMLScene ( NULL );
  this->SetAndObserveMRMLScene ( NULL );

}


//----------------------------------------------------------------------------
void vtkFetchMIGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::LoadTclPackage ( )
{
  if (!this->GetLogic())
    {
    return;
    }
  std::string qaTclCommand;
  if (0)
    {
  std::string dir(this->GetLogic()->GetModuleShareDirectory());
  qaTclCommand =  "set ::FETCHMI_PACKAGE {}; ";
  qaTclCommand += "package forget FetchMI; ";
  qaTclCommand += "  set dir \"" + dir + "\";";
  qaTclCommand += "  if { [ file exists \"$dir/Tcl/pkgIndex.tcl\" ] } { ";
  qaTclCommand += "    lappend ::auto_path $dir; ";
  qaTclCommand += "    package require FetchMI ";
  qaTclCommand += "  }";
    }
  else
    {
    // it's not a package that defines a tcl scripted module, just a file with
    // some helper procs in it, so just source it
    std::string dir(this->GetLogic()->GetModuleLibDirectory());
    qaTclCommand = "  set dir \"" + dir + "\";";
    qaTclCommand += " if  { [ file exists \"$dir/Tcl/FetchMIXMLUtilities.tcl\" ] } { ";
    qaTclCommand += " source \"$dir/Tcl/FetchMIXMLUtilities.tcl\"";
    qaTclCommand += "  }";
    }
  this->Script ( qaTclCommand.c_str() ); 
}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::AddGUIObservers ( ) 
{
  this->QueryList->AddWidgetObservers();
  this->QueryList->AddObserver(vtkFetchMIQueryTermWidget::TagChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->QueryList->AddObserver(vtkFetchMIQueryTermWidget::QuerySubmittedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ResourceList->AddObserver (vtkFetchMIFlatResourceWidget::DeleteResourceEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ResourceList->AddWidgetObservers();
  this->TaggedDataList->AddObserver(vtkFetchMIResourceUploadWidget::TagSelectedDataEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->AddObserver(vtkFetchMIResourceUploadWidget::RemoveTagSelectedDataEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->AddObserver(vtkFetchMIResourceUploadWidget::ShowAllTagViewEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->AddObserver(vtkFetchMIResourceUploadWidget::ShowSelectionTagViewEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->AddObserver(vtkFetchMIResourceUploadWidget::UploadRequestedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->AddWidgetObservers();
  this->ServerMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->AddServerEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
//  this->CloseNewServerButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->AddServerButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkFetchMIGUI::RemoveGUIObservers ( )
{
  this->QueryList->RemoveWidgetObservers();
  this->QueryList->RemoveObservers(vtkFetchMIQueryTermWidget::TagChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->QueryList->RemoveObservers(vtkFetchMIQueryTermWidget::QuerySubmittedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ResourceList->RemoveObservers(vtkFetchMIFlatResourceWidget::DeleteResourceEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ResourceList->RemoveWidgetObservers();
  this->TaggedDataList->RemoveObservers(vtkFetchMIResourceUploadWidget::TagSelectedDataEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->RemoveObservers(vtkFetchMIResourceUploadWidget::RemoveTagSelectedDataEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->RemoveObservers(vtkFetchMIResourceUploadWidget::ShowAllTagViewEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->RemoveObservers(vtkFetchMIResourceUploadWidget::UploadRequestedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TaggedDataList->RemoveWidgetObservers();
  this->ServerMenuButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->AddServerEntry->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
//  this->CloseNewServerButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->AddServerButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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
void vtkFetchMIGUI::WriteDocumentDeclaration_XND ( )
{
  const char *filename = this->Logic->GetXMLDocumentDeclarationFileName();
  if ( filename == NULL )
    {
    vtkErrorMacro ("WriteDocumentDeclaration_XND: got null filename" );
    return;
    }
  this->Script ( "FetchMIWriteDocumentDeclaration_XND \"%s\"",  filename );
}


//---------------------------------------------------------------------------
int vtkFetchMIGUI::WriteMetadataForUpload_XND (const char *nodeID )
{
  //  return 1 if ok, 0 if not.
  if ( this->FetchMINode == NULL) 
    {
    vtkErrorMacro ( "WriteMetadataForUpload_XND: FetchMINode is NULL.");
    return 0;
    }
  if (this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "WriteMetadataForUpload_XND has null MRMLScene." );
    return 0;        
    }
  const char *metadataFilename = Logic->GetXMLUploadFileName();
  if ( metadataFilename == NULL )
    {
    vtkErrorMacro ("WriteMetadataForUpload_XND: got null filename" );
    return 0;
    }
  const char *declarationFilename = Logic->GetXMLDocumentDeclarationFileName();
  if ( declarationFilename == NULL )
    {
    vtkErrorMacro ("WriteMetadataForUpload_XND: got null filename" );
    return 0;
    }


  if ( !(strcmp (nodeID, "MRMLScene" )))
    {
    this->Script ( "FetchMIWriteMetadataForScene_XND \"%s\" \"%s\"", metadataFilename, declarationFilename );
    }
  else
    {
    this->Script ( "FetchMIWriteMetadataForNode_XND \"%s\" \"%s\" \"%s\"", metadataFilename, declarationFilename, nodeID);
    }
  return 1;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIGUI::ParseMetadataPostResponse ( )
{
  
  if ( this->Logic->GetTemporaryResponseFileName() == NULL )
    {
    vtkErrorMacro ("ParseMetadataPostResponse got null filename to parse");
    return (NULL);
    }
  
  // method sets this->ReservedURI; clear it first.
  this->SetReservedURI ( NULL );
  const char *responseFilename = this->Logic->GetTemporaryResponseFileName();
  if ( responseFilename == NULL )
    {
    vtkErrorMacro ( "ParseMetadataPostResponse: got null filename" );
    return (NULL);
    }

  this->Script ( "FetchMIParseMetadataPostResponse_XND \"%s\"", responseFilename );
  const char * uri = this->GetReservedURI();
  return ( uri );
}




//---------------------------------------------------------------------------
void vtkFetchMIGUI::RequestUpload ( )
{
  //---
  //--- for a test, this method combines our original
  //--- Logic->RequestResourceUpload() and
  //--- Logic->RequestResourceUploadToXND().
  //--- In particular, it breaks the latter workhorse method
  //--- into chunks to take advantage of Tcl's UTF-8 char encoding.
  //---
  //--- We're letting gui get at tcl to do utf-8 char encoding
  //--- via kww Script method. 
  //--- use:
  //--- encoding convertfrom data (from other to utf-8)
  //--- encoding convertto data (from utf-8 to other)
  //---

  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "RequestUpload: MRMLScene is NULL.");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "RequestUpload: FetchMINode is NULL.");
    return;
    }
  if ( this->GetApplicationGUI() == NULL )
    {
    vtkErrorMacro ( "RequestUpload: ApplicationGUI is NULL ");
    }
  
  int retval;
  

  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( svr == NULL || svctype == NULL )
    {
    vtkErrorMacro ("RequestUpload: Null server or servicetype" );
    return;
    }

  
  //--- SAVE ORIGINAL SELECTION STATE
  //--- For now, override GUI selection state --
  //--- select everything, so we upload scene + all data.

  std::vector<std::string> tmpSelected;
  int tmp = this->Logic->GetSceneSelected();
  for ( int i=0; i< this->Logic->SelectedStorableNodeIDs.size(); i++)
    {
    tmpSelected.push_back(this->Logic->SelectedStorableNodeIDs[i] );
    }
      

  //--- SELECT ALL
  this->Logic->SceneSelected=1;
  this->Logic->SelectedStorableNodeIDs.clear();      
  const char *nodeID;
  const char *uri;
  for ( int i=0; i < this->TaggedDataList->GetNumberOfItems(); i++)
    {
    nodeID = this->TaggedDataList->GetNthDataTarget(i);
    if ( nodeID != NULL )
      {
      if ( (strcmp(nodeID, "Scene description" )))
        {
        this->Logic->SelectedStorableNodeIDs.push_back( nodeID );
        }
      }
    }
  

  if ( !(strcmp ("HID", svctype )) )
    {
    //no-op
    vtkWarningMacro("RequestUpload: HID upload not implemented yet.");
    }


  
  if ( !(strcmp ("XND", svctype )) )
    {
    vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
    if ( handler == NULL )
      {
      vtkErrorMacro ("RequestUpload: Null URIHandler. ");
    return;
      }
    //---
    //--- request upload
    //---
    // if nodes have been modified since read, prompt user  to save them first.
    retval = this->Logic->CheckStorageNodeFileNames();
    if ( retval == 0 )
      {
      //--- TODO: put up save-stuff message dialog
      return;
      }
    //--- explicitly set the cache filenames and the URI Handler to be XND
    //--- for all storables.
    this->Logic->SetCacheFileNamesAndXNDHandler(handler);

    //--- write the XML doc description and header info in utf-8 format.
    this->WriteDocumentDeclaration_XND();
    
    handler->SetHostName(svr);
    
    // add all tags in case they're not present. COMMENTED OUT TO DEMO

    std::map<std::string, std::vector<std::string> >::iterator iter;  
    for ( iter = this->Logic->AllValuesForAllTagsOnServer.begin();
          iter != this->Logic->AllValuesForAllTagsOnServer.end();
          iter++ )
      {
      if ( iter->first.c_str() != NULL &&  (strcmp (iter->first.c_str(), "" ) ) )
        {
        int returnval = handler->PostTag ( svr, iter->first.c_str(),this->Logic->GetTemporaryResponseFileName() );
        if ( returnval = 0 )
          {
          vtkErrorMacro ("vtkFetchMIGUI:RequestUpload: couldn't add new tag to server.");
          return;
          }
        //add do error checking... if there's a bug in Logic->GetTemporaryResponseFileName()
        }
      }
    
    //---
    //--- for each storable node:
    //--- generate metadata in utf-8 format (in gui)
    //--- post metadata (in logic)
    //--- parse metadata and set URIs (in gui)
    vtkMRMLStorableNode *storableNode;
    for (unsigned int n = 0; n < this->Logic->SelectedStorableNodeIDs.size(); n++)
      {
      std::string nodeID = this->Logic->SelectedStorableNodeIDs[n];
      vtkDebugMacro("RequestUpload: generating metadata for selected storable node " << nodeID.c_str());
      storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));
      // for each storage node
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      vtkMRMLStorageNode *storageNode;
      for (int i = 0; i < numStorageNodes; i++)
        {
        storageNode = storableNode->GetNthStorageNode(i);
        vtkDebugMacro("RequestUpload: have storage node " << i << ", calling write metadata for upload with id " << nodeID.c_str() << " and file name " << storageNode->GetFileName());

        //--- write header and metadata
        const char *snodeFileName = storageNode->GetFileName();
        vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName ( snodeFileName );
        const char *strippedFileName = vtkFileName.c_str();
        retval = this->WriteMetadataForUpload_XND(nodeID.c_str());
        if ( retval == 0 )
          {
          //TODO: error dialog 
          return;
          }

        //--- post and parse response
        this->Logic->PostMetadataToXND(handler, strippedFileName);
        uri = this->ParseMetadataPostResponse();
        if (uri == NULL)
          {
          storageNode->SetURI(NULL);
          storageNode->ResetURIList();
          this->Logic->DeselectNode(nodeID.c_str());
          // bail out of the rest of the storage nodes
          i = numStorageNodes;
          // for now, decrement the node number, since DeselectNode removes an
          // element from the list we're iterating over
          n--;
          }
        else
          {
          vtkDebugMacro("RequestUpload: parsed out a return metadatauri : " << uri);
          // then save it in the storage node
          storageNode->SetURI(uri);
        
          // now deal with the rest of the files in the storage node
          int numFiles = storageNode->GetNumberOfFileNames();

          for (int filenum = 0; filenum < numFiles; filenum++)
            {
            //--- write header and metadata
            const char *nodeFileName = storageNode->GetNthFileName(filenum);
            vtkFileName = vtksys::SystemTools::GetFilenameName ( nodeFileName );
            strippedFileName = vtkFileName.c_str();
            retval = this->WriteMetadataForUpload_XND(nodeID.c_str());
            if ( retval == 0 )
              {
              //TODO: error dialog 
              return;
              }

            //--- post and parse response
            this->Logic->PostMetadataToXND(handler, strippedFileName);
            const char *uri = this->ParseMetadataPostResponse();
            if (uri == NULL)
              {
              //--- TODO: clean up filenames now. they are set to cache path.
              vtkErrorMacro("RequestUpload:  error parsing uri from post meta data call for file # " << filenum); //, response = " << metadataResponse);
              storageNode->SetURI(NULL);
              storageNode->ResetURIList();
              vtkKWMessageDialog *message = vtkKWMessageDialog::New();
              message->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
              message->SetStyleToYesNo();
              std::string msg = "File " + std::string(storageNode->GetNthFileName(filenum)) + " unable to upload to remote host.\nDo you want to continue saving data?";
              message->SetText(msg.c_str());
              message->Create();
              int response = message->Invoke();
              if (response)
                {
                this->Logic->DeselectNode(nodeID.c_str());
                // for now, decrement the node number, since DeselectNode removes an
                // element from the list we're iterating over
                n--;
              
                // bail out of the rest of the files
                filenum = numFiles;
                }
              else
                {
                // bail out of the rest of the storage nodes
                i = numStorageNodes;
                }
              }
            else
              {
              vtkDebugMacro("RequestUpload: parsed out a return metadatauri : " << uri << ", adding it to storage node " << storageNode->GetID());
              // then save it in the storage node
              storageNode->AddURI(uri);
              }         
            }
          }
        }
      }

    //--- Now all storable nodes have their metadata written and uris set.

    //--- post data (in logic pass3)
    handler->SetHostName(svr);
    this->Logic->PostStorableNodesToXND();

    //--- post metadata (in logic)
    //--- parse metadata (in gui)

    if ( this->Logic->SceneSelected )
      {
      //--- explicitly write the scene to cache (uri already points to cache)
      this->MRMLScene->Commit();

      //--- write header and metadata
      const char *sceneFileName =this->GetMRMLScene()->GetURL();
      vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (  sceneFileName );
      const char *strippedFileName = vtkFileName.c_str();
      retval = this->WriteMetadataForUpload_XND("MRMLScene");
      if (retval == 0 )
        {
        //TODO: error dialog 
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent (this->GetApplicationGUI()->GetMainSlicerWindow() );
        dialog->SetStyleToMessage();
        std::string msg = "Error writing metadata for upload. Aborting upload.";
        dialog->SetText ( msg.c_str() );
        dialog->Create();
        dialog->Invoke();
        dialog->Delete();
        return;
        }
      //--- generate scene metatdata (in gui)
      this->Logic->PostMetadataToXND(handler, strippedFileName);
      uri = this->ParseMetadataPostResponse();
      if ( uri != NULL )
        {
        // set particular XND host in the XNDhandler
        handler->SetHostName(svr);
        handler->StageFileWrite(sceneFileName, uri);
        }
      else
        {
        vtkErrorMacro("RequestUpload: unable to parse out response from posting metadata for mrml scene, uri is null. ");
        return;
        }
      }

    //--- comment this out... and replace
    //--- with above for test Originally, this was only thing here.
    //this->Logic->RequestResourceUpload ( );
    // end XND
    } 


  //--- RESET SELECTION STATE
  this->Logic->SceneSelected = tmp;
  this->Logic->SelectedStorableNodeIDs.clear();
  for ( int i=0; i< tmpSelected.size(); i++)
    {
    this->Logic->SelectedStorableNodeIDs.push_back(tmpSelected[i] );
    }
}




//---------------------------------------------------------------------------
void vtkFetchMIGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  if ( this->FetchMINode == NULL )
    {
    //TODO: error macro
    return;
    }
  if ( this->Logic == NULL )
    {
    //TODO: error macro
    return;
    }

  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
  vtkFetchMIResourceUploadWidget *w = vtkFetchMIResourceUploadWidget::SafeDownCast ( caller );
  vtkFetchMIFlatResourceWidget *f = vtkFetchMIFlatResourceWidget::SafeDownCast ( caller );
  vtkFetchMIQueryTermWidget *q= vtkFetchMIQueryTermWidget::SafeDownCast ( caller );

  if ( f != NULL )
    {
    if ( (f == this->ResourceList) && (event == vtkFetchMIFlatResourceWidget::DeleteResourceEvent) )
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
      message->SetStyleToOkCancel();
      std::string msg = "This action will delete the selected resources from the selected server and is NOT undoable. \n Delete the selected resources?";
      message->SetText(msg.c_str());
      message->Create();
      int ok = message->Invoke();
      message->Delete(); 
      if (ok)
        {
        this->DeleteSelectedResourcesFromServer();
        }
      }
    }
  
  if ( w != NULL )
    {
    if ( (w== this->TaggedDataList) && (event == vtkFetchMIResourceUploadWidget::TagSelectedDataEvent) )
      {
      this->TagSelectedData();
      }
    if ( (w== this->TaggedDataList) && (event == vtkFetchMIResourceUploadWidget::RemoveTagSelectedDataEvent) )
      {
      this->RemoveTagFromSelectedData();
      }
    else if ( (w== this->TaggedDataList) && (event == vtkFetchMIResourceUploadWidget::ShowSelectionTagViewEvent) )
      {
      this->ShowSelectionTagView();
      }
    else if ( (w== this->TaggedDataList) && (event == vtkFetchMIResourceUploadWidget::ShowAllTagViewEvent) )
      {
      this->ShowAllTagView();
      }
    else if ( (w== this->TaggedDataList) && (event == vtkFetchMIResourceUploadWidget::UploadRequestedEvent) )
      {
      this->RequestUpload();
      }
    }
  if ( q != NULL )
    {
    if ( (q== this->QueryList) && (event == vtkFetchMIQueryTermWidget::TagChangedEvent) )
      {
      this->UpdateTagTableFromGUI();
      }
    if ( (q== this->QueryList) && (event == vtkFetchMIQueryTermWidget::QuerySubmittedEvent) )
      {
      this->ResourceList->DeleteAllItems();
      vtkTagTable *t = this->FetchMINode->GetResourceDescription();
      t->ClearTagTable();
      }    
    }


  if ( e && event == vtkKWEntry::EntryValueChangedEvent )
    {
    if (e == this->AddServerEntry )
      {
      if ( e->GetValue() != NULL )
        {
        this->FetchMINode->AddNewServer(e->GetValue());
        this->FetchMINode->SetServer ( this->ServerMenuButton->GetValue() );
        }
      }
    }
  if ( b && event == vtkKWPushButton::InvokedEvent )
    {
    if ( b == this->AddServerButton )
      {
      if ( this->GetAddServerEntry()->GetValue() != NULL )
        {
        this->FetchMINode->AddNewServer (this->GetAddServerEntry()->GetValue() );
        this->FetchMINode->SetServer ( this->ServerMenuButton->GetValue() );        
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
          //--- if server is same, just being re-selected,
          //--- set state variable in logic so that all a user's
          //--- tag selections can be restored once GUI is refreshed.
          if ( (this->FetchMINode->GetSelectedServer() != NULL) && (this->ServerMenuButton->GetValue() != NULL) )
            {
            if ( !strcmp (this->FetchMINode->GetSelectedServer(), this->ServerMenuButton->GetValue() ) )
              {
              this->Logic->SetRestoreTagSelectionState(1);
              }
            else
              {
              this->TaggedDataList->ResetCurrentTagLabel();
              }
            }
          if ( !(strcmp (this->ServerMenuButton->GetValue(), "Add new server" )))
            {
            this->RaiseNewServerWindow ();
            return;
            }

          this->FetchMINode->SetServer ( this->ServerMenuButton->GetValue() );

          //--- this queries server for tags
          vtkDebugMacro ("--------------------GUI event calling Query.");
          this->SetStatusText ( "Querying selected server for metadata (may take a little while)..." );
          this->Logic->QueryServerForTags();
          this->SetStatusText ( "Querying selected server for metadata (may take a little while)......." );
          this->Logic->QueryServerForTagValues( );
          this->SetStatusText ( "Querying selected server for metadata (may take a little while)....... done." );
          // TODO: temporary fix for HID which we are
          // not yet querying for available tags. Just
          // repopulate from default tags in FetchMINode
          this->SetStatusText ( "" );
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::SetStatusText(const char *txt)
{
  if ( this->GetApplicationGUI() )
    {
    if ( this->GetApplicationGUI()->GetMainSlicerWindow() )
      {
      this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
      }
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateResourceTableFromMRML ( )
{
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateResourceTableFromMRML got a NULL FetchMINode." );
    return;
    }
  if ( this->ResourceList == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateResourceTableFromMRML got a NULL ResourceList widget." );
    return;
    }

  //--- clear the widget
  this->ResourceList->DeleteAllItems();
  
  //--- update the Resource Table from the FetchMINode
  vtkTagTable *t = this->FetchMINode->GetResourceDescription();
  if ( t != NULL )
    {
    //--- see if we get this far ok.
    const char *uri;
    const char *val;
    int i, row;
    for (i=0; i < t->GetNumberOfTags(); i++ )
      {
      uri = t->GetTagAttribute(i);
      val = t->GetTagValue(i);
      this->ResourceList->AddNewItem (uri, val);
      row = this->ResourceList->GetRowForURI ( uri );
      if ( row >= 0 && (t->IsTagSelected(uri)) )
        {
        this->ResourceList->SelectRow(row);              
        }
      }
    }
}





//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateSceneTableFromMRML()
{

  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateSceneTableFromMRML got a NULL FetchMINode." );
    return;
    }
  if ( this->TaggedDataList == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateSceneTableFromMRML got a NULL TaggedDataList widget." );
    return;
    }

  this->TaggedDataList->DeleteAllItems();
  this->Logic->ClearModifiedNodes();
  this->AddMRMLSceneRow();
  this->AddVolumeNodes();
  this->AddModelNodes();
  this->AddUnstructuredGridNodes();
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::AddVolumeNodes()
{

  vtkMRMLNode *node;
  vtkMRMLStorableNode *stnode;
  vtkTagTable *t;
  
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  int n;
  int row = this->TaggedDataList->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  const char *dtype;
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLVolumeNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = vnode->GetStorageNode();
    //--- if a storage node doesn't yet exist for the node, add it.
    if (snode == NULL) 
      {
      vtkMRMLStorageNode *storageNode;
      if ( vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(node) || 
            vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(node) )
        {
        storageNode = vtkMRMLNRRDStorageNode::New();
        }
      else
        {
        storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
        }
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      vnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }

    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) 
      {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      name += std::string(".nrrd");
      snode->SetFileName(name.c_str());
      }

    // get absolute filename
    std::string name;
    if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->MRMLScene->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();

    // Set the SlicerDataType
    dtype = "Volume";
    vtkMRMLScalarVolumeNode *vsnode = vtkMRMLScalarVolumeNode::SafeDownCast (vnode );
    stnode = vtkMRMLStorableNode::SafeDownCast ( vsnode );
    vtkMRMLDiffusionTensorVolumeNode *dtinode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast (vnode );
    vtkMRMLDiffusionWeightedVolumeNode *dwinode = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast (vnode);
    if ( vsnode != NULL )
      {
      if ( vsnode->GetLabelMap() )
        {
        dtype = "LabelMap";
        }
      else
        {
        dtype = "ScalarVolume";
        }
      }
    if ( dtinode != NULL )
      {
      dtype = "DTIVolume";
      }
    if ( dwinode != NULL )
      {
      dtype = "DWIVolume";
      }
    
    this->TaggedDataList->AddNewItem ( node->GetID(), dtype );

    //--- store node's slicerdatatype in its UserTagTable
    if ( stnode != NULL && (strcmp(dtype, "")) )
      {
      t = stnode->GetUserTagTable();
      if ( t != NULL )
        {
        t->AddOrUpdateTag ( "SlicerDataType", dtype, 1 );
        }
      }
    
    if (node->GetModifiedSinceRead()) 
      {
      this->TaggedDataList->SelectRow ( row );
      this->Logic->AddModifiedNode(node->GetID());
      //--- TODO: can we get rid of this?
      //--- this should be getting called by the TaggedDataList when SelectRow
      //--- triggers a widget event...
      this->Logic->AddSelectedStorableNode(node->GetID() );
      }
    row++;
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::AddModelNodes()
{
  vtkMRMLNode *node;
  vtkMRMLStorableNode *stnode;
  vtkTagTable *t;
  
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  int n;
  int row = this->TaggedDataList->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  const char *dtype;
  
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLModelNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    stnode = vtkMRMLStorableNode::SafeDownCast ( node );
    vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = mnode->GetStorageNode();
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    if (snode == NULL && node->GetModifiedSinceRead()) 
      {
      vtkMRMLModelStorageNode *storageNode = vtkMRMLModelStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      mnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }

    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->MRMLScene->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();
    
    // Set the SlicerDataType
    vtkMRMLFreeSurferModelStorageNode *fsnode = vtkMRMLFreeSurferModelStorageNode::SafeDownCast (snode);
    if ( fsnode != NULL )
      {
      if ( snode->IsA("vtkMRMLFreeSurferModelStorageNode") )
        {
        dtype = "FreeSurferModel";
        }
      }
    else
      {
      dtype = "VTKModel";
      }


    this->TaggedDataList->AddNewItem ( node->GetID(), dtype );

    //--- store node's slicerdatatype in its UserTagTable.
    if ( stnode != NULL && (strcmp(dtype, "")) )
      {
      t = stnode->GetUserTagTable();
      if ( t != NULL )
        {
        t->AddOrUpdateTag ( "SlicerDataType", dtype, 1 );
        }
      }

    if (node->GetModifiedSinceRead()) 
      {
      this->TaggedDataList->SelectRow ( row );
      this->Logic->AddModifiedNode(node->GetID());
      //--- TODO: can we get rid of this?
      //--- this should be getting called by the TaggedDataList when SelectRow
      //--- triggers a widget event...
      this->Logic->AddSelectedStorableNode(node->GetID() );      
      }
    row++;
    }
}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::AddUnstructuredGridNodes()
{
  //--- UNSTRUCTURED GRID NODES
#if !defined(MESHING_DEBUG) && defined(Slicer3_BUILD_MODULES)  
  // *** add UnstructuredGrid types 
  // An additional datatype, MRMLUnstructuredGrid and its subclasses are 
  // also searched in the MRML tree.  This is done so instances of FiniteElement
  // meshes and other vtkUnstructuredGrid datatypes can be stored persistently.
  // this code is gated by MESHING_DEBUG since the MEshing MRML modules 
  
  vtkMRMLNode *node;
  vtkMRMLStorableNode *stnode;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLUnstructuredGridNode");
  int n;
  int row = this->TaggedDataList->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  const char *dtype;
  
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLUnstructuredGridNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    stnode = vtkMRMLStorableNode::SafeDownCast ( node );
    vtkMRMLUnstructuredGridNode *gnode = vtkMRMLUnstructuredGridNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = gnode->GetStorageNode();
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    if (snode == NULL && node->GetModifiedSinceRead()) 
      {
        vtkMRMLUnstructuredGridStorageNode *storageNode = vtkMRMLUnstructuredGridStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      gnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }
    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->MRMLScene->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();


    dtype = "UnstructuredGrid";
    this->TaggedDataList->AddNewItem ( node->GetID(), dtype );
    //--- store node's slicerdatatype in its UserTagTable.
    if ( stnode != NULL && (strcmp(dtype, "")) )
      {
      t = stnode->GetUserTagTable();
      if ( t != NULL )
        {
        t->AddOrUpdateTag ( "SlicerDataType", dtype, 1 );
        }
      }

    if (node->GetModifiedSinceRead()) 
      {
      this->TaggedDataList->SelectRow(row);
      this->Logic->AddModifiedNode(node->GetID());
      //--- TODO: can we get rid of this?
      //--- this should be getting called by the TaggedDataList when SelectRow
      //--- triggers a widget event...
      this->Logic->AddSelectedStorableNode(node->GetID() );      
      }
    row++;
    }
    // end of UGrid MRML node processing
#endif  
}




//---------------------------------------------------------------------------
void vtkFetchMIGUI::AddMRMLSceneRow()
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: AddMRMLSceneRow got a NULL MRMLScene.");
    return;
    }


  std::string dir = this->MRMLScene->GetRootDirectory();
  if (dir[dir.size()-1] != '/')
    {
    dir += std::string("/");
    }
  this->SetDataDirectoryName ( dir.c_str());
  
  //--- make sure the scene has a selected SlicerDataType tag.
  this->MRMLScene->GetUserTagTable()->AddOrUpdateTag ( "SlicerDataType", "MRML", 0);

  std::string uriName;
  const char *url = this->MRMLScene->GetURL();
  if (!url || !(*url))
    {
    uriName = dir.append("SlicerScene1");
    }
  else
    {
    uriName = url;
    }

  if(!uriName.empty())
    {
    //--- put a row in the TaggedDataList with selected, datatype, and url.
    this->TaggedDataList->AddNewItem ( "Scene description", "MRML");
    this->TaggedDataList->SelectRow ( 0 );
    }
}




//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateTagTableFromGUI ( )
{
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateTagTableFromGUI got a NULL FetchMINode." );
    return;
    }
  if ( this->QueryList == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateTagTableFromGUI got a NULL QueryList widget." );
    return;
    }
  
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if (svctype == NULL)
    {
    return;
    }

  int num = this->QueryList->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  std::string att;
  std::string val;
  int sel;
  //--- update the FetchMINode, depending on what service is selected.
  if ( !strcmp ( "XND", svctype ))
    {
    vtkXNDTagTable *t;
    if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ) != NULL)
      {
      t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetSelectedTagTable() );
//      t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDDTags" ));
      if ( t == NULL )
        {
        // TODO: vtkErrorMacro
        return;
        }
      for ( int i=0; i < num; i++ )
        {
        att = this->QueryList->GetAttributeOfItem ( i );
        val = this->QueryList->GetValueOfItem ( i );
        sel = this->QueryList->IsItemSelected ( i );
        t->AddOrUpdateTag ( att.c_str(), val.c_str(), sel );
        }
      }
    }
  else if ( !strcmp ( "HID", svctype))
    {
    vtkHIDTagTable *t;
    if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ) != NULL)
      {
      t = vtkHIDTagTable::SafeDownCast ( this->FetchMINode->GetSelectedTagTable() );
//      t = vtkHIDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
      if ( t == NULL )
        {
        // TODO: vtkErrorMacro
        return;
        }
      for ( int i=0; i < num; i++ )
        {
        att = this->QueryList->GetAttributeOfItem ( i );
        val = this->QueryList->GetValueOfItem ( i );
        if ( this->QueryList->IsItemSelected ( i ) )
          {
          t->AddOrUpdateTag ( att.c_str(), val.c_str(), 1 );
          }
        else
          {
          t->AddOrUpdateTag ( att.c_str(), val.c_str(), 0 );
          }
        }
      }
    }
}





//---------------------------------------------------------------------------
void vtkFetchMIGUI::UpdateTagTableFromMRML ( )
{

  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateTagTableFromMRML got a NULL FetchMINode." );
    return;
    }
  if ( this->QueryList == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: UpdateTagTableFromMRML got a NULL QueryList widget." );
    return;
    }
  
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if (svctype == NULL)
    {
    return;
    }
    

  //--- now restore user's selection state for all tags.
  if ( !strcmp ( "XND", svctype ))
    {
    if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ) != NULL)
      {
//      vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
      vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetSelectedTagTable() );      
      if ( t != NULL )
        {
        const char *att;
        int i, row;
        for (i=0; i < t->GetNumberOfTags(); i++ )
          {
          att = t->GetTagAttribute(i);
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
//      vtkHIDTagTable *t = vtkHIDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
      vtkHIDTagTable *t = vtkHIDTagTable::SafeDownCast ( this->FetchMINode->GetSelectedTagTable() );
      if ( t != NULL )
        {
        const char *att;
        int i, row;
        for (i=0; i < t->GetNumberOfTags(); i++ )
          {
          att = t->GetTagAttribute(i);
          row = this->QueryList->GetRowForAttribute ( att );
          if ( row >= 0 && (t->IsTagSelected(att)) )
            {
            this->QueryList->SelectRow(row);              
            }
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::PopulateQueryListFromServer()
{
  //--- This flag causes QueryList to hold off on invoking any
  //--- events (via the FetchMINode that cause the GUI to
  //--- update. We do this once at the end.
  this->QueryList->SetInPopulateWidget(1);
  this->QueryList->PopulateFromServer();
  if ( this->Logic->GetRestoreTagSelectionState() )
    {
    //--- This means that the same server has been
    //--- selected by user, causing the list of available
    //--- tags for query to be refreshed. If possible, we
    //--- want to restore the 'selected' status of tags
    //--- and the values that users had for each.
    //---
    //--- NOTE: this is not essential, but seems friendly... so try it out.
    //--- Check to see if previously selected value for each tag
    //--- (stored in node's tagtable) is in the new list of known
    //--- values for tag, just populated in Logic's AllValuesForAllTagsOnServer.
    //--- If so, select it. If this behavior is annoying, then just comment
    //--- out this block.
    //---
    this->RestoreSelectedValuesForTagsFromMRML();
    }
  //--- Finally, make sure the restore flag off.
  this->Logic->SetRestoreTagSelectionState(0);  
  //--- And the populate widget flag is off
  this->QueryList->SetInPopulateWidget(0);
}

//----------------------------------------------------------------------------
void vtkFetchMIGUI::RestoreSelectedValuesForTagsFromMRML()
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "RestoreSelectedValuesForTags: got NULL FetchMINode");
    return;
    }
  if ( this->FetchMINode->GetTagTableCollection() == NULL )
    {
    vtkErrorMacro ( "RestoreSelectedValuesForTags: got NULL TagTableCollection in FetchMINode");
    return;
    }


  //--- check node's tag table and see what values were stored for each attribute.
  //--- Then, go thru AllValuesForAllTagsOnServer and select the value if it's present.
  std::map<std::string, std::vector<std::string> >::iterator iter;
  if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ) != NULL)
    {
//    vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
      vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetSelectedTagTable() );    
    if ( t != NULL )
      {
      const char *att;
      const char *val;
      int i, j;
      int numTags = t->GetNumberOfTags();
      int numValues = 0;
      int widgetRow = 0;
      // For each tag in mrml, get the attribute, and value.
      for (i=0; i < numTags; i++ )
        {
        att = t->GetTagAttribute(i);
        val = t->GetTagValue(i);
        //--- Search AllValuesForAllTagsOnServer for the attribute.
        if ( att != NULL && val != NULL )
          {
          for ( iter = this->Logic->AllValuesForAllTagsOnServer.begin();
                iter != this->Logic->AllValuesForAllTagsOnServer.end();
                iter++ )
            {
            if ( ! (strcmp (iter->first.c_str(), att ) ) )
              {
              //--- Search thru attributes values to see if the value is present.
              int numValues = iter->second.size();
              for (j=0; j< numValues; j++ )
                {
                if ( !(strcmp(val, iter->second[j].c_str()) ) )
                  {
                  //--- Value is present for tag. Find the corresponding
                  //--- row in the GUI, and select the value.
                  widgetRow = this->QueryList->GetRowForAttribute ( att );
                  this->QueryList->SelectValueOfItem(widgetRow, val);
                  if (t->IsTagSelected(att) )
                    {
                    this->QueryList->SelectRow(widgetRow);
                    }
                  }
                }
              }
            }
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
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ("FetchMIGUI: ProcessMRMLEvents has a NULL FetchMINode");
    return;
    }

  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLFetchMINode* node = vtkMRMLFetchMINode::SafeDownCast(caller);

  if ( this->ApplicationGUI == NULL )
    {
    //TODO vtkErrorMacro();
    return;    
    }
  
  if (event == vtkMRMLFetchMINode::SelectedServerModifiedEvent )
    {
    if ( (strcmp (this->FetchMINode->GetSelectedServer(), this->ServerMenuButton->GetValue() ) ) )
      {
      this->ServerMenuButton->SetValue ( this->FetchMINode->GetSelectedServer() );
      }
    }

  if ( event == vtkMRMLScene::SceneCloseEvent )
    {
    this->Logic->ClearModifiedNodes();
    this->Logic->ClearSelectedStorableNodes();
    this->UpdateSceneTableFromMRML();
    }

  if (( event == vtkMRMLScene::NodeAddedEvent) ||
      ( event == vtkMRMLScene::NodeRemovedEvent) ||
      ( event == vtkMRMLScene::NewSceneEvent ))
    {
    this->UpdateSceneTableFromMRML();
    }
  
  else if (node != NULL && this->GetFetchMINode() == node) 
    {
    if ( event == vtkMRMLFetchMINode::RemoteIOErrorEvent )
      {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
          dialog->SetStyleToMessage();
          dialog->SetText ( this->FetchMINode->GetErrorMessage() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
      }
    if (event == vtkMRMLFetchMINode::TagResponseReadyEvent )
      {
      //--- check for error
      if ( this->FetchMINode->GetErrorMessage() != NULL )
        {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
          dialog->SetStyleToMessage();
          dialog->SetText ( this->FetchMINode->GetErrorMessage() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
        }
      vtkDebugMacro ("--------------------Populating the Query Widget");
      this->PopulateQueryListFromServer();
      }
    if (event == vtkMRMLFetchMINode::ResourceResponseReadyEvent )
      {
      //--- check for error
      if ( this->FetchMINode->GetErrorMessage() != NULL )
        {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
          dialog->SetStyleToMessage();
          dialog->SetText ( this->FetchMINode->GetErrorMessage() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
        }
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
      //TODO: hook up these commands!
      this->ServerMenuButton->GetMenu()->AddSeparator();
      this->ServerMenuButton->GetMenu()->AddRadioButton("Add new server");
      
      //--- select active server in the ServerMenuButton
      if ( this->FetchMINode->GetSelectedServer() != NULL )
        {
        this->ServerMenuButton->SetValue ( this->FetchMINode->GetSelectedServer() );
        }
      else
        {
        this->ServerMenuButton->SetValue ( "none" );
        }
      }
    
    }
  else
    {
    vtkErrorMacro ("FetchMIGUI: UpdateGUI has a NULL FetchMINode." );
    }
  // n->Delete();
  this->UpdateTagTableFromMRML();
  this->UpdateSceneTableFromMRML();
  this->UpdatingGUI = 0;
}





//---------------------------------------------------------------------------
void vtkFetchMIGUI::DestroyNewServerWindow( )
{
  if ( !this->NewServerWindow )
    {
    return;
    }
  if ( !this->NewServerWindow->IsCreated() )
    {
    vtkErrorMacro ("DestroyNewServerWindow: NewServerWindow is not created.");
    return;
    }
  this->NewServerWindow->Withdraw();
  this->UnBindNewServerWindow();

  if ( this->AddServerEntry )
    {
    this->AddServerEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->AddServerEntry->SetParent ( NULL );
    this->AddServerEntry->Delete();
    this->AddServerEntry = NULL;
    }
  if ( this->AddServerButton )
    {
    this->AddServerButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddServerButton->SetParent ( NULL );
    this->AddServerButton->Delete();
    this->AddServerButton = NULL;
    }

  if ( this->CloseNewServerButton )
    {
    this->CloseNewServerButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->CloseNewServerButton->SetParent ( NULL );
    this->CloseNewServerButton->Delete();    
    this->CloseNewServerButton = NULL;
    }
  this->NewServerWindow->Delete();
  this->NewServerWindow = NULL;
}



//---------------------------------------------------------------------------
void vtkFetchMIGUI::BindNewServerWindow ( )
{
  if (!this->CloseNewServerButton )
    {
    return;
    }
  if ( this->CloseNewServerButton->IsCreated() )
    {
    this->CloseNewServerButton->SetBinding ( "<ButtonPress>", this, "DestroyNewServerWindow" );
    }

}

//---------------------------------------------------------------------------
void vtkFetchMIGUI::UnBindNewServerWindow ( )
{

  if ( !this->CloseNewServerButton )
    {
    return;
    }
  if (this->CloseNewServerButton->IsCreated() )
    {
    this->CloseNewServerButton->RemoveBinding ( "<ButtonPress>" );
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::RaiseNewServerWindow()
{
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ("RaiseNewServerWindow: Got NULL Logic.");
    return;
    }
  if ( this->Logic->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("RaiseNewServerWindow: Got NULL FetchMINode");
    return;
    }

  this->DestroyNewServerWindow();

  int px, py;
  //--- top level container.
  this->NewServerWindow = vtkKWTopLevel::New();
  this->NewServerWindow->SetMasterWindow (this->GetServerMenuButton() );
  this->NewServerWindow->SetApplication ( this->GetApplication() );
  this->NewServerWindow->Create();
  vtkKWTkUtilities::GetWidgetCoordinates(this->GetServerMenuButton(), &px, &py);
  this->NewServerWindow->SetPosition ( px + 10, py + 10) ;
  this->NewServerWindow->SetBorderWidth ( 1 );
  this->NewServerWindow->SetReliefToFlat();
  this->NewServerWindow->SetTitle ("Add a new (XNAT Desktop only) server");
  this->NewServerWindow->SetSize (450, 75);
  this->NewServerWindow->Withdraw();
  this->NewServerWindow->SetDeleteWindowProtocolCommand ( this, "DestroyNewServerWindow");

  vtkKWFrame *f1 = vtkKWFrame::New();
  f1->SetParent ( this->NewServerWindow );
  f1->Create();
  f1->SetBorderWidth ( 1 );
  this->Script ( "pack %s -side top -anchor nw -fill x -expand n -padx 0 -pady 1", f1->GetWidgetName() );

  //--- new tag entry
  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent (f1);
  l1->Create();
  l1->SetText ( "Server:" );
  l1->SetWidth ( 12 );
  this->AddServerEntry = vtkKWEntry::New();
  this->AddServerEntry->SetParent ( f1 );
  this->AddServerEntry->Create();
  this->AddServerEntry->SetWidth(20);
  this->AddServerEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AddServerButton = vtkKWPushButton::New();
  this->AddServerButton->SetParent ( f1);
  this->AddServerButton->Create();
  this->AddServerButton->SetReliefToFlat();
  this->AddServerButton->SetBorderWidth ( 0 );
  this->AddServerButton->SetImageToIcon ( this->FetchMIIcons->GetAddNewIcon() );
  this->AddServerButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand);

  this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", l1->GetWidgetName() );
  this->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 2", this->AddServerEntry->GetWidgetName() );
  this->Script ( "grid %s -row 0 -column 2 -sticky ew -padx 2 -pady 2", this->AddServerButton->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 0 -weight 0", f1->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 1 -weight 1", f1->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 2 -weight 0", f1->GetWidgetName() );

  //--- close button (destroys win and widgets when closed.
  //--- TODO: this is causing an error when window is destroyed.
  //--- I think because the button is destroyed while the binding
  //--- to it is still active. Not sure how to fix, so leaving out the
  //--- close button for now; window can be closed using the 'x'
  //--- in the title bar.
/*
  this->CloseNewServerButton = vtkKWPushButton::New();
  this->CloseNewServerButton->SetParent ( f3 );
  this->CloseNewServerButton->Create();
  this->CloseNewServerButton->SetText ( "close" );
  this->Script ( "pack %s -side top -anchor c  -expand n -padx 2 -pady 6",
                 this->CloseNewServerButton->GetWidgetName() );
  this->CloseNewServerButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand);
*/

  this->BindNewServerWindow();
  f1->Delete();
  l1->Delete();

  //-- display
  this->NewServerWindow->DeIconify();
  this->NewServerWindow->Raise();
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::ShowSelectionTagView()
{
  if ( this->MRMLScene == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->ResourceList == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->ApplicationGUI == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }

  if ( this->TagViewer == NULL )
    {
    int px, py;
    this->TagViewer = vtkFetchMITagViewWidget::New();
    this->TagViewer->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
    this->TagViewer->Create();
    vtkKWTkUtilities::GetWidgetCoordinates(this->GetServerMenuButton(), &px, &py);
    this->TagViewer->GetTagViewWindow()->SetPosition ( px + 10, py + 10) ;
    this->TagViewer->SetTagTitle ("Tags for scene and all data:");
    }

  std::stringstream ss;
  vtkMRMLStorableNode *node;
  vtkTagTable *t;

  //--- figure out the text
//  int dnum = this->TaggedDataList->GetNumberOfSelectedItems();
  int dnum = this->TaggedDataList->GetNumberOfItems();
  int i, j;
  int numtags;
  const char *nodeID;
  const char *att;
  const char *val;
  for (i=0; i<dnum; i++)
    {
//    nodeID = this->TaggedDataList->GetNthSelectedDataTarget(i);
    nodeID = this->TaggedDataList->GetMultiColumnList()->GetWidget()->GetCellText(i,3);
    if ( nodeID != NULL )
      {
      //--- get tags on the data.
      ss << "\n";
      ss << "**";
      ss << nodeID;
      ss << ":**\n";
      if ( !(strcmp (nodeID, "Scene description")))
        {
        t = this->MRMLScene->GetUserTagTable();
        }
      else
        {
        node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNodeByID(nodeID));
        if ( node != NULL )
          {
          t = node->GetUserTagTable();
          }
        }
      if ( t != NULL )
        {
        numtags = t->GetNumberOfTags();
        for ( j=0; j <numtags; j++)
          {
          att = t->GetTagAttribute(j);
          val = t->GetTagValue(j);
          if ( att!= NULL && val != NULL )
            {
            ss << att;
            ss << " = ";
            ss << val;
            ss << "\n";
            }
          }
        }
      }
    }
  
  this->TagViewer->SetTagText ( ss.str().c_str() );
  this->TagViewer->DisplayTagViewWindow();
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::ShowAllTagView()
{
  if ( this->MRMLScene == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->ResourceList == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->ApplicationGUI == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }

  if ( this->TagViewer == NULL )
    {
    int px, py;
    this->TagViewer = vtkFetchMITagViewWidget::New();
    this->TagViewer->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
    this->TagViewer->Create();
    vtkKWTkUtilities::GetWidgetCoordinates(this->GetServerMenuButton(), &px, &py);
    this->TagViewer->GetTagViewWindow()->SetPosition ( px + 10, py + 10) ;
    this->TagViewer->SetTagTitle ("Tags for scene and all data:");
    }

  std::stringstream ss;
  vtkMRMLStorableNode *node;
  vtkTagTable *t = NULL;

  //--- figure out the text
//  int dnum = this->TaggedDataList->GetNumberOfSelectedItems();
  int dnum = this->TaggedDataList->GetNumberOfItems();
  int i, j;
  int numtags;
  const char *nodeID;
  const char *att;
  const char *val;
  for (i=0; i<dnum; i++)
    {
//    nodeID = this->TaggedDataList->GetNthSelectedDataTarget(i);
    nodeID = this->TaggedDataList->GetMultiColumnList()->GetWidget()->GetCellText(i,4);
    if ( nodeID != NULL )
      {
      //--- get tags on the data.
      ss << "\n";
      ss << "**";
      ss << nodeID;
      ss << ":**\n";
      if ( !(strcmp (nodeID, "Scene description")))
        {
        t = this->MRMLScene->GetUserTagTable();
        }
      else
        {
        node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNodeByID(nodeID));
        if ( node != NULL )
          {
          t = node->GetUserTagTable();
          }
        }
      if ( t != NULL )
        {
        numtags = t->GetNumberOfTags();
        for ( j=0; j <numtags; j++)
          {
          att = t->GetTagAttribute(j);
          val = t->GetTagValue(j);
          if ( att!= NULL && val != NULL )
            {
            ss << att;
            ss << " = ";
            ss << val;
            ss << "\n";
            }
          }
        }
      }
    }
  
  this->TagViewer->SetTagText ( ss.str().c_str() );
  this->TagViewer->DisplayTagViewWindow();

}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::DeleteSelectedResourcesFromServer()
{
  int deleteError = 0;

  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ("vtkFetchMIGUI:DeleteSelectedResourcesFromServer got NULL MRMLScene" );
    return;
    }
  if ( this->ResourceList == NULL)
    {
    vtkErrorMacro ("vtkFetchMIGUI:DeleteSelectedResourcesFromServer got NULL ResourceList" );
    return;
    }
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ("vtkFetchMIGUI:DeleteSelectedResourcesFromServer got NULL Logic" );
    return;
    }
  
  //--- This method loops through all resources selected in the ResourceList.
  //--- For each selected resource, the DeleteResourceFromServer method on the Logic
  //--- is called, which in turn requests the DeleteResource method on the appropriate handler.
  //--- Then handler returns 0 if an error occurs, and the logic passes this back.
  //--- For each resource successfully deleted, the ResourceList is updated; and if any
  //--- resource deletion causes an error, a message dialog is posted warning the user
  //--- that not all resources may have been deleted.
  int num = this->ResourceList->GetNumberOfSelectedItems();
  // loop over all selected data; 
  int retval = 1;
  int r;
  std::string deleteURI;
  for (int i=(num-1); i>=0; i-- )
    {
    if ( this->ResourceList->GetNthSelectedURI(i) != NULL && strcmp(this->ResourceList->GetNthSelectedURI(i), "" ) )
      {
      deleteURI.clear();
      deleteURI =  this->ResourceList->GetNthSelectedURI(i);
      retval = this->Logic->DeleteResourceFromServer ( deleteURI.c_str() );
      if ( retval != 1 )
        {
        deleteError = 1;
        }
      else
        {
        // update the ResourceList: find the row for this uri and delete the row.
        r = this->ResourceList->GetRowForURI( deleteURI.c_str() );
        if ( r >= 0 )
          {
          this->ResourceList->GetMultiColumnList()->GetWidget()->DeleteRow ( r );
          }
        }
      }
    }
  
  if ( deleteError )
    {
    //Pop up error message; not all resources were deleted.
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
    dialog->SetStyleToMessage();
    dialog->SetText ("Warning: there was a problem deleting some of the selected resources; please refresh your query to determine the resources' status.");
    dialog->Create();
    dialog->Invoke();
    dialog->Delete();
    return;
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::RemoveTagFromSelectedData()
{
  if ( this->MRMLScene == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->QueryList == NULL)
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->ResourceList == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  
  int j;
  int dnum;
  vtkMRMLStorableNode *node;
  vtkTagTable *t;
  std::string att = this->TaggedDataList->GetCurrentTagAttribute();
  std::string val = this->TaggedDataList->GetCurrentTagValue();

  if ( (att.c_str() != NULL) && (val.c_str() != NULL) &&
       (strcmp(att.c_str(), "")) && (strcmp(val.c_str(), "")) )
    {
    //--- apply to all selected data in TaggedDataList
    dnum = this->TaggedDataList->GetNumberOfSelectedItems();
    const char *nodeID;
    for (j=0; j<dnum; j++)
      {
      nodeID = this->TaggedDataList->GetNthSelectedDataTarget(j);
      if ( nodeID != NULL )
        {
        if ( !(strcmp(att.c_str(), "SlicerDataType")) )
          {
          //--- can't delete Slicer data type from data.
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetApplicationGUI()->GetMainSlicerWindow() );
          dialog->SetStyleToMessage();
          dialog->SetText ("Slicer requires data to have a SlicerDataType tag; it cannot be deleted.");
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
          return;
          }
        if ( !(strcmp (nodeID, "Scene description")))
          {
          //--- want to delete tag on the scene 
          t = this->MRMLScene->GetUserTagTable();
          }
        else
          {
          //--- want to delete tag on the storable node.
          node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNodeByID(nodeID));
          if ( node != NULL )
            {
            t = node->GetUserTagTable();
            }        
          }          
        //--- delete tag if it's in the tagtable.
        if ( t != NULL )
          {
          if (t->CheckTableForTag(att.c_str() ) >= 0 )
            {
            t->DeleteTag ( att.c_str() );
            }
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::TagSelectedData()
{
  if ( this->MRMLScene == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }
  if ( this->ResourceList == NULL )
    {
    //TODO vtkErrorMacro();
    return;
    }

  int j;
  int dnum;
  vtkMRMLStorableNode *node;
  vtkTagTable *t;
  std::string att = this->TaggedDataList->GetCurrentTagAttribute();
  std::string val = this->TaggedDataList->GetCurrentTagValue();

  if ( (att.c_str() != NULL) && (val.c_str() != NULL) &&
       (strcmp(att.c_str(), "")) && (strcmp(val.c_str(), "")) )
    {
    //--- apply to all selected data in TaggedDataList
    dnum = this->TaggedDataList->GetNumberOfSelectedItems();
    const char *nodeID;
    for (j=0; j<dnum; j++)
      {
      nodeID = this->TaggedDataList->GetNthSelectedDataTarget(j);
      if ( nodeID != NULL )
        {
        //--- tag the scene
        if ( !(strcmp (nodeID, "Scene description")))
          {
          t = this->MRMLScene->GetUserTagTable();
          if ( t != NULL )
            {
            //--- add current tag
            t->AddOrUpdateTag ( att.c_str(), val.c_str(), 1 );
            //--- enforce this tag on the scene.
            t->AddOrUpdateTag ( "SlicerDataType", "MRML", 1 );
            }
          }
        else
          {
          //--- or tag the storable node.
          node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNodeByID(nodeID));
          if ( node != NULL )
            {
            t = node->GetUserTagTable();
            if ( t != NULL )
              {
              t->AddOrUpdateTag ( att.c_str(), val.c_str(), 1 );
              }
            }
          }
        }
      }
    }
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
  const char *help = "FetchMI (Medical Informatics) help.\n\n *** Select a server\n\n  *** Query the server for tags. If server has any defined,  it'll fill up the top listbox\n\n *** You can add attributes for tags in the top listbox\n\n *** Select the tags you want to use in your query\n\n *** Click the spyglass to search the server for matching resources.\n\n *** Get a list of resources back and they're parsed and listed in the second listbox.\n\n *** Select a MRML scene file from that list\n\n *** And click download.\n\n \n\n ***The bottom listbox initializes with all data in scene (and scene file) just like the savedatawidget.\n\n ***Each time a node added or deleted event occurs, it updates\n\n ***user selects tags in top box, and applies them to selected datasets in bottom box using the Apply tags button. Tags are preserved in node and MRML scene.\n\n ***user can click tag-view icon to show all tags on any individual dataset or scene in a popup widget.\n\n *** user can upload selected data or scene to selected server. (in our first pass, the scene and all nodes).";

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
  this->ServerMenuButton->SetValue ( "none" );

  
  this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", l1->GetWidgetName() );
  this->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 2", this->ServerMenuButton->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 0 -weight 0", serverFrame->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 1 -weight 1", serverFrame->GetWidgetName() );
  
  // Query Frame
  vtkSlicerModuleCollapsibleFrame *queryFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  queryFrame->SetParent(page);
  queryFrame->Create();
  queryFrame->SetLabelText("Query Web Services for Data");
  queryFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    queryFrame->GetWidgetName(), page->GetWidgetName());

  // Download Frame
  vtkSlicerModuleCollapsibleFrame *resourceFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  resourceFrame->SetParent(page);
  resourceFrame->Create();
  resourceFrame->SetLabelText("Browse Query Results & Download (Scenes only)");
  resourceFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    resourceFrame->GetWidgetName(), page->GetWidgetName());

  // Tag & Upload Frame
  vtkSlicerModuleCollapsibleFrame *descriptionFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  descriptionFrame->SetParent(page);
  descriptionFrame->Create();
  descriptionFrame->SetLabelText("Describe Data & Upload (Scenes+data only)");
  descriptionFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    descriptionFrame->GetWidgetName(), page->GetWidgetName());

  this->QueryList = vtkFetchMIQueryTermWidget::New();
  this->QueryList->SetParent ( queryFrame->GetFrame() );
  this->QueryList->Create();
  this->QueryList->SetApplication ( app );
  this->QueryList->SetLogic ( this->Logic );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->QueryList->GetWidgetName() );

  this->ResourceList = vtkFetchMIFlatResourceWidget::New();
  this->ResourceList->SetParent ( resourceFrame->GetFrame() );
  this->ResourceList->Create();
  this->ResourceList->SetApplication ( app );
  this->ResourceList->SetLogic ( this->Logic );
  this->ResourceList->SetMRMLScene ( this->MRMLScene );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->ResourceList->GetWidgetName() );

  this->TaggedDataList = vtkFetchMIResourceUploadWidget::New();
  this->TaggedDataList->SetParent ( descriptionFrame->GetFrame() );
  this->TaggedDataList->Create();
  this->TaggedDataList->SetApplication ( app );
  this->TaggedDataList->SetLogic ( this->Logic );
  this->TaggedDataList->SetMRMLScene ( this->MRMLScene );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->TaggedDataList->GetWidgetName() );


  l1->Delete();
  
  serverFrame->Delete();
  queryFrame->Delete();
  resourceFrame->Delete();
  descriptionFrame->Delete();  

  this->UpdateGUI();
  this->Logic->CreateTemporaryFiles();
  this->LoadTclPackage();
}


//---------------------------------------------------------------------------
void vtkFetchMIGUI::Init ( )
{
}
