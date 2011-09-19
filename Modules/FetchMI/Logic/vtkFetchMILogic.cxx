


#include "vtkCacheManager.h"
#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiducialListStorageNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"




#include "vtkFetchMILogic.h"

#include "vtkFetchMIWriterXND.h"
#include "vtkFetchMIParserXND.h"
#include "vtkFetchMIWebServicesClientXND.h"

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


vtkFetchMILogic* vtkFetchMILogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFetchMILogic");
  if(ret)
    {
      return (vtkFetchMILogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFetchMILogic;
}





//----------------------------------------------------------------------------
vtkFetchMILogic::vtkFetchMILogic()
{
  this->FetchMINode = NULL;
  this->SceneTags = vtkTagTable::New();
  this->SaveSceneDescription = 1;
  this->SceneSelected = 1;
  this->TemporarySceneSelected = 1;
  this->ResourceQuery = 0;
  this->TagAttributesQuery = 0;
  this->TagValuesQuery = 0;
  this->ErrorChecking = 0;
  this->ParsingError = 0;
 
  this->ServerCollection = vtkFetchMIServerCollection::New();
  this->CurrentWebService = NULL;
  this->ReservedURI = NULL;

  // Flags infrastructure for performance profiling
  this->Profiling = false;

  // Temporary vars used for parsing xml.
  this->NumberOfTagsOnServer = 0;
//  this->DebugOn();

   //--- Initialize the XML writer collection with writers for all known services.
   //--- Extend here as we support new web services.
   vtkFetchMIWriterXND *xndw = vtkFetchMIWriterXND::New();
   vtkFetchMIParserXND *xndp = vtkFetchMIParserXND::New();
   vtkFetchMIWebServicesClientXND *xndc = vtkFetchMIWebServicesClientXND::New();

   //--- Add all known servers and set their service type.
   //--- Extend here as we support new web services.
   const char *IDString = "XND";
   const char *HandlerString = "XNDHandler";
   vtkFetchMIServer *s1 = vtkFetchMIServer::New();
   s1->SetParser ( xndp );
   s1->SetWriter ( xndw );
   s1->SetWebServicesClient ( xndc );
   s1->SetName ( "http://xnd.slicer.org:8000" );
   s1->SetServiceType ( IDString);
   s1->SetURIHandlerName ( HandlerString);
   s1->SetTagTableName ( IDString);
   this->ServerCollection->AddItem ( s1 );
   s1->Delete();
   
   vtkFetchMIServer *s2 = vtkFetchMIServer::New();
   s2->SetParser ( xndp);
   s2->SetWriter ( xndw);
   s2->SetWebServicesClient (xndc);
   s2->SetName ( "http://localhost:8081");
   s2->SetServiceType ( IDString);
   s2->SetURIHandlerName ( HandlerString);
   s2->SetTagTableName ( IDString);
   this->ServerCollection->AddItem ( s2 );
   s2->Delete();

   /*
   IDString = "HID";
   HandlerString = "HIDHandler";
   vtkFetchMIServer *s3 = vtkFetchMIServer::New();
   s3->SetName ( "https://loci.ucsd.edu/hid");
   s3->SetServiceType ( IDString );
   s3->SetURIHandlerName ( HandlerString);
   s3->SetTagTableName ( IDString);
   this->ServerCollection->AddItem ( s3 );
   s3->Delete();
   */
   xndw->Delete();
   xndp->Delete();
   xndc->Delete();
   
   if ( this->FetchMINode != NULL )
     {
     this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::KnownServersModifiedEvent );
     }

   this->Visited = false;
   this->Raised = false;
//   this->DebugOn();
}




//----------------------------------------------------------------------------
vtkFetchMILogic::~vtkFetchMILogic()
{ 
  this->ClearExistingTagsForQuery();

  this->CurrentWebService = NULL;
    
  if ( this->ServerCollection != NULL )
    {
    int num = this->ServerCollection->GetNumberOfItems();
    for ( int i=0; i < num; i++ )
      {
      vtkFetchMIServer *s = vtkFetchMIServer::SafeDownCast (this->ServerCollection->GetItemAsObject (i) );
      if ( s )
        {
        if ( s->GetParser() )
          {
          s->SetParser ( NULL );
          }
        if ( s->GetWriter() )
          {
          s->SetWriter ( NULL );
          }
        if ( s->GetWebServicesClient() )
          {
          s->SetWebServicesClient ( NULL );
          }
        if ( s->GetURIHandler() )
          {
          s->SetURIHandler ( NULL );
          }
        s->SetURIHandlerName ( NULL );
        }
      }
    this->ServerCollection->RemoveAllItems();
    this->ServerCollection->Delete();
    this->ServerCollection = NULL;
    }
  if ( this->ReservedURI != NULL )
    {
    delete [] this->ReservedURI;
    this->ReservedURI = NULL;
    }
  this->SetFetchMINode(NULL);
  this->SceneTags->Delete();
  this->ResourceQuery = 0;
  this->TagAttributesQuery = 0;
  this->TagValuesQuery = 0;
  this->ErrorChecking = 0;
  this->ParsingError = 0;
  this->SetAndObserveMRMLScene ( NULL );
  this->Visited = false;  
  this->Raised = false;
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::Enter()
{

  this->Visited = true;
  this->Raised = true;

  //---
  //--- Set up Logic observers on enter, and released on exit.
  vtkIntArray *logicEvents = this->NewObservableEvents();
  if ( logicEvents != NULL )
    {
    this->SetAndObserveMRMLSceneEvents ( this->GetMRMLScene(), logicEvents );
    logicEvents->Delete();
    }

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::Exit()
{
  this->Raised = false;
}



//----------------------------------------------------------------------------
vtkIntArray* vtkFetchMILogic::NewObservableEvents()
{

  if ( !this->Visited )
    {
    return (NULL);
    }

 vtkIntArray *events = vtkIntArray::New();
 events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  // Slicer3.cxx calls delete on events
  return events;
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::InitializeInformatics ( )
{
  this->CreateTemporaryFiles();
  this->SetHandlersOnServers();
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::SetHandlersOnServers ( )
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "SetHandlersOnServers got NULL MRMLScene.");
    return;
    }
  if ( this->ServerCollection == NULL )
    {
    vtkErrorMacro ( "SetHandlersOnServers got a NULL Server Collection." );
    return;
    }

  int num = this->ServerCollection->GetNumberOfItems();
  for ( int i=0; i < num; i++ )
    {
    vtkFetchMIServer *s = vtkFetchMIServer::SafeDownCast ( this->ServerCollection->GetItemAsObject ( i ) );
    vtkURIHandler *handler = vtkURIHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( s->GetURIHandlerName() ));
    s->SetURIHandler ( handler );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::CreateTemporaryFiles ( )
{
  // This method creates three temporary filenames, using the cache dir, one
  // for holding http from server responses to queries, one
  // for holding xml metadata created for XNAT Desktop uploads,
  // and one for holding mrml scenes downloaded to cache.

  if ( this->GetMRMLScene() != NULL )
    {
    if ( this->GetMRMLScene()->GetCacheManager() != NULL )
      {
      if ( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory() != NULL )
        {
        this->XMLDirName = this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory();
        if ( this->XMLDirName.c_str() != NULL )
          {
          if (vtksys::SystemTools::FileIsDirectory( this->XMLDirName.c_str() ))
            {
            //--- for now, create temporary query response file in cache dir.
            std::vector<std::string> pathComponents;
            vtksys::SystemTools::SplitPath( this->XMLDirName.c_str(), pathComponents);
            // now add the new file name to the end of the path
            pathComponents.push_back("FetchMI_QueryResponse.xml");
            this->HTTPResponseFileName = vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "HTTPResponseFileName = " << this->HTTPResponseFileName );

            //--- for now, create temporary xml metadata description file in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_UploadMetadata.xml");
            this->XMLUploadFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "XMLUploadFileName = " << this->XMLUploadFileName );

            //--- for now, create temporary xml response file in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_TemporaryResponse.xml");
            this->TemporaryResponseFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "TemporaryResponseFileName = " << this->TemporaryResponseFileName );

            //--- for now, create temporary xml document description in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_DocumentDeclaration.xml");
            this->DocumentDeclarationFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "XMLDocumentDeclarationFileName = " << this->DocumentDeclarationFileName );
            
            //--- for now, create temporary xml header in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_UploadHeader.xml");
            this->HeaderFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "XMLHeaderFileName = " << this->HeaderFileName );

            //--- for now, create temporary cache file where downloaded mrml files go.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_MRMLCache.xml");
            this->MRMLCacheFileName = vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "MRMLCacheFileName = " << this->MRMLCacheFileName );

                
            // Wendy: do we need to touch these files, or just create the file names?
            }
          }
        }
      else
        {
        vtkWarningMacro ("RemoteCacheDirectory is not set! Creating files in current dir");
        this->HTTPResponseFileName = std::string("FetchMI_QueryResponse.xml");
        this->XMLUploadFileName = std::string("FetchMI_UploadMetadata.xml");
        this->MRMLCacheFileName = std::string("FetchMI_MRMLCache.xml");
        this->TemporaryResponseFileName = std::string ("FetchMI_TemporaryResponse.xml");
        }
      }
    }
}



//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetServiceTypeForServer ( const char *svc )
{
  if ( this->ServerCollection )
    {
    vtkFetchMIServer *s = this->ServerCollection->FindServerByName ( svc );
    if ( s )
      {
      return ( s->GetServiceType() );
      }
    }
  return (NULL);
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::AddNewServer (const char *name,
                                    const char *type,
                                    const char *URIHandlerName,
                                    const char *TagTableName )

{
  if ( name == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic::AddNewServer: got NULL Server name.");
    return;
    }
  if ( URIHandlerName == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic::AddNewServer: got NULL URIHandler name.");
    return;
    }
  if ( TagTableName == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic::AddNewServer: got NULL TagTable name");
    return;
    }
  // Developers note: extend this check as new service types are added.
  if ( !(strcmp(type, "XND")))
    {
    vtkFetchMIServer *localhost = this->GetServerCollection()->FindServerByName ( "http://localhost:8081");
    if ( localhost != NULL )
      {
      vtkFetchMIServer *s1 = vtkFetchMIServer::New();
      s1->SetName ( name );
      s1->SetServiceType ( type );
      s1->SetParser ( localhost->GetParser() );
      s1->SetWriter (localhost->GetWriter() );
      s1->SetWebServicesClient ( localhost->GetWebServicesClient() );
      s1->SetURIHandlerName ( URIHandlerName );
      s1->SetTagTableName ( TagTableName );
      s1->SetTagTable ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ) );
      this->ServerCollection->AddItem ( s1 );
      s1->Delete();
      }
    else
      {
    vtkErrorMacro ( "Server is of unknown or unsupported type." );
    return;
      }
    if ( this->FetchMINode != NULL )
      {
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::KnownServersModifiedEvent );
      }
    }
  else
    {
    vtkErrorMacro ( "Server is of unknown or unsupported type." );
    return;
    }
}



//---------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUpload ( )
{
  //---
  //--- do some error checking
  //---
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: MRMLScene is NULL.");
    std::string msg = "No upload initiated; upload methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: FetchMINode is NULL.");
    std::string msg = "No upload initiated; upload methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetCurrentWebService() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload:Got null server.");
    std::string msg = "No upload initiated; upload methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;    
    }
  if ( this->GetCurrentWebService()->GetWebServicesClient()  == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload:Got a null WebServicesClient" );
    std::string msg = "No upload initiated; upload methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetCurrentWebService()->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: Got a NULL URIHandler." );
    std::string msg = "No upload initiated; upload methods cannot find URI handler." ;
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  
  const char *svrName = this->GetCurrentWebService()->GetName();
  if ( svrName == NULL || !(strcmp(svrName, "" ) ) )
    {
    vtkErrorMacro ( "RequestResourceUpload: Got a NULL Server name." );
    std::string msg = "No upload initiated; upload methods can not file server name.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }

  //---
  // Temporarily save /upload everything until we can handle
  // uploads & downloads on individual datasets.
  //---
  this->SaveResourceSelectionState();
  
  //---
  // Check to make sure all storable nodes have
  // set storage nodes with filenames
  //---
  int retval = this->CheckStorageNodeFileNames();
  // TO DO -- use this.
//  int retval = this->CheckModifiedSinceRead();
  if ( retval == 0 )
    {
    std::string msg = "Some files have been modified since read. Please save all unsaved data to local disk first to ensure some safe copy exists.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }

  //---
  //--- Set filename on all storable nodes to include the cache path
  //--- so they are saved to cache prior to upload.
  //--- And set their write state to idle to ready them
  //--- for the remoteIO pipeline.
  this->SetIdleWriteStateOnSelectedResources ();
  this->SetCacheFileNamesOnSelectedResources();

  //---
  //--- make sure that the Slicer data type is set on all storables.
  this->ApplySlicerDataTypeTag();
  //---

  //---
  //--- update the handler to match Currently selected server.
  //--- and set the URIHandler on the clientcaller and all selected storage nodes.
  //---
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  if ( handler == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: Got a NULL URIHandler." );
    std::string msg = "No upload initiated; upload methods cannot find URI handler." ;
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  handler->SetHostName(svrName);
  this->SetURIHandlerOnSelectedResources( handler );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( this->GetCurrentWebService()->GetURIHandler() );

  this->SaveOldURIsOnSelectedResources();
  
  //---
  //--- request the upload
  //---
  this->WriteDocumentDeclaration();
  retval = this->AddNewTagsToServer();
  if ( retval == 0 )
    {
    //--- CATCH ERROR AND RESTORE!
    //--- post error message
    std::string msg = "Error checking tags against those on the selected server. Upload cancelled.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );

    //--- clean up and exit
    this->SetCancelledWriteStateOnSelectedResources ();
    int success = this->RestoreFileNamesOnSelectedResources();
    if ( !success )
      {
      //--- post message to user. filenames may be corrupt.
      msg = "Error restoring filenames associated with loaded datasets after a remote upload failure. Warning: file paths may be corrupted. As a precautionary measure, please save all datasets and scene file to a safe location on local disk or file system.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      }
    vtkErrorMacro ( "RequestResourceUpload: Failed to add new tags to server. Upload aborted." );
    return;
    }

  retval = this->RequestStorableNodesUpload();
  if ( retval == 0 )
    {
    //--- CATCH ERROR AND RESTORE!
    //--- post error message
    std::string msg = "Error uploading datasets to the selected server. Upload cancelled.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    
    //--- clean up and exit
    this->SetCancelledWriteStateOnSelectedResources ();
    int success = this->RestoreURIsOnSelectedResources();
    if ( !success )
      {
      //--- post message to user. filenames may be corrupt.
      msg = "Error restoring uris associated with loaded datasets after a remote upload failure. Warning: uris may be corrupted. As a precautionary measure, please save all datasets and scene file to a safe location on local disk or file system.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      }
    success = this->RestoreFileNamesOnSelectedResources();
    if ( !success )
      {
      //--- post message to user. filenames may be corrupt.
      msg = "Error restoring filenames associated with loaded datasets after a remote upload failure. Warning: file paths may be corrupted. As a precautionary measure, please save all datasets and scene file to a safe location on local disk or file system.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      }
    vtkErrorMacro ( "RequestResourceUpload: Failed to post selected storable nodes. Upload aborted." );
    return;
    }

  retval = this->RequestSceneUpload();
  if ( retval == 0 )
    {
    //--- CATCH ERROR AND RESTORE!
    //--- post error message
    std::string msg = "Error uploading scene description to the selected server. Upload cancelled.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );

    //--- clean up and exit
    this->SetCancelledWriteStateOnSelectedResources ();
    int success = this->RestoreFileNamesOnSelectedResources();
    if ( !success )
      {
      //--- post message to user. filoenames may be corrupt.
      msg = "Error restoring filenames associated with loaded datasets after a remote upload failure. Warning: file paths may be corrupted. As a precautionary measure, please save all datasets and scene file to a safe location on local disk or file system.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      }
    vtkErrorMacro ( "RequestResourceUpload: Failed to post scene. Upload aborted.");
    return;
    }         


  //---`
  //--- clean up
  //---
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( NULL );
  this->RestoreResourceSelectionState();
}




//---------------------------------------------------------------------------
int vtkFetchMILogic::RequestStorableNodesUpload ( )
{

  int retval;
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "RequestStorableNodesUpload: Got Null MRMLScene" );
    return 0;
    }
  if ( this->GetCurrentWebService() == NULL )
    {
    vtkErrorMacro ( "RequestStorableNodesUpload: Got null current server.");
    return 0;    
    }
  if (  this->GetCurrentWebService()->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "RequestStorableNodesUpload: Got null client caller.");
    return 0;    
    }
  vtkURIHandler *handler = vtkURIHandler::SafeDownCast ( this->GetCurrentWebService()->GetURIHandler() );
  if ( handler == NULL )
    {
    vtkErrorMacro ("RequestStorableNodesUpload: Null URIHandler. ");
    return 0;
    }

  //---
  //--- For each selected storable node:
  //---     * Generate FileNameList any multi-file volumes.
  //---     * generate metadata using the writer
  //---     * post metadata thru the client caller
  //---     * parse post response and set URIs using the parser
  //---     * get a new uri from the post and set it on the node.
  //-- Note: this may be too specific for XND: may need to generalize later.
  //---
  vtkMRMLStorableNode *storableNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    std::string nodeID = this->SelectedStorableNodeIDs[n];
    vtkDebugMacro("RequestStorableNodesUpload: generating metadata for selected storable node " << nodeID.c_str());
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));
    //---
    // FOR EACH SELECTED STORAGE NODE
    //---
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      if ( storageNode == NULL )
        {
        vtkErrorMacro ( "RequestStorableNodesUpload: got NULL StorageNode.");
        continue;
        }

      //---
      //--- Populate the multi-file list for any storage node that may have them.
      //---

      if ( storageNode->IsA("vtkMRMLVolumeArchetypeStorageNode") )
        {
        std::string moveFromDir = "";
        vtkMRMLVolumeArchetypeStorageNode *s =vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(storageNode);
        moveFromDir = s->UpdateFileList(storableNode, 1);
        if (moveFromDir == std::string(""))
          {
          vtkErrorMacro ( "RequestStorableNodesUpload: unable to create file list for storageNode." );
          return 0;
          }
        }
      
      //---
      //--- empty out the URIList from past uploads/downloads in the storage node.
      //---
      storageNode->ResetURIList();
      vtkDebugMacro("RequestStorableNodesUpload: have storage node " << i << ", calling write metadata for upload with id " << nodeID.c_str() << " and file name " << storageNode->GetFileName());

      //---
      //--- write header and metadata
      //--- invoke event that user sees if something goes wrong.
      //---
      const char *snodeFileName = storageNode->GetFileName();
      vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName ( snodeFileName );
      const char *strippedFileName = vtkFileName.c_str();
      retval = this->WriteMetadataForUpload(nodeID.c_str());
      if ( retval == 0 )
        {
        std::string msg = "Error writing metadata for upload. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return 0;
        }

      //---
      //--- post metadata to create a footprint for resource on server and get back URI.
      //--- invoke event that user sees if something goes wrong.
      //---
      retval = (this->GetCurrentWebService()->GetWebServicesClient()->CreateNewURIForResource( strippedFileName,
                                                                                      this->GetXMLHeaderFileName(),
                                                                                      this->GetXMLUploadFileName(),
                                                                                      this->GetTemporaryResponseFileName() ) );
      if ( retval == 0 )
        {
        std::string msg = "Error posting data. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return 0;
        }

      //---
      //--- parse post response to get a uri to associate with the data
      //---
      this->SetReservedURI ( NULL );
      this->GetCurrentWebService()->GetParser()->ParseMetadataPostResponse(this->GetTemporaryResponseFileName());
      const char *uri = this->GetCurrentWebService()->GetParser()->GetResponse();
      this->SetReservedURI ( uri );
      if (uri == NULL)
        {
        //--- keep old uri.
        //storageNode->SetURI(NULL);
        //storageNode->ResetURIList();
        this->DeselectNode(nodeID.c_str());
        // bail out of the rest of the storage nodes
        i = numStorageNodes;
        std::string msg = "Server did not return a valid uri for " + std::string(strippedFileName) + ". Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        // for now, decrement the node number, since DeselectNode removes an
        // element from the list we're iterating over
        return 0;
        }
      else
        {
        vtkDebugMacro("RequestStorableNodesUpload: parsed out a return metadatauri : " << uri);
        // Set each storable node's URI, where they'll be saved in the final post of data
        // via the PostStorableNodes() method.
        this->SaveNewURIOnSelectedResource ( storageNode->GetURI(), uri );
        storageNode->SetURI(uri);

        /*
        //--- Since the FileNameList has node->GetFileName
        //--- as its first entry, let's do the parallel thing and
        //--- add node->GetURI as the first entry of the URIList.
        //--- Don't do this for now; this will impact backward
        //--- compatibility across mrml scenes. But Discuss with Nicole.
        storageNode->AddURI(storageNode->GetURI());
        */

        // now deal with the rest of the files in the storage node
        int numFiles = storageNode->GetNumberOfFileNames();
        //--- the storageNode's FileName is repeated in its
        //--- FileListMember0 *if* there are multiple files for the dataset.
        //--- So, if there are multiple files, we don't want to repeat the
        //--- metadatapost and uri-provisioning for the first file name.
        //--- For that reason, we start the loop over the rest of the
        //--- files in the storage node at 1, skipping 0.
        if ( numFiles > 0 )
          {
          for (int filenum = 1; filenum < numFiles; filenum++)
            {
            //--- write header and metadata
            const char *nodeFileName = storageNode->GetNthFileName(filenum);
            vtkFileName = vtksys::SystemTools::GetFilenameName ( nodeFileName );
            strippedFileName = vtkFileName.c_str();
            retval = this->WriteMetadataForUpload(nodeID.c_str());
            if ( retval == 0 )
              {
              std::string msg = "Error writing metadata for upload. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
              this->FetchMINode->SetErrorMessage (msg.c_str() );
              this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
              return 0;
              }
            
            //---
            //--- post metadata to create a footprint for resource on server and get back URI.
            //--- invoke event that user sees if something goes wrong.
            //---
            retval = (this->GetCurrentWebService()->GetWebServicesClient()->CreateNewURIForResource( strippedFileName,
                                                                                            this->GetXMLHeaderFileName(),
                                                                                            this->GetXMLUploadFileName(),
                                                                                            this->GetTemporaryResponseFileName() ) );
            if ( retval == 0 )
              {
              std::string msg = "Error posting data. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
              this->FetchMINode->SetErrorMessage (msg.c_str() );
              this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
              return 0;
              }

            //---
            //--- Parse out the URI
            //---
            this->SetReservedURI ( NULL );
            this->GetCurrentWebService()->GetParser()->ParseMetadataPostResponse(this->GetTemporaryResponseFileName());
            uri = this->GetCurrentWebService()->GetParser()->GetResponse();
            this->SetReservedURI ( uri );
            if (uri == NULL)
              {
              //--- TODO: clean up filenames now. they are set to cache path.
              //--- fill up errormessage and invoke event via the node.
              vtkErrorMacro("RequestStorableNodesUpload:  error parsing uri from post meta data call for file # " << filenum); //, response = " << metadataResponse);
              //--- save the old uri
              //storageNode->SetURI(NULL);
              //storageNode->ResetURIList();
              std::string msg = "Error encountered while uploading " + std::string(storageNode->GetNthFileName(filenum)) + " to remote host. Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
              this->FetchMINode->SetErrorMessage (msg.c_str() );
              this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
              //--- TODO: manage upload failure properly.
              //--- would like to keep track of uris for files in filenamelist
              //--- and delete them from server if any one of them fails so we
              //--- don't have a messy partial upload.
              //--- For now, bail out and leave dirty pile of data on server.
              return 0;
              }
            else
              {
              vtkDebugMacro("RequestStorableNodesUpload: parsed out a return metadatauri : " << uri << ", adding it to storage node " << storageNode->GetID());
              // then save it in the storage node
              if ( storageNode->GetNumberOfURIs() > (filenum-1) )
                {
                this->SaveNewURIOnSelectedResource ( storageNode->GetNthURI(filenum-1), uri );
                }
              storageNode->AddURI(uri);
              }         
            }
          }
        }
      }
    }

  //--- Now all storable nodes have their metadata written and uris set.
  //--- post data (in logic pass3)
  if ( this->PostStorableNodes() == 0)
    {
    std::string msg = "Error encountered while uploading data files to remote host. Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return 0;
    }

  return 1;
}






//---------------------------------------------------------------------------
int vtkFetchMILogic::RequestSceneUpload ( )
{ 

  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "RequestSceneUpload: Got Null MRML Scene" );
    return 0;
    }
  if ( this->GetCurrentWebService() == NULL )
    {
    vtkErrorMacro ( "RequestSceneUpload: Got Null CurrentWebService" );
    return 0;
    }
  if (  this->GetCurrentWebService()->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "RequestSceneUpload: Got Null Client Caller" );
    return 0;
    }
  vtkURIHandler *handler = vtkURIHandler::SafeDownCast ( this->GetCurrentWebService()->GetURIHandler() );
  if ( handler == NULL )
    {
    vtkErrorMacro ("RequestSceneUpload: Null URIHandler. ");
    return 0;
    }
  const char *svrName = this->GetCurrentWebService()->GetName();
  if ( svrName == NULL || !(strcmp(svrName, "" ) ) )
    {
    vtkErrorMacro ( "RequestSceneUpload: Got a NULL Server name." );
    return 0;
    }

  
  if ( this->SceneSelected )
    {
    //---
    //--- explicitly write the scene to cache (uri already points to cache)
    //---
    this->GetMRMLScene()->Commit();


    //---
    //--- write header and metadata
    //---
    const char *sceneFileName =this->GetMRMLScene()->GetURL();
    vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (  sceneFileName );
    const char *strippedFileName = vtkFileName.c_str();
    int retval = this->WriteMetadataForUpload("MRMLScene");
    if (retval == 0 )
      {
      std::string msg = "Error encountered while uploading MRML Scene description to remote host. Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return 0;
      }

    //---
    //--- post metadata to create a footprint for resource on server and get back URI.
    //--- invoke event that user sees if something goes wrong.
    //---
    retval = (this->GetCurrentWebService()->GetWebServicesClient()->CreateNewURIForResource( strippedFileName,
                                                                                        this->GetXMLHeaderFileName(),
                                                                                        this->GetXMLUploadFileName(),
                                                                                        this->GetTemporaryResponseFileName() ) );
    if ( retval == 0 )
      {
      std::string msg = "Error posting scene. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return 0;
      }

    //---
    //--- parse out the uri for the Scene Description in the responseFile
    //---
    this->SetReservedURI ( NULL );
    this->GetCurrentWebService()->GetParser()->ParseMetadataPostResponse(this->GetTemporaryResponseFileName());
    const char *uri = this->GetCurrentWebService()->GetParser()->GetResponse();
    this->SetReservedURI ( uri );
    if ( uri != NULL )
      {
      // set particular host in the handler
      handler->SetHostName(svrName);
      handler->StageFileWrite(sceneFileName, uri);
      }
    else
      {
      std::string msg = "Error uploading scene. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return 0;
      }
    }

  return 1;
}






//---------------------------------------------------------------------------
int vtkFetchMILogic::AddNewTagsToServer ( )
{
  std::map<std::string, std::vector<std::string> >::iterator iter;  
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( iter->first.c_str() != NULL &&  (strcmp (iter->first.c_str(), "" ) ) )
      {
      //---
      // TODO: keep track of only new tags, and add only new ones
      // instead of adding all tags each time.
      // Make a call for each tag.
      //---
      int retval = this->GetCurrentWebService()->GetWebServicesClient()->AddTagToServer ( iter->first.c_str(), NULL, this->GetTemporaryResponseFileName() );
      if ( retval == 0 )
        {
        vtkErrorMacro ("AddNewTagsToServer: couldn't add new tag to server.");
        std::string msg = "Error adding new tags. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return 0;
          }
        //--- TODO: add error checking here...
        //--- look for complaints in this->GetTemporaryResponseFileName()
        }
      }
  return 1;
}


//---------------------------------------------------------------------------
void vtkFetchMILogic::SaveResourceSelectionState ( )
{
  //--- Temporary Save All
  //--- SAVE ORIGINAL RESOURCE SELECTION STATE
  //--- TODO: figure out how to upload individual resources.
  //--- For now, override selection state reflected by GUI--
  //--- select everything, so we upload scene + all data.
  //--- then restores GUI selection state reflected by GUI.
  //---
  //--- Note to Developers: extend this as new storagenode types 
  //--- are added to Slicer.
  this->TemporarySelectedStorableNodeIDs.clear();
  this->SetTemporarySceneSelected  (this->GetSceneSelected() );
  for ( unsigned int i=0; i< this->SelectedStorableNodeIDs.size(); i++)
    {
    TemporarySelectedStorableNodeIDs.push_back(this->SelectedStorableNodeIDs[i] );
    }

  //--- Get all storables in the scene
  this->SceneSelected=1;
  this->SelectedStorableNodeIDs.clear();      
  vtkMRMLNode *node = NULL;
  //---Volumes
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLVolumeNode");
  for (int n=0; n<nnodes; n++ )
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLVolumeNode");
    if ( node->GetHideFromEditors() )
      {
      continue;
      }
    vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(node);
    if (vnode != NULL )
      {
      vtkMRMLStorageNode *snode = vnode->GetStorageNode();
      if ( snode)
        {
        this->SelectedStorableNodeIDs.push_back ( node->GetID() );
        }
      }
    }
  //---Fiducials
  nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode");
  for (int n=0; n<nnodes; n++ )
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLFiducialListNode");
    if ( node->GetHideFromEditors() )
      {
      continue;
      }
    vtkMRMLFiducialListNode *flnode = vtkMRMLFiducialListNode::SafeDownCast(node);
    if (flnode != NULL )
      {
      vtkMRMLStorageNode *snode = flnode->GetStorageNode();
      if ( snode)
        {
        this->SelectedStorableNodeIDs.push_back ( node->GetID() );
        }
      }
    }
  
  //---ColorTables
  nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLColorTableNode");
  for (int n=0; n<nnodes; n++ )
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLColorTableNode");
    if ( node->GetHideFromEditors() )
      {
      continue;
      }
    vtkMRMLColorTableNode *cnode = vtkMRMLColorTableNode::SafeDownCast(node);
    if (cnode != NULL )
      {
      vtkMRMLStorageNode *snode = cnode->GetStorageNode();
      if ( snode)
        {
        this->SelectedStorableNodeIDs.push_back ( node->GetID() );
        }
      }
    }

  //---Models
  nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLModelNode");
  for (int n=0; n<nnodes; n++ )
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLModelNode");
    if ( node->GetHideFromEditors() )
      {
      continue;
      }
    vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(node);
    if (mnode != NULL )
      {
      vtkMRMLStorageNode *snode = mnode->GetStorageNode();
      if ( snode)
        {
        this->SelectedStorableNodeIDs.push_back ( node->GetID() );
        }
      }
    }
  //--- UnstructuredGrids
  nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLUnstructuredGridNode");
  for (int n=0; n<nnodes; n++ )
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLUnstructuredGridNode");
    if ( node->GetHideFromEditors() )
      {
      continue;
      }
    vtkMRMLUnstructuredGridNode *mnode = vtkMRMLUnstructuredGridNode::SafeDownCast(node);
    if (mnode != NULL )
      {
      vtkMRMLStorageNode *snode = mnode->GetStorageNode();
      if ( snode)
        {
        this->SelectedStorableNodeIDs.push_back ( node->GetID() );
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkFetchMILogic::RestoreResourceSelectionState ( )
{
  //--- RESET SELECTION STATE
  this->SceneSelected = this->GetTemporarySceneSelected();
  this->SelectedStorableNodeIDs.clear();
  for ( unsigned int i=0; i< TemporarySelectedStorableNodeIDs.size(); i++)
    {
    this->SelectedStorableNodeIDs.push_back(TemporarySelectedStorableNodeIDs[i] );
    }
}





//---------------------------------------------------------------------------
int vtkFetchMILogic::WriteDocumentDeclaration ( )
{
  if ( this->GetXMLDocumentDeclarationFileName() == NULL )
    {
    vtkErrorMacro ("WriteDocumentDeclaration: got null Document Declaration filename" );
    return 0;
    }
  this->GetCurrentWebService()->GetWriter()->SetDocumentDeclarationFilename ( this->GetXMLDocumentDeclarationFileName() );
  this->GetCurrentWebService()->GetWriter()->WriteDocumentDeclaration ( );
  return 1;
}


//---------------------------------------------------------------------------
int vtkFetchMILogic::WriteMetadataForUpload (const char *nodeID )
{
  //  return 1 if ok, 0 if not.
  if ( this->FetchMINode == NULL) 
    {
    vtkErrorMacro ( "WriteMetadataForUpload: FetchMINode is NULL.");
    return 0;
    }
  if (this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "WriteMetadataForUpload has null MRMLScene." );
    return 0;        
    }
  const char *metadataFilename = this->GetXMLUploadFileName();
  if ( metadataFilename == NULL )
    {
    vtkErrorMacro ("WriteMetadataForUpload: got null filename" );
    return 0;
    }
  const char *declarationFilename = this->GetXMLDocumentDeclarationFileName();
  if ( declarationFilename == NULL )
    {
    vtkErrorMacro ("WriteMetadataForUpload: got null filename" );
    return 0;
    }

  //--- configure the Writer
  this->GetCurrentWebService()->GetWriter()->SetMetadataFilename( this->GetXMLUploadFileName() );
  this->GetCurrentWebService()->GetWriter()->SetDocumentDeclarationFilename ( this->GetXMLDocumentDeclarationFileName() );

  //--- write either the Scene's metadata if nodeID = MRMLScene or the node's metadata.
  if ( !(strcmp (nodeID, "MRMLScene" )))
    {
    this->GetCurrentWebService()->GetWriter()->WriteMetadataForScene (this->GetMRMLScene());
    }
  else
    {
    this->GetCurrentWebService()->GetWriter()->WriteMetadataForNode (nodeID, this->GetMRMLScene());
    }
  return 1;
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::SetFetchMINode( vtkMRMLFetchMINode *node )
{
  if ( node != this->FetchMINode )
    {
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue ( vtkMRMLFetchMINode::SelectedServerModifiedEvent );
    events->InsertNextValue ( vtkMRMLScene::NodeAddedEvent );
    vtkSetAndObserveMRMLNodeEventsMacro ( this->FetchMINode, node, events );
    events->Delete();
    }
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::ProcessMRMLEvents(vtkObject *caller,
                                        unsigned long event,
                                        void *vtkNotUsed(callData))
{

  if ( !this->Visited )
    {
    return;
    }

  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "FetchMILogic::ProcessMRMLEvents: got null FetchMINode." );
    return;
    }
  if ( this->FetchMINode->GetTagTableCollection() == NULL )
    {
    vtkErrorMacro ( "FetchMILogic::ProcessMRMLEvents: got null FetchMINode TagTableCollection." );
    return;
    }

  vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
  if ( scene == this->GetMRMLScene() && event == vtkMRMLScene::NodeAddedEvent )
    {
    this->ApplySlicerDataTypeTag();
    }

  // only do what we need it to do if module is hidden.
  if ( !this->Raised )
    {
    return;
    }

  vtkMRMLFetchMINode* node = vtkMRMLFetchMINode::SafeDownCast ( caller );
  if ( node == this->FetchMINode && event == vtkMRMLFetchMINode::SelectedServerModifiedEvent )
    {
    this->CurrentWebService = this->GetServerCollection()->FindServerByName ( this->FetchMINode->GetSelectedServer() );
    this->CurrentWebService->SetTagTable (this->FetchMINode->GetTagTableCollection()->
                                          FindTagTableByName (  this->CurrentWebService->GetTagTableName() ) );
    }
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "XMLDirName: " << this->XMLDirName.c_str() << "\n";
  os << indent << "HTTPResponseFileName: " << this->HTTPResponseFileName.c_str() << "\n";
  os << indent << "XMLUploadFileName: " << this->XMLUploadFileName.c_str() << "\n";
  os << indent << "XMLDocumentDeclarationFileName: " << this->DocumentDeclarationFileName.c_str() << "\n";
  os << indent << "HeaderFileName: " << this->HeaderFileName.c_str() << "\n";
  os << indent << "TemporaryResponseFileName: " << this->TemporaryResponseFileName.c_str() << "\n";
  os << indent << "MRMLCacheFileName: " << this->MRMLCacheFileName.c_str() << "\n";
  os << indent << "DownloadDirName: " << this->DownloadDirName.c_str() << "\n";
  os << indent << "ResourceQuery: " << this->GetResourceQuery() << "\n";
  os << indent << "TagAttributesQuery: " << this->GetTagAttributesQuery() << "\n";
  os << indent << "TagValuesQuery: " << this->GetTagValuesQuery() << "\n";
  os << indent << "Selected Storable Node IDs:\n";
  for (unsigned int i = 0; i < this->SelectedStorableNodeIDs.size(); i++)
    {
    os << indent.GetNextIndent() << i << ": " << this->SelectedStorableNodeIDs[i].c_str() << "\n";
    }
  os << indent << "Modified Node IDs:\n";
  for (unsigned int i = 0; i < this->ModifiedNodes.size(); i++)
    {
    os << indent.GetNextIndent() << i << ": " << this->ModifiedNodes[i].c_str() << "\n";
    }  
  os << indent << "SaveSceneDsecription: " << this->SaveSceneDescription << "\n";
  os << indent << "SceneSelected: " << this->SceneSelected << "\n";

  os << indent << "SceneTags: (TBD)\n";

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::DeselectNode( const char *nodeID)
{
  std::string s;
  std::vector<std::string>tmp;
  unsigned int n = this->SelectedStorableNodeIDs.size();
  // copy all elements but one marked for delete to a temporary vector
  for ( unsigned int i = 0; i < n; i++ )
    {
    s = this->SelectedStorableNodeIDs[i];
    if ( (strcmp ( s.c_str(), nodeID )) )
      {
      tmp.push_back (std::string (nodeID ));
      }
    }

  // clear the vector and copy in new elements
  n = tmp.size();
  this->SelectedStorableNodeIDs.clear();
  for ( unsigned int i = 0; i < n; i++ )
    {
    this->SelectedStorableNodeIDs.push_back ( tmp[i] );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::SelectNode( const char *nodeID)
{
  int unique = 1;
  std::string s;

  unsigned int n = this->SelectedStorableNodeIDs.size();
  for ( unsigned int i = 0; i < n; i++ )
    {
    s = this->SelectedStorableNodeIDs[i];
    if ( !(strcmp ( s.c_str(), nodeID )) )
      {
      unique = 0;
      break;
      }
    }
    //--- add the server name if it's not already here.
    if ( unique )
      {
      this->SelectedStorableNodeIDs.push_back ( std::string ( nodeID ) );
      }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SelectScene()
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  this->SceneSelected = 1;

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::DeselectScene()
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  this->SceneSelected = 0;

}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ApplySlicerDataTypeTag()
{
  //--- Note to developers: expand logic here as new node types are added.
  //--- always make sure the scene as a selected SlicerDataType tag.
  //--- NOTE:
  //--- currently, valid SlicerDataTypes include these:
  //---
  //--- MRML
  //--- ScalarVolume
  //--- LabelMap
  //--- VTKModel
  //--- FiberBundle
  //--- FreeSurferModel
  //--- FreeSurferModelWithOverlay
  //--- DTIVolume
  //--- DWIVolume
  //--- UnstructuredGrid
  //--- FiducialList 
  //--- ColorTable 
  //---
  this->GetMRMLScene()->GetUserTagTable()->AddOrUpdateTag ( "SlicerDataType", "MRML", 0);
  this->SetSlicerDataTypeOnVolumeNodes();
  // do model tagging first -- and refine fiberbundles afterward.
  this->SetSlicerDataTypeOnModelNodes();
  this->SetSlicerDataTypeOnFiberBundleNodes();
  this->SetSlicerDataTypeOnUnstructuredGridNodes();
  this->SetSlicerDataTypeOnFiducialListNodes();
  this->SetSlicerDataTypeOnColorTableNodes();
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::SetSlicerDataTypeOnVolumeNodes()
{

  if ( this->FetchMINode == NULL )
    {
    return;
    }
  if (this->GetMRMLScene() == NULL )
    {
    return;
    }
  
  vtkMRMLNode *node=NULL;
  vtkMRMLStorableNode *stnode=NULL;
  vtkTagTable *t=NULL;
  
  int n;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  const char *dtype = NULL;
  for (n=0; n<nnodes; n++)
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLVolumeNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(node);

    //--- if its tag is set, we're done.
    if ( (vnode->GetSlicerDataType() != NULL) && (strcmp(vnode->GetSlicerDataType(), "")) )
      {
      continue;
      }

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
      this->GetMRMLScene()->AddNode(storageNode);  
      vnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = vnode->GetStorageNode();
      }

    std::string dir = this->GetMRMLScene()->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    if (snode->GetFileName() == NULL && dir.c_str() != NULL) 
      {
      std::string name =dir;
      name += std::string(node->GetName());
      name += std::string(".nrrd");
      snode->SetFileName(name.c_str());
      }

    // get absolute filename
    std::string name;
    if (this->GetMRMLScene()->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->GetMRMLScene()->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();

    // Set the SlicerDataType
    dtype = "Volume";
    vtkMRMLScalarVolumeNode *vsnode = vtkMRMLScalarVolumeNode::SafeDownCast (vnode );
    vtkMRMLDiffusionTensorVolumeNode *dtinode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast (vnode );
    vtkMRMLDiffusionWeightedVolumeNode *dwinode = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast (vnode);
    if ( vsnode != NULL )
      {
      if ( vsnode->GetLabelMap() )
        {
        vsnode->SetSlicerDataType ( "LabelMap" );
        dtype = vsnode->GetSlicerDataType();
        }
      else
        {
        vsnode->SetSlicerDataType ( "ScalarVolume");
        dtype = vsnode->GetSlicerDataType();
        }
      }
    if ( dtinode != NULL )
      {
      dtinode->SetSlicerDataType ("DTIVolume");
      dtype = dtinode->GetSlicerDataType();
      }
    if ( dwinode != NULL )
      {
      dwinode->SetSlicerDataType ( "DWIVolume" );
      dtype = dwinode->GetSlicerDataType();
      }

    //--- store node's slicerdatatype in its UserTagTable
    stnode = vtkMRMLStorableNode::SafeDownCast ( vsnode );
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
      this->AddModifiedNode(node->GetID());
      this->AddSelectedStorableNode(node->GetID() );
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SetSlicerDataTypeOnModelNodes()
{
  if ( this->FetchMINode == NULL )
    {
    return;
    }
  if (this->GetMRMLScene() == NULL )
    {
    return;
    }

  vtkMRMLNode *node = NULL;
  vtkMRMLStorableNode *stnode = NULL;
  vtkTagTable *t = NULL;
  
  int n;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
  

  for (n=0; n<nnodes; n++)
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLModelNode");
    vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = mnode->GetStorageNode();

    if (snode == NULL )
      {
      continue;
      }
    if (node->GetHideFromEditors()) 
      {
      //--- keep going for freesurfer overlay nodes;
      //--- they are not seen by editor, but we want to tag them.
      if ( !(snode->IsA("vtkMRMLFreeSurferModelOverlayStorageNode")) )
        {
        continue;
        }
      }

    //-- assign a default filename if none is there.
    std::string dir = this->GetMRMLScene()->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    if (snode->GetFileName() == NULL && dir.c_str() != NULL)
      {
      std::string name =dir; 
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());
      }

    // get absolute filename
    std::string name;
    if (this->GetMRMLScene()->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->GetMRMLScene()->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();
    

    const char *dtype = NULL;
    // Set the SlicerDataType in node
    //--- Models can have different kinds of storage nodes.

    int numStorageNodes = mnode->GetNumberOfStorageNodes();
    vtkMRMLFreeSurferModelOverlayStorageNode *fsonode;
    vtkMRMLFreeSurferModelStorageNode *fsnode;
    vtkMRMLStorageNode* sn;
    int taggedOverlay = 0;

    //--- look thru all storage nodes and if one is an overlay, then tag the model properly.
    for ( int num=0; num<numStorageNodes; num++ )
      {
      sn = mnode->GetNthStorageNode(num);
      fsonode = vtkMRMLFreeSurferModelOverlayStorageNode::SafeDownCast (sn);
      if ( fsonode != NULL )
        {
        if ( sn->IsA("vtkMRMLFreeSurferModelOverlayStorageNode") )
          {
          //--- if its tag is set, we're done.
          if ( (mnode->GetSlicerDataType() != NULL) && (strcmp(mnode->GetSlicerDataType(), "")) )
            {
            if ( !(strcmp(mnode->GetSlicerDataType(), "FreeSurferModelWithOverlay")) )
              {
              taggedOverlay = 1;
              break;
              }
            }
          }
        mnode->SetSlicerDataType ( "FreeSurferModelWithOverlay");
        dtype = mnode->GetSlicerDataType();
        taggedOverlay = 1;
        break;
        }
      }
      
    //--- if we found an overlay, keep this tag on node, move on to the next model node.
    if ( taggedOverlay )
      {
      continue;
      }

    //--- otherwise see if it's a freesurfer or vtk model
    fsnode = vtkMRMLFreeSurferModelStorageNode::SafeDownCast (snode);
    if ( fsnode != NULL )
      {
      if ( snode->IsA("vtkMRMLFreeSurferModelStorageNode") )
        {
        //--- if its tag is set, we're done.
        if ( (mnode->GetSlicerDataType() != NULL) && (strcmp(mnode->GetSlicerDataType(), "")) )
          {
          if ( !(strcmp(mnode->GetSlicerDataType(), "FreeSurferModel")) )
            {
            continue;
            }
          }
        mnode->SetSlicerDataType ( "FreeSurferModel");
        dtype = mnode->GetSlicerDataType();
        }
      }
    else 
      {
      if ( !(strcmp(mnode->GetSlicerDataType(), "VTKModel")) )
        {
        continue;
        }
      mnode->SetSlicerDataType ( "VTKModel" );
      dtype = mnode->GetSlicerDataType();
      }

    //--- store node's slicerdatatype in its UserTagTable.
    stnode = vtkMRMLStorableNode::SafeDownCast ( node );
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
      this->AddModifiedNode(node->GetID());
      this->AddSelectedStorableNode(node->GetID() );      
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SetSlicerDataTypeOnFiberBundleNodes()
{
  if ( this->FetchMINode == NULL )
    {
    return;
    }
  if (this->GetMRMLScene() == NULL )
    {
    return;
    }

  vtkMRMLNode *node = NULL;
  
  int n;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLFiberBundleNode");
  
  for (n=0; n<nnodes; n++)
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLFiberBundleNode");
    // use a storable node superclass to avoid having to link to the
    // Tractography mrml library
    vtkMRMLStorableNode *fiberBundleNode = vtkMRMLStorableNode::SafeDownCast(node);

    //--- if its tag is set (but not set to generic VTKModel), we're done.
    if ( (fiberBundleNode->GetSlicerDataType() != NULL) &&
         (strcmp(fiberBundleNode->GetSlicerDataType(), "")) &&
         (strcmp(fiberBundleNode->GetSlicerDataType(), "VTKModel")) )
      {
      continue;
      }

    vtkMRMLStorageNode* snode = fiberBundleNode->GetStorageNode();
    if (snode == NULL )
      {
      continue;
      }

    //--- make sure we have a filename
    std::string dir = this->GetMRMLScene()->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    if (snode->GetFileName() == NULL && dir.c_str() != NULL) 
      {
      std::string name =dir;
      name += std::string(node->GetName());
      name += std::string(".vtk");
      // TODO: what about vtp???
      snode->SetFileName(name.c_str());
      }

    //--- store node's slicerdatatype in its UserTagTable
    fiberBundleNode->SetSlicerDataType ( "FiberBundle" );
    vtkTagTable *tt = fiberBundleNode->GetUserTagTable();
    tt->AddOrUpdateTag ( "SlicerDataType", fiberBundleNode->GetSlicerDataType(), 1 );
    
    if (node->GetModifiedSinceRead()) 
      {
      this->AddModifiedNode(node->GetID());
      this->AddSelectedStorableNode(node->GetID() );
      }
    }
}
  

//----------------------------------------------------------------------------
void vtkFetchMILogic::SetSlicerDataTypeOnUnstructuredGridNodes()
{
  //--- UNSTRUCTURED GRID NODES
#if !defined(MESHING_DEBUG) && defined(Slicer_BUILD_MODULES)  
  // *** add UnstructuredGrid types 
  // An additional datatype, MRMLUnstructuredGrid and its subclasses are 
  // also searched in the MRML tree.  This is done so instances of FiniteElement
  // meshes and other vtkUnstructuredGrid datatypes can be stored persistently.
  // this code is gated by MESHING_DEBUG since the MEshing MRML modules 
  
  if ( this->FetchMINode == NULL )
    {
    return;
    }
  if (this->GetMRMLScene() == NULL )
    {
    return;
    }

  int n;
  vtkMRMLNode *node;
  vtkMRMLStorableNode *stnode;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLUnstructuredGridNode");
  
  for (n=0; n<nnodes; n++)
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLUnstructuredGridNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    stnode = vtkMRMLStorableNode::SafeDownCast ( node );
    vtkMRMLUnstructuredGridNode *gnode = vtkMRMLUnstructuredGridNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = gnode->GetStorageNode();
    //--- if its tag is set, we're done.
    if ( (gnode->GetSlicerDataType() != NULL) && (strcmp(gnode->GetSlicerDataType(), "")) )
      {
      continue;
      }
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    if (snode == NULL && node->GetModifiedSinceRead()) 
      {
        vtkMRMLUnstructuredGridStorageNode *storageNode = vtkMRMLUnstructuredGridStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      gnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }

    std::string dir = this->GetMRMLScene()->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    if (snode->GetFileName() == NULL && dir.c_str() != NULL)
      {
      std::string name =dir; 
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->GetMRMLScene()->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->GetMRMLScene()->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();

    gnode->SetSlicerDataType ( "UnstructuredGrid" );

    //--- store node's slicerdatatype in its UserTagTable.
    if ( stnode != NULL  )
      {
      t = stnode->GetUserTagTable();
      if ( t != NULL )
        {
        t->AddOrUpdateTag ( "SlicerDataType", gnode->GetSlicerDataType(), 1 );
        }
      }

    if (node->GetModifiedSinceRead()) 
      {
      this->AddModifiedNode(node->GetID());
      this->AddSelectedStorableNode(node->GetID() );      
      }
    }
    // end of UGrid MRML node processing
#endif  

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SetSlicerDataTypeOnFiducialListNodes()
{
  if ( this->FetchMINode == NULL )
    {
    return;
    }
  if (this->GetMRMLScene() == NULL )
    {
    return;
    }

  vtkMRMLNode *node = NULL;
  vtkMRMLStorableNode *stnode = NULL;
  vtkTagTable *t = NULL;
  
  int n;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  
  //--- sniff thru the scene; get each vtkMRMLFiducialListNode and tag it.
  for (n=0; n<nnodes; n++)
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLFiducialListNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLFiducialListNode *flnode = vtkMRMLFiducialListNode::SafeDownCast(node);
    //--- if its tag is set, we're done.
    if ( (flnode->GetSlicerDataType() != NULL) && (strcmp(flnode->GetSlicerDataType(), "")) )
      {
      continue;
      }

    vtkMRMLStorageNode* snode = flnode->GetStorageNode();
    /*
      // comment this out for fiducials. Seems the node can be
      // created but storage node is not created until fiducials
      // are added to the list... But no event is triggered when a
      // storage node is created, so we may never fall into this
      // method and will consequently skip tagging a
      // fiducialListNode when it's created (but empty)
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    */
    if (snode == NULL ) 
      {
      vtkMRMLFiducialListStorageNode *storageNode = vtkMRMLFiducialListStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      flnode->SetAndObserveStorageNodeID(storageNode->GetID());
      flnode->ModifiedSinceReadOn();
      storageNode->Delete();
      snode = flnode->GetStorageNode();
      }

    std::string dir = this->GetMRMLScene()->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    if (snode->GetFileName() == NULL && dir.c_str() != NULL)
      {
      std::string name =dir;
      name += std::string(node->GetName());
      name += std::string(".fcsv");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->GetMRMLScene()->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->GetMRMLScene()->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();
    // Set the SlicerDataType
    vtkMRMLFiducialListStorageNode *flsnode = vtkMRMLFiducialListStorageNode::SafeDownCast (snode);
    if ( flsnode != NULL )
      {
      if ( snode->IsA("vtkMRMLFiducialListStorageNode") )
        {
        flnode->SetSlicerDataType ( "FiducialList" );
        }
      }
    //--- store node's slicerdatatype in its UserTagTable.
    stnode = vtkMRMLStorableNode::SafeDownCast ( node );
    if ( stnode != NULL && (flnode->GetSlicerDataType() != NULL) )
      {
      t = stnode->GetUserTagTable();
      if ( t != NULL )
        {
        t->AddOrUpdateTag ( "SlicerDataType", flnode->GetSlicerDataType(), 1 );
        }
      }
    if (node->GetModifiedSinceRead()) 
      {
      this->AddModifiedNode(node->GetID());
      this->AddSelectedStorableNode(node->GetID() );      
      }
    
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SetSlicerDataTypeOnColorTableNodes()
{
  if ( this->FetchMINode == NULL )
    {
    return;
    }
  if (this->GetMRMLScene() == NULL )
    {
    return;
    }

  vtkMRMLNode *node = NULL;
  vtkMRMLStorableNode *stnode = NULL;
  vtkTagTable *t = NULL;
  
  int n;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLColorTableNode");
  
  //--- sniff thru the scene; get each vtkMRMLColorTableNode and tag it.
  for (n=0; n<nnodes; n++)
    {
    node = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLColorTableNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLColorTableNode *cnode = vtkMRMLColorTableNode::SafeDownCast(node);
    //--- if its tag is set, we're done.
    if ( (cnode->GetSlicerDataType() != NULL) && (strcmp(cnode->GetSlicerDataType(), "")) )
      {
      continue;
      }
    vtkMRMLStorageNode* snode = cnode->GetStorageNode();
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    if (snode == NULL && node->GetModifiedSinceRead()) 
      {
      vtkMRMLColorTableStorageNode *storageNode = vtkMRMLColorTableStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      cnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = cnode->GetStorageNode();
      }

    std::string dir = this->GetMRMLScene()->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    if (snode->GetFileName() == NULL && dir.c_str() != NULL)
      {
      std::string name =dir;
      name += std::string(node->GetName());
      name += std::string(".ctbl");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->GetMRMLScene()->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->GetMRMLScene()->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();
    // Set the SlicerDataType
    vtkMRMLColorTableStorageNode *csnode = vtkMRMLColorTableStorageNode::SafeDownCast (snode);
    if ( csnode != NULL )
      {
      if ( snode->IsA("vtkMRMLColorTableStorageNode") )
        {
        cnode->SetSlicerDataType ( "ColorTable" );
        }
      }
    //--- store node's slicerdatatype in its UserTagTable.
    stnode = vtkMRMLStorableNode::SafeDownCast ( node );
    if ( stnode != NULL && (cnode->GetSlicerDataType() != NULL) )
      {
      t = stnode->GetUserTagTable();
      if ( t != NULL )
        {
        t->AddOrUpdateTag ( "SlicerDataType", cnode->GetSlicerDataType(), 1 );
        }
      }
    if (node->GetModifiedSinceRead()) 
      {
      this->AddModifiedNode(node->GetID());
      this->AddSelectedStorableNode(node->GetID() );      
      }
    // after each new tag, invoke this event which causes the GUI to update.
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::NewNodeTaggedEvent );
    }
}



//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetHTTPResponseFileName ( )
{
  return ( this->HTTPResponseFileName.c_str() );
}



//----------------------------------------------------------------------------
const char* vtkFetchMILogic::GetXMLHeaderFileName ( )
{
  return ( this->HeaderFileName.c_str() );
}



//----------------------------------------------------------------------------
const char* vtkFetchMILogic::GetXMLDocumentDeclarationFileName ( )
{
  return ( this->DocumentDeclarationFileName.c_str() );
}


//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetXMLUploadFileName ( )
{
  return ( this->XMLUploadFileName.c_str() );
}



//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetTemporaryResponseFileName ( )
{
  return ( this->TemporaryResponseFileName.c_str() );
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForTags ( )
{
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: FetchMINode is NULL.");
    std::string msg = "Error in query configuration. Server was not queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetServerCollection() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: FetchMINode is NULL.");
    std::string msg = "Error in query configuration. Server was not queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  this->CurrentWebService = this->GetServerCollection()->FindServerByName ( this->FetchMINode->GetSelectedServer() );
  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: CurrentWebService is NULL.");
    std::string msg = "Current web service is not set. Server was not queried";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if  ( this->GetCurrentWebService()->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: got Null WebServicesClient" );
    std::string msg = "Current web service has no web service client set. Server was not queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if  ( this->GetCurrentWebService()->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: got NULL URIHandler" );
    std::string msg = "Current web service has no URI handler set. Server was not queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetHTTPResponseFileName() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: CurrentWebService is NULL.");
    std::string msg = "Remote IO does not have default response file set. Server was not queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }

  const char *svrName = this->GetCurrentWebService()->GetName();
  const char *svctype = this->GetCurrentWebService()->GetServiceType();          
  if ( svrName == NULL || !(strcmp(svrName, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    std::string msg = "Remote IO got a null server name. Server can not be queried" ;
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "QueryServerForTags:Got unknown web service type");
    std::string msg = "Remote IO got a null web service type. Server cannot be queried";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }

  //--- Always reset the number of known tags on the server to 0 before query.
  this->NumberOfTagsOnServer = 0;

  //--- configure the client's handler in case the hostname or handler has been changed...
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  handler->SetHostName (svrName );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( handler );
  
  if (this->CurrentWebService->GetWebServicesClient()->QueryServerForTags( this->GetHTTPResponseFileName() ) )
    {
    //--- then parse new tags into the container.
    this->ParseTagQueryResponse ( );
    // check to see if "SlicerDataType" is an existing tag on the server.
    // if not, then post it.
    if ( ! this->IsTagOnServer ( "SlicerDataType" ) )
      {
      int returnval = this->CurrentWebService->GetWebServicesClient()->AddTagToServer ( "SlicerDataType", NULL, this->GetTemporaryResponseFileName() );
      // if it was added successfully, then add it to our list of tags.
      //otherwise return an error message.
      if ( returnval )
        {
        this->AddUniqueTag ( "SlicerDataType" );
        }
      else
        {
        std::string msg = "Web service doesn't know about the SlicerDataType tag, which will make it impossible to up/download data. Slicer failed to successfully post this new tag. Please check your network connection, or whether the server is down.";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return;
        }
      }
    }
  else
    {
    std::string msg = "Failed to communicate with the server. Please check your network connection, or whether the server is down.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForTagValues ( )
{

  vtkDebugMacro ("--------------------Querying server for tag values.");
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: FetchMINode is NULL.");
    std::string msg = "Error in remote IO configuration: Server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetHTTPResponseFileName( ) == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: ResponseFileName is not set.");
    std::string msg = "Remote IO does not have default response filename set. Server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetServerCollection() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: FetchMINode is NULL.");
    std::string msg = "Error in query configuration. Server was not queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  this->CurrentWebService = this->GetServerCollection()->FindServerByName ( this->FetchMINode->GetSelectedServer() );
  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: CurrentWebService is NULL.");
    std::string msg = "Current web service is NULL; server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if  ( this->GetCurrentWebService()->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: got Null WebServicesClient" );
    std::string msg = "Current web service has a NULL web services client. Server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if  ( this->GetCurrentWebService()->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: got NULL URIHandler" );
    std::string msg = "Current web service has a NULL URI handler. Server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  const char *svrName = this->GetCurrentWebService()->GetName();
  if (svrName == NULL || !(strcmp(svrName, "") ) )
    {
    vtkErrorMacro ("QueryServerForTagValues: Null or empty server name." );
    std::string msg = "Current web service has a NULL server name. Server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  const char *svctype = this->GetCurrentWebService()->GetServiceType();
  if ( !this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "QueryServerForTagValues:Got unknown web service type");
    std::string msg = "Current web service is an unknown type. Server cannot be queried.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }

  //--- configure the client's handler in case hostname has changed...
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  handler->SetHostName (svrName );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( handler );


  // For each tag, get the attribute, and then ask server for all values for that attribute.
  // Parse the response from the server. Invoke an event on the node that will be caught by GUI.
  // In GUI, the value menus for each attribute will be updated to show all current values
  // for each tag in the DB.
  std::string att;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    att.clear();
    att = iter->first;
    int retval = this->CurrentWebService->GetWebServicesClient()->QueryServerForTagValues ( att.c_str(),  this->GetHTTPResponseFileName() );
    if ( retval)
      {
      //--- clear out the container for values for this tagname.
      this->ParseValuesForTagQueryResponse ( att.c_str() );
      //--- and Update MRML's tagtable. make sure
      //--- each tag's value is in the logic's new list.
      //--- if so, leave it selected. otherwise, reset
      //--- the value to the first value in logic's list
      this->UpdateMRMLQueryTags();
      }
    else
      {
      vtkErrorMacro ( "QueryServerforTagValues: XML Response dir not set." );
      std::string msg = "Either failed to communicate with the server, or could not save the query response. Please check your network connection, or whether the server is down.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return;
      }
    }
  //--- invoke an event that updates GUI once we have queried for values for all tags.
  vtkDebugMacro ("--------------------Invoking TagResponseReadyEvent.");
  this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::TagResponseReadyEvent );          
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::UpdateMRMLQueryTags()
{

  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "UpdateMRMLQueryTags: CurrentWebService is NULL.");
    return;
    }
  if ( this->CurrentWebService->GetTagTable() == NULL )
    {
    this->GetFetchMINode()->AddTagTablesForWebServices();
    if ( this->CurrentWebService->GetTagTable() == NULL )
      {
      this->GetFetchMINode()->AddTagTablesForWebServices();
      if ( this->CurrentWebService->GetTagTable() == NULL )
        {
        vtkErrorMacro ( "UpdateMRMLQueryTags: CurrentWebService has NULL TagTable.");
        return;
        }
      }
    }
  const char *svr = this->GetCurrentWebService()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("UpdateMRMLQueryTags: Null or empty server name." );
    return;
    }
  const char  *svctype = this->GetCurrentWebService()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "UpdateMRMLQueryTags:Got unknown web service type");
    return;
    }

  //--
  //--- Make sure MRML tag table is compatible with logic's
  //--- list of tags on server.
  //--- For each tag in MRML's tagtable, get the old value.
  //--- Find that tag in the logic's CurrentWebServiceMetadata.
  //--- if it's present, then keep its value in MRML.
  //--- if it's not present, then replace the tag's value in MRML
  //--- with the first value in the logic's list.
  //--
  vtkTagTable *t = this->CurrentWebService->GetTagTable();
    
  std::map<std::string, std::vector<std::string> >::iterator iter;
  const char *att;
  const char *val;
  if ( t != NULL )
    {
    int numTagsInMRML = t->GetNumberOfTags();
    //--- for each tag in mrml...
    for (int i=0; i < numTagsInMRML; i++ )
      {

      //--- find matching tag in AllValuesForTag
      att = t->GetTagAttribute ( i );
      int foundAtt = 0;
      for ( iter = this->CurrentWebServiceMetadata.begin();
            iter != this->CurrentWebServiceMetadata.end();
            iter++ )
        {
        if ( !strcmp(iter->first.c_str(),att ))
          {
          //--- found tag in logic. see if value is present.
          foundAtt = 1;
          val = t->GetTagValue ( i );
          int foundVal = 0;
          for ( unsigned int j=0; j < iter->second.size(); j++ )
            {
            if ( !(strcmp(iter->second[j].c_str(), val) ))
              {
              foundVal = 1;
              break;
              }
            }
          if ( !foundVal )
            {
            //--- tag value in MRML is no longer available. clear out.
            //--- and deselect.
            if ( iter->second.size() == 0 )
              {
              t->AddOrUpdateTag ( att, "no values found", 0 );
              }
            else
              {
              t->AddOrUpdateTag ( att, iter->second[0].c_str(), 0 );
              }
            }
          }
        if ( foundAtt )
          {
          break;
          }
        } //end iter
      }
    }
}


//----------------------------------------------------------------------------
unsigned int vtkFetchMILogic::GetNumberOfURIsDeletedOnServer ( )
{
  return ( this->URIsDeletedOnServer.size() );
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ResetURIsDeletedOnServer ( )
{
  this->URIsDeletedOnServer.clear();
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::DeleteSceneFromServer ( const char *uri )
{
  int retval = 1;

  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: CurrentWebService is NULL.");
    return 0;
    }
  if ( this->CurrentWebService->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: Web service client is NULL.");
    return 0;
    }
  if ( this->CurrentWebService->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: URI handler is NULL.");
    return 0;
    }
  // check for which server and make sure it's of known type.
  const char *svr = this->GetCurrentWebService()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("DeleteSceneFromServer: Null or empty server name." );
    return 0;
    }
  const char *svctype = this->GetCurrentWebService()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: Got unknown web service type");
    return 0;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: FetchMINode is NULL.");
    return 0;
    }
  if ( this->GetMRMLScene()->GetCacheManager() == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: CacheManager is NULL.");
    return 0;
    }
  if ( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory() == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: No Remote Cache Directory is set.");
    return 0;
    }

  //--- configure the client's handler in case the hostname or handler has been changed...
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  if ( handler == NULL )
    {
    vtkErrorMacro ( "DeleteSceneFromServer: Got NULL URIHandler");
    return 0;
    }
  handler->SetHostName (svr );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( handler );

  this->ResetURIsDeletedOnServer ( );

  //--- Download scene
  std::string remoteURL = uri;
  std::string localURL;

  if ( this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri) != NULL )
    {
    localURL = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri);    
    }
  else
    {
    //--- explicitly set this in cache.
    std::string tmp = "FetchMIDeleteTargetScene.mrml";
    std::vector<std::string> pathComponents;
    vtksys::SystemTools::SplitPath( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory(), pathComponents);
    pathComponents.push_back(tmp.c_str() );
    localURL = vtksys::SystemTools::JoinPath(pathComponents);
    }

  if ( localURL.c_str() != NULL && (strcmp(localURL.c_str(), "" ) ) )
    {
      //---
      //--- download
      //---
      this->GetCurrentWebService()->GetWebServicesClient()->Download ( remoteURL.c_str(), localURL.c_str() );
      //---
      //--- get filename and path for scene we expect to have downloaded.
      //---
      std::vector<std::string> deleteTargets;
      deleteTargets.push_back ( remoteURL );
  
      //---
      //--- try opening file.
      //---
      std::ifstream pfile;
      std::string line;
#ifdef _WIN32
      pfile.open ( localURL.c_str(), ios::binary | ios::in );
#else
      pfile.open ( localURL.c_str(), ios::in );
#endif
  
      if ( !pfile.is_open() )
      {
      vtkErrorMacro ( "DeleteSceneFromServer: error opening temporary scene file to parse for uris to delete." );
      std::string msg = "Unable to delete this scene from server.";
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return 0;
      }

      //---
      //--- parse out all uris (scene's first) and uriListMembers into a vector of strings.
      //---
      size_t pos;
      size_t pos2;
      size_t pos3;
      std::string target;
      while ( !pfile.eof() )
      {
      line.clear();
      pfile >> line;

      //--- look for uri: should only be one on a line.
      pos = line.find ( "uri=", 0 );
      if ( pos != std::string::npos )
        {
        //--- get everything between non-escaped quotes.
        // start at pos, look for openquote.
        pos2 = line.find ( "\"", pos );
        if ( (pos2 != std::string::npos) && ((pos2+1) != std::string::npos))
          {
          // start at pos, look for closequote.
          pos3 = line.find ( "\"", pos2+1);
          if ( pos3 != std::string::npos )
            {
            // grab what's between quotes
            target = line.substr ((pos2+1), (pos3-pos2-1));
            deleteTargets.push_back( target );
            }
          }
        }

      //--- look for uriListMembers -- could be more than one on a line.
      pos = 0;
      while ( (pos=line.find ("uriListMember", pos) != std::string::npos ) )
        {
        //--- get everything between non-escaped quotes.
        // start at pos, look for openquote.
        pos2 = line.find ( "\"", pos );
        if ( (pos2+1) < std::string::npos )
          {
          // start at pos, look for closequote.
          pos3 = line.find ( "\"", pos2+1 );
          if ( pos3 != std::string::npos )
            {
            // grab what's between quotes
            target = line.substr ((pos2+1), (pos3-pos2-1));
            deleteTargets.push_back( target );
            }
          }
        pos ++;
        }
      }
      pfile.close();

      //---
      //--- delete all uris starting with scene file.
      //---
      int allgood = 1;
      int deleted = 0;
      for ( unsigned int i=0; i < deleteTargets.size(); i++ )
      {
      retval = this->CurrentWebService->GetWebServicesClient()->DeleteResourceFromServer ( deleteTargets[i].c_str(),
                                                                                      this->GetHTTPResponseFileName() );
      if ( retval == 0 )
        {
        allgood = 0;
        }
      else
        {
        this->URIsDeletedOnServer.push_back ( deleteTargets[i] );
        deleted++;
        }
      }
      if ( !allgood )
      {
        std::stringstream ss;
        ss << "Warning: only " << deleted << " of " << deleteTargets.size() << " resources including and associated with the scene were deleted.";
        std::string msg = ss.str();
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return 0;    
      }
       return (1);
    }
  return 0;
}




//----------------------------------------------------------------------------
int vtkFetchMILogic::DeleteResourceFromServer ( const char *uri )
{
  int retval = 1;

  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServer: CurrentWebService is NULL.");
    return 0;
    }
  if ( this->CurrentWebService->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServer: Web service client is NULL.");
    return 0;
    }
  if ( this->CurrentWebService->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServer: URI handler is NULL.");
    return 0;
    }
  // check for which server and make sure it's of known type.
  const char *svr = this->GetCurrentWebService()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("DeleteResourceFromServer: Null or empty server name." );
    return 0;
    }
  const char *svctype = this->GetCurrentWebService()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "DeleteResourceFromServer: Got unknown web service type");
    return 0;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return 0;
    }
  //--- configure the client's handler in case the hostname or handler has been changed...
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  if ( handler == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: Got NULL URIHandler");
    return 0;
    }

  handler->SetHostName (svr );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( handler );
  retval = this->CurrentWebService->GetWebServicesClient()->DeleteResourceFromServer ( uri,  this->GetHTTPResponseFileName() );
  this->ResetURIsDeletedOnServer ( );
  if ( retval )
    {
    std::string uriStr = uri;
    this->URIsDeletedOnServer.push_back ( uriStr );
    }
  return ( retval );
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForResources ( )
{
  //--- get selected tags
  //--- for now, use SlicerDataType=MRML
  //--- and form a uri.
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: FetchMINode is NULL.");
    return;
    }
  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: CurrentWebService is NULL.");
    return;
    }
  if ( this->CurrentWebService->GetWebServicesClient() == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: got NULL WebServicesClient" );
    return;
    }
  if ( this->CurrentWebService->GetTagTable() == NULL )
    {
    this->GetFetchMINode()->AddTagTablesForWebServices();
    if ( this->CurrentWebService->GetTagTable() == NULL )
      {
      this->GetFetchMINode()->AddTagTablesForWebServices();
      if ( this->CurrentWebService->GetTagTable() == NULL )
        {
        vtkErrorMacro ( "QueryServerForResources: got NULL TagTable" );
        return;
        }
      }
    }
  const char *svrName = this->GetCurrentWebService()->GetName();
  if (svrName == NULL || !(strcmp(svrName, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  const char *svctype = this->GetCurrentWebService()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "QueryServerForResources: Got unknown web service type");
    return;
    }
  //--- configure the client's handler in case the hostname or handler has been changed...
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  if ( handler == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: Got NULL URIHandler");
    return;
    }
  handler->SetHostName (svrName );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( handler );

  vtkTagTable *t = this->GetCurrentWebService()->GetTagTable();
  int retval = this->GetCurrentWebService()->GetWebServicesClient()->QueryServerForResources (t, this->GetHTTPResponseFileName() );
  if (retval)
    {
    this->ParseResourceQueryResponse ( );
    }
  else
    {
    vtkErrorMacro ( "XML Response dir not set." );
    }

  /*
  // TODO: move this to vtkFetchMIWebServicesClientHID when built.
  std::string att;
  std::string val;
  std::stringstream q;
  if ( !(strcmp(svctype, "HID")))
    {
    //---
    //--- HID
    //---
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    vtkHIDTagTable* t = vtkHIDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HID" ));
    if ( h && t )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- TODO: develop all HID logic. Now, not finished.
        //--- for now only these queries are supported:
        //--- find a mrml file
        //curl -k "https://loci.ucsd.edu/hid/search?exp=fBIRNPhaseII__0010&subject=000670986943&file_type=MRML" 
        //--- query for subjects by experiment
        //curl -k https://loci.ucsd.edu/hid/experiments/subjects
        //--- query for experiments
        //curl -k https://loci.ucsd.edu/hid/experiments 

        // h->QueryServer ( uri, responseFile );
        //--- Limited query capability, so just query for mrml now.
        //--- Look thru tag table for values for Experiment
        //--- and values for Subject.
        //--- if we don't find them, note as much to the user,
        //--- and don't make the query.

        //--- do a case insensitive find on the attributes
        int subjectIndex = t->CheckTableForTag("Subject");
        int experimentIndex = t->CheckTableForTag("Experiment");
        int filetypeIndex = t->CheckTableForTag("File_type");

        //--- if any of the key tags are missing.... report to user and abort query.
        if ( subjectIndex < 0 || experimentIndex < 0 || filetypeIndex < 0 )
          {
          vtkErrorMacro ( "vtkFetchMILogic: Not all required tags are on data");
          return;
          }

        //--- otherwise assemble the hid query.
        q << svrName << "/search?";

        att =  t->GetTagAttribute(experimentIndex);
        if ( att.c_str() != NULL )
          {
          if ( t->IsTagSelected(att.c_str()) )
            {
            val = t->GetTagValue(experimentIndex);
            }
          }
        if ( att.c_str() != NULL && val.c_str() != NULL )
          {
          q << "exp=";
          q << val;
          q << "&";
          }

        att =  t->GetTagAttribute(subjectIndex);
        if ( att.c_str() != NULL )
          {
          if ( t->IsTagSelected(att.c_str()) )
            {
            val = t->GetTagValue(subjectIndex);
            }
          }
        if ( att.c_str() != NULL && val.c_str() != NULL )
          {
          q << "subject=";
          q << val;
          q << "&";
          }

        att =  t->GetTagAttribute(filetypeIndex);
        if ( att.c_str() != NULL )
          {
          if ( t->IsTagSelected(att.c_str()) )
            {
            val = t->GetTagValue(filetypeIndex);
            }
          }
        if ( att.c_str() != NULL && val.c_str() != NULL )
          {
          q << "file_type=";
          q << val;
          }

        const char *errorString = h->QueryServer ( q.str().c_str(), this->GetHTTPResponseFileName() );
        if ( !strcmp(errorString, "OK" ))
          {
          this->ParseResourceQueryResponse ( );
          }
        }
      else
        {
        vtkErrorMacro ( "XML Response dir not set." );
        }
      }
    }
  */

}





//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseValuesForTagQueryResponse ( const char *att )
{
  if ( this->GetCurrentWebService()->GetParser () )
    {
    this->GetCurrentWebService()->GetParser()->SetFetchMINode ( this->FetchMINode );
    this->ClearExistingValuesForTag ( att );
    this->GetCurrentWebService()->GetParser()->ParseValuesForAttributeQueryResponse ( this->GetHTTPResponseFileName(), att );
    this->RefreshValuesForTag ( att );
    this->GetCurrentWebService()->GetParser()->SetFetchMINode ( NULL );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseTagQueryResponse ( )
{

  if ( this->GetCurrentWebService()->GetParser() )
    {
    this->GetCurrentWebService()->GetParser()->SetFetchMINode ( this->FetchMINode );
    this->ClearExistingTagsForQuery ( );
    this->GetCurrentWebService()->GetParser()->ParseMetadataQueryResponse( this->GetHTTPResponseFileName() );
    this->RefreshTags();
    this->GetCurrentWebService()->GetParser()->SetFetchMINode ( NULL );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseResourceQueryResponse ( )
{
  if ( this->GetCurrentWebService()->GetParser() )
    {
    this->GetCurrentWebService()->GetParser()->SetFetchMINode ( this->FetchMINode );
    this->GetCurrentWebService()->GetParser()->ParseResourceQueryResponse( this->GetHTTPResponseFileName() );
    this->GetCurrentWebService()->GetParser()->SetFetchMINode ( NULL );
    }

}





//----------------------------------------------------------------------------
void vtkFetchMILogic::RefreshTags()
{
  this->ClearExistingTagsForQuery();
  //--- copy parser's tags to Logic.
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->GetCurrentWebService()->GetParser()->MetadataInformation.begin();
        iter != this->GetCurrentWebService()->GetParser()->MetadataInformation.end();
        iter++ )
    {
    if ( (iter->first.c_str() != NULL) && (strcmp(iter->first.c_str(), "" )) )
      {
      this->AddUniqueTag (iter->first.c_str() );
      }
    for (unsigned int i=0; i < iter->second.size(); i++ )
      {
      if ( (iter->second[i].c_str() != NULL) && (strcmp(iter->second[i].c_str(), "")) )
        {
        this->AddUniqueValueForTag ( iter->first.c_str(), iter->second[i].c_str() );
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::RefreshValuesForTag( const char *tagname )
{
  if ( this->CurrentWebService == NULL )
    {
    vtkErrorMacro ( "RefreshValuesForTag: Got NULL CurrentWebService." );
    return;
    }
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "RefreshValuesForTag: Got NULL FetchMINode." );
    return;
    }
  if ( this->CurrentWebService->GetTagTable() == NULL )
    {
    this->GetFetchMINode()->AddTagTablesForWebServices();
    if ( this->CurrentWebService->GetTagTable() == NULL )
      {    
      this->GetFetchMINode()->AddTagTablesForWebServices();
      if ( this->CurrentWebService->GetTagTable() == NULL )
        {
        vtkErrorMacro ( "QueryServerForResources: got NULL TagTable" );
        return;
        }
      }
    }
  
  this->ClearExistingValuesForTag(tagname);
  vtkTagTable *t = this->GetCurrentWebService()->GetTagTable();
  if ( t != NULL )
    {
    //--- copy parser's values for this attribute to Logic
    std::map<std::string, std::vector<std::string> >::iterator iter;
    for ( iter = this->GetCurrentWebService()->GetParser()->MetadataInformation.begin();
          iter != this->GetCurrentWebService()->GetParser()->MetadataInformation.end();
          iter++ )
      {
      if ( !(strcmp (iter->first.c_str(), tagname) ) )
        {
        for (unsigned int i=0; i < iter->second.size(); i++ )
          {
          if ( (iter->second[i].c_str() != NULL) && (strcmp(iter->second[i].c_str(), "")) )
            {
            this->AddUniqueValueForTag ( iter->first.c_str(), iter->second[i].c_str() );
            // and update the tag table we'll use to tag data.
            t->AddOrUpdateTag (iter->first.c_str(), iter->second[i].c_str() );
            }
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearExistingTagsForQuery ( )
{

  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    iter->second.clear();
    }
  this->CurrentWebServiceMetadata.clear();

  // enforce that SlicerDataType always be present.
//  this->AddUniqueTag ( "SlicerDataType" );
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearExistingValuesForTag ( const char *tagname )
{

  //--- first find the tag, then clear all the values.
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( !(strcmp (iter->first.c_str(), tagname) ) )
      {
      iter->second.clear();
      break;
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::AddUniqueTag ( const char *tagname )
{
  //--- search thru vector of strings to see if we can find the tag.
  int unique = 1;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      unique = 0;
      break;
      }
    }

  //--- add tag if it's not already in list.
  if ( unique )
    {
    // TODO: check to see if this works
    // create and initialize a vector of strings 
    std::vector<std::string> v;
    v.clear();
    // create an entry for it in the map.
    this->CurrentWebServiceMetadata.insert ( std::make_pair (tagname, v ));
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::AddUniqueValueForTag ( const char *tagname, const char *val )
{
  //--- search thru vector of strings for tag to see if we can find the tag.
  int tagFound = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      tagFound=1;
      break;
      }
    }

  //--- add value for tag if it's not already in list.
  if ( tagFound )
    {
    int valFound = 0;
    for (unsigned int i=0; i < iter->second.size(); i++ )
      {
      if ( ! (strcmp(iter->second[i].c_str(), val)) )
        {
        valFound = 1;
        break;
        }
      }
    if ( !valFound )
      {
      iter->second.push_back ( val );
      }
    }
}




//----------------------------------------------------------------------------
int vtkFetchMILogic::GetNumberOfTagValues( const char *tagname)
{

  unsigned int numValues = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      numValues = iter->second.size();
      break;
      }
    }
  if ( numValues <= 0 )
    {
    return 0;
    }
  else
    {
    return ((int) numValues);
    }
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::GetNumberOfTagsOnServer ( )
{

  unsigned int numTags = this->CurrentWebServiceMetadata.size();
  return ( numTags );

}



//----------------------------------------------------------------------------
const char * vtkFetchMILogic::GetNthTagAttribute ( int n)
{

  int cnt = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( cnt < n )
      {
      cnt++;
      }
    else
      {
      return ( iter->first.c_str() );
      }
    }
  return ( NULL );

}


//----------------------------------------------------------------------------
const char * vtkFetchMILogic::GetNthValueForTag(const char *tagname, int n)
{

  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      if ( n <= (int)(iter->second.size()-1) )
        {
        return ( iter->second[n].c_str() );
        }
      else
        {
        return NULL;
        }
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkFetchMILogic::IsTagOnServer ( const char *tagname )
{
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentWebServiceMetadata.begin();
        iter != this->CurrentWebServiceMetadata.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      return 1;
      }
    }
  return 0;
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::GetCurrentListOfStorableNodes ( )
{
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckStorableNodesForTags ( )
{
  return 1;
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::TagStorableNodes ( )
{
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownload ( const char *uri, const char *slicerDataType)
{

  if ( this->GetCurrentWebService() == NULL )
    {
    vtkErrorMacro ( "RequestResourceDownload:Got null server.");
    std::string msg = "No download initiated; download methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;    
    }
  const char *svrName = this->GetCurrentWebService()->GetName();
  if ( svrName == NULL || !(strcmp(svrName, "" ) ) )
    {
    vtkErrorMacro ( "RequestResourceDownload: Got a NULL Server name." );
    std::string msg = "No download initiated; download methods can not file server name.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  const char *svctype = this->GetCurrentWebService()->GetServiceType();          
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "RequestResourceDownload: Got unknown web service type");
    std::string msg = "No download initiated; download methods can not file server name.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( uri == NULL || slicerDataType == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null uri or slicerDataType." );
    return;
    }
  
  //--- FOR NOW, ONLY mrml...
  //--- TODO: make this work!
  //--- make sure we have a data type we know how to handle.
/*
  if ( (strcmp (slicerDataType, "VTKModel")) &&
       (strcmp (slicerDataType, "FreeSurferModel")) &&
       (strcmp (slicerDataType, "FreeSurferModelOverlay")) &&
       (strcmp (slicerDataType, "FiducialList")) &&
       (strcmp (slicerDataType, "DTIVolume")) &&
       (strcmp (slicerDataType, "DWIVolume")) &&
       (strcmp (slicerDataType, "UnstructuredGrid")) &&
       (strcmp (slicerDataType, "ScalarVolume")) &&
       (strcmp (slicerDataType, "MRML" )))
    {
    //--- retval reports an unknown slicerDataType.
    return;    
    }
*/

  if ( (strcmp (slicerDataType, "MRML")))
    {
    //--- retval reports an unknown slicerDataType.
    vtkErrorMacro ( "RequestResourceDownload: Got non-MRML SlicerDataType");
    std::string msg = "No download initiated; download methods can not currently handle non-MRML resources.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;    
    }

  //--- handle scene with separate set of methods.
  if ( !(strcmp(slicerDataType, "MRML")))
    {
    this->RequestSceneDownload ( uri );
    }
  else 
    {
    //--- someday download other resource types
    }
  

}






//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestStorableNodeDownload ( const char *uri, const char *slicerDataType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.
  // NOT FULLY IMPLEMENTED OR TESTED

  if (uri == NULL || slicerDataType == NULL)
    {
    vtkErrorMacro("vtkFetchMILogic::RequestResourceDownload: uri isn't set or slicerDataType isn't set. uri = " << (uri == NULL ? "null" : uri) << ", slicerDataType = " << (slicerDataType == NULL ? "null" : slicerDataType));
    return;
    }
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownload: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownload: Null FetchMI node. ");
    return;
    }

  
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownload: Null URIHandler. ");
    return;
    }

  std::string uriString(uri);
  std::string suffix;
  // get all charactersup to the ://
  size_t index;
  if ((index = uriString.find("://", 0) ) != std::string::npos)
    {
    suffix = uriString.substr(index, std::string::npos);
    uriString = std::string("xnd") + suffix;
    vtkDebugMacro("vtkFetchMILogic::RequestResourceDownload: new uri string = " << uriString.c_str());
    }
    
  if ( !(strcmp(slicerDataType, "LabelMap" )))
    {
    // is it a label map?
    }
  if ( !(strcmp(slicerDataType, "ScalarVolume" )))
    {
    //--- create the node, assume scalar
    vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
    vtkMRMLScalarVolumeDisplayNode  *sdisplayNode = NULL;
    vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
    // set the volume name
    std::string volumeName;
    volumeName = vtksys::SystemTools::GetFilenameName(uriString);
    scalarNode->SetName(volumeName.c_str());
    // set the uri
    storageNode->SetURI(uriString.c_str());
     // need to set the scene on the storage node so that it can test looking
     // for file handlers, but setting ti explicitly
    storageNode->SetScene(this->GetMRMLScene());

    //--- set its URIhandler
    storageNode->SetURIHandler(handler);
    // set other flags on the storage node
    /*
    storageNode->SetCenterImage(centerImage);
    storageNode->SetSingleFile(singleFile);
    storageNode->SetUseOrientationFromFile(useOrientationFromFile);
    */
    this->GetMRMLScene()->SaveStateForUndo();
   
    // add storage nodes to the scene so can observe them
    this->GetMRMLScene()->AddNodeNoNotify(storageNode);
    this->GetMRMLScene()->AddNodeNoNotify(scalarNode);
    scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
    this->GetMRMLScene()->AddNodeNoNotify(sdisplayNode);
    scalarNode->SetAndObserveDisplayNodeID(sdisplayNode->GetID());

    //--- download to cache and load.
    storageNode->ReadData(scalarNode);

    // since added the node w/o notification, let the scene know now that it
    // has a new node
    this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, scalarNode);

    // clean up
    if (scalarNode)
      {
      scalarNode->Delete();
      }
    if (storageNode)
      {
      storageNode->Delete();
      }
    if (sdisplayNode)
      {
      sdisplayNode->Delete();
      }

    }
  
  // get the models logic
  if ( !(strcmp(slicerDataType, "VTKModel" ) ||
         !(strcmp(slicerDataType, "FreeSurferModel" )) ) )
    {
    }
  if ( !(strcmp(slicerDataType, "VTKModel" )))
    {
    //--- create the node
    vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
    vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
    vtkMRMLModelStorageNode *mStorageNode = vtkMRMLModelStorageNode::New();

    // set the uri
    mStorageNode->SetURI(uriString.c_str());
    modelNode->SetName("XNDModel");

    //--- set its URIhandler
    mStorageNode->SetURIHandler(handler);

    //--- download to cache and load.
    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    mStorageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene()); 

    this->GetMRMLScene()->AddNodeNoNotify(mStorageNode);  
    this->GetMRMLScene()->AddNodeNoNotify(displayNode);
    modelNode->SetAndObserveStorageNodeID(mStorageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  
    displayNode->SetPolyData(modelNode->GetPolyData());
    
    this->GetMRMLScene()->AddNode(modelNode);  
    // the scene points to it still
    modelNode->Delete();

    int retval = mStorageNode->ReadData(modelNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddModel: error reading " << uriString.c_str());
      this->GetMRMLScene()->RemoveNode(modelNode);
      this->GetMRMLScene()->RemoveNode(mStorageNode);
      this->GetMRMLScene()->RemoveNode(displayNode);
      }
    mStorageNode->Delete();
    displayNode->Delete();
    }

  if ( !(strcmp(slicerDataType, "FreeSurferModel" )))
    {
    //--- create the node, assuming a free surfer model for now
    vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
    vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
    vtkMRMLFreeSurferModelStorageNode *fsmStorageNode = vtkMRMLFreeSurferModelStorageNode::New();
    fsmStorageNode->SetUseStripper(0);  // turn off stripping by default (breaks some pickers)

    // set the uri
    fsmStorageNode->SetURI(uriString.c_str());
    modelNode->SetName("XNDFSModel");
    
    //--- set its URIhandler
    fsmStorageNode->SetURIHandler(handler);
    
    //--- download to cache and load.
    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    fsmStorageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene()); 

    this->GetMRMLScene()->AddNodeNoNotify(fsmStorageNode);  
    this->GetMRMLScene()->AddNodeNoNotify(displayNode);
    modelNode->SetAndObserveStorageNodeID(fsmStorageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  
    displayNode->SetPolyData(modelNode->GetPolyData());
    
    this->GetMRMLScene()->AddNode(modelNode);  
    // the scene points to it still
    modelNode->Delete();

    int retval = fsmStorageNode->ReadData(modelNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddModel: error reading " << uriString.c_str());
      this->GetMRMLScene()->RemoveNode(modelNode);
      this->GetMRMLScene()->RemoveNode(fsmStorageNode);
      this->GetMRMLScene()->RemoveNode(displayNode);
      }
    fsmStorageNode->Delete();
    displayNode->Delete();
    }
}







//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownload ( const char *uri )
{
  // This method will download a resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( uri == NULL )
    {
    vtkErrorMacro ( "RequestSceneDownload: uri for MRMLScene is NULL.");
    std::string msg = "No download initiated; URI for MRMLScene is NULL.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "RequestSceneDownload: MRMLScene is NULL.");
    std::string msg = "No download initiated; download methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetMRMLScene()->GetCacheManager() == NULL )
    {
    vtkErrorMacro ( "RequestSceneDownload: MRMLscene's CacheManager is NULL.");
    std::string msg = "No download initiated; download methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "RequestSceneDownload: FetchMINode is NULL.");
    std::string msg = "No download initiated; download methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  if ( this->GetCurrentWebService() == NULL )
    {
    vtkErrorMacro ( "RequestSceneDownload:Got null server.");
    std::string msg = "No download initiated; download methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;    
    }
  if ( this->GetCurrentWebService()->GetWebServicesClient()  == NULL )
    {
    vtkErrorMacro ( "RequestSceneDownload:Got a null WebServicesClient" );
    std::string msg = "No download initiated; download methods are not properly configured.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  std::string svrName = this->GetCurrentWebService()->GetName();
  if (svrName.c_str() == NULL || !(strcmp(svrName.c_str(), "") ) )
    {
    vtkErrorMacro ("RequestSceneDownload: Null or empty server name." );
    std::string msg = "Null or empty server name; no download initiated.";
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  vtkURIHandler *handler = this->GetCurrentWebService()->GetURIHandler();
  if ( handler == NULL )
    {
    vtkErrorMacro ("RequestSceneDownload: Null URI handler." );
    std::string msg = "No download initiated; download methods cannot find URI handler." ;
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }
  handler->SetHostName (svrName.c_str() );
  this->GetCurrentWebService()->GetWebServicesClient()->SetURIHandler ( handler );

  std::string remoteURL = uri;
  std::string localURL;
  if ( this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri) != NULL )
    {
    localURL = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri);    
    }
  else
    {
    localURL = "DownloadedSlicerScene1.mrml";
    }

  // do a synchronous dl
  this->GetCurrentWebService()->GetWebServicesClient()->Download ( remoteURL.c_str(), localURL.c_str() );

  //---
  //--- check to see if the downloaded MRML file is valid, or it it contains
  //--- an HTTP error response.
  //---
  ifstream newfile;
#ifdef _WIN32
      newfile.open ( localURL.c_str(), ios::binary | ios::in );
#else
      newfile.open ( localURL.c_str(), ios::in );
#endif
  
      int downloadError = 0;
      //--- did it open correctly?
      if ( !newfile.is_open() )
      {
      vtkErrorMacro ( "DeleteSceneFromServer: error opening temporary scene file to parse for uris to delete." );
      std::string msg = "Cannot open downloaded file ";
      msg += localURL.c_str();
      this->FetchMINode->SetErrorMessage (msg.c_str() );
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return;
      }
      //--- is it bigger than 0 bytes?
      newfile.seekg(0, std::ios::end);
      int fsize = newfile.tellg();
      newfile.seekg(0);
      if ( fsize <= 0 )
        {
        vtkErrorMacro ( "DeleteSceneFromServer: error opening temporary scene file to parse for uris to delete." );
        std::string msg = "Downloaded file ";
        msg += localURL.c_str();
        msg += " is zero length.";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return;
        }

      //--- does it contain an http error code?      
      //--- for now, look for a line with t<title> tag and see if it contains the word "Error"
      std::string line;
      size_t pos;
      while ( !newfile.eof() )
      {
      line.clear();
      newfile >> line;
      pos = line.find ( "HTTP ERROR", 0 );
      if ( pos != std::string::npos )
        {
        downloadError = 1;
        break;
        }
      }
      newfile.close();

      //--- return an error message and abort if bad DL.
      if ( downloadError )
        {
        vtkErrorMacro ( "" );
        std::string msg = "Error downloading uri";
        msg += uri;
        this->FetchMINode->SetErrorMessage ( msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return;
        }
      
  //---
  //--- If no errors, proceed with DL
  //---
  // now override the mrml scene's url to point to file on disk
  this->GetMRMLScene()->SetURL(localURL.c_str());
  //--- load the remote scene
  this->GetMRMLScene()->Connect();

  if (  this->GetMRMLScene()->GetErrorCode() != 0 ) 
    {
    vtkErrorMacro ("RequestSceneDownload: Null or empty server name." );
    std::string msg = this->GetMRMLScene()->GetErrorMessage();
    this->FetchMINode->SetErrorMessage (msg.c_str() );
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return;
    }

  //--- The scene may have a new interaction node which
  //--- specifies a Mouse Mode -- but no InteractionModeChangedEvent
  //--- will have been invoked for the GUI to capture.
  //--- So we invoke the event here after the scene is finished loading.
  if ( this->GetApplicationLogic() )
    {
    if ( this->GetApplicationLogic()->GetInteractionNode() )
      {
      this->GetApplicationLogic()->GetInteractionNode()->InvokeEvent (
        vtkMRMLInteractionNode::InteractionModeChangedEvent );
      }
    }
}







//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearModifiedNodes()
{
  this->ModifiedNodes.clear();
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::AddModifiedNode( const char *nodeID)
{
  unsigned int z = this->ModifiedNodes.size();
  int unique = 1;

  for (unsigned int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->ModifiedNodes[i].c_str(), nodeID )))
      {
      unique = 0;
      }
    }


  if ( unique )
    {
    this->ModifiedNodes.push_back(nodeID);
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::RemoveModifiedNode( const char *nodeID)
{
  unsigned int z = this->ModifiedNodes.size();
  for (unsigned int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->ModifiedNodes[i].c_str(), nodeID )))
      {
      this->ModifiedNodes.erase(this->ModifiedNodes.begin()+i);
      }
    }
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearSelectedStorableNodes ()
{
  this->SelectedStorableNodeIDs.clear();
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::AddSelectedStorableNode( const char *nodeID)
{
  unsigned int z = this->SelectedStorableNodeIDs.size();
  int unique = 1;

  for (unsigned int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->SelectedStorableNodeIDs[i].c_str(), nodeID )))
      {
      unique = 0;
      }
    }

  if ( unique )
    {
    this->SelectedStorableNodeIDs.push_back(nodeID);
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::RemoveSelectedStorableNode( const char *nodeID)
{
  unsigned int z = this->SelectedStorableNodeIDs.size();
  for (unsigned int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->SelectedStorableNodeIDs[i].c_str(), nodeID )))
      {
      this->SelectedStorableNodeIDs.erase(this->SelectedStorableNodeIDs.begin()+i);
      }
    }
}



//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckStorageNodeFileNames()
{
  // Before upload, check to make sure all storable nodes have
  // set storage nodes with filenames

  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("CheckStorageNodeFileNames: got a Null scene. ");
    return(0);
    }

  vtkMRMLNode *node;
  vtkMRMLStorageNode *snode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    node = this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] );
    if ( node )
      {
      snode = vtkMRMLStorableNode::SafeDownCast(node)->GetStorageNode();
      if (! snode )
        {
        vtkErrorMacro("CheckStorageNodeFileNames: found a node with null storage node checked for upload.");
        return (0);
        }
      }
    }
  return (1);

}


//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckModifiedSinceRead()
{
  // Before upload, check to make sure all storable nodes have
  // set storage nodes with filenames

  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("CheckModifiedSinceRead: Null scene. ");
    return(0);
    }

  vtkMRMLNode *node;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    node = this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] );
    if (node->GetModifiedSinceRead())
      {
      vtkErrorMacro("CheckModifiedSinceRead: error, node " << this->SelectedStorableNodeIDs[n] << " has been modified. Please save all unsaved nodes first");
      return (0);
      }
    }
  return (1);
  
}



//---------------------------------------------------------------------------
void vtkFetchMILogic::SaveNewURIOnSelectedResource ( const char *olduri, const char *newuri )
{
  //--- find old uri in the list and map it to the  new uri name.
  if ( newuri == NULL )
    {
    return;
    }

  std::string newstr = newuri;
  std::string oldstr;
  if (  olduri == NULL )
    {
    oldstr = "NULL";
    }
  else
    {
    oldstr = olduri;
    }
  const char *otst;
  const char *ntst;

  std::map<std::string, std::string>::iterator iter;
  for ( iter = this->OldAndNewURIs.begin();
        iter != this->OldAndNewURIs.end(); iter++)
    {
    if ( iter->first == oldstr )
      {
      iter->second.clear();
      iter->second = newstr;
      //--- test
      otst = iter->first.c_str();
      ntst = iter->second.c_str();
      }
    }
}




//---------------------------------------------------------------------------
void vtkFetchMILogic::SaveOldURIsOnSelectedResources()
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("RestoreFileNamesOnSelectedResources Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("RestoreFileNamesOnSelectedResources Null FetchMI node. ");
    return;
    }
  
  //--- clear out the map.
  this->OldAndNewURIs.clear();
  std::string tmp = "none";
  std::string uri;
  std::string sceneFileName = this->GetMRMLScene()->GetURL();
  
  this->OldAndNewURIs.insert ( std::make_pair (sceneFileName, tmp ) );

  vtkMRMLStorableNode *storableNode;  
  vtkMRMLStorageNode *storageNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    //--- get the storable node
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    if ( storableNode )
      {
      //--- for each storage node on the storable node.
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      //
      for (int i = 0; i < numStorageNodes; i++)
        {
        //--- get the storage node
        storageNode = storableNode->GetNthStorageNode(i);
        if ( storageNode )
          {
          //--- add this.
          uri.clear();
          if ( storageNode->GetURI() != NULL )
            {
            uri = storageNode->GetURI();
            this->OldAndNewURIs.insert ( std::make_pair (uri, tmp ) );
            }

          //--- now grab all its uri list members.
          int numStorageNodeURIs = storageNode->GetNumberOfURIs();
          for ( int j = 0; j < numStorageNodeURIs; j++ )
            {
            uri.clear();
            if ( storageNode->GetNthURI(j) != NULL )
              {
              uri = storageNode->GetNthURI(j);
              this->OldAndNewURIs.insert ( std::make_pair (uri, tmp ) );
              }
            }
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
int vtkFetchMILogic::RestoreURIsOnSelectedResources ( )
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("RestoreURIsOnSelectedResources Null scene. ");
    return 0;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("RestoreURIsOnSelectedResources Null FetchMI node. ");
    return 0;
    }

  //--- how many things to restore?
  unsigned int numToRestore = this->OldAndNewURIs.size();
  if (numToRestore == 0 )
    {
    vtkWarningMacro ( "RestoreURIsOnSelectedResources: found no files to restore.");
    return 0;    
    }

  // restore the scene filename.

  vtkMRMLStorableNode *storableNode;
  vtkMRMLStorageNode *storageNode;
  unsigned int restoreCount = 0;
  std::string tstURI;
  std::map<std::string, std::string>::iterator iter;
  std::map<std::string, std::string>::iterator iter2;

  //--- not sure we need to do this here; might be undoing the restore of filenames.
  //--- which includes the MRML scene's URL. (nodes have uri and filename, scene has url only)
  /*
  for ( iter = this->OldAndNewURIs.begin();
        iter != this->OldAndNewURIs.end(); )
    {
    if ( this->SceneSelected )
      {
      //--- find new name
      tstURI = this->GetMRMLScene()->GetURL();
      if ( iter->second == tstURI )
        {
        //--- restore to previous name and check for no-name.
        this->GetMRMLScene()->SetURL ( iter->first.c_str() );
        if ( iter->first.c_str() == NULL || !(strcmp (iter->first.c_str(), "" )) )
          {
          this->GetMRMLScene()->SetURL ( "SlicerScene1.mrml" );
          }
        restoreCount ++;
        this->OldAndNewURIs.erase(iter++);
        break;
        }
      else
        {
        ++iter;
        }
      }
    }
  */

  //--- account for the scene uri which is in the list,
  //--- but which we've chosen not to restore by
  //--- commenting out loop above.
  restoreCount++;

  //--- restore storable nodes' filenames and filenamelists.
  //--- For each URI or URIListMember, if there's no appropriate
  //--- value to restore it to, then restore it to NULL, which
  //--- is its initial value.
  std::vector<std::string> URIList;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    if ( storableNode )
      {
      //--- for each storage node on the storable node.
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      int numStorageNodeURIs;
      int foundURIOnNode;
      int foundURIListMemberOnNode;
      for (int i = 0; i < numStorageNodes; i++)
        {
        foundURIOnNode = 0;
        //--- get the storage node
        storageNode = storableNode->GetNthStorageNode(i);
        if ( storageNode != NULL && storageNode->GetURI() != NULL )
          {
          tstURI.clear();
          //--- look thru the names map
          //--- here we have two iterators moving thru the map,
          //--- and modifying it. So take care to make sure maps
          //--- are valid and iterators don't overstep. I think this
          //--- works.
          tstURI = storageNode->GetURI();
          iter = this->OldAndNewURIs.begin();
          while ( this->OldAndNewURIs.size() != 0 && iter != this->OldAndNewURIs.end() )
            {
            if  (iter->second == tstURI )  
              {
              //--- restore the filename, count it and mark found
              if ( iter->first == "NULL" )
                {
                storageNode->SetURI ( NULL );
                }
              else
                {
                storageNode->SetURI ( iter->first.c_str() );
                }
              restoreCount++;
              foundURIOnNode = 1;
              //--- get rid of the name just restored in the map.
              this->OldAndNewURIs.erase ( iter++);

              //--- prepare to restore the filenamelist enmasse
              URIList.clear();
              numStorageNodeURIs = storageNode->GetNumberOfURIs();
              for (int filenum = 0; filenum < numStorageNodeURIs; filenum++)
                {
                URIList.push_back( storageNode->GetNthURI(filenum) );
                }

              //--- reset all the filenames on the node.
              storageNode->ResetURIList();

              //--- check to see if we have what we expect...
              if ( (int)(URIList.size()) != numStorageNodeURIs )
                {
                vtkErrorMacro ("RestoreURIsOnSelectedResources: can not restore the number of File names expected.");
                storageNode->ResetURIList();
                storageNode->SetURI ( NULL );
                return 0;    
                }

              //--- search thru the map for each uri list member
              //--- and restore them if their new values correspond.
              for (int filenum = 0; filenum < numStorageNodeURIs; filenum++)
                {
                foundURIListMemberOnNode = 0;
                for ( iter2 = this->OldAndNewURIs.begin();
                      iter2 != this->OldAndNewURIs.end();  )
                  {
                  if  (iter2->second == URIList[filenum] )
                    {
                    if ( iter2->first != "NULL")
                      {
                      storageNode->AddURI ( iter2->first.c_str() );
                      }
                    restoreCount ++;
                    foundURIListMemberOnNode = 1;
                    this->OldAndNewURIs.erase( iter2++);
                    }
                  else
                    {
                    ++iter2;
                    }
                  }
                if ( !foundURIListMemberOnNode )
                  {
                  // not so sure about this one...
                  // null out the works.
                  storageNode->ResetURIList();
                  storageNode->SetURI ( NULL );
                  break;
                  }
                }
              }
            else
              {
              ++iter;
              }
            }
          }
        //--- finally, if we haven't found an old uri to restore on the node,
        //--- set its uri to null.
        if ( !foundURIOnNode )
          {
          storageNode->ResetURIList();
          storageNode->SetURI ( NULL );
          }
        }
      }
    }

  this->OldAndNewURIs.clear();
  if ( restoreCount != numToRestore )
    {
    vtkErrorMacro ("RestoreURIsOnSelectedResources: did not restore the number of File names expected.");
    return 0;    
    }
  return 1;
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::RestoreFileNamesOnSelectedResources ( )
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("RestoreFileNamesOnSelectedResources Null scene. ");
    return 0;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("RestoreFileNamesOnSelectedResources Null FetchMI node. ");
    return 0;
    }


  //--- how many things to restore?
  unsigned int numToRestore = this->OldAndNewFileNames.size();
  if (numToRestore == 0 )
    {
    vtkWarningMacro ( "RestoreFileNamesOnSelectedResources: found no files to restore.");
    return 0;    
    }

  // restore the scene filename.
  vtkMRMLStorableNode *storableNode;
  vtkMRMLStorageNode *storageNode;
  unsigned int restoreCount = 0;
  std::string tstName;
  std::map<std::string, std::string>::iterator iter;
  std::map<std::string, std::string>::iterator iter2;
  for ( iter = this->OldAndNewFileNames.begin();
        iter != this->OldAndNewFileNames.end(); )
    {
    if ( this->SceneSelected )
      {
      //--- find new name
      if ( this->GetMRMLScene()->GetURL() != NULL )
        {
        tstName = this->GetMRMLScene()->GetURL();
        if ( iter->second == tstName )
          {
          //--- restore to previous name and check for no-name.
          this->GetMRMLScene()->SetURL ( iter->first.c_str() );
          if ( iter->first.c_str() == NULL || !(strcmp (iter->first.c_str(), "" )) )
            {
            this->GetMRMLScene()->SetURL ( "SlicerScene1.mrml" );
            }
          restoreCount ++;
          this->OldAndNewFileNames.erase(iter++);
          break;
          }
        else
          {
          ++iter;
          }
        }
      }
    }


  //--- restore storable nodes' filenames and filenamelists.
  std::vector<std::string> FileNameList;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    if ( storableNode )
      {

      //--- for each storage node on the storable node.
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      int numStorageNodeFileNames;
      for (int i = 0; i < numStorageNodes; i++)
        {
        //--- get the storage node
        storageNode = storableNode->GetNthStorageNode(i);
        if ( storageNode != NULL && storageNode->GetFileName() != NULL )
          {
          tstName.clear();
          tstName = storageNode->GetFileName();

          //--- look thru the names map
          //--- here we have two iterators moving thru the map,
          //--- and modifying it. So take care to make sure maps
          //--- are valid and iterators don't overstep. I think this
          //--- works.
          iter = this->OldAndNewFileNames.begin();
          while ( this->OldAndNewFileNames.size() != 0 && iter != this->OldAndNewFileNames.end() )
            {
            if  (iter->second == tstName )  
              {
              //--- restore the filename
              storageNode->SetFileName ( iter->first.c_str() );              
              restoreCount++;

              //--- prepare to restore the filenamelist enmasse
              FileNameList.clear();
              numStorageNodeFileNames = storageNode->GetNumberOfFileNames();
              for (int filenum = 0; filenum < numStorageNodeFileNames; filenum++)
                {
                if ( storageNode->GetNthFileName(filenum) != NULL )
                  {
                  FileNameList.push_back( storageNode->GetNthFileName(filenum) );
                  }
                }

              //--- check to see if we have what we expect...
              if ( (int)(FileNameList.size()) != numStorageNodeFileNames )
                {
                vtkErrorMacro ("RestoreFileNamesOnSelectedResources: can not restore the number of File names expected.");
                return 0;    
                }

              //--- get rid of the name just restored.
              this->OldAndNewFileNames.erase ( iter++);
              
              //--- reset all the filenames.
              storageNode->ResetFileNameList();

              //--- search thru the map for each name,
              //--- add them back in if they correspond.
              for (int filenum = 0; filenum < numStorageNodeFileNames; filenum++)
                {
                for ( iter2 = this->OldAndNewFileNames.begin();
                      iter2 != this->OldAndNewFileNames.end();  )
                  {
                  if  (iter2->second == FileNameList[filenum] )
                    {
                    storageNode->AddFileName ( iter2->first.c_str() );
                    restoreCount ++;
                    this->OldAndNewFileNames.erase( iter2++);
                    }
                  else
                    {
                    ++iter2;
                    }
                  }
                }
              }
            else
              {
              ++iter;
              }
            }
          }
        }
      // end for each storage node on the storable node
      }
    }

  this->OldAndNewFileNames.clear();
  if ( restoreCount != numToRestore )
    {
    vtkErrorMacro ("RestoreFileNamesOnSelectedResources: did not restore the number of File names expected.");
    return 0;    
    }
  return 1;
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::SetCacheFileNamesOnSelectedResources ( )
{
  //---
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // (This vector of strings is populated by the GUI when upload button is
  // selected)
  //---
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetCacheFileNamesOnSelectedResources Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetCacheFileNamesOnSelectedResources Null FetchMI node. ");
    return;
    }

  // clear buffer to save original filenames (scene first).
  this->OldAndNewFileNames.clear();
  
  // get the cache dir
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory(), pathComponents);

  //--- scene
  if ( this->SceneSelected )
    {
    //--- save in case we need to restore.
    vtksys_stl::string sceneFileName = this->GetMRMLScene()->GetURL();


    vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (sceneFileName );
    const char *mrmlFileName = vtkFileName.c_str();
    if ( mrmlFileName== NULL || (!strcmp(mrmlFileName, "" )) )
      {
      // this mimics what SlicerMRMLSaveDataWidget does.
      mrmlFileName = "SlicerScene1.mrml";
      }
    
    // addthe mrml file
    pathComponents.push_back(mrmlFileName);
    // set the new url
    vtksys_stl::string tmp = vtksys::SystemTools::JoinPath(pathComponents);
    mrmlFileName = tmp.c_str();
    vtkDebugMacro("vtkFetchMILogic::SetCacheFileNamesOnSelectedResources setting scene url to " << mrmlFileName);
    this->GetMRMLScene()->SetURL(mrmlFileName);

    //--- add both old and new filenames as a pair in case we need to restore..
    this->OldAndNewFileNames.insert ( std::make_pair (sceneFileName, mrmlFileName ) );
    }


  //--- storable nodes
  vtkMRMLStorableNode *storableNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    if ( storableNode )
      {

      // for each storage node
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      vtkMRMLStorageNode *storageNode;
      for (int i = 0; i < numStorageNodes; i++)
        {
        storageNode = storableNode->GetNthStorageNode(i);
        if ( storageNode )
          {
          //---
          // SPECIAL CASE: NEED TO HANDLE  A FEW NODE TYPES SEPARATELY WAY BECAUSE THEY HAVE NO WRITER!
          //---
          vtkMRMLFreeSurferModelStorageNode *fsmnode = vtkMRMLFreeSurferModelStorageNode::SafeDownCast ( storageNode );
          if ( fsmnode != NULL )
            {
            //--- no writer to write vtkMRMLFreeSurferModelStorageNodes or
            //--- vtkMRMLFreeSurferModelOverlayStorageNodes to cache.
            //--- instead, we'll find them later and copy them from disk to cache.
            //--- keep their original filename.
            continue;
            }
          vtkMRMLFreeSurferModelOverlayStorageNode *fsmonode = vtkMRMLFreeSurferModelOverlayStorageNode::SafeDownCast ( storageNode );
          if ( fsmonode != NULL )
            {
            //--- no writer to write vtkMRMLFreeSurferModelStorageNodes or
            //--- vtkMRMLFreeSurferModelOverlayStorageNodes to cache.
            //--- instead, we'll find them later and copy them from disk to cache.
            //--- keep their original filename.
            continue;
            }
          //---
          // END SPECIAL CASE
          //---

          // FOR EACH FILENAME & FILELISTMEMBER IN EACH NODE:
          // {
          //--- Set Filename to be cachedir/filename
          // get out the file name and save in case we need to restore.
          vtksys_stl::string nodeFileName = storageNode->GetFileName();
      
          vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (nodeFileName );
          const char *filename = vtkFileName.c_str();
     
          // add the file name to the cache dir
          pathComponents.pop_back();
          pathComponents.push_back(filename);
          // set it
          vtkDebugMacro("SetCacheFileNamesOnSelectedResources setting file name " << vtksys::SystemTools::JoinPath(pathComponents).c_str() << " to storage node " << storageNode->GetID());
          vtksys_stl::string tmp = vtksys::SystemTools::JoinPath(pathComponents);
          storageNode->SetFileName( tmp.c_str() );

          //--- add both old and new filenames as a pair, in case we need to restore.
          this->OldAndNewFileNames.insert ( std::make_pair (nodeFileName.c_str(), tmp.c_str() ) );          

          //--- If the node is a multivolume, set the filename and all FileListMembers to
          //--- corresponding cachedir/filenames using AddFileName() method.
          // make up a vector of the new file names
          std::vector<std::string> CacheFileNameList;
          std::vector<std::string> orig;

          CacheFileNameList.clear();
          orig.clear();
          for (int filenum = 0; filenum < storageNode->GetNumberOfFileNames(); filenum++)
            {
            vtksys_stl::string nthFilename = storageNode->GetNthFileName(filenum);
            vtkFileName = vtksys::SystemTools::GetFilenameName(nthFilename);
            //--- save in case we need to restore.
            //--- add with path.
            orig.push_back ( nthFilename.c_str() );

            nthFilename =  vtkFileName.c_str();
            pathComponents.pop_back();
            pathComponents.push_back(nthFilename);
            vtkDebugMacro("SetCacheFileNamesOnSelectedResources adding file name " << vtksys::SystemTools::JoinPath(pathComponents).c_str() << " to list of new file names");
            vtksys_stl::string ttmp = vtksys::SystemTools::JoinPath(pathComponents);
            CacheFileNameList.push_back(ttmp.c_str());
            }
          // reset the file list
          storageNode->ResetFileNameList();
          // now add the new ones back in
          for (unsigned int filenum = 0; filenum < CacheFileNameList.size(); filenum++)
            {
            vtkDebugMacro("SetCacheFileNamesOnSelectedResources adding file name " << CacheFileNameList[filenum] << " to storage node " << storageNode->GetID());
            storageNode->AddFileName(CacheFileNameList[filenum].c_str());

            //--- add both old and new filenames as a pair, in case we need to restore.
            this->OldAndNewFileNames.insert ( std::make_pair (orig[filenum].c_str(), CacheFileNameList[filenum].c_str() ) );          
            }

          //--- Write the file (or multivolume set of files) to cache.
          //--- USE GetNumberOfFileNames to get the number of FileListMembers.
          //--- USE GetNthFileName to get each, probably SetNthFileName to set each.
          //--- If this fails, error message and return.
      
          //--- Set the URIHandler on the storage node
          vtkDebugMacro("SetCacheFileNamesOnSelectedResources setting handler on storage node " << (storageNode->GetID() == NULL ? "(null)" : storageNode->GetID()));
          // *NOTE: make sure to see that DataIOManagerLogic (or whatever) checks to see
          // if the URIHandler is set before calling CanHandleURI() on all scene handlers,
          // and if not, fix this logic to use the storage node's handler as set.
          // } // end LOOP THRU NODES.
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::SetIdleWriteStateOnSelectedResources ( )
{
    if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("SetIdleWriteStateOnSelectedResources: Null scene. ");
    return;
    }

  //--- storable nodes
  vtkMRMLStorableNode *storableNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      storageNode->SetWriteStateIdle();
      }
    }

}

//----------------------------------------------------------------------------
void vtkFetchMILogic::SetTransferDoneWriteStateOnSelectedResources ( )
{
    if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("SetTransferDoneWriteStateOnSelectedResources: Null scene. ");
    return;
    }

  //--- storable nodes
  vtkMRMLStorableNode *storableNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      storageNode->SetWriteStateTransferDone();
      }
    }

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SetCancelledWriteStateOnSelectedResources ( )
{
    if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetCancelledWriteStateOnSelectedResources: Null scene. ");
    return;
    }

  //--- storable nodes
  vtkMRMLStorableNode *storableNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      storageNode->SetWriteStateCancelled();
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SetURIHandlerOnSelectedResources(vtkURIHandler *handler)
{
  //---
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // (This vector of strings is populated by the GUI when upload button is
  // selected) Set handler.
  //---

  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetURIHandlerOnSelectedResources: Null URIHandler. ");
    return;
    }
    if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetURIHandlerOnSelectedResources: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetURIHandlerOnSelectedResources: Null FetchMI node. ");
    return;
    }
  
  //--- scene
  if ( this->SceneSelected )
    {
    }

  //--- storable nodes
  vtkMRMLStorableNode *storableNode;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      storageNode->SetURIHandler(handler);
      }
    }
}


//----------------------------------------------------------------------------
int  vtkFetchMILogic::PostStorableNodes()
{
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // FOR EACH FILENAME & FILELISTMEMBER IN EACH SELECTED NODE with valid uri:

  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "PostStorableNodes: got NULL mrml Scene." );
    return 0;
    }
  if ( this->GetMRMLScene()->GetCacheManager() == NULL )
    {
    vtkErrorMacro ( "PostStorableNodes: Got NULL cachemanager." );
    return 0;
    }
  if ( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory() == NULL )
    {
    vtkErrorMacro ( "PostStorableNodes: RemoteCacheDirectory not set!" );
    return 0;
    }

  
  vtkMRMLStorableNode *storableNode;
  vtkMRMLStorageNode *storageNode;

  //--- get the cache directory.
  // get the cache dir
  vtksys_stl::string dummy = "dummy.txt";
  vtksys_stl::vector<std::string> pathComponents;
  vtksys_stl::string nodeFileName;
  vtksys_stl::string vtkFileName;

  vtksys::SystemTools::SplitPath( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory(), pathComponents);
  pathComponents.push_back(dummy.c_str());
  
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    std::string nodeID = this->SelectedStorableNodeIDs[n];
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));

    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();

    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      if (storageNode->GetURIHandler() == NULL)
        {
        //  we set it to null on error above
        vtkWarningMacro("PostStorableNodes: not writing on storage node " << i);
        return ( 0 );
        }
      else
        {
        //---
        // SPECIAL CASE: NEED TO HANDLE  A FEW NODE TYPES SEPARATELY WAY BECAUSE THEY HAVE NO WRITER.
        //---
        vtkMRMLFreeSurferModelOverlayStorageNode *fsmo_snode = vtkMRMLFreeSurferModelOverlayStorageNode::SafeDownCast ( storageNode );
        vtkMRMLFreeSurferModelStorageNode *fsm_snode = vtkMRMLFreeSurferModelStorageNode::SafeDownCast ( storageNode );
        if ( fsmo_snode != NULL ) 
          {
          //--- add the original file name to the cache dir
          vtkFileName.clear();
          pathComponents.pop_back();

          vtkFileName = vtksys::SystemTools::GetFilenameName (storageNode->GetFileName() );
          pathComponents.push_back(vtkFileName.c_str() );
          vtksys_stl::string cacheName = vtksys::SystemTools::JoinPath(pathComponents);

          if ( !(fsmo_snode->CopyData ( storableNode, cacheName.c_str() )) )
            {
            vtkErrorMacro ( "PostStorableNodes: Write call failed to create instance of FreeSurfer data in cache." );
            return ( 0 );
            }
          }
        else if ( fsm_snode != NULL  )
          {
          //--- add the original file name to the cache dir
          vtkFileName.clear();
          pathComponents.pop_back();

          vtkFileName = vtksys::SystemTools::GetFilenameName (storageNode->GetFileName() );
          pathComponents.push_back(vtkFileName.c_str() );
          vtksys_stl::string cacheName = vtksys::SystemTools::JoinPath(pathComponents);

          if ( !(fsm_snode->CopyData ( storableNode, cacheName.c_str() )) )
            {
            vtkErrorMacro ( "PostStorableNodes: Write call failed to create instance of FreeSurfer data in cache." );
            return ( 0 );
            }
          }
        else
          {
        //---
        // END SPECIAL CASE
        //---
          vtkDebugMacro("PostStorableNodes: calling write data on storage node " << i << ": " << storageNode->GetID());
          if (!storageNode->WriteData(storableNode))
            {
            vtkErrorMacro("PostStorableNodes: WriteData call failed on storage node " << storageNode->GetID() << " for node " << storableNode->GetName());
            return ( 0 );
            }
          }
        }
      }
    }
  return ( 1 );
}




