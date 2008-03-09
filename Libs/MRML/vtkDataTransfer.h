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
      case vtkDataTransfer::Idle: return "Idle";
      case vtkDataTransfer::CancelPending: return "CancelPending";
      case vtkDataTransfer::Running: return "Running";
      case vtkDataTransfer::Completed: return "Completed";
      case vtkDataTransfer::CompletedWithErrors: return "CompletedWithErrors";
      case vtkDataTransfer::TimedOut: return "TimedOut";
      case vtkDataTransfer::Cancelled: return "Cancelled";
      case vtkDataTransfer::Ready: return "Ready";
      }
    return "Unknown";
  }

  const char* GetTransferTypeString( ) {
    switch (this->TransferStatus)
      {
      case vtkDataTransfer::RemoteDownload: return "RemoteDownload";
      case vtkDataTransfer::RemoteUpload: return "RemoteUpload";
      case vtkDataTransfer::LocalLoad: return "LocalUpload";
      case vtkDataTransfer::LocalSave: return "LocalSave";
      case vtkDataTransfer::Unspecified: return "Unspecified";
      }
    return "Unknown";
  }


  //BTX
  enum
    {
      Idle=0,
      Running,
      Completed,
      CompletedWithErrors,
      CancelPending,
      Cancelled,
      Ready,
      TimedOut,
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

