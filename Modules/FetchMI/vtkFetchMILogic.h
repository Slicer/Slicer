#ifndef __vtkFetchMILogic_h
#define __vtkFetchMILogic_h

#include <vtksys/SystemTools.hxx>
#include "vtkObject.h"

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"

#include "vtkTagTable.h"
#include "vtkFetchMI.h"
#include "vtkMRMLFetchMINode.h"
#include <string>
#include <vector>
#include <map>
#include <iterator>

class vtkXNDHandler;
class VTK_FETCHMI_EXPORT vtkFetchMILogic : public vtkSlicerModuleLogic
{
  public:
  static vtkFetchMILogic *New();
  vtkTypeMacro(vtkFetchMILogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description:
  // Get/Set MRML node storing parameter values
  vtkGetObjectMacro (FetchMINode, vtkMRMLFetchMINode);
  vtkSetObjectMacro (FetchMINode, vtkMRMLFetchMINode);

  // Description:
  // Flag allowing user to select whether or not to save scene description. Not currrently used.
  vtkGetMacro (SaveSceneDescription, int );
  vtkSetMacro (SaveSceneDescription, int );
  // Description:
  // Flag detecting whether scene is selected for saving (otherwise just selected data is saved.)
  vtkGetMacro (SceneSelected, int );
  vtkSetMacro (SceneSelected, int );
  // Description:
  // String containing the current user-query. Currently not used, but might be useful for something...
  vtkGetStringMacro (CurrentQuery);
  vtkSetStringMacro (CurrentQuery);

  // Description:
  // String containing the currently selected URI.
  vtkGetStringMacro (CurrentURI);
  vtkSetStringMacro (CurrentURI);
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
  vtkGetMacro (RestoreTagSelectionState, int );
  vtkSetMacro (RestoreTagSelectionState, int );

  
  // Description:
  //Get/Set MRML node storing parameter values
  void SetAndObserveFetchMINode(vtkMRMLFetchMINode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->FetchMINode, n);
    }

  //---------------------------------------------------------------------
  // Query, Parsing, And Download Methods
  //---------------------------------------------------------------------
  // Description:
  // This method queries the selected webservice for available tags,
  // checks to see if the query result is valid and if so, calls the
  // ParseTagQueryResponse() method to parse the resulting XML
  // into the FetchMINode's tagtable with default values (used for resource query and markup),
  // and into the class's container for all the selected webservice's existing metadata,
  // called  AllValuesForAllTagsOnServer
  void QueryServerForTags ( );
  // Description:
  // This method queries the selected webservice for values for
  // each tag the server knows about. It initiates parsing of the
  // XML response by the ParseTagValuesQueryResponse() method
  // into the class's container for all the selected webservice's existing 
  // metadata, called AllValuesForAllTagsOnServer (used for Query).
  void QueryServerForTagValues ( );
  // Description:
  // Method cleans out AllValuesForAllTagsOnServer each time a
  // host is queried for the tags it supports.
  void ClearExistingTagsForQuery ( );
  // Description:
  // Method clears out vector of strings corresponding to a specific
  // tag name in the AllValuesForAllTagsOnServer when a host is
  // queried for the known values for that tag.
  void ClearExistingValuesForTag ( const char *tagname );

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

  // Description:
  // In XND, we can query to see what tags are available.
  // this method fills up the FetchMINode's XNDTagTable (for resource query and markup)
  // which is used to populate the list of available tags in the GUI,
  // and into the class's container for the selected webservices's known
  // metadata, called AllValuesForAllTagsOnServer.
  // if the query returns with an error, the node's error message
  // is filled.
  void ParseTagQueryResponse ( );
  // Description:
  // In XND, we can query to see what attributes are listed
  // currently for a given tag.
  // this method fills up the class's AllValuesForAllTagsOnServer vector
  // of strings, which is used to populate the menu of possible
  // values for each tag in the GUI. If the query returns with an error,
  // the node's error message is filled.
  void ParseTagValuesQueryResponse ( );

  // Description:
  // Using XND supported and user specified tags,
  // queries for resources are made. This method parses
  // the response to those queries, and fills up the logic's
  // ResourceDescription (table) -- which is used to
  // populate the list of returned resources in the GUI.
  void ParseResourceQueryResponse ( );
  
  // Description:
  // This method assembles the appropriate uri,
  // filters the slicerDataType to make sure it is a
  // known type, and sets up its download.
  int RequestResourceDownload ( const char *uri, const char *slicerDataType );

  // Description:
  // This method requests download of the scene
  // from a supported XNAT Desktop webservice.
  void RequestSceneDownloadFromXND ( const char *uri);
  // Description:
  // not fully implemented
  void RequestSceneDownloadFromHID ( const char *uri);
  
  // Description:
  // This method requests download of a slicerDataType
  // from a supported XNAT Desktop webservice.
  void RequestResourceDownloadFromXND ( const char *uri, const char *slicerDataType);
  // Description:
  // not fully implemented
  void RequestResourceDownloadFromHID ( const char *uri, const char *slicerDataType );


  
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
  // Method takes a data type as a string and
  // returns the index of the known data type,
  // or a -1 if the type is unknown.
  int CheckValidSlicerDataType ( const char *dtype);
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
  

  //---------------------------------------------------------------------
  // Upload Tagged Data Methods
  //---------------------------------------------------------------------
  // Description:
  // This method takes each dataset selected for upload
  // and writes the metadata for it.
  // Method returns 1 if write is successful, and 0 if not.
  int WriteMetadataForUpload ( const char *filename, const char *ID, vtkXNDHandler *handler);
  // Description:
  // Posts metadata to XND and  returns 1 if OK, 0 if not.
  int PostMetadataToXND( vtkXNDHandler *handler, const char *dataFilename );
  // Description:
  // This method takes care of posting a storable node that has multiple
  // storage nodes.
  void PostStorableNodesToXND();
  // Description:
  // This method Parses the response to a Metadata post which
  // should return a uri to which the data can be associated.
  const char *ParsePostMetadataResponse(const char * response);

  // Description:
  // Method checks to see if the data to be uploaded has the
  // required tags. Current list of required tags:
  // Experiment
  // Project
  // Scan
  // Subject
  // Modality
  // SlicerDataType
  // TODO: Check to see if this is still valid: used to be important for XNE...
  // Method return 1 if all tags are present, 0 if not.
  int TestForRequiredTags ( );

  // Description:
  // Before an upload, this method checks to make sure all storable
  // nodes have set storage nodes with valid filenames.
  int CheckStorageNodeFileNames();
  // Description:
  // This method is called when user requests data upload to XND.
  // It changes a storable node's filename to include the cache path, and
  // and to set its URIHandler to be the XNDHandler.
  void SetCacheFileNamesAndXNDHandler(vtkXNDHandler *handler);

  // Description:
  // General method that returns the number of XML Elements in any XML response.
  // Method is used during parsing of ALL Webservices responses.
  // NOTE: Eventually we may have to break this into webservice-specific methods,
  // but for now, it's all in here.
  void GetNumberOfXMLElements(vtkXMLDataElement *element);
  // Description:
  // General method that gets an XML element from any XML response.
  // Method is used during parsing of ALL Webservices responses.
  // NOTE: Eventually we may have to break this into webservice-specific methods,
  // but for now, it's all in here.
  void GetXMLElement(vtkXMLDataElement *element );
  // Description:
  // Gets an entry from XND WebServices response
  // Method is used during parsing of ALL Webservices responses.
  // NOTE: Eventually we may have to break this into query-specific methods,
  // but for now, it's all in here.  
  void GetXNDXMLEntry ( vtkXMLDataElement *element );
  // Description:
  // not fully implemented
  void GetHIDXMLEntry ( vtkXMLDataElement *element );

  // Description:
  // Used when parsing XML files.
  vtkGetMacro ( NumberOfElements, int );
  vtkSetMacro ( NumberOfElements, int );

  // Description:
  // This method checks to see where the cache directory is
  // and creates filenames for query responses and for
  // upload metadata descriptions.
  void CreateTemporaryFiles ( );
  
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
  // Flag that's set if scene description is marked for save.
  int SaveSceneDescription;
  // Description:
  // Flag that's set if a scene is selected for upload.
  int SceneSelected;
  // Description:
  // Tag Table that contains tags that apply to the scene.
  vtkTagTable *SceneTags;

  //BTX
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
  std::vector<std::string> ModifiedNodes;

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
  std::map<std::string, std::vector< std::string> > AllValuesForAllTagsOnServer;
  //ETX

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
  // Search thru a vector of strings that contains the set of values
  // for a specified attribute, in a selected repository, and add the
  // value if it's not already in the vector.
  virtual void AddUniqueValueForTag ( const char *tagname, const char *val );

  // Description:
  // Search thru a map of strings that contains the list of tags (attributes)
  // for the selected repository, and add the tagname if not present.
  virtual void AddUniqueTag (const char *tagname );
    
protected:
  vtkFetchMILogic();
  ~vtkFetchMILogic();
  vtkFetchMILogic(const vtkFetchMILogic&);
  void operator=(const vtkFetchMILogic&);

  vtkMRMLFetchMINode* FetchMINode;
  // number of elements in a scene being parsed.
  int NumberOfElements;
  // Description:
  // Number of Metadata attributes a database currently supports.
  int NumberOfTagsOnServer;
  char *CurrentURI;
  char *CurrentQuery;

  // Description:
  // Flag that turns off when we're just querying host for metadata
  int ResourceQuery;
  int TagAttributesQuery;
  int TagValuesQuery;
  int ErrorChecking;
  int ParsingError;
  int RestoreTagSelectionState;
};


#endif

