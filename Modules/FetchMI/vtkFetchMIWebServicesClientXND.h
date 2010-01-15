#ifndef __vtkFetchMIWebServicesClientXND_h
#define __vtkFetchMIWebServicesClientXND_h

#include "vtkObject.h"
#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIWebServicesClient.h"

class VTK_FETCHMI_EXPORT vtkFetchMIWebServicesClientXND : public vtkFetchMIWebServicesClient
{
  
public:
  static vtkFetchMIWebServicesClientXND* New();
  vtkTypeRevisionMacro(vtkFetchMIWebServicesClientXND,vtkFetchMIWebServicesClient);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int QueryServerForTags ( const char *responseFileName );
  virtual int QueryServerForTagValues ( const char *att, const char *responseFilename );
  virtual int QueryServerForResources ( vtkTagTable *table, const char *responseFileName );
  virtual int DeleteResourceFromServer ( const char *uri, const char *responseFileName );

  // Description:
  // Creates a new tag on a server; method used during upload
  virtual int AddTagToServer ( const char *att, const char *val, const char *responseFileName);
  
  // Description:
  // Posts metadata describing resource; method used during upload
  virtual int CreateNewURIForResource( const char *resourceName,
                               const char *headerFileName,
                               const char *uploadFileName,
                               const char *responseFileName);
  
  virtual void Download ( const char *vtkNotUsed(src), const char *vtkNotUsed(dest) );
  virtual void Upload ( const char *vtkNotUsed(src), const char *vtkNotUsed(dest) ) { };
  
 protected:
  vtkFetchMIWebServicesClientXND();
  virtual ~vtkFetchMIWebServicesClientXND();

  vtkFetchMIWebServicesClientXND(const vtkFetchMIWebServicesClientXND&); // Not implemented
  void operator=(const vtkFetchMIWebServicesClientXND&); // Not Implemented
};

#endif

