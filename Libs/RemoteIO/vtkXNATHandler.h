#ifndef __vtkXNATHandler_h
#define __vtkXNATHandler_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include <vtksys/SystemTools.hxx>
#include <vtksys/Process.h>

//--- derived from libMRML class
#include "vtkURIHandler.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

class VTK_RemoteIO_EXPORT vtkXNATHandler : public vtkURIHandler 
{
  public:
  
  // The Usual vtk class functions
  static vtkXNATHandler *New();
  vtkTypeRevisionMacro(vtkXNATHandler, vtkURIHandler);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This methods returns 1 if the handler matches the uri's required 
  // protocol and returns 0 if it's not appropriate for the uri. uri must
  // start with srb://
  virtual int CanHandleURI ( const char *uri );

  // Description:
  // This function wraps SCommand functionality to download a specified URI to
  // a specified destination file
  void StageFileRead(const char * source,
                                   const char * destination,
                                   const char *username,
                                   const char *password,
                                   const char *hostname);
  void StageFileWrite(const char * source, const char * destination);

 private:

 protected:
  vtkXNATHandler();
  virtual ~vtkXNATHandler();
  vtkXNATHandler(const vtkXNATHandler&);
  void operator=(const vtkXNATHandler&);

};

#endif

