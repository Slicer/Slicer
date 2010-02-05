#ifndef __vtkFetchMIServer_h
#define __vtkFetchMIServer_h

// FetchMI includes
#include "vtkFetchMIParser.h"
#include "vtkFetchMIWriter.h"
#include "vtkFetchMIWebServicesClient.h"

// MRML includes
#include "vtkTagTable.h"
#include "vtkURIHandler.h"

// VTK includes
#include "vtkObject.h"

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIServer : public vtkObject
{
public:
  static vtkFetchMIServer* New();
  vtkTypeRevisionMacro(vtkFetchMIServer, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set on the name of the host
  vtkGetStringMacro ( Name );
  vtkSetStringMacro ( Name );

  // Description:
  // Get/Set on the Server's parser
  vtkSetObjectMacro ( Parser, vtkFetchMIParser );
  vtkGetObjectMacro ( Parser, vtkFetchMIParser );

  // Description:
  // Get/Set on the Server's writer
  vtkSetObjectMacro ( Writer, vtkFetchMIWriter );
  vtkGetObjectMacro ( Writer, vtkFetchMIWriter );

  // Description:
  // Get/Set on the Server's clientcaller
  vtkSetObjectMacro ( WebServicesClient, vtkFetchMIWebServicesClient );
  vtkGetObjectMacro ( WebServicesClient, vtkFetchMIWebServicesClient );

  // Description:
  // Get/Set on the Server's clientcaller
  vtkSetObjectMacro ( URIHandler, vtkURIHandler );
  vtkGetObjectMacro ( URIHandler, vtkURIHandler );

  // Description:
  // The logic's ServerCollection contains an enumeration of all
  // supported service types; Slicer uses these types to determine
  // what kind of webservice goes with this server.
  vtkGetStringMacro ( ServiceType);
  vtkSetStringMacro ( ServiceType);

  // Description:
  // Get/Set on the name of a URIHandler that goes with this server.
  // The table can retrieved from the MRMLScene's handler collection by name.
  vtkGetStringMacro ( URIHandlerName );
  vtkSetStringMacro ( URIHandlerName );

  
  // Description:
  // Get/Set on the name of a TagTable that goes with this server.
  // Certain web services may have their own required tags, which
  // should populate the table. The table can be retrieved from the
  // logic's collection of tag tables by name.
  vtkGetStringMacro ( TagTableName );
  vtkSetStringMacro ( TagTableName );
  vtkGetObjectMacro ( TagTable, vtkTagTable );
  vtkSetObjectMacro ( TagTable, vtkTagTable );

 protected:
  vtkFetchMIServer();
  virtual ~vtkFetchMIServer();
  char *ServiceType;
  char *Name;
  char *URIHandlerName;
  char *TagTableName;

  vtkFetchMIParser *Parser;
  vtkFetchMIWriter *Writer;
  vtkFetchMIWebServicesClient *WebServicesClient;
  vtkURIHandler *URIHandler;
  vtkTagTable *TagTable;
  
  vtkFetchMIServer(const vtkFetchMIServer&); // Not implemented
  void operator=(const vtkFetchMIServer&); // Not Implemented
};

#endif

