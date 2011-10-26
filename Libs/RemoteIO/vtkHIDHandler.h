#ifndef __vtkHIDHandler_h
#define __vtkHIDHandler_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkHTTPHandler.h"
#include "vtkHIDTagTable.h"

class VTK_RemoteIO_EXPORT vtkHIDHandler : public vtkHTTPHandler 
{
  public:
  
  /// The Usual vtk class functions
  static vtkHIDHandler *New();
  vtkTypeRevisionMacro(vtkHIDHandler, vtkHTTPHandler);
  void PrintSelf(ostream& os, vtkIndent indent);

 /// Description:
  /// To prevent XND handler from capturing any non-xnd requests
  /// that start with http://, this test will fail. The handler must be
  /// called by name.
  virtual int CanHandleURI ( const char *vtkNotUsed(uri) ) { return 0; };

  /// 
  /// This function downloads the specified source (uri) to 
  /// a specified destination file, from a specified host.
  virtual void StageFileRead(const char * source,
                             const char * destination);

  using vtkURIHandler::StageFileRead; 
  
  /// 
  /// This function uploads the specified file (source) to
  /// the specified destination (uri), on a specified host.
  virtual void StageFileWrite(const char * source,
                              const char *destination);

  using vtkURIHandler::StageFileWrite; 
  
  virtual const char *QueryServer ( const char *uri, const char *destination);

 private:

 protected:
  vtkHIDHandler();
  virtual ~vtkHIDHandler();
  vtkHIDHandler(const vtkHIDHandler&);
  void operator=(const vtkHIDHandler&);

};

#endif

