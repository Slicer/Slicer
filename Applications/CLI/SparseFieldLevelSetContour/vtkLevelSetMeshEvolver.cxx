#include "vtkLevelSetMeshEvolver.h"

#include "vtkCellData.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "LSops.h"

vtkCxxRevisionMacro(vtkLevelSetMeshEvolver, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkLevelSetMeshEvolver);

vtkLevelSetMeshEvolver::vtkLevelSetMeshEvolver()
{

}

void vtkLevelSetMeshEvolver::SetSource(vtkPolyData *source)
{
  this->SetInput(1, source);
}

vtkPolyData *vtkLevelSetMeshEvolver::GetSource()
{
  if (this->GetNumberOfInputConnections(1) < 1)
    {
    return NULL;
    }
  return vtkPolyData::SafeDownCast(
    this->GetExecutive()->GetInputData(1, 0));
}

// Where the bulk of work is done!
int vtkLevelSetMeshEvolver::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
   // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  //vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  // update progress bar at some intervals
  this->UpdateProgress(0.15);

    // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  /*
  vtkPolyData *source = 0;
  if (sourceInfo)    {
    source = vtkPolyData::SafeDownCast(
      sourceInfo->Get(vtkDataObject::DATA_OBJECT
    ()));}*/
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
  
  { // copy everything through first, before updating...
    output->CopyStructure(input);
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());
    vtkDebugMacro(<<"Analyzing ____ ...");
  }

  

  // Perform LS-Sparse on Mesh 
  // Create New Scalar Array called "ActiveContourFunctionValues"
  // Update the vtkIntArray called "ActiveContourVertexIndices"
  {
    int EVOLVE_ITER = 300;
    SparseFieldLS* sfls = new SparseFieldLS( this->myMeshData,   L_z, L_p1,   L_n1,   L_p2, L_n2, map );
    std::vector<int> NewLZ    = sfls->Evolve( EVOLVE_ITER );
    std::vector<double>* vals = sfls->GetPhi( );
    // Update the array!
    vtkDataArray* contourIdxArrayIn = input->GetPointData()->GetArray("ActiveContourVertexIndices");
     vtkSmartPointer<vtkIntArray>  activeContourVertIdx = vtkSmartPointer<vtkIntArray>::New();
     activeContourVertIdx = vtkIntArray::SafeDownCast( contourIdxArrayIn );
     for( ::size_t i = 0; i < map.size(); i++ ) {
        int val = vtkMath::Round( float( (*vals)[i] ) );
       activeContourVertIdx->SetValue( i, val );
     }
     for( ::size_t i = 0; i < NewLZ.size(); i++ ) {
        int idx = NewLZ[i];
        activeContourVertIdx->SetValue( idx, 1 );
     }
     output->GetPointData()->AddArray( activeContourVertIdx );
  }

  // update progress bar at some intervals
  this->UpdateProgress(0.75);
  return 1;
 }
 
 // what does this do, and is it necessary?
int vtkLevelSetMeshEvolver::FillInputPortInformation(int port, vtkInformation *info)
{
  if (!this->Superclass::FillInputPortInformation(port, info)) {
    return 0;
    }
  if (port == 1) {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}


void vtkLevelSetMeshEvolver::PrintSelf(ostream& os, vtkIndent indent)
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
