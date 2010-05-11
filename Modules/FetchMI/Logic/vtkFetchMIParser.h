#ifndef __vtkFetchMIParser_h
#define __vtkFetchMIParser_h

// MRML includes
#include "vtkMRMLFetchMINode.h"
#include "vtkTagTable.h"

// VTK includes
#include "vtkObject.h"
#include "vtkXMLDataParser.h"
#include "vtkXMLDataElement.h"

// STD includes
#include <string>
#include <map>
#include <vector>
#include <iterator>

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIParser : public vtkObject
{
  
public:
  static vtkFetchMIParser* New();
  vtkTypeRevisionMacro(vtkFetchMIParser,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set on the Name of this Parser
  vtkGetStringMacro ( Name );
  vtkSetStringMacro ( Name );

  // Description:
  // Get/Set on the Parser's response string.
  // Some Parser methods will fill this string so
  // Logic and/or GUI can access results.
  vtkGetStringMacro ( Response );
  vtkSetStringMacro ( Response );

  // Description:
  // Get/Set on the Number Of XML Elements parsed in the file.
  // Used for parsing XML files.
  vtkGetMacro ( NumberOfElements, int );
  vtkSetMacro ( NumberOfElements, int );
  
  // Description:
  // Get/Set on an error Flag. Flag is set to 1 if an error in
  // parsing has occurred, and set to 0 if no errors are detected.
  vtkGetMacro ( ErrorFlag, int );
  vtkSetMacro ( ErrorFlag, int );

  // Description:
  // Get/Set on an error checking setting. If ErrorChecking is set
  // to 1, then an error checking pass is made prior to parsing the
  // file to see if web service has sent an HTML error response.
  vtkGetMacro ( ErrorChecking, int );
  vtkSetMacro ( ErrorChecking, int );

  // Description:
  // Get/Set on the type of response the parser is expecting.
  // This var is a convenience to be used in the GetXMLEntry()
  // method. Known response types are defined in an enum in
  // this class and include:
  // vtkFetchMIParser::IdentityQueryResponse,
  // vtkFetchMIParser::TagValuesQueryResponse,
  // vtkFetchMIParser::TagAttributesQueryResponse, and
  // vtkFetchMIParser:: ResourceQueryResponse.
  vtkGetMacro ( ResponseType, int );
  vtkSetMacro ( ResponseType, int );
  
  // Description:
  // Get/Set on the FetchMINode. 
  vtkGetObjectMacro ( FetchMINode, vtkMRMLFetchMINode );
  vtkSetObjectMacro ( FetchMINode, vtkMRMLFetchMINode );

  // Description:
  // Function for parsing the response to query for server-supported metadata
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseMetadataQueryResponse( const char * vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to query for all values for any metadata attribute.
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseValuesForAttributeQueryResponse( const char *vtkNotUsed(filename),
                                                    const char *vtkNotUsed(att)) { return 0; };

  // Description:
  // Function for parsing the response to query for resources on the server.
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseResourceQueryResponse( const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to requests that metadata be deleted on server
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseMetatdataDeleteResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to requests that a resource be deleted on server
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseResourceDeleteResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to posting of new metadata to server.
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseMetadataPostResponse( const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to posting of reource to server.
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseResourcePostResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Virtual functions for parsing download requests.
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseResourceDownloadResponse(const char *vtkNotUsed(filename)) { return 0;};
  
  // Description:
  // Virtual function for looking for errors in response to posts or queries on the server.
  // Derived classes must define these, and any other parsing methods.
  virtual int ParseForErrors (const char *vtkNotUsed(filename)) { return 0;};
  
  // Description:
  // Function Getting an error message from an XML response.
  // Provides an alternative to filling the FetchMINode's ErrorMessage and
  // invoking an error event on the node to propagate.
  virtual const char* GetErrorString();

  // Description:
  // Function Setting an error message from an XML response.
  // Provides an alternative to filling the FetchMINode's ErrorMessage and
  // invoking an error event on the node to propagate.
  virtual void SetErrorString( const char *errorMessage);

  // Description:
  // Function to clear the ErrorString.
  virtual void ClearErrorString ();

  // Description:
  // Gets the number of XMLElements during parsing.
  virtual void GetNumberOfXMLElements ( vtkXMLDataElement *element );

  // Description:
  // Gets an individual XMLElement during XML parsing.
  virtual void GetXMLElement ( vtkXMLDataElement *element );

  // Description:
  // Gets an XMLEntry during XML parsing; this method must be redefined in each derived class.
  virtual void GetXMLEntry ( vtkXMLDataElement *vtkNotUsed(element) ) { };

  // Description:
  // Method adds a unique keyword (or attribute) in the temporary set of
  // tags defined in this->MetadataInformation
  virtual void AddUniqueTag ( const char *tagname);

  // Description:
  // Method adds a unique value for a keyword (or attribute) in the temporary set of
  // tags defined in this->MetadataInformation
  virtual void AddUniqueValueForTag ( const char *tagname, const char *value );

  // Description:
  // Method returns the number of values for any keyword (or attribute) in
  // the temporary set of tags defined in this->MetadataInformation
  virtual int GetNumberOfTagValues ( const char *tagname );
  
  // Description:
  // Method returns the number of tags (keyword/value pairs, or attribute/value pairs)
  // currently contained in the temporary this->MetadataInformation 
  virtual int GetNumberOfTagsOnServer ( );

  // Description:
  // Method returns the Nth keyword (or attribute) in the temporary
  // this->MetadataInformation
  virtual const char *GetNthTagAttribute ( int n );
  
  // Description:
  // Method returns the Nth Value for specified keyword (or attribute) in the 
  // temporary this->MetadataInformation  
  virtual const char * GetNthValueForTag(const char *tagname, int n);

  // Description:
  // Method clears out all tags in the temporary this->MetadataInformation
  virtual void ClearMetadataInformation ( );

  // Description:
  // Method clears out all values for a specified keyword (or attribute) in
  // the temporary this->MetadataInformation
  virtual void ClearValuesForAttribute ( const char *att);

  //BTX
  // kinds of responses we expect.
  enum {
    IdentityQueryResponse=1,
    TagValuesQueryResponse,
    TagAttributesQueryResponse,
    ResourceQueryResponse,
  };
  // To-Do, build an api for this so it's accessible by script.
  // Temporary map of tagnames to all values for that tag
  // Looks like:
  // Attribute1 --> {Value1, Value2, Value3...ValueI}
  // Attribute2 --> {Value1, Value2, Value3...ValueJ}
  // Attribute3 --> {Value1, Value2, Value3...ValueK}
  //...
  // AttributeM --> {Value1, Value2, Value3...ValueN}
  std::map<std::string, std::vector< std::string> > MetadataInformation;
  //ETX
  
 protected:
  vtkFetchMIParser();
  virtual ~vtkFetchMIParser();
  char *Name;
  char *Response;
  int NumberOfElements;
  int ErrorFlag;
  int ErrorChecking;
  int ResponseType;
  vtkMRMLFetchMINode *FetchMINode;

  //BTX
  std::string ErrorString;
  //ETX

  vtkFetchMIParser(const vtkFetchMIParser&); // Not implemented
  void operator=(const vtkFetchMIParser&); // Not Implemented
};

#endif

