#include "vtkComputeLocalGeometry.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTriangleFilter.h"

vtkCxxRevisionMacro(vtkComputeLocalGeometry, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkComputeLocalGeometry);

vtkComputeLocalGeometry::vtkComputeLocalGeometry()
{
  // assign values
  // this->MemberVar = value

  // optional second input
  this->SetNumberOfInputPorts(2);
}

void vtkComputeLocalGeometry::SetSource(vtkPolyData *source)
{
  this->SetInput(1, source);
}

vtkPolyData *vtkComputeLocalGeometry::GetSource()
{
  if (this->GetNumberOfInputConnections(1) < 1)
    {
    return NULL;
    }
  return vtkPolyData::SafeDownCast(
    this->GetExecutive()->GetInputData(1, 0));
}

// Where the bulk of work is done!
int vtkComputeLocalGeometry::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
   // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  // update progress bar at some intervals
  this->UpdateProgress(0.15);

    // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *source = 0;
  if (sourceInfo)
    {
    source = vtkPolyData::SafeDownCast(
      sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
    }
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  
  // Check input
  //
  int numPts=input->GetNumberOfPoints();
  int numCells=input->GetNumberOfCells();
  if (numPts < 1 || numCells < 1)
    {
    vtkErrorMacro(<<"No data to smooth!");
    return 1;
    }
  
  bool bDoNothing = true;
    if ( bDoNothing ) 
    { //don't do anything! pass data through
    output->CopyStructure(input);
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());
    return 1;
    }
  vtkDebugMacro(<<"Analyzing ____ ...");
  
// assign some data from curvature computation 
// in the future check for the prescence of this named array!
  //vtkFloatArray *scalars2 = vtkFloatArray::New();
  //scalars2->SetName("SmoothedCurvature");
  //for( ::size_t i = 0; i < meshdata->MeanCurv.size(); i++ )
  //  {
  //  scalars2->InsertTuple1(i, meshdata->MeanCurv[i] );
  //  }
  //smooth_brain->GetPointData()->AddArray(scalars2);
  //scalars2->Delete();



  // update progress bar at some intervals
  this->UpdateProgress(0.75);
  
  vtkPoints* newPts = vtkPoints::New();
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetVerts(input->GetVerts());
  output->SetLines(input->GetLines());
  output->SetPolys(input->GetPolys());
  output->SetStrips(input->GetStrips());
 
  return 1;
 }
 
 // what does this do, and is it necessary?
int vtkComputeLocalGeometry::FillInputPortInformation(int port,
                                                      vtkInformation *info)
{
  if (!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  
  if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}


void vtkComputeLocalGeometry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Truth: " << 42 << "\n";
  if ( this->GetSource() )
    {
      os << indent << "Source: " << static_cast<void *>(this->GetSource()) << "\n";
    }
  else
    {
    os << indent << "Source (none)\n";
    }
}

