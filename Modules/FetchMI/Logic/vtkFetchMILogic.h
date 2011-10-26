#ifndef __vtkFetchMILogic_h
#define __vtkFetchMILogic_h

// FetchMI includes
#include "vtkFetchMIServer.h"
#include "vtkFetchMIServerCollection.h"

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLFetchMINode.h"
#include "vtkMRMLScene.h"
#include "vtkTagTable.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include "vtkObject.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"

// STL includes
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include "vtkSlicerFetchMIModuleLogicExport.h"

class vtkXNDHandler;
class vtkIntArray;
class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMILogic : public vtkSlicerModuleLogic
{
  public:
  static vtkFetchMILogic *New();
  vtkTypeMacro(vtkFetchMILogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // These methods are used to turn observers on/off when module is entered/exited.
  virtual void Enter ( );
  virtual void Exit ( ) ;

  // Description:
  // Get/Set MRML node storing parameter values
  vtkGetObjectMacro (FetchMINode, vtkMRMLFetchMINode);
  virtual void SetFetchMINode( vtkMRMLFetchMINode *node );

  // Description:
  // Get/Set on collection of web services
  vtkGetObjectMacro ( ServerCollection, vtkFetchMIServerCollection );

  // Description:
  // Get/Set on the currently selected web service
  vtkGetObjectMacro ( CurrentWebService, vtkFetchMIServer );
  vtkSetObjectMacro ( CurrentWebService, vtkFetchMIServer );

  // Description:
  // Flag allowing user to select whether or not to save scene description. Not currrently used.
  vtkGetMacro (SaveSceneDescription, int );
  vtkSetMacro (SaveSceneDescription, int );

  // Description:
  // Flag detecting whether scene is selected for saving (otherwise just selected data is saved.)
  vtkGetMacro (SceneSelected, int );
  vtkSetMacro (SceneSelected, int );

  // Description:
  // Get/Set on a flag used internally to store the user's save setting on the scene.
  vtkGetMacro (TemporarySceneSelected, int );
  vtkSetMacro (TemporarySceneSelected, int );

  // Description:
  // Flag we use to indicate that we're searching specifically for resources on the host.
  // Otherwise, only queries about metadata are assumed. This lets us update uri query results
  // only when user is looking for them.
  vtkGetMacro (ResourceQuery, int);

  // Description:
  // Flag we use to indicate that we're searching specifically for tags that describe
  // data existing on the selected server.
  vtkGetMacro (TagAttributesQuery, int);

  // Description:
  // Flag we use to indicate that we're searching specifically for values of tags that describe
  // data existing on the selected server.
  vtkGetMacro (TagValuesQuery, int);

  vtkGetStringMacro ( ReservedURI );
  vtkSetStringMacro ( ReservedURI );
  
  //---------------------------------------------------------------------
  // Methods that set up the Informatics Framework
  //---------------------------------------------------------------------
  // Description:
  //Get/Set MRML node storing parameter values
  void SetAndObserveFetchMINode(vtkMRMLFetchMINode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->FetchMINode, n);
    }

  virtual vtkIntArray* NewObservableEvents();

  // Description:
  // Creates temporary files in cache for up and download, and
  // sets all URI handlers on servers in ServerCollection
  // NOTE: If using module logic without gui, then make this call.
  void InitializeInformatics();

  // Description:
  // Sets all URIHandlers on servers in ServerCollection
  void SetHandlersOnServers ( );
  
  // Description:
  // This method checks to see where the cache directory is
  // and creates filenames for query responses and for
  // upload metadata descriptions.
  void CreateTemporaryFiles ( );
  
  //---------------------------------------------------------------------
  // Query, Parsing, And Download Methods
  //---------------------------------------------------------------------
  // Description:
  // This method queries the selected webservice for available tags,
  // checks to see if the query result is valid and if so, calls the
  // ParseTagQueryResponse() method to parse the resulting XML
  // into the FetchMINode's tagtable with default values (used for resource query and markup),
  // and into the class's container for all the selected webservice's existing metadata,
  // called  CurrentWebServiceMetadata
  void QueryServerForTags ( );

  // Description:
  // This method queries the selected webservice for values for
  // each tag the server knows about. It initiates parsing of the
  // XML response by the ParseTagValuesQueryResponse() method
  // into the class's container for all the selected webservice's existing 
  // metadata, called CurrentWebServiceMetadata (used for Query).
  void QueryServerForTagValues ( );

  // Description:
  // Method cleans out CurrentWebServiceMetadata each time a
  // host is queried for the tags it supports.
  void ClearExistingTagsForQuery ( );

  // Description:
  // Method clears out vector of strings corresponding to a specific
  // tag name in the CurrentWebServiceMetadata when a host is
  // queried for the known values for that tag.
  void ClearExistingValuesForTag ( const char *tagname );

  void RefreshTags();
  void RefreshValuesForTag( const char *att );
  
  // Description:
  // This method ensures that MRML state tracks
  // logic's list of webservice-known tags and values.
  void UpdateMRMLQueryTags ( );

  // Description:
  // This method forms appropriate URIs from a list of tags
  // selected in the FetchMINode.
  void QueryServerForResources ( );

  // Description:
  // This method deletes a resource specified by uri from
  // the selected server.
  virtual int DeleteResourceFromServer ( const char *uri );
  virtual int DeleteSceneFromServer ( const char *uri );

  virtual const char *GetServiceTypeForServer ( const char *svc );
  
  // Description:
  // In XND, we can query to see what tags are available.
  // this method fills up the FetchMINode's XNDTagTable (for resource query and markup)
  // which is used to populate the list of available tags in the GUI,
  // and into the class's container for the selected webservices's known
  // metadata, called CurrentWebServiceMetadata.
  // if the query returns with an error, the node's error message
  // is filled.
  //(Parser->ParseMetadataQueryResponse() )
  void ParseTagQueryResponse ( );

  // Description:
  // In XND, we can query to see what attributes are listed
  // currently for a given tag.
  // this method fills up the class's CurrentWebServiceMetadata vector
  // of strings, which is used to populate the menu of possible
  // values for each tag in the GUI. If the query returns with an error,
  // the node's error message is filled.
  // moved to Parser->ParseMetadataValuesQueryResponse()
  void ParseValuesForTagQueryResponse ( const char *att);

  // Description:
  // Using XND supported and user specified tags,
  // queries for resources are made. This method parses
  // the response to those queries, and fills up the logic's
  // ResourceDescription (table) -- which is used to
  // populate the list of returned resources in the GUI.
  // move to Parser->ParseResourceQueryResponse()
  void ParseResourceQueryResponse ( );
  
  // Description:
  // Workhorse method for uploading datafiles and scene.
  // Currently the method ignores selected resources and instead
  // uploads all storable nodes and the scene description.
  // This is the method developers should extend as new
  // webservice types are supported.
  virtual void RequestResourceUpload ();

  // Description:
  // Workhorse method fo ruploading datafiles.
  virtual int RequestStorableNodesUpload ( );

  // Description:
  // Workhorse method fo ruploading scene file.
  virtual int RequestSceneUpload ( );

  // Description:
  // Adds new tags to the server.
  virtual int AddNewTagsToServer ( );

  // Description:
  // Workhorse method assembles the appropriate uri,
  // filters the slicerDataType to make sure it is a
  // known type, and sets up its download.
  // This is the method developers should extend
  // as new webservice types are supported.
  void RequestResourceDownload ( const char *uri, const char *slicerDataType );

  // Description:
  // Workhorse method that downloads a MRML scene
  // from remote repository.
  void RequestSceneDownload ( const char *uri );

  // Description:
  // This method requests download of a slicerDataType
  // from a supported webservice. (not yet fully implemented
  // since we currently only download MRML scenes, and remote
  // resources referenced by them.) Do not use until completed.
  void RequestStorableNodeDownload ( const char *uri, const char *slicerDataType);
  
  //---------------------------------------------------------------------
  // Tag Data Methods
  //---------------------------------------------------------------------
  // Description:
  // Thid method creates a list of storable node IDs
  // and fills the CurrentStorableNodeIDs class member.
  // This list is used by the GUI to populate its list of
  // data that can be tagged and uploaded...
  void GetCurrentListOfStorableNodes ( );

  // Description:
  // This method applies tags selected and specified by the user to
  // selected vtkMRMLStorableNodes. Each vtkMRMLStorableNode has
  // a tag table that gets populated by these tags.
  void TagStorableNodes ( );

  // Description:
  // Makes sure all data is appropriately tagged
  // before upload happens.
  int CheckStorableNodesForTags ( );
  
  // Description:
  // Marks a node for tagging or uploading.
  void SelectNode ( const char *nodeID );

  // Description:
  // Deselects a node for tagging or uploading.
  void DeselectNode ( const char *nodeID );

  // Description:
  // Marks a scene for tagging or uploading.
  void SelectScene();

  // Description:
  // Deselects the scene for tagging or uploading.
  void DeselectScene();
  
  // Description:
  // Invoked when Logic sees a NodeAddedEvent on the Scene,
  // this method applies the "SlicerDataType" tag to any new node.
  // This tag helps Slicer figure out what kind of node to create to
  // store the data when it's downloaded from a remote host.
  void ApplySlicerDataTypeTag();
  void SetSlicerDataTypeOnVolumeNodes();
  void SetSlicerDataTypeOnFiberBundleNodes();
  void SetSlicerDataTypeOnModelNodes();
  void SetSlicerDataTypeOnUnstructuredGridNodes();
  void SetSlicerDataTypeOnFiducialListNodes();
  void SetSlicerDataTypeOnColorTableNodes();


  //---------------------------------------------------------------------
  // Upload Tagged Data Methods
  //---------------------------------------------------------------------
  // Description:
  // This method takes care of posting a storable node that has multiple
  // storage nodes.
  int PostStorableNodes();

  // Description:
  // Before an upload, this method checks to make sure all storable
  // nodes have set storage nodes with valid filenames.
  int CheckStorageNodeFileNames();

  // Description:
  // Before an upload, this method checks to make sure all nodes
  // have been saved to local disk (as safety catch.)
  int CheckModifiedSinceRead ( );

  // Description:
  // This method is called when user requests data upload to XND.
  // It changes a storable node's write state to 'idle' which readies
  // it for the remoteIO pipeline.
  void SetIdleWriteStateOnSelectedResources ( );

  // Description:
  // This method is called internally when a remote upload or download
  // fails. It resets the transfer status on a node to make it usable
  // in future transfers. 
  void SetTransferDoneWriteStateOnSelectedResources ( );

  // Description:
  // This method is called internally when a remote upload or download
  // fails. It resets the transfer status on a node to make it usable
  // in future transfers. 
  void SetCancelledWriteStateOnSelectedResources ( );

  // Description:
  // This method is called when user requests data upload to XND.
  // It changes sets a storable node's URIhandler.
  void SetURIHandlerOnSelectedResources (vtkURIHandler *handler);

  // Description:
  // This method is called when user requests data upload to XND.
  // It changes a storable node's filename to include the cache path.
  void SetCacheFileNamesOnSelectedResources ( );

  
  // Description:
  // This method checks to see if the file for xml query responses
  // exists in the cache. If yes, returns the filename, and if not,
  // it returns NULL;
  const char *GetHTTPResponseFileName ( );

  // Description:
  // This method checks to see if the file for xml upload
  // exists. If yes, returns the filename, and if not,
  // it returns NULL;
  const char *GetXMLUploadFileName ( );

  // Description:
  // This method gets the name of the temporary file holding
  // xml data from webserivce. If no file exists, 
  // it returns NULL;
  const char *GetTemporaryResponseFileName ( );

  // Description:
  // This method gets the name of the file into which the
  // XML header is written for uploads. If no file exists,
  // returns NULL.
  const char *GetXMLHeaderFileName ( );

  // Description:
  // This method gets the name of the file containing the DtD for upload.
  // If no file exists, it returns NULL.
  const char *GetXMLDocumentDeclarationFileName ( );

  // Description:
  // Convenience methods for adding to and clearing
  // vector of modified nodes. If a user opts to save data, we can
  // make sure to recommend all modified nodes should be saved
  // and tagged.
  void ClearModifiedNodes();
  void AddModifiedNode (const char *nodeID);
  void RemoveModifiedNode (const char *nodeID);

  // Description:
  // Convenience methods for adding to and clearing
  // vector of selected nodes. A node listed as 'selected'
  // is marked for saving or tagging.
  void ClearSelectedStorableNodes();
  void AddSelectedStorableNode ( const char *nodeID);
  void RemoveSelectedStorableNode ( const char *nodeID);

  // Description:
  // Method that saves a user's list of selected resources to
  // upload and sets the current list of resources to include ALL.
  // This method is required until we can load/save any dataset,
  // and not just the scene file. NOTE TO DEVELOPERS: extend
  // this method as new storable node types are added to Slicer.
  void SaveResourceSelectionState ( );

  // Description:
  // Method restores the list of selected resources to a user's
  // selection specification.
  // This method is required until we can load/save any dataset,
  // and not just the scene file.
  void RestoreResourceSelectionState();

  // Description:
  // Flag that's set if scene description is marked for save.
  int SaveSceneDescription;

  // Description:
  // Flag that's set if a scene is selected for upload.
  int SceneSelected;

  // Description:
  // Flag that's set when internally adjusting selection state; this
  // flag records user's setting.
  int TemporarySceneSelected;

  // Description:
  // Tag Table that contains tags that apply to the scene.
  vtkTagTable *SceneTags;


  //---
  // These are names of the directory and files
  // that hold the xml responses we get back
  // from a query of any supported web service.
  //---
  // Query responses get written to these files by
  // the URIHandler, and Logic methods read and
  // parse contents of these files.
  //---
  std::string XMLDirName;
  std::string HTTPResponseFileName;
  std::string XMLUploadFileName;
  std::string TemporaryResponseFileName;
  std::string MRMLCacheFileName;
  std::string DownloadDirName;
  std::string HeaderFileName;
  std::string DocumentDeclarationFileName;

  // Vector of strings populated by the GUI when
  // the upload button is selected. 
  std::vector<std::string> SelectedStorableNodeIDs;
  std::vector<std::string> TemporarySelectedStorableNodeIDs;

  // The vector of uris that a delete action has removed on server.
  std::vector<std::string> URIsDeletedOnServer;

  std::vector<std::string> ModifiedNodes;

  // Map of strings populated by original filenames and new
  // file names changed for RemoteIO. Used in the event
  // that upload fails before all files are copied
  // to cache, and we need to revert back to original filenames. 
  std::map<std::string, std::string> OldAndNewFileNames;
  // Map of strings populated by original uris and new
  // uris changed for RemoteIO. Used in the event
  // that upload fails before all files are copied
  // to cache, and we need to revert back to original uris
  std::map<std::string, std::string> OldAndNewURIs;
  
  // Temporary map of tagnames to all values for that tag on a server.
  // Looks like:
  // Attribute1 --> {Value1, Value2, Value3...ValueI}
  // Attribute2 --> {Value1, Value2, Value3...ValueJ}
  // Attribute3 --> {Value1, Value2, Value3...ValueK}
  //...
  // AttributeM --> {Value1, Value2, Value3...ValueN}
  // This container is used to populate the Query GUI with
  // all known tags and their values on a server; exposing
  // ALL queriable metadata for a user. As attributes are
  // assigned values and selected, those populate the vtkTagTable for
  // the selected webservice, and are used to form query or to tag
  // individual storable nodes or the scene itself.
  std::map<std::string, std::vector< std::string> > CurrentWebServiceMetadata;

  virtual unsigned int GetNumberOfURIsDeletedOnServer ( );
  virtual void ResetURIsDeletedOnServer ( );

  // Description:
  // Method adds a new server to the collection.
  // Note to developers: Currently this method needs to provide a case
  // for each web service type available. TODO: generalize.
  virtual void AddNewServer ( const char *name,
                              const char *type,
                              const char *uriHandlerName,
                              const char *tagTableName );

  // Description:
  // Find number of known values for a given attribute for current XND server.
  virtual int GetNumberOfTagValues( const char *tagname);

 // Description:
  // Find number of Tags on the selected XND server;
  virtual int GetNumberOfTagsOnServer ( );
  
  // Description:
  // Get the Nth attribute (tagname)
  // in the list of tags from the current XND host
  const char *GetNthTagAttribute( int n);

  // Description:
  // Get the Nth value for a given attribute, in the
  // list of tags from the current XND host.
  const char *GetNthValueForTag(const char *tagname, int n);

  // Description:
  // Check to see if tagname is the name of an attribute
  // in the list of tags from the current XND host.
  int IsTagOnServer ( const char *tagname );

  // Description:
  // Search thru a vector of strings that contains the set of values
  // for a specified attribute, in a selected repository, and add the
  // value if it's not already in the vector.
  virtual void AddUniqueValueForTag ( const char *tagname, const char *val );

  // Description:
  // Search thru a map of strings that contains the list of tags (attributes)
  // for the selected repository, and add the tagname if not present.
  virtual void AddUniqueTag (const char *tagname );
    
  virtual int WriteDocumentDeclaration ( );
  virtual int WriteMetadataForUpload ( const char *ID );
  
 protected:
  vtkFetchMILogic();
  ~vtkFetchMILogic();
  vtkFetchMILogic(const vtkFetchMILogic&);
  void operator=(const vtkFetchMILogic&);

  virtual void ProcessMRMLSceneEvents(vtkObject *caller, unsigned long event,
                                      void *callData);
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event,
                                      void *callData);

  vtkFetchMIServerCollection *ServerCollection;
  vtkFetchMIServer *CurrentWebService;
  vtkMRMLFetchMINode* FetchMINode;

  // Description:
  // Number of Metadata attributes a database currently supports.
  int NumberOfTagsOnServer;

  // Description:
  // For internal use only; requires that this->OldAndNewFileNames
  // is populated properly, and if not, this could corrupt node filenames.
  // This method is called when an upload fails before all files
  // are successfully written to cache. In this event, all files marked
  // for upload will have filenames pointing to cache, but not all
  // files will be there. So we restore the original filenames, saved
  // in string map this->OldAndNewFileNames, to all resources marked for 
  // upload. Returns 1 for success, 0 for failure.
  int RestoreFileNamesOnSelectedResources ( );

  // Description:
  // For internal use only; requires that this->OldAndNewURIs
  // is populated properly, and if not, this could corrupt node filenames.
  // This method is called when an upload fails before all files
  // are successfully written to cache. In this event, all files marked
  // for upload will have filenames pointing to cache, but not all
  // files will be there. So we restore the original filenames, saved
  // in string map this->OldAndNewURIs, to all resources marked for 
  // upload. Returns 1 for success, 0 for failure.
  int RestoreURIsOnSelectedResources ( );

  // Description:
  // For internal use only; requires that this->OldAndNewURIs
  // is populated properly. It's called before an upload to save the
  // uri names on all resources in case the upload fails and we need
  // to restore uris.
  void SaveOldURIsOnSelectedResources( );

  // Description:
  // For internal use only; requires that this->OldAndNewURIs
  // is populated properly. It's called during the upload of each
  // resource just before the new uri is set on a storage node or
  // on the storagenode's urilist. If upload fails for some reason,
  // then the uri can be restored to the old value.
  void SaveNewURIOnSelectedResource ( const char *olduri, const char *newuri );
  
  // Description:
  // Flag that turns off when we're just querying host for metadata
  int ResourceQuery;
  int TagAttributesQuery;
  int TagValuesQuery;
  int ErrorChecking;
  int ParsingError;
  char *ReservedURI;
  bool Visited;
  bool Raised;

  // Description:
  // Toggles temporary infrastructure for performance profiling.
  bool Profiling;
};


#endif

