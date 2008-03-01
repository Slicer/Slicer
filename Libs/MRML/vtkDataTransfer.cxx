
#include "vtkDataTransfer.h"
#include "vtkCallbackCommand.h"


vtkStandardNewMacro ( vtkDataTransfer );
vtkCxxRevisionMacro ( vtkDataTransfer, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkDataTransfer::vtkDataTransfer()
{
  this->SourceURI = NULL;
  this->DestinationURI = NULL;
//  this->Handler = NULL;
  this->TransferStatus = vtkDataTransfer::Initialized;
  this->TransferID = -1;
  this->TransferType = vtkDataTransfer::Unspecified;


}


//----------------------------------------------------------------------------
vtkDataTransfer::~vtkDataTransfer()
{
   
  this->SourceURI = NULL;
  this->DestinationURI = NULL;
//  this->Handler = NULL;
  this->TransferStatus = vtkDataTransfer::Initialized;
  this->TransferID = -1;
  this->TransferType = vtkDataTransfer::Unspecified;

}


//----------------------------------------------------------------------------
void vtkDataTransfer::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
  os << indent << "SourceURI: " <<
    ( this->SourceURI ? this->SourceURI : "(none)") << "\n";
  os << indent << "DestinationURI: " <<
    ( this->DestinationURI ? this->DestinationURI : "(none)") << "\n";
  os << indent << "TransferStatus: " << this->GetTransferStatus() << "\n";
  os << indent << "TransferID: " << this->GetTransferID() << "\n";
  os << indent << "TransferType: " << this->GetTransferType() << "\n";

}
