#ifndef __vtkURIHandler_h
#define __vtkURIHandler_h

#include "vtkObject.h"
#include "vtkMRML.h"
#include "vtkPermissionPrompter.h"

class VTK_MRML_EXPORT vtkURIHandler : public vtkObject 
{
  public:
  // The Usual vtk class functions
  static vtkURIHandler *New() { return NULL; };
  vtkTypeRevisionMacro(vtkURIHandler, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // virtual methods to be defined in subclasses.
  // (Maybe these should be defined to handle default file operations)
  virtual void StageFileRead ( const char *source, const char * destination ) { };
  virtual void StageFileWrite ( const char *source, const char * destination ) { };

  // Description:
  // various Read/Write method footprints useful to redefine in specific handlers.
  virtual void StageFileRead(const char * source,
                             const char * destination,
                             const char *username,
                             const char *password,
                             const char *hostname) { };

  virtual void StageFileWrite(const char *source,
                              const char *username,
                              const char *password,
                              const char *hostname,
                              const char *sessionID ) { };

  // need something that goes the other way too...

  //BTX
  // Description:
  // Determine whether protocol is appropriate for this handler.
  // NOTE: Subclasses should implement this method
  virtual int CanHandleURI ( const char *uri ) = 0;

  // Description:
  // This function writes the downloaded data in a buffered manner
  size_t BufferedWrite ( char *buffer, size_t size, size_t nitems );

  // Description:
  // Use this function to set LocalFile
  //virtual void SetLocalFile ( std::ofstream * localFile );
  virtual void SetLocalFile (FILE *localFile);

  // Description:
  // This function gives us some feedback on how our download is going.
  int ProgressCallback(FILE* outputFile, double dltotal, double dlnow, double ultotal, double ulnow);
  //ETX

  vtkGetMacro ( RequiresPermission, int );
  vtkSetMacro ( RequiresPermission, int );
  vtkGetObjectMacro ( PermissionPrompter, vtkPermissionPrompter );
  vtkSetObjectMacro ( PermissionPrompter, vtkPermissionPrompter );
  vtkGetStringMacro ( Prefix );
  vtkSetStringMacro ( Prefix );

 private:

  //--- Methods to configure and close transfer
  // NOTE: Subclasses should implement these method
  virtual void InitTransfer ( ) {};
  virtual int CloseTransfer ( )
      {
      return 0;
      }

  int RequiresPermission;
  vtkPermissionPrompter *PermissionPrompter;


 protected:
  vtkURIHandler();
  virtual ~vtkURIHandler();
  vtkURIHandler(const vtkURIHandler&);
  void operator=(const vtkURIHandler&);

  // Description:
  // local file, it gets passed to C functions in libcurl 
  //BTX
  //std::ofstream* LocalFile;
  //ETX
  FILE *LocalFile;
  char *Prefix;
};

#endif






