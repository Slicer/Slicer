#ifndef __vtkFetchMILogic_h
#define __vtkFetchMILogic_h

#include <vtksys/SystemTools.hxx>
#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"

#include "vtkTagTable.h"
#include "vtkFetchMI.h"
#include "vtkMRMLFetchMINode.h"
#include <string>
#include <vector>

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

  vtkGetMacro (SaveSceneDescription, int );
  vtkSetMacro (SaveSceneDescription, int );
  vtkGetMacro (SceneSelected, int );
  vtkSetMacro (SceneSelected, int );
  
  // Description:
  //Get/Set MRML node storing parameter values
  void SetAndObserveFetchMINode(vtkMRMLFetchMINode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->FetchMINode, n);
    }

  //---------------------------------------------------------------------
  // Query And Download Methods
  //---------------------------------------------------------------------
  // Description:
  // This method queries the selected webservice for available tags,
  // checks to see if the query result is valid and if so, initiates
  // the parsing of that result into the FetchMINode's tagtable.
  void QueryServerForTags ( );

  // Description:
  // This method forms appropriate URIs from a list of tags
  // selected in the FetchMINode.
  void QueryServerForResources ( );

  // Description:
  // In XND, we can query to see what tags are available.
  // this method fills up the FetchMINode's XNDTagTable
  // which is used to populate the list of available tags in the GUI.
  // if the query returns with an error, the node's error message
  // is filled.
  void ParseTagQueryResponse ( );

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
  // from a supported webservice.
  void RequestSceneDownloadFromXND ( const char *uri);
  void RequestSceneDownloadFromHID ( const char *uri);
  
  // Description:
  // This method requests download of a slicerDataType
  // from a supported webservice.
  void RequestResourceDownloadFromXND ( const char *uri, const char *slicerDataType);
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
  
  void SelectNode ( const char *nodeID );
  void DeselectNode ( const char *nodeID );
  void SelectScene();
  void DeselectScene();
  
  //---------------------------------------------------------------------
  // Upload Tagged Data Methods
  //---------------------------------------------------------------------
  // Description:
  // This method takes each dataset selected for upload

  // return 1 if file is written and 0 if not.
  int WriteMetadataForUpload ( const char *filename, const char *ID );

  // Description:
  // This method sets up the upload of each storable node
  // in the scene (in this->SelectedStorableNodeIDs).
  // For all resources, it generates metadata and uplaods it,
  // then it uploads data, and finally uploads scene.
  int TestForRequiredTags ( );
  void RequestResourceUpload ();
  void RequestResourceUploadToXND ();
  void RequestResourceUploadToHID ();

  const char* PostMetadata();
  const char *ParsePostMetadataResponse(const char * response);
  void GetNumberOfXMLElements(vtkXMLDataElement *element);
  void GetXMLElement(vtkXMLDataElement *element );
  void GetXNDXMLEntry ( vtkXMLDataElement *element );
  void GetHIDXMLEntry ( vtkXMLDataElement *element );
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
  const char *GetXMLUploadFileName ( );

  // Description:
  // Convenience methods for adding to and clearing
  // vector of modified nodes
  void ClearModifiedNodes();
  void AddModifiedNode (const char *nodeID);
  void RemoveModifiedNode (const char *nodeID);

  // Description:
  // Convenience methods for adding to and clearing
  // vector of selected nodes
  void ClearSelectedStorableNodes();
  void AddSelectedStorableNode ( const char *nodeID);
  void RemoveSelectedStorableNode ( const char *nodeID);
  
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
  std::string MRMLCacheFileName;
  std::string DownloadDirName;
  // Vector of strings populated by the GUI when
  // the upload button is selected. 
  std::vector<std::string> SelectedStorableNodeIDs;
  std::vector<std::string> ModifiedNodes;
  //ETX
  // flag that's set if scene is selected for uplaod.
  int SaveSceneDescription;
  int SceneSelected;

  // contains tags that apply to the scene.
  vtkTagTable *SceneTags;

protected:
  vtkFetchMILogic();
  ~vtkFetchMILogic();
  vtkFetchMILogic(const vtkFetchMILogic&);
  void operator=(const vtkFetchMILogic&);

  vtkMRMLFetchMINode* FetchMINode;
  // number of elements in a scene being parsed.
  int NumberOfElements;
  const char *CurrentURI;
};

#endif

