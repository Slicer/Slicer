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
  
  /// The Usual vtk class functions
  static vtkXNDHandler *New();
  vtkTypeRevisionMacro(vtkXNDHandler, vtkHTTPHandler);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// To prevent XND handler from capturing any non-xnd requests
  /// that start with http://, this test will fail. The handler must be
  /// called by name.
  virtual int CanHandleURI ( const char *vtkNotUsed(uri) ) { return 0; };

  /// 
  /// This function downloads the specified source (uri) to 
  /// a specified destination file, from a specified host.
  virtual void StageFileRead(const char * source,
                             const char * destination);

  //BTX
  using vtkURIHandler::StageFileRead;
  //ETX

  int PostTag (const char *svr,  const char *label, const char *temporaryResponseFileName);
  
  /// 
  /// serverPath will contain $srv/data, for instance http://localhost:8081/data
  /// metaDataFileName is a filename of a file that has metadata in it.
  /// dataFileName is the name of the file for which we are uploading metadata
  /// temporaryResponseFileName is the name of the file into which
  /// the server response to the POST is sent. This file is parsed for error or uri.
  int PostMetadata ( const char *serverPath,
                         const char *headerFileName,
                         const char *dataFileName,
                         const char *metaDataFileName,
                         const char *temporaryResponseFileName )  ;
  /// 
  /// This function uploads the specified file (source) to
  /// the specified destination (uri), on a specified host.
  virtual void StageFileWrite(const char * source,
                              const char *destination);

  //BTX
  using vtkURIHandler::StageFileWrite;
  //ETX
  
  virtual const char *QueryServer ( const char *uri, const char *destination);
  const char *GetXMLDeclaration();
  const char *GetNameSpace();

  virtual int DeleteResource ( const char *uri, const char *temporaryResponseFileName); 

 private:

 protected:
  vtkXNDHandler();
  virtual ~vtkXNDHandler();
  vtkXNDHandler(const vtkXNDHandler&);
  void operator=(const vtkXNDHandler&);

};

#endif

