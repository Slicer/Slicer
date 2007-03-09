
#include "vtkIGTDataStream.h"
#include "vtkIGTMatrixState.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro (vtkIGTDataStream);
vtkCxxRevisionMacro (vtkIGTDataStream, "$Revision: 1.0 $");


vtkIGTDataStream::vtkIGTDataStream()
{
    this->MatrixState = NULL;

}



vtkIGTDataStream::~vtkIGTDataStream ( ) 
{ 
    this->MatrixState = NULL;
}


void vtkIGTDataStream::PrintSelf(ostream& os, vtkIndent indent)
{


}
//  
// Methods
//  
