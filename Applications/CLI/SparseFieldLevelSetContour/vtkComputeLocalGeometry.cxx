#include "vtkComputeLocalGeometry.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSmartPointer.h"

// DEBUG_DISPLAY_1:
void Debug_Display( vtkIntArray* activeContourVertIdx,
                    const std::vector<int>& Lstar)
{
  int numVerts = Lstar.size();

  for( int i = 0; i < numVerts; i++ )
    {
    int val = ( Lstar[i] );
    activeContourVertIdx->SetTupleValue(i, &val);
    }
}

vtkCxxRevisionMacro(vtkComputeLocalGeometry, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkComputeLocalGeometry);

vtkComputeLocalGeometry::vtkComputeLocalGeometry()
{
  // this->SetNumberOfInputPorts(2);
  // this->SetNumberOfOutputPorts(1);
}

void vtkComputeLocalGeometry::SetSource(vtkPolyData *source)
{
  this->SetInput(1, source);
}

vtkPolyData * vtkComputeLocalGeometry::GetSource()
{
  if( this->GetNumberOfInputConnections(1) < 1 )
    {
    return NULL;
    }
  return vtkPolyData::SafeDownCast(
           this->GetExecutive()->GetInputData(1, 0) );
}

// Where the bulk of work is done!
int vtkComputeLocalGeometry::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector * *inputVector,
  vtkInformationVector *outputVector)
{

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  // vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // update progress bar at some intervals
  this->UpdateProgress(0.15);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT() ) );
//  vtkPolyData *source = 0;
/* if (sourceInfo)
   {
   source = vtkPolyData::SafeDownCast(
     sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
   }*/
  vtkPolyData *output = vtkPolyData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

  // Check input
  //
  size_t numVerts = input->GetNumberOfPoints();
  size_t numCells = input->GetNumberOfCells();
  if( numVerts < 1 || numCells < 1 )
    {
    vtkErrorMacro(<< "No data to smooth!");
    return 1;
    }
  output->CopyStructure(input);
  output->GetPointData()->PassData(input->GetPointData() );
  output->GetCellData()->PassData(input->GetCellData() );
  vtkDebugMacro(<< "Analyzing ____ ...");

// assign some data from curvature computation
// check for the prescence of this named array!
//  vtkDataArray* surfaceCurvature = input->GetPointData()->GetArray("SurfaceMeanCurvature");
  if( 1 /*NULL == surfaceCurvature*/ )
    {
    vtkSmartPointer<vtkFloatArray> surfaceCurvature = vtkSmartPointer<vtkFloatArray>::New();
    surfaceCurvature->SetName("SurfaceMeanCurvature");
    surfaceCurvature->SetNumberOfComponents(1);
    surfaceCurvature->SetNumberOfTuples( numVerts );
    std::valarray<double> surfH(numVerts);

    this->myMeshData = new MeshData;
      { // compute the mean curvature using K-neighborhood paraboloid-fitting

      // do the curvature computations / pre-processing
      myMeshData->polydata = output;
      myMeshData->smoothH_its = H_smooth_its;
      myMeshData->adj_levels = adj_levels;
      myMeshData->showLS = 0;
      myMeshData->rightHandMesh = rightHandMesh;
      ComputeCurvatureData( myMeshData );
      surfH = myMeshData->MeanCurv;
      }

//    double Hmin = abs(surfH).min();
    double Hmax = abs(surfH).max();
    double scale = 256 / (Hmax);
      { // perform the assignment to scalar array attached to vtkPolyData
      for( ::size_t i = 0; i < numVerts; i++ )
        {
        surfH[i] = scale * surfH[i];
        surfaceCurvature->SetTuple1(i, surfH[i]);
        }
      output->GetPointData()->AddArray(surfaceCurvature);
      // output->GetPointData()->SetScalars( surfaceCurvature );
      // output->GetPointData()->SetActiveScalars( surfaceCurvature->GetName() );
      }

    // /////////////////////////////////////////////////////////////////
    // Update the active contour indices using the mesh neigbhor information!
    vtkDataArray*                contourIdxArrayIn = input->GetPointData()->GetArray("ActiveContourVertexIndices");
    vtkSmartPointer<vtkIntArray> activeContourVertIdx = vtkSmartPointer<vtkIntArray>::New();
    activeContourVertIdx = vtkIntArray::SafeDownCast( contourIdxArrayIn );
    std::vector<int> point_type = std::vector<int>(numVerts);
    std::list<int>   interior_init = std::list<int>(0);
    std::list<int>   exterior_init = std::list<int>(0);
    std::vector<int> visited = std::vector<int>(numVerts);
    for( unsigned int i = 0; i < numVerts; i++ )
      {
      int val = activeContourVertIdx->GetValue( i );
      if( 1 == val )
        {
        point_type[i] = 2;
        interior_init.push_back(i);
        }
      else
        {
        point_type[i] = -2;
        exterior_init.push_back(i);
        }
      visited[i] = 0;
      }

    int            vprime_idx = exterior_init.front();
    std::list<int> visited_idx = std::list<int>(0);
    visited_idx.push_back( vprime_idx );

    int              Ck = 1; // first 'exterior class' of blobs
    std::vector<int> Ak;     // area of each Ck blob
    std::vector<int> class_of_blob(numVerts);

    // visited[vprime_idx] = 1;
    // visited.push_back( vprime_idx );
    int  num_ones_prev = 0;
    bool bFoundUnclassified = true;
    while( bFoundUnclassified )
      {
      int j = 0;
//        bool bFoundUnvisited = true;
      while( visited_idx.size() > 0 )
        {
//        ::size_t len_vis = visited.size();
        // look at all neigbhors of all
        // indices in the visited list

        int               idx = visited_idx.front();
        std::vector<int> *neigh = &(myMeshData->adjimm[idx].myNeighbs);
        for( ::size_t k = 0; k < neigh->size(); k++ )
          {
          int nid = (*neigh)[k];
          if( ( point_type[nid] == -2) && visited[nid] == 0 )
            {
            visited[nid] = 1;
            visited_idx.push_back( nid );
            class_of_blob[nid] = Ck;
            }
          }
        visited_idx.pop_front();
        j++;
        }

      // everything part of that first 'blob' is in visited...
      int num_ones = std::count( visited.begin(), visited.end(), 1 );
//        int numel = visited.size();
      int num_ones_delta = num_ones - num_ones_prev;
      Ak.push_back(num_ones_delta);   // area of this blob....
      num_ones_prev = num_ones;

      // these Ones are either the entire exterior or some chunk of it.
      // go through vertices and see who is in neither this new list nor the interior
      // ie go thru 'exterior' list
      Ck++;
      bFoundUnclassified = false;
      for( unsigned int k = 0; k < exterior_init.size(); k++ )
        {
        int idx = exterior_init.front();
        exterior_init.pop_front();
        exterior_init.push_back(idx);
        if( visited[idx] == 0 )     // didn't hit this one during previous blob
          {
          visited_idx.push_back( idx );
          bFoundUnclassified = true;
          break;
          }

        }
      }

    // Now: take everything smaller than some percent of the largest Ak and move it to interior
    int CkMax = 0;
    int AkMax = 0;
    for( unsigned int k = 0; k < Ak.size(); k++ )
      {
      if( Ak[k] > AkMax )
        {
        AkMax = Ak[k];
        CkMax = k;
        }
      }
    for( unsigned int k = 0; k < numVerts; k++ )
      {
      int Ck = class_of_blob[k];   // Ck=0 if this is interior point
      if( Ck > 0 )
        {
        if( Ak[Ck - 1] < AkMax / 4 )
          {
          interior_init.push_back( k );
          class_of_blob[k] = 0;   // Interior!
          }
        else
          {
          class_of_blob[k] = 1;   // Exterior!
          }
        }
      }

    std::vector<int> label = std::vector<int>(numVerts);
    for( ::size_t i = 0; i < numVerts; i++ )
      {
      if( class_of_blob[i] == 0 )
        {
        label[i] = 3;
        }
      else
        {
        label[i] = -3;
        }
      }

    std::list<int> Lz = std::list<int>(0);
    for( ::size_t i = 0; i < numVerts; i++ )
      {
      for( ::size_t k = 0; k < myMeshData->adjimm[i].myNeighbs.size(); k++ )
        {
        if( label[i] == 3 &&
            0 > label[myMeshData->adjimm[i].myNeighbs[k]] )
          {
          Lz.push_back( i );
          label[i] = 0;
          }
        }
      }

    std::list<int> Lp1 = std::list<int>(0);
    std::list<int> Ln1 = std::list<int>(0);
    for( ::size_t idx = 0; idx < Lz.size(); idx++ )
      {
      ::size_t i = Lz.front();
      Lz.pop_front();
      for( ::size_t k = 0; k < myMeshData->adjimm[i].myNeighbs.size(); k++ )
        {
        int idx = myMeshData->adjimm[i].myNeighbs[k];
        if( -3 == label[idx] )
          {
          Ln1.push_back( idx );
          label[idx] = -1;
          }
        else if( 3 == label[idx] )
          {
          Lp1.push_back( idx );
          label[idx] = 1;
          }
        }
      Lz.push_back(i);
      }

    std::list<int> Ln2 = std::list<int>(0);
    for( ::size_t idx = 0; idx < Ln1.size(); idx++ )
      {
      ::size_t i = Ln1.front();
      Ln1.pop_front();
      for( ::size_t k = 0; k < myMeshData->adjimm[i].myNeighbs.size(); k++ )
        {
        int idx = myMeshData->adjimm[i].myNeighbs[k];
        if( -3 == label[idx] )
          {
          Ln2.push_back( idx );
          label[idx] = -2;
          }
        }
      Ln1.push_back(i);
      }

    std::list<int> Lp2 = std::list<int>(0);
    for( ::size_t idx = 0; idx < Lp1.size(); idx++ )
      {
      ::size_t i = Lp1.front();
      Lp1.pop_front();
      for( ::size_t k = 0; k < myMeshData->adjimm[i].myNeighbs.size(); k++ )
        {
        int idx = myMeshData->adjimm[i].myNeighbs[k];
        if( 3 == label[idx] )
          {
          Lp2.push_back( idx );
          label[idx] = 2;
          }
        }
      Lp1.push_back(i);
      }

    // L_z: the array of vertex indices that have both neighbor INSIDE and OUTSIDE

    Debug_Display( activeContourVertIdx,  class_of_blob);
    output->GetPointData()->AddArray( activeContourVertIdx );
    this->map = label;
    this->Lp1 = Lp1;
    this->Ln1 = Ln1;
    this->Lp2 = Lp2;
    this->Ln2 = Ln2;
    this->Lz  = Lz;
    // ok now all points are either "interior" or "exterior".

    }
  else
    {
    // Do nothing: the surface geometry is already computed for this poly data.
    // copy everything through first, before updating...

    }

  // update progress bar at some intervals
  this->UpdateProgress(1.0);
  return 1;
}

// what does this do, and is it necessary?
int vtkComputeLocalGeometry::FillInputPortInformation(int port,
                                                      vtkInformation *info)
{
  if( !this->Superclass::FillInputPortInformation(port, info) )
    {
    return 0;
    }

  if( port == 1 )
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

void vtkComputeLocalGeometry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Truth: " << 42 << "\n";
  if( this->GetSource() )
    {
    os << indent << "Source: " << static_cast<void *>(this->GetSource() ) << "\n";
    }
  else
    {
    os << indent << "Source (none)\n";
    }
}
