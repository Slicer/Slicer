#ifndef __vtkDataTransfer_h
#define __vtkDataTransfer_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRML.h"

class VTK_MRML_EXPORT vtkDataTransfer : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkDataTransfer *New();
  vtkTypeRevisionMacro(vtkDataTransfer, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkGetStringMacro ( SourceURI );
  vtkSetStringMacro ( SourceURI );
  vtkGetStringMacro ( DestinationURI );
  vtkSetStringMacro ( DestinationURI );
//  vtkGetObjectMacro ( Handler, vtkURIHandler );
//  vtkSetObjectMacro ( Handler, vtkURIHandler );
  vtkGetMacro ( TransferStatus, int );
  vtkSetMacro ( TransferStatus, int );
  vtkGetMacro ( TransferType, int );
  vtkSetMacro ( TransferType, int );
  vtkGetMacro ( TransferID, int );
  vtkSetMacro ( TransferID, int );

  //BTX
  enum
    {
      Done=0,
      Cancelled,
      Running,
      TimedOut,
      Initialized,
    };
  enum
    {
      RemoteDownload=0,
      RemoteUpload,
      LocalLoad,
      LocalSave,
      Unspecified,
    };
  //ETX

 private:

 protected:
  vtkDataTransfer();
  virtual ~vtkDataTransfer();
  vtkDataTransfer(const vtkDataTransfer&);
  void operator=(const vtkDataTransfer&);

  // Description:
  char *SourceURI;
  char *DestinationURI;
//  vtkURIHandler *Handler;
  int TransferStatus;
  int TransferID;
  int TransferType;

};

#endif

