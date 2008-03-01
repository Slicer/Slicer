#ifndef __vtkHTTPHandler_h
#define __vtkHTTPHandler_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <slicerlibcurl/slicerlibcurl.h>

#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include <vtksys/SystemTools.hxx>
#include <vtksys/Process.h>

//--- derived from libMRML class
#include "vtkURLHandler.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

class VTK_RemoteIO_EXPORT vtkHTTPHandler : public vtkURLHandler 
{
  public:
  
  // The Usual vtk class functions
  static vtkHTTPHandler *New();
  vtkTypeRevisionMacro(vtkHTTPHandler, vtkURLHandler);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This methods returns 1 if the handler matches the uri's required 
  // protocol and returns 0 if it's not appropriate for the uri.
  virtual int CanHandleURI ( const char *uri );

  // Description:
  // This function wraps curl functionality to download a specified URL to a specified dir
  void StageFileRead(const char * source, const char * destination);
  void StageFileWrite(const char * source, const char * destination);
  CURL* CurlHandle = NULL;  

 private:
  virtual void InitTransfer ( );
  virtual void CloseTransfer ( );

 protected:
  vtkHTTPHandler();
  virtual ~vtkHTTPHandler();
  vtkHTTPHandler(const vtkHTTPHandler&);
  void operator=(const vtkHTTPHandler&);

};

#endif

