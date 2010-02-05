#ifndef __vtkFetchMIWebServicesClient_h
#define __vtkFetchMIWebServicesClient_h

// MRML includes
#include "vtkURIHandler.h"
#include "vtkTagTable.h"

// VTK includes
#include "vtkObject.h"

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIWebServicesClient : public vtkObject
{
  
public:
  static vtkFetchMIWebServicesClient* New();
  vtkTypeRevisionMacro(vtkFetchMIWebServicesClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set on the Name of this WebServicesClient
  vtkGetStringMacro ( Name );
  vtkSetStringMacro ( Name );

  vtkGetObjectMacro ( URIHandler, vtkURIHandler );
  vtkSetObjectMacro ( URIHandler, vtkURIHandler );
  
  virtual int QueryServerForTags ( const char *vtkNotUsed(responseFileName) ) { return 0; };
  virtual int QueryServerForTagValues ( const char *vtkNotUsed(att),
                                        const char *vtkNotUsed(responseFilename) ) { return 0; };
  virtual int QueryServerForResources ( vtkTagTable *vtkNotUsed(table),
                                        const char *vtkNotUsed(responseFileName) ) { return 0; };
  virtual int DeleteResourceFromServer ( const char *vtkNotUsed(uri),
                                         const char *vtkNotUsed(responseFileName) ) { return 0; };

  // Description:
  // Creates a new tag on a server; method used during upload
  virtual int AddTagToServer ( const char *vtkNotUsed(att),
                               const char *vtkNotUsed(val),
                               const char *vtkNotUsed(responseFileName) ) {return 0; };
  
  // Description:
  // Posts metadata describing resource; method used during upload
  virtual int CreateNewURIForResource( const char *vtkNotUsed(resourceName),
                               const char *vtkNotUsed(headerFileName),
                               const char *vtkNotUsed(uploadFileName),
                               const char *vtkNotUsed(responseFileName))  {return 0; };

  virtual void Download ( const char *vtkNotUsed(src), const char *vtkNotUsed(dest) ) { };
  virtual void Upload ( const char *vtkNotUsed(src), const char *vtkNotUsed(dest) ) { };
  
 protected:
  vtkFetchMIWebServicesClient();
  virtual ~vtkFetchMIWebServicesClient();
  vtkURIHandler *URIHandler;
  char *Name;
  
  vtkFetchMIWebServicesClient(const vtkFetchMIWebServicesClient&); // Not implemented
  void operator=(const vtkFetchMIWebServicesClient&); // Not Implemented
};

#endif

