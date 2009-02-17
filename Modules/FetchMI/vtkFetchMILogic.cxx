#include <string>
#include <iostream>
#include <sstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObjectFactory.h"

#include "vtkMRMLScene.h"
#include "vtkCacheManager.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkHTTPHandler.h"
#include "vtkHIDHandler.h"
#include "vtkXNDHandler.h"
#include "vtkXMLDataParser.h"

#include "vtkFetchMILogic.h"
#include "vtkFetchMI.h"

#include "vtkFetchMIWriterXND.h"
#include "vtkFetchMIParserXND.h"
#include "vtkFetchMIClientCallerXND.h"

#include "vtkKWMessageDialog.h"

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
  this->ResourceQuery = 0;
  this->TagAttributesQuery = 0;
  this->TagValuesQuery = 0;
  this->ErrorChecking = 0;
  this->ParsingError = 0;
 
  this->ServerCollection = vtkFetchMIServerCollection::New();
  this->CurrentServer = NULL;
  this->ReservedURI = NULL;

  // Temporary vars used for parsing xml.
  this->NumberOfTagsOnServer = 0;
//  this->DebugOn();

   //--- Initialize the XML writer collection with writers for all known services.
   //--- Extend here as we support new web services.
   vtkFetchMIWriterXND *xndw = vtkFetchMIWriterXND::New();
   vtkFetchMIParserXND *xndp = vtkFetchMIParserXND::New();
   vtkFetchMIClientCallerXND *xndc = vtkFetchMIClientCallerXND::New();

   //--- Add all known servers and set their service type.
   //--- Extend here as we support new web services.
   const char *IDString = "XND";
   const char *HandlerString = "XNDHandler";
   vtkFetchMIServer *s1 = vtkFetchMIServer::New();
   s1->SetParser ( xndp );
   s1->SetWriter ( xndw );
   s1->SetClientCaller ( xndc );
   s1->SetName ( "http://xnd.slicer.org:8000" );
   s1->SetServiceType ( IDString);
   s1->SetURIHandlerName ( HandlerString);
   s1->SetTagTableName ( IDString);
   this->ServerCollection->AddItem ( s1 );
   s1->Delete();
   
   vtkFetchMIServer *s2 = vtkFetchMIServer::New();
   s2->SetParser ( xndp);
   s2->SetWriter ( xndw);
   s2->SetClientCaller (xndc);
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
}




//----------------------------------------------------------------------------
vtkFetchMILogic::~vtkFetchMILogic()
{ 
  this->ClearExistingTagsForQuery();
  if ( this->ServerCollection != NULL )
    {
    int num = this->ServerCollection->GetNumberOfItems();
    for ( int i=0; i < num; i++ )
      {
      vtkFetchMIServer *s = vtkFetchMIServer::SafeDownCast (this->ServerCollection->GetItemAsObject (i) );
      s->SetParser ( NULL );
      s->SetWriter ( NULL );
      s->SetClientCaller ( NULL );
      s->SetURIHandlerName ( NULL );
      s->SetURIHandler ( NULL );
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
  if ( this->MRMLScene == NULL )
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
    vtkFetchMIServer *s1 = vtkFetchMIServer::New();
    vtkFetchMIWriterXND *xndw = vtkFetchMIWriterXND::New();
    vtkFetchMIParserXND *xndp = vtkFetchMIParserXND::New();
    vtkFetchMIClientCallerXND *xndc = vtkFetchMIClientCallerXND::New();
    s1->SetName ( name );
    s1->SetServiceType ( type );
    s1->SetParser ( xndp);
    s1->SetWriter (xndw);
    s1->SetClientCaller ( xndc);
    s1->SetURIHandlerName ( URIHandlerName );
    s1->SetTagTableName ( TagTableName );
    this->ServerCollection->AddItem ( s1 );
    s1->Delete();
    xndw->Delete();
    xndp->Delete();
    xndc->Delete();
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
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: MRMLScene is NULL.");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: FetchMINode is NULL.");
    return;
    }
  if ( this->GetCurrentServer() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload:Got null server.");
    return;    
    }


  int retval;
  

  //--- Temporary Save All
  //--- SAVE ORIGINAL RESOURCE SELECTION STATE
  //--- TODO: figure out how to upload individual resources.
  //--- For now, override selection state reflected by GUI--
  //--- select everything, so we upload scene + all data.
  //--- then restores GUI selection state reflected by GUI.
  std::vector<std::string> tmpSelected;
  int tmp = this->GetSceneSelected();
  for ( unsigned int i=0; i< this->SelectedStorableNodeIDs.size(); i++)
    {
    tmpSelected.push_back(this->SelectedStorableNodeIDs[i] );
    }

  //--- Get all storables in the scene
  this->SceneSelected=1;
  this->SelectedStorableNodeIDs.clear();      
  int n;
  vtkMRMLNode *node = NULL;
  //---Volumes
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLVolumeNode");
  for (n=0; n<nnodes; n++ )
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLVolumeNode");
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
  //---Models
  nnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLModelNode");
  for (n=0; n<nnodes; n++ )
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLModelNode");
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
  nnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLUnstructuredGridNode");
  for (n=0; n<nnodes; n++ )
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLUnstructuredGridNode");
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

  retval = this->CheckStorageNodeFileNames();
  if ( retval == 0 )
    {
    //--- TODO: put up save-stuff message dialog
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
  //--- wjp test
  if ( this->GetCurrentServer()->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUpload: Got a NULL URIHandler." );
    return;
    }
  vtkURIHandler *handler = this->GetCurrentServer()->GetURIHandler();
  this->SetURIHandlerOnSelectedResources( handler );
  const char *svrName = this->GetCurrentServer()->GetName();
  if ( svrName == NULL || !(strcmp(svrName, "" ) ) )
    {
    vtkErrorMacro ( "RequestResourceUpload: Got a NULL Server name." );
    return;
    }
  handler->SetHostName(svrName);
  //--- end wjp test

  //---
  //--- request the upload
  //---
  const char *svctype = this->GetCurrentServer()->GetServiceType();
  if ( !strcmp (svctype, "XND"))
    {
    this->RequestResourceUploadToXND ( );
    }

  //--- RESET SELECTION STATE
  this->SceneSelected = tmp;
  this->SelectedStorableNodeIDs.clear();
  for ( unsigned int i=0; i< tmpSelected.size(); i++)
    {
    this->SelectedStorableNodeIDs.push_back(tmpSelected[i] );
    }
}



//---------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUploadToXND ( )
{
  //--- if Handler is XND, need to add any new metadata to server first.
  int retval;
  
  if ( this->GetCurrentServer() == NULL )
    {
    vtkErrorMacro ( "RequestResourceUploadToXND:Got null server.");
    return;    
    }
  const char *svrName = this->GetCurrentServer()->GetName();
  const char *svctype = this->GetCurrentServer()->GetServiceType();

  if ( strcmp(svctype, "XND"))
    {
    vtkErrorMacro ( "RequestResourceUploadToXND:Got unknown web service type");
    return;
    }
  //--- Safely downcast the handler
  const char *hname = this->GetCurrentServer()->GetURIHandlerName();
  vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("RequestResourceUploadToXND: Null URIHandler. ");
    return;
    }
  //--- Make sure all files have the right uri handler set.

/* wjp test */
//  this->SetURIHandlerOnSelectedResources(handler);
//  handler->SetHostName(svrName);
/* end wjp test */

  //--- write the XML doc description and header info
  this->WriteDocumentDeclaration();
  //--- create all new tags to the server.

  std::map<std::string, std::vector<std::string> >::iterator iter;  
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
        iter++ )
    {
    if ( iter->first.c_str() != NULL &&  (strcmp (iter->first.c_str(), "" ) ) )
      {
        int returnval = handler->PostTag ( svrName, iter->first.c_str(),this->GetTemporaryResponseFileName() );
        if ( returnval == 0 )
          {
          vtkErrorMacro ("vtkFetchMIGUI:RequestResourceUploadToXND: couldn't add new tag to server.");
          std::string msg = "Error adding new tags. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
          this->FetchMINode->SetErrorMessage (msg.c_str() );
          this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
          return;
          }
        //--- TODO: add error checking here...
        //--- look for complaints in this->GetTemporaryResponseFileName()
        }
      }

    //---
    //--- for each storable node:
    //--- generate metadata in utf-8 format (in gui)
    //--- post metadata (in logic)
    //--- parse metadata and set URIs (in gui)
    vtkMRMLStorableNode *storableNode;
    for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
      {
      std::string nodeID = this->SelectedStorableNodeIDs[n];
      vtkDebugMacro("RequestResourceUploadToXND: generating metadata for selected storable node " << nodeID.c_str());
      storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));
      // for each storage node
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      vtkMRMLStorageNode *storageNode;
      for (int i = 0; i < numStorageNodes; i++)
        {
        storageNode = storableNode->GetNthStorageNode(i);
        if ( storageNode == NULL )
          {
          vtkErrorMacro ( "RequestResourceUploadtoXND: got NULL StorageNode.");
          continue;
          }
        //--- empty out the URIList from past uploads/downloads in the storage node.
        storageNode->ResetURIList();
        vtkDebugMacro("RequestResourceUploadToXND: have storage node " << i << ", calling write metadata for upload with id " << nodeID.c_str() << " and file name " << storageNode->GetFileName());

        //--- write header and metadata
        const char *snodeFileName = storageNode->GetFileName();
        vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName ( snodeFileName );
        const char *strippedFileName = vtkFileName.c_str();
        retval = this->WriteMetadataForUpload(nodeID.c_str());
        if ( retval == 0 )
          {
          std::string msg = "Error writing metadata for upload. Aborting remaining upload. Please check your network connection or whether the selected server is down.";
          this->FetchMINode->SetErrorMessage (msg.c_str() );
          this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
          return;
          }

        //--- post and parse response
        this->PostMetadataToXND(handler, strippedFileName);
        this->SetReservedURI ( NULL );
        this->GetCurrentServer()->GetParser()->ParseMetadataPostResponse(this->GetTemporaryResponseFileName());
        const char *uri = this->GetCurrentServer()->GetParser()->GetResponse();
        this->SetReservedURI ( uri );
        if (uri == NULL)
          {
          storageNode->SetURI(NULL);
          storageNode->ResetURIList();
          this->DeselectNode(nodeID.c_str());
          // bail out of the rest of the storage nodes
          i = numStorageNodes;
          std::string msg = "Server did not return a valid uri for " + std::string(strippedFileName) + ". Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
          this->FetchMINode->SetErrorMessage (msg.c_str() );
          this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
          // for now, decrement the node number, since DeselectNode removes an
          // element from the list we're iterating over
          return;
          }
        else
          {
          vtkDebugMacro("RequestResourceUploadToXND: parsed out a return metadatauri : " << uri);
          // Set each storable node's URI, where they'll be saved in the final post of data
          // via the PostStorableNodes() method.
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
                return;
                }

              //--- post and parse response
              this->PostMetadataToXND(handler, strippedFileName);
              this->SetReservedURI ( NULL );
              this->GetCurrentServer()->GetParser()->ParseMetadataPostResponse(this->GetTemporaryResponseFileName());
              const char *uri = this->GetCurrentServer()->GetParser()->GetResponse();
              this->SetReservedURI ( uri );
              if (uri == NULL)
                {
                //--- TODO: clean up filenames now. they are set to cache path.
                //--- fill up errormessage and invoke event via the node.
                vtkErrorMacro("RequestResourceUploadToXND:  error parsing uri from post meta data call for file # " << filenum); //, response = " << metadataResponse);
                storageNode->SetURI(NULL);
                storageNode->ResetURIList();
                std::string msg = "Error encountered while uploading " + std::string(storageNode->GetNthFileName(filenum)) + " to remote host. Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
                this->FetchMINode->SetErrorMessage (msg.c_str() );
                this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
                //--- TODO: manage upload failure properly.
                //--- would like to keep track of uris for files in filenamelist
                //--- and delete them from server if any one of them fails so we
                //--- don't have a messy partial upload.
                //--- For now, bail out and leave dirty pile of data on server.
                return;
                }
              else
                {
                vtkDebugMacro("RequestResourceUploadToXND: parsed out a return metadatauri : " << uri << ", adding it to storage node " << storageNode->GetID());
                // then save it in the storage node
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
     return;
     }

    if ( this->SceneSelected )
      {
      //--- explicitly write the scene to cache (uri already points to cache)
      this->MRMLScene->Commit();

      //--- write header and metadata
      const char *sceneFileName =this->GetMRMLScene()->GetURL();
      vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (  sceneFileName );
      const char *strippedFileName = vtkFileName.c_str();
      retval = this->WriteMetadataForUpload("MRMLScene");
      if (retval == 0 )
        {
        std::string msg = "Error encountered while uploading MRML Scene description to remote host. Aborting remaining upload. Please check your network connection or whether the selected server is down.\n";
        this->FetchMINode->SetErrorMessage (msg.c_str() );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return;
        }
      //--- generate scene metatdata (in gui)
      this->PostMetadataToXND(handler, strippedFileName);
      this->SetReservedURI ( NULL );
      this->GetCurrentServer()->GetParser()->ParseMetadataPostResponse(this->GetTemporaryResponseFileName());
      const char *uri = this->GetCurrentServer()->GetParser()->GetResponse();
      this->SetReservedURI ( uri );
      if ( uri != NULL )
        {
        // set particular XND host in the XNDhandler
        handler->SetHostName(svrName);
        handler->StageFileWrite(sceneFileName, uri);
        }
      else
        {
        vtkErrorMacro("RequestResourceUploadToXND: unable to parse out response from posting metadata for mrml scene, uri is null. ");
        return;
        }
      }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUploadToHID ( )
{
}



//----------------------------------------------------------------------------
int vtkFetchMILogic::PostMetadataToXND ( vtkXNDHandler *handler, const char *dataFilename )
{
  // return 1 if OK, 0 if not.
  int returnval = 0;
  
  if ( handler == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got a NULL handler" );
    return (returnval);
    }
  if (this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got a NULL FetchMINode" );
    return ((returnval));
    }
  if (this->GetXMLUploadFileName() == NULL)
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND Got NULL file for Metadata upload.");
    return (returnval);
    }
  if ( this->GetXMLHeaderFileName() == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND Got NULL file for Metadata upload.");
    return (returnval);    
    }
  if (this->GetTemporaryResponseFileName() == NULL)
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got NULL file for Metadata upload.");
    return (returnval);
    }
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got NULL server.");
    return (returnval);
    }

  // set particular XND host in the XNDhandler
  std::string svr = this->GetCurrentServer()->GetName();
  if (svr.c_str() == NULL || !(strcmp(svr.c_str(), "") ))
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return (returnval);
    }
  std::string::size_type index =  svr.find("://", 0);
  if ( index  != std::string::npos)
    {
    std::string hostname = svr.substr( index+3, std::string::npos );
    handler->SetHostName(hostname.c_str());
    }
     
  
  // set the post destination for data being described
  std::stringstream ss;
  ss << svr.c_str();
  ss << "/data";
  returnval = handler->PostMetadata ( ss.str().c_str(), this->GetXMLHeaderFileName(), dataFilename, this->GetXMLUploadFileName(), this->GetTemporaryResponseFileName() );
  return (returnval);
}




//---------------------------------------------------------------------------
int vtkFetchMILogic::WriteDocumentDeclaration ( )
{
  if ( this->GetXMLDocumentDeclarationFileName() == NULL )
    {
    vtkErrorMacro ("WriteDocumentDeclaration: got null Document Declaration filename" );
    return 0;
    }
  this->GetCurrentServer()->GetWriter()->SetDocumentDeclarationFilename ( this->GetXMLDocumentDeclarationFileName() );
  this->GetCurrentServer()->GetWriter()->WriteDocumentDeclaration ( );
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
  if (this->MRMLScene == NULL )
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
  this->GetCurrentServer()->GetWriter()->SetMetadataFilename( this->GetXMLUploadFileName() );
  this->GetCurrentServer()->GetWriter()->SetDocumentDeclarationFilename ( this->GetXMLDocumentDeclarationFileName() );

  //--- write either the Scene's metadata if nodeID = MRMLScene or the node's metadata.
  if ( !(strcmp (nodeID, "MRMLScene" )))
    {
    this->GetCurrentServer()->GetWriter()->WriteMetadataForScene (this->GetMRMLScene());
    }
  else
    {
    this->GetCurrentServer()->GetWriter()->WriteMetadataForNode (nodeID, this->GetMRMLScene());
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
    vtkSetAndObserveMRMLNodeEventsMacro ( this->FetchMINode, node, events );
    events->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "FetchMILogic::ProcessMRMLEvents: got null FetchMINode." );
    return;
    }
  vtkMRMLFetchMINode* node = vtkMRMLFetchMINode::SafeDownCast ( caller );

  //--- Note to developers: expand logic here as new services are added.
  if ( node == this->FetchMINode && event == vtkMRMLFetchMINode::SelectedServerModifiedEvent )
    {
    this->CurrentServer = this->GetServerCollection()->FindServerByName ( this->FetchMINode->GetSelectedServer() );
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

  vtkDebugMacro ("--------------------Querying server for tags.");
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return;
    }

  const char *svr = this->GetCurrentServer()->GetName();
  if ( svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  const char *svctype = this->GetCurrentServer()->GetServiceType();          
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "QueryServerForTags:Got unknown web service type");
    return;
    }

  //--- Always reset the number of known tags on the server to 0 before query.
  this->NumberOfTagsOnServer = 0;

  //--- do the right thing based on which kind of webservice:
  if ( !(strcmp(this->CurrentServer->GetServiceType(), "HID")))
    {
    //--- HID
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    if ( h )
      {
      // for now assume we can't query for tags.
      // just fill the table up with default ones.
      // TODO: find out how to query HID webservices for tags.
      // h->QueryServer ( uri, responseFile );
      }
    }
  else if ( !(strcmp(this->CurrentServer->GetServiceType(), "XND")))
    {
    //--- XND
    const char *hname = this->GetCurrentServer()->GetURIHandlerName();
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
    if ( h )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- Build query
        std::stringstream q;
        q << svr;
        q << "/tags";
        const char *errorString = h->QueryServer ( q.str().c_str(), this->GetHTTPResponseFileName() );
        if ( !strcmp(errorString, "OK" ))
          {
          //--- then parse new tags into the container.
          this->ParseTagQueryResponse ( );
          // check to see if "SlicerDataType" is an existing tag on the server.
          // if not, then post it.
          if ( ! this->IsTagOnServer ( "SlicerDataType" ) )
            {
            const char *svrName = this->GetCurrentServer()->GetName();
            if ( svrName != NULL )
              {
              h->SetHostName ( svrName );
              int returnval = h->PostTag ( svrName, "SlicerDataType", this->GetTemporaryResponseFileName() );
              // if it was added successfully, then add it to our list of tags.
              // otherwise return an error message.
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
          }
        }
      else
        {
        vtkErrorMacro ( "XML Response dir not set." );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIODirectoryErrorEvent );
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForTagValues ( )
{

  vtkDebugMacro ("--------------------Querying server for tag values.");
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  
  if ( this->GetHTTPResponseFileName( ) == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: ResponseFileName is not set.");
    return;
    }
  
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return;
    }

  const char *svr = this->GetCurrentServer()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  const char *svctype = this->GetCurrentServer()->GetServiceType();
  if ( !this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "QueryServerForTagValues:Got unknown web service type");
    return;
    }


  if ( !(strcmp(svctype, "HID")))
    {
    //--- HID
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    if ( h )
      {
      // TODO: find out how to query HID webservices for tags.
      // h->QueryServer ( uri, responseFile );
      }
    }
  else if ( !(strcmp(svctype, "XND")))
    {
    //--- XND
    const char *hname = this->GetCurrentServer()->GetURIHandlerName();
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
    // for each tag attribute in the tag table, get all values represented in the DB.
    if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ) != NULL)
      {
      vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
      if ( t != NULL )
        {
        std::string att;
        std::string query;
        // For each tag, get the attribute, and then
        // Ask server for all values for that attribute.
        // Parse the response from the server.
        // Invoke an event on the node that will be caught by GUI
        // In GUI, the value menus for each attribute will be updated
        // to show all current values for each tag in the DB.
        std::map<std::string, std::vector<std::string> >::iterator iter;
        for ( iter = this->CurrentServerMetadata.begin();
              iter != this->CurrentServerMetadata.end();
              iter++ )
          {
          att.clear();
          att = iter->first.c_str();
          std::stringstream q;
          q << svr;
          q << "/search??";
          q << att.c_str();
          query = q.str();
          const char *errorString = h->QueryServer ( query.c_str(), this->GetHTTPResponseFileName() );
          if ( !strcmp(errorString, "OK" ))
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
            vtkErrorMacro ( "XML Response dir not set." );
            this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIODirectoryErrorEvent );
            break;
            }
          }
        //--- invoke an event that updates GUI once we have queried for values for all tags.
        vtkDebugMacro ("--------------------Invoking TagResponseReadyEvent.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::TagResponseReadyEvent );          
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::UpdateMRMLQueryTags()
{

  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return;
    }

  const char *svr = this->GetCurrentServer()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  const char  *svctype = this->GetCurrentServer()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "UpdateMRMLQueryTags:Got unknown web service type");
    return;
    }

  //--
    //--- Make sure MRML tag table is compatible with logic's
    //--- list of tags on server.
    //--- For each tag in MRML's tagtable, get the old value.
    //--- Find that tag in the logic's CurrentServerMetadata.
    //--- if it's present, then keep its value in MRML.
    //--- if it's not present, then replace the tag's value in MRML
    //--- with the first value in the logic's list.
  //--

  if ( !(strcmp(svctype, "HID")))
    {
    //--- HID
    vtkHIDTagTable* t = vtkHIDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
    if ( t != NULL )
      {
      //--- TODO: not sure how this will work with HID yet.
      }
    }
  else if ( !(strcmp(svctype, "XND")))
    {
    //--- XND
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
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
        for ( iter = this->CurrentServerMetadata.begin();
              iter != this->CurrentServerMetadata.end();
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
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::DeleteResourceFromServer ( const char *uri )
{
  int retval = 1;

  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return 0;
    }

  // check for which server
  // send delete request.
  const char *svr = this->GetCurrentServer()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return 0;
    }
  const char *svctype = this->GetCurrentServer()->GetServiceType();
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

  // put up wait dialog...
  // now update the resource list in the GUI.
  if ( !(strcmp(svctype, "HID")))
    {
    //---
    //--- HID
    //---
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    vtkHIDTagTable* t = vtkHIDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HID" ));
    if ( h && t )
      {
      }
    }
  else if ( !(strcmp(svctype, "XND")))
    {
    //---
    //--- XND
    //---
    const char *hname = this->GetCurrentServer()->GetURIHandlerName();
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
    if ( h && t )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        retval = h->DeleteResource ( uri, this->GetHTTPResponseFileName() );
        }
      }
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
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return;
    }

  const char *svr = this->GetCurrentServer()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  const char *svctype = this->GetCurrentServer()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "QueryServerForResources: Got unknown web service type");
    return;
    }

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
        q << svr << "/search?";

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
  else if ( !(strcmp(svctype, "XND")))
    {
    //---
    //--- XND
    //---
    const char *hname = this->GetCurrentServer()->GetURIHandlerName();
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
    if ( h && t )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- Build query
        q << svr << "/search?";

        //--- TODO: is there a better way to filter out weird tags?
        //--- Append all selected XND tags that are reasonably
        //--- formed. That means they have both an attribute
        //--- and a value, and both are not "", "NULL" or "no values found"
        //--- etc.
        int num = t->GetNumberOfTags();

        for ( int i =0; i<num; i++)
          {
          att = t->GetTagAttribute(i);
          val = t->GetTagValue(i);
          if ( t->IsTagSelected(att.c_str()))
            {
            //--- do some checking...
            if ( att.c_str() != NULL && val.c_str() != NULL )
              {
              if ( (strcmp(att.c_str(), "NULL")) &&
                   (strcmp(att.c_str(), "null")) &&
                   (strcmp(att.c_str(), "none")) &&
                   (strcmp(att.c_str(), "None")) &&
                   (strcmp(att.c_str(), "" )))
                {
                if ( (strcmp(val.c_str(), "NULL")) &&
                     (strcmp(val.c_str(), "null")) &&
                     (strcmp(val.c_str(), "no values found")) &&
                     (strcmp(val.c_str(), "No values found")) &&
                     (strcmp(val.c_str(), "" )))
                  {
                  q << att;
                  q << "=";
                  q << val;
                  q << "&";
                  }
                }
              }
            }
          }
        //---
        //--- TODO: trim off the last ampersand in the query string!!!
        //---
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
}







//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckValidSlicerDataType ( const char *dtype)
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: Got NULL FetchMINode pointer.");
    return -1;
    }

  vtksys_stl::string s = dtype;
  vtksys_stl::string tmp;

  int num = this->FetchMINode->GetSlicerDataTypes()->GetNumberOfValues();
  for ( int i=0; i < num; i++ )
    {
    tmp = this->FetchMINode->GetSlicerDataTypes()->GetValue(i);
    if ( tmp.c_str() != NULL )
      {
      if ( tmp == s )
        {
        return (i);
        }
      }
    }

  return -1;
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseValuesForTagQueryResponse ( const char *att )
{
  if ( this->GetCurrentServer()->GetParser () )
    {
    this->GetCurrentServer()->GetParser()->SetFetchMINode ( this->FetchMINode );
    this->ClearExistingValuesForTag ( att );
    this->GetCurrentServer()->GetParser()->ParseValuesForAttributeQueryResponse ( this->GetHTTPResponseFileName(), att );
    this->RefreshValuesForTag ( att );
    this->GetCurrentServer()->GetParser()->SetFetchMINode ( NULL );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseTagQueryResponse ( )
{

  if ( this->GetCurrentServer()->GetParser() )
    {
    this->GetCurrentServer()->GetParser()->SetFetchMINode ( this->FetchMINode );
    this->ClearExistingTagsForQuery ( );
    this->GetCurrentServer()->GetParser()->ParseMetadataQueryResponse( this->GetHTTPResponseFileName() );
    this->RefreshTags();
    this->GetCurrentServer()->GetParser()->SetFetchMINode ( NULL );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseResourceQueryResponse ( )
{
  if ( this->GetCurrentServer()->GetParser() )
    {
    this->GetCurrentServer()->GetParser()->SetFetchMINode ( this->FetchMINode );
    this->GetCurrentServer()->GetParser()->ParseResourceQueryResponse( this->GetHTTPResponseFileName() );
    this->GetCurrentServer()->GetParser()->SetFetchMINode ( NULL );
    }

}





//----------------------------------------------------------------------------
void vtkFetchMILogic::RefreshTags()
{
  this->ClearExistingTagsForQuery();
  //--- copy parser's tags to Logic.
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->GetCurrentServer()->GetParser()->MetadataInformation.begin();
        iter != this->GetCurrentServer()->GetParser()->MetadataInformation.end();
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
  this->ClearExistingValuesForTag(tagname);

  if ( this->FetchMINode != NULL )
    {
    if ( this->FetchMINode->GetTagTableCollection() != NULL )
      {
      vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
      if ( t != NULL )
        {
        //--- copy parser's values for this attribute to Logic
        std::map<std::string, std::vector<std::string> >::iterator iter;
        for ( iter = this->GetCurrentServer()->GetParser()->MetadataInformation.begin();
              iter != this->GetCurrentServer()->GetParser()->MetadataInformation.end();
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
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearExistingTagsForQuery ( )
{

  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
        iter++ )
    {
    iter->second.clear();
    }
  this->CurrentServerMetadata.clear();

  // enforce that SlicerDataType always be present.
//  this->AddUniqueTag ( "SlicerDataType" );
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearExistingValuesForTag ( const char *tagname )
{

  //--- first find the tag, then clear all the values.
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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
    this->CurrentServerMetadata.insert ( std::make_pair (tagname, v ));
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::AddUniqueValueForTag ( const char *tagname, const char *val )
{
  //--- search thru vector of strings for tag to see if we can find the tag.
  int tagFound = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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

  unsigned int numTags = this->CurrentServerMetadata.size();
  return ( numTags );

}



//----------------------------------------------------------------------------
const char * vtkFetchMILogic::GetNthTagAttribute ( int n)
{

  int cnt = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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
  for ( iter = this->CurrentServerMetadata.begin();
        iter != this->CurrentServerMetadata.end();
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



// filename is a dataset's filename or mrmlscene filename.
// ID is a nodeID or is the text "MRML".
//----------------------------------------------------------------------------
int vtkFetchMILogic::WriteMetadataForUpload ( const char *filename, const char *ID, vtkXNDHandler *handler)
{

  if ( this->FetchMINode == NULL) 
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return 0;
    }
  if (this->GetXMLUploadFileName() == NULL)
    {
    vtkErrorMacro ( "vtkFetchMILogic: XMLUploadFileName is not set." );
    return 0;
    }
  if (this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload has null MRMLScene." );
    return 0;        
    }  
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload has null server.");
    return 0;        
    }

  ofstream file;
  // open the file for writing.
#ifdef _WIN32
  file.open(this->GetXMLUploadFileName(), std::ios::out | std::ios::binary);
#else
  file.open(this->GetXMLUploadFileName(), std::ios::out);
#endif
  if ( file.fail() )
    {
    vtkErrorMacro ("WriteMetadataForUpload: could not open file " << this->GetXMLUploadFileName() );
    return 0;
    }


/*
  const char *svr = this->GetCurrentServer()->GetName();
  if (svr == NULL || !(strcmp(svr, "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
*/
  const char *svctype = this->GetCurrentServer()->GetServiceType();
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "WriteMetadataForUpload: Got unknown web service type");
    return 0;
    }

  const char *att;
  const char *val;

  if ( !(strcmp(svctype, "XND")))
    {
    // doing a SafeDownCast in FindURIHandlerByName  was causing a crash...
//    const char *hname = this->GetCurrentServer()->GetURIHandlerName();
/*    vtkURIHandler *uriHandler = this->GetMRMLScene()->FindURIHandlerByName ( hname );
    if (uriHandler == NULL)
      {
      vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null URIHandler." );
      return 0;
      }
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (uriHandler);
*/
    if ( handler == NULL )
      {
      vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null XNDHandler." );
      return 0;
      }

    //--- Check to see if ID == MRML.
    if ( !(strcmp(ID, "MRML" ) ) )
      {
      vtkTagTable *t = this->GetMRMLScene()->GetUserTagTable();
      if ( t == NULL )
        {
      vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null tag table for the scene." );
      return 0;
        }
      // make sure SlicerDataType tag is set to 'MRML'
      t->AddOrUpdateTag ( "SlicerDataType", "MRML" );
      int num = t->GetNumberOfTags();
      //------ Write XML for the scene file using this->SceneTags

      file << handler->GetXMLDeclaration();
      file << "\n";
      file << "<Metadata ";
      file << handler->GetNameSpace();
      file << ">";
      file << "\n";
        
      for ( int i=0; i < num; i++ )
        {
        att = t->GetTagAttribute(i);
        val = t->GetTagValue(i);
//        if ( t->IsTagSelected(att))
        // use all tags for now
        if ( 1 )
          {
          file << "<Tag Label=";
          file << "\"";
          file << att;
          file << "\"";
          file << ">\n";
          file << "<Value>";
          file << val;
          file << "</Value>\n";
          file << "</Tag>\n";
          }
        }
      file << "</Metadata>\n";
      }
    else
      {
      //------ Check to see if the ID is a storable node in scene.
      //------ if NO, return 0.
      //------ Check to see if the node is tagged for upload
      //------ if not, return 0.
      //------ Write the xml description for each filename
      //------ into file with filename=this->XMLUploadFileName
      //------ return 1 if file writes ok, else return 0

      vtkMRMLStorableNode *n = vtkMRMLStorableNode::SafeDownCast(this->MRMLScene->GetNodeByID ( ID ));
      if ( n == NULL )
        {
        vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null node to write." );
        return 0;        
        }
      
      // Set up a temporary tag table in case the node doesn't have one.
      vtkTagTable *tmpTags= vtkTagTable::New();
      vtkTagTable *t = n->GetUserTagTable();
      if ( t == NULL)
        {
        t = tmpTags;
        }

      // make sure there's a SlicerDataType tag.
      int num = t->GetNumberOfTags();
      int found = 0;
      for ( int i=0; i < num; i++ )
        {
        att = t->GetTagAttribute(i);
        if ( !strcmp (att, "SlicerDataType" ))
          {
          found = 1;
          break;
          }
        }
      if ( !found )
        {
        t->AddOrUpdateTag ( "SlicerDataType", "unknown" );
        }

      //------ Write XML for the scene file using this->SceneTags
      num = t->GetNumberOfTags();
      file << handler->GetXMLDeclaration();
      file << "\n";
      file << "<Metadata ";
      file << handler->GetNameSpace();
      file << ">";
      file << "\n";

      for (int i=0; i < num; i++ )
        {
        att = t->GetTagAttribute(i);
        val = t->GetTagValue(i);
//        if ( t->IsTagSelected(att))
        // use all tags for now.
        if ( 1 )
          {
          file << "<Tag Label=";
          file << "\"";
          file << att;
          file << "\"";
          file << ">\n";
          file << "<Value>";
          file << val;
          file << "</Value>\n";
          file << "</Tag>\n";
          }
        }
      file << "</Metadata>\n";

      tmpTags->Delete();
      }
    }
  else
    {
    //no-op
    }

  file.close();
  return (1);
}




//----------------------------------------------------------------------------
int vtkFetchMILogic::TestForRequiredTags ( )
{

  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null FetchMINode" );
    return 0;
    }

  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return 0;
    }

  const char *svctype = this->GetCurrentServer()->GetServiceType();          
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "TestForRequiredTags: Got unknown web service type");
    return 0;
    }
  
  if ( !(strcmp(svctype, "HID")))
    {
    //no-op
    }
  if ( !(strcmp(svctype, "XND")))
    {
    //--- check scene and all selected nodes.
    //---Required tags: experiment, project, subject, scan, modality, SlicerDataType.
    //--- this may change!
    vtkTagTable *t;
    if ( this->SceneSelected )
      {
      if ( this->MRMLScene == NULL )
        {
        vtkErrorMacro ("vtkFetchMILogic: Null MRMLScene." );
        return 0;        
        }
      //--- look at scene tags
      t = this->MRMLScene->GetUserTagTable();
      if ( t == NULL)
        {
        return 0;
        }
      for ( unsigned int i=0; i < this->FetchMINode->RequiredXNDTags.size(); i++ )
        {
        if ( t->CheckTableForTag (this->FetchMINode->RequiredXNDTags[i].c_str()) < 0 )
          {
          return 0;
          }
        }
      }

    //--- and look at each node.
    for ( unsigned int n=0; n < this->SelectedStorableNodeIDs.size(); n++)
      {
      const char *nodeID = this->SelectedStorableNodeIDs[n].c_str();
      vtkMRMLStorableNode *node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNodeByID (nodeID));
      if ( node != NULL )
        {
        t = this->FetchMINode->GetTagTableCollection()->FindTagTableByName ("XND");
        if ( t == NULL )
          {
          return 0;
          }
        for ( unsigned int i=0; i < this->FetchMINode->RequiredXNDTags.size(); i++ )
          {
          if ( t->CheckTableForTag (this->FetchMINode->RequiredXNDTags[i].c_str()) < 0 )
            {
            return 0;
            }
          }
        }
      }
    }

  return 1;
}

//---
//--- TODO: right now only allow MRML scene downloads. Generalize this.
//---
//--- This method has to figure out whether we are downloading
//--- a scene or a dataset then has to figure out which service is queried
//--- and call the correct method to download a scene or resource
//----------------------------------------------------------------------------
int vtkFetchMILogic::RequestResourceDownload ( const char *uri, const char *slicerDataType)
{

  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return -1;
    }

  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: CurrentServer is NULL.");
    return -1;
    }

  const char *svctype = this->GetCurrentServer()->GetServiceType();          
  if (! this->ServerCollection->IsKnownServiceType(svctype) )
    {
    vtkErrorMacro ( "RequestResourceDownload: Got unknown web service type");
    return -1;
    }

  
  if ( uri == NULL || slicerDataType == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null uri or slicerDataType." );
    return -1;
    }
  

  //--- TODO: make this work!
  //--- make sure we have a data type we know how to handle.
/*
  if ( (strcmp (slicerDataType, "VTKModel")) &&
       (strcmp (slicerDataType, "FreeSurferModel")) &&
       (strcmp (slicerDataType, "ScalarVolume")) &&
       (strcmp (slicerDataType, "MRML" )))
    {
    //--- retval reports an unknown slicerDataType.
    return 0;    
    }
*/
  //--- FOR NOW, ONLY mrml...

  if ( (strcmp (slicerDataType, "MRML")))
    {
    //--- retval reports an unknown slicerDataType.
    return 0;    
    }


  //--- handle scene with separate set of methods.
  if ( !(strcmp(slicerDataType, "MRML")))
    {
    //--- check all known webservices to see which is selected
    if ( !(strcmp(svctype, "XND")))
      {
      this->RequestSceneDownloadFromXND( uri );
      }
    if ( !(strcmp(svctype, "HID")))
      {
      this->RequestSceneDownloadFromHID( uri );
      }
    }
  else 
    {
    //--- check all known webservices to see which is selected
    if ( !(strcmp(svctype, "XND")))
      {
      this->RequestResourceDownloadFromXND( uri, slicerDataType );
      }
    if ( !(strcmp(svctype, "HID")))
      {
      this->RequestResourceDownloadFromHID( uri, slicerDataType );
      }
    }

  //--- retval means good as far as we can tell.
  return 1;
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownloadFromHID ( const char *uri, const char *slicerDataType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null FetchMI node. ");
    return;
    }

  vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null URIHandler. ");
    return;
    }

  if ( !(strcmp(slicerDataType, "ScalarVolume" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  if ( !(strcmp(slicerDataType, "VTKModel" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  if ( !(strcmp(slicerDataType, "FreeSurferModel" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownloadFromXND ( const char *uri, const char *slicerDataType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.

  if (uri == NULL || slicerDataType == NULL)
    {
    vtkErrorMacro("vtkFetchMILogic::RequestResourceDownloadFromXND: uri isn't set or slicerDataType isn't set. uri = " << (uri == NULL ? "null" : uri) << ", slicerDataType = " << (slicerDataType == NULL ? "null" : slicerDataType));
    return;
    }
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromXND: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromXND: Null FetchMI node. ");
    return;
    }

  
  const char *hname = this->GetCurrentServer()->GetURIHandlerName();
  vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromXND: Null URIHandler. ");
    return;
    }

  // for the storage nodes to find the right handler, use xnd:// instead of
  // http
  std::string::size_type index;
  std::string uriString(uri);
  std::string suffix;
  // get all charactersup to the ://
  if ((index = uriString.find("://", 0) ) != std::string::npos)
    {
    suffix = uriString.substr(index, std::string::npos);
    uriString = std::string("xnd") + suffix;
    vtkDebugMacro("vtkFetchMILogic::RequestResourceDownloadFromXND: new uri string = " << uriString.c_str());
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
void vtkFetchMILogic::RequestSceneDownloadFromHID ( const char *uri)
{
  // This method will a resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null FetchMI node. ");
    return;
    }
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null server.");
    return;
    }

  vtkHIDHandler *handler = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null URIHandler. ");
    return;
    }

  std::string localURL = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri);
  std::string remoteURL = uri;

  // currently have to pass a host name
  // TODO: test this!
  std::string svr = this->GetCurrentServer()->GetName();
  if (svr.c_str() == NULL || !(strcmp(svr.c_str(), "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  std::string hostname;
  std::string::size_type index;
  if ( (index = svr.find("://", 0)) != std::string::npos)
    {
    hostname = svr.substr( index+3, std::string::npos );
    // do a synchronous dl
    handler->SetHostName(hostname.c_str());
    handler->StageFileRead(remoteURL.c_str(), localURL.c_str());
    
    // now override the mrml scene's url to point to file on disk
    this->GetMRMLScene()->SetURL(localURL.c_str());


    //--- load the remote scene
    this->MRMLScene->Connect();

    }
}






//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownloadFromXND ( const char *uri )
{
  // This method will download a resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null FetchMI node. ");
    return;
    }
  if ( this->CurrentServer == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null server.");
    return;
    }

    const char *hname = this->GetCurrentServer()->GetURIHandlerName();
  vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( hname ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null URIHandler. ");
    return;
    } 

  std::string localURL = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri);
  std::string remoteURL = uri;

  // currently have to pass a host name
  // TODO: test this!
  std::string svr = this->GetCurrentServer()->GetName();
  if (svr.c_str() == NULL || !(strcmp(svr.c_str(), "") ) )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null or empty server name." );
    return;
    }
  std::string hostname;
  std::string::size_type index;
  index = svr.find("://", 0);
  if ( index != std::string::npos)
    {
    hostname = svr.substr( index+3, std::string::npos );
    // do a synchronous dl
    handler->SetHostName(hostname.c_str());
    handler->StageFileRead(remoteURL.c_str(), localURL.c_str());
    
    // now override the mrml scene's url to point to file on disk
    this->GetMRMLScene()->SetURL(localURL.c_str());

    //--- load the remote scene
    //--- TODO:
    //--- this downloads the scene file, but looks
    //--- like its referenced data don't download
    //--- (at least they don't show up in the cachedir).
    //--- Not sure what's going on.
    //--- Scene shows up with empty nodes created.
    this->MRMLScene->Connect();
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
    vtkErrorMacro ("vtkFetchMILogic::CheckStorageNodeFileNames: Null scene. ");
    return(0);
    }

  vtkMRMLNode *node;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    node = this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] );
    if (node->GetModifiedSinceRead())
      {
      vtkErrorMacro("vtkFetchMILogic::CheckStorageNodeFileNames: error, node " << this->SelectedStorableNodeIDs[n] << " has been modified. Please save all unsaved nodes first");
      return (0);
      }
    }
  return (1);
  
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

  // get the cache dir
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory(), pathComponents);

  //--- scene
  if ( this->SceneSelected )
    {
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
      
      // FOR EACH FILENAME & FILELISTMEMBER IN EACH NODE:
      // {
      //--- Set Filename to be cachedir/filename
      // get out the file name
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
      //--- If the node is a multivolume, set the filename and all FileListMembers to
      //--- corresponding cachedir/filenames using AddFileName() method.
      // make up a vector of the new file names
      std::vector<std::string> CacheFileNameList;
      for (int filenum = 0; filenum < storageNode->GetNumberOfFileNames(); filenum++)
        {
        vtksys_stl::string nthFilename = storageNode->GetNthFileName(filenum);
        vtkFileName = vtksys::SystemTools::GetFilenameName(nthFilename);
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



//----------------------------------------------------------------------------
void vtkFetchMILogic::SetIdleWriteStateOnSelectedResources ( )
{
    if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetIdleWriteStateOnSelectedResources: Null scene. ");
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

  vtkMRMLStorableNode *storableNode;
  vtkMRMLStorageNode *storageNode;
  
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
        vtkDebugMacro("PostStorableNodes: calling write data on storage node " << i << ": " << storageNode->GetID());
        if (!storageNode->WriteData(storableNode))
          {
          vtkErrorMacro("PostStorableNodes: WriteData call failed on storage node " << storageNode->GetID() << " for node " << storableNode->GetName());
          return ( 0 );
          }
        }
      }
    }
  return ( 1 );
}




