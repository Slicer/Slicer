#ifndef __vtkFetchMIParserHID_h
#define __vtkFetchMIParserHID_h

// FetchMI includes
#include "vtkFetchMIParser.h"

// VTK includes
#include "vtkObject.h"
#include "vtkXMLDataParser.h"
#include "vtkXMLDataElement.h"

// STD includes
#include <string>

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIParserHID : public vtkFetchMIParser
{
  
public:
  static vtkFetchMIParserHID* New();
  vtkTypeRevisionMacro(vtkFetchMIParserHID, vtkFetchMIParser);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Method returns the class's vtkXMLDataParser
  vtkGetObjectMacro ( Parser, vtkXMLDataParser );

  // Description:
  // Function for parsing the response to query for server-supported metadata
  virtual int ParseMetadataQueryResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to query for all values for any metadata attribute.
  virtual int ParseValuesforAttributeQueryResponse(const char *vtkNotUsed(filename),
                                                   const char *vtkNotUsed(att))
    { return 0; };

  // Description:
  // Function for parsing the response to query for resources on the server.
  virtual int ParseResourceQueryResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to requests that metadata be deleted on server
  virtual int ParseMetatdataDeleteResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to requests that a resource be deleted on server
  virtual int ParseResourceDeleteResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to posting of new metadata to server.
  virtual int ParseMetadataPostResponse( const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Function for parsing the response to posting of reource to server.
  virtual int ParseResourcePostResponse(const char *vtkNotUsed(filename)) { return 0; };

  // Description:
  // Virtual functions for parsing download requests.
  virtual int ParseResourceDownloadResponse(const char *vtkNotUsed(filename)) { return 0; };
  
  // Description:
  // Virtual function for looking for errors in response to posts or queries on the server.
  virtual int ParseForErrors (const char *vtkNotUsed(filename)) { return 0; };
  
  // Description:
  // Gets an XMLEntry and depending on the ResponseType the
  // parser is expecting, does the appropriate thing to process it.
  virtual void GetXMLEntry ( vtkXMLDataElement *element );
  
 protected:
  vtkFetchMIParserHID();
  virtual ~vtkFetchMIParserHID();

  vtkXMLDataParser *Parser;
  std::string ErrorString;

  vtkFetchMIParserHID(const vtkFetchMIParserHID&); // Not implemented
  void operator=(const vtkFetchMIParserHID&); // Not Implemented
};

#endif

