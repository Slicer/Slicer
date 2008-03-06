#ifndef __vtkDataTransfer_h
#define __vtkDataTransfer_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRML.h"
#include "vtkURIHandler.h"

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
  vtkGetObjectMacro ( Handler, vtkURIHandler );
  vtkSetObjectMacro ( Handler, vtkURIHandler );
  vtkGetMacro ( TransferType, int );
  vtkSetMacro ( TransferType, int );
  vtkGetMacro ( TransferID, int );
  vtkSetMacro ( TransferID, int );
  vtkGetStringMacro ( TransferNodeID);
  vtkSetStringMacro ( TransferNodeID);  
  vtkGetMacro ( Progress, int );
  vtkSetMacro ( Progress, int );
  vtkGetMacro ( TransferStatus, int );

  vtkGetMacro ( CancelRequested, int );
  vtkSetMacro ( CancelRequested, int );

  // Description:
  // Set the status of a data transfer (Idle, Scheduled, Cancelled Running,
  // Completed).  The "modify" parameter indicates whether the object
  // can be modified by the call.
  void SetTransferStatus(int status, bool modify);
  
  const char* GetTransferStatusString( ) {
    switch (this->TransferStatus)
      {
      case Idle: return "Idle";
      case Scheduled: return "Scheduled";
      case Running: return "Running";
      case Completed: return "Completed";
      case CompletedWithErrors: return "CompletedWithErrors";
      case Cancelled: return "Cancelled";
      case Ready: return "Ready";
      }
    return "Unknown";
  }

  const char* GetTransferTypeString( ) {
    switch (this->TransferStatus)
      {
      case RemoteDownload: return "RemoteDownload";
      case RemoteUpload: return "RemoteUpload";
      case LocalLoad: return "LocalUpload";
      case LocalSave: return "LocalSave";
      case Unspecified: return "Unspecified";
      }
    return "Unknown";
  }


  //BTX
  enum
    {
      Idle=0,
      Scheduled=1,
      Running=2,
      Completed=3,
      CompletedWithErrors=4,
      Cancelled=5,
      Ready=6,
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
  vtkURIHandler *Handler;
  int TransferStatus;
  int TransferID;
  int TransferType;
  char* TransferNodeID;
  int Progress;
  int CancelRequested;

};

#endif

