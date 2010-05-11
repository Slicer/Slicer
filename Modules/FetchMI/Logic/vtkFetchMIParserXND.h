#ifndef __vtkFetchMIParserXND_h
#define __vtkFetchMIParserXND_h

// FetchMI includes
#include "vtkFetchMIParser.h"

// VTK includes
#include "vtkObject.h"
#include "vtkXMLDataParser.h"
#include "vtkXMLDataElement.h"

// STD includes
#include <string>

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIParserXND : public vtkFetchMIParser
{
  
public:
  static vtkFetchMIParserXND* New();
  vtkTypeRevisionMacro(vtkFetchMIParserXND, vtkFetchMIParser);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Method returns the class's vtkXMLDataParser
  vtkGetObjectMacro ( Parser, vtkXMLDataParser );

  // Description:
  // Function for parsing the response to query for server-supported metadata
  virtual int ParseMetadataQueryResponse(const char *filename );

  // Description:
  // Function for parsing the response to query for all values for any metadata attribute.
  virtual int ParseValuesForAttributeQueryResponse(const char *filename, const char *att );

  // Description:
  // Function for parsing the response to query for resources on the server.
  virtual int ParseResourceQueryResponse(const char *filename );

  // Description:
  // Function for parsing the response to requests that metadata be deleted on server
  virtual int ParseMetatdataDeleteResponse(const char *filename );

  // Description:
  // Function for parsing the response to requests that a resource be deleted on server
  virtual int ParseResourceDeleteResponse(const char *filename );

  // Description:
  // Function for parsing the response to posting of new metadata to server.
  virtual int ParseMetadataPostResponse( const char *filename);

  // Description:
  // Function for parsing the response to posting of reource to server.
  virtual int ParseResourcePostResponse(const char *filename );

  // Description:
  // Virtual functions for parsing download requests.
  virtual int ParseResourceDownloadResponse(const char *filename );
  
  // Description:
  // Virtual function for looking for errors in response to posts or queries on the server.
  virtual int ParseForErrors (const char *val );
  
  // Description:
  // Gets an XMLEntry and depending on the ResponseType the
  // parser is expecting, does the appropriate thing to process it.
  virtual void GetXMLEntry ( vtkXMLDataElement *element );
  
 protected:
  vtkFetchMIParserXND();
  virtual ~vtkFetchMIParserXND();

  vtkXMLDataParser *Parser;
  //BTX
  std::string ErrorString;
  //ETX

  vtkFetchMIParserXND(const vtkFetchMIParserXND&); // Not implemented
  void operator=(const vtkFetchMIParserXND&); // Not Implemented
};

#endif

