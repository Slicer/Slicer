#ifndef __vtkDataTransfer_h
#define __vtkDataTransfer_h

// MRML includes
#include "vtkMRML.h"
class vtkURIHandler;

// VTK includes
#include <vtkObject.h>

class VTK_MRML_EXPORT vtkDataTransfer : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkDataTransfer *New();
  vtkTypeMacro(vtkDataTransfer, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkGetStringMacro ( SourceURI );
  vtkSetStringMacro ( SourceURI );
  vtkGetStringMacro ( DestinationURI );
  vtkSetStringMacro ( DestinationURI );
  vtkGetObjectMacro ( Handler, vtkURIHandler );
  virtual void SetHandler(vtkURIHandler* uriHandler);
  vtkGetMacro ( TransferType, int );
  vtkSetMacro ( TransferType, int );
  vtkGetMacro ( TransferID, int );
  vtkSetMacro ( TransferID, int );
  vtkGetMacro ( SizeOnDisk, int );
  vtkSetMacro ( SizeOnDisk, int );
  vtkGetStringMacro ( TransferNodeID);
  vtkSetStringMacro ( TransferNodeID);
  vtkGetMacro ( Progress, int );
  vtkSetMacro ( Progress, int );
  vtkGetMacro ( TransferStatus, int );
  vtkSetMacro ( TransferStatus, int );

  vtkGetMacro ( CancelRequested, int );
  vtkSetMacro ( CancelRequested, int );

  vtkGetMacro (TransferCached, int );
  vtkSetMacro (TransferCached, int );

  void SetTransferStatusNoModify ( int val)
      {
      this->TransferStatus = val;
      }

  const char* GetTransferStatusString( ) {
    switch (this->TransferStatus)
      {
      case vtkDataTransfer::Idle: return "Idle";
      case vtkDataTransfer::CancelPending: return "CancelPending";
      case vtkDataTransfer::Pending: return "Pending";
      case vtkDataTransfer::Running: return "Running";
      case vtkDataTransfer::Completed: return "Completed";
      case vtkDataTransfer::CompletedWithErrors: return "CompletedWithErrors";
      case vtkDataTransfer::TimedOut: return "TimedOut";
      case vtkDataTransfer::Cancelled: return "Cancelled";
      case vtkDataTransfer::Deleted: return "Deleted";
      case vtkDataTransfer::Ready: return "Ready";
      }
    return "Unknown";
  }

  const char* GetTransferTypeString( ) {
    switch (this->TransferType)
      {
      case vtkDataTransfer::RemoteDownload: return "RemoteDownload";
      case vtkDataTransfer::RemoteUpload: return "RemoteUpload";
      case vtkDataTransfer::LocalLoad: return "LocalUpload";
      case vtkDataTransfer::LocalSave: return "LocalSave";
      case vtkDataTransfer::Unspecified: return "Unspecified";
      }
    return "Unknown";
  }

  /// Transfer status values
  /// ready means ready to read into storage node
  enum
    {
      Idle=0,
      Pending,
      Running,
      Completed,
      CompletedWithErrors,
      CancelPending,
      Cancelled,
      Ready,
      Deleted,
      TimedOut
    };
  /// transfer type values
  enum
    {
      RemoteDownload=0,
      RemoteUpload,
      LocalLoad,
      LocalSave,
      Unspecified
    };

 private:

 protected:
  vtkDataTransfer();
  ~vtkDataTransfer() override;
  vtkDataTransfer(const vtkDataTransfer&);
  void operator=(const vtkDataTransfer&);

  ///
  char *SourceURI;
  char *DestinationURI;
  vtkURIHandler *Handler;
  int TransferStatus;
  int TransferID;
  int TransferType;
  int TransferCached;
  int SizeOnDisk;
  char* TransferNodeID;
  int Progress;
  int CancelRequested;

};

#endif

