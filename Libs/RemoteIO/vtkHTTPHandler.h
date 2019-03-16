#ifndef __vtkHTTPHandler_h
#define __vtkHTTPHandler_h

// RemoteIO includes
#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>

// MRML includes
#include "vtkURIHandler.h"

class VTK_RemoteIO_EXPORT vtkHTTPHandler : public vtkURIHandler
{
public:

  /// The Usual vtk class functions
  static vtkHTTPHandler *New();
  vtkTypeMacro(vtkHTTPHandler, vtkURIHandler);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// This methods returns 1 if the handler matches the uri's required
  /// protocol and returns 0 if it's not appropriate for the uri.
  int CanHandleURI ( const char *uri ) override;

  /// Some web servers don't handle 'keep alive' socket transactions
  /// in a way that's compatible with curl on windows.  When this flag is set
  /// curl will do one transaction per connection with the side-effect
  /// that more network resources are used (so avoid this if you can).
  void SetForbidReuse(int value);
  int GetForbidReuse();

  /// This function wraps curl functionality to download a specified URL to a specified dir
  void StageFileRead(const char * source, const char * destination) override;
  using vtkURIHandler::StageFileRead;
  void StageFileWrite(const char * source, const char * destination) override;
  using vtkURIHandler::StageFileWrite;
  void InitTransfer () override;
  int CloseTransfer () override;

protected:
  vtkHTTPHandler();
  ~vtkHTTPHandler() override;
  vtkHTTPHandler(const vtkHTTPHandler&);
  void operator=(const vtkHTTPHandler&);

private:
  class vtkInternal;
  vtkInternal* Internal;
};

#endif
