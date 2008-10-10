#ifndef __vtkXNDHandler_h
#define __vtkXNDHandler_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkHTTPHandler.h"
#include "vtkXNDTagTable.h"

class VTK_RemoteIO_EXPORT vtkXNDHandler : public vtkHTTPHandler 
{
  public:
  
  // The Usual vtk class functions
  static vtkXNDHandler *New();
  vtkTypeRevisionMacro(vtkXNDHandler, vtkHTTPHandler);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetStringMacro (HostName);
  vtkSetStringMacro (HostName);

  // Description:
  // To prevent XND handler from capturing any non-xnd requests
  // that start with http://, this test will fail. The handler must be
  // called by name.
  virtual int CanHandleURI ( const char *uri ) { return 0; };

  // Description:
  // This function downloads the specified source (uri) to 
  // a specified destination file, from a specified host.
  virtual void StageFileRead(const char * source,
                             const char * destination);


  // Description:
  // This function uploads the specified file (source) to
  // the specified destination (uri), on a specified host.
  virtual void StageFileWrite(const char * source,
                              const char *destination);


  virtual const char *QueryServer ( const char *uri, const char *destination);
  const char *GetXMLDeclaration();
  const char *GetNameSpace();

 private:
  char *HostName;

 protected:
  vtkXNDHandler();
  virtual ~vtkXNDHandler();
  vtkXNDHandler(const vtkXNDHandler&);
  void operator=(const vtkXNDHandler&);

};

#endif

