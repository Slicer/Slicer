#include "vtkComputeLocalGeometry.h"
#include "MeshOps.h"

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
#include "vtkSmartPointer.h"

#include <valarray>
#include <vector>
#include <list>

// DEBUG_DISPLAY_1:
void Debug_Display( vtkIntArray* activeContourVertIdx, 
                          const std::vector<int>& Lstar) 
{
      int numVerts = Lstar.size();
      for( int i = 0; i < numVerts; i++ ) {
        int val = ( Lstar[i] );
        activeContourVertIdx->SetTupleValue(i, &val);
      }
}

vtkCxxRevisionMacro(vtkComputeLocalGeometry, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkComputeLocalGeometry);

vtkComputeLocalGeometry::vtkComputeLocalGeometry() {
 // this->SetNumberOfInputPorts(2);
 // this->SetNumberOfOutputPorts(1);
}

void vtkComputeLocalGeometry::SetSource(vtkPolyData *source) {
  this->SetInput(1, source);
}

vtkPolyData *vtkComputeLocalGeometry::GetSource() {
  if (this->GetNumberOfInputConnections(1) < 1) {
    return NULL;
  }
  return vtkPolyData::SafeDownCast(
    this->GetExecutive()->GetInputData(1, 0));
}

// Where the bulk of work is done!
int vtkComputeLocalGeometry::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector) {
   
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
 // vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  // update progress bar at some intervals
  this->UpdateProgress(0.15);

    // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *source = 0;
 /* if (sourceInfo)
    {
    source = vtkPolyData::SafeDownCast(
      sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
    }*/
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Check input
  //
  size_t numVerts=input->GetNumberOfPoints();
  size_t numCells=input->GetNumberOfCells();
  if (numVerts < 1 || numCells < 1)
  {
    vtkErrorMacro(<<"No data to smooth!");
    return 1;
  }
  output->CopyStructure(input);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());
  vtkDebugMacro(<<"Analyzing ____ ...");
   


// assign some data from curvature computation 
// check for the prescence of this named array!
  vtkDataArray* surfaceCurvature = input->GetPointData()->GetArray("SurfaceMeanCurvature");
  if( NULL == surfaceCurvature ) {
    vtkSmartPointer<vtkFloatArray> surfaceCurvature = vtkSmartPointer<vtkFloatArray>::New();
    surfaceCurvature->SetName("SurfaceMeanCurvature");
    surfaceCurvature->SetNumberOfComponents(1);
    surfaceCurvature->SetNumberOfTuples( numVerts );
    std::valarray<double> surfH(numVerts);
    
    this->myMeshData = new MeshData;
    { // compute the mean curvature using K-neighborhood paraboloid-fitting

      // do the curvature computations / pre-processing
      myMeshData->polydata = output;
      myMeshData->smoothH_its = 10;
      myMeshData->adj_levels = 1;
      myMeshData->showLS = 0;
      myMeshData->rightHandMesh = 1;
      ComputeCurvatureData( myMeshData );
      surfH = myMeshData->MeanCurv;
    }

    double Hmin = abs(surfH).min();
    double Hmax = abs(surfH).max();
    double scale = 256 / (Hmax);
    { // perform the assignment to scalar array attached to vtkPolyData
      for( ::size_t i = 0; i < numVerts; i++ ) {
        surfH[i] = scale * surfH[i];
        surfaceCurvature->SetTuple1(i,surfH[i]);
      }
      output->GetPointData()->AddArray(surfaceCurvature);
      //output->GetPointData()->SetScalars( surfaceCurvature );
      //output->GetPointData()->SetActiveScalars( surfaceCurvature->GetName() );
    }

    
  ///////////////////////////////////////////////////////////////////
    // Update the active contour indices using the mesh neigbhor information!
     vtkDataArray* contourIdxArrayIn = input->GetPointData()->GetArray("ActiveContourVertexIndices");
     vtkSmartPointer<vtkIntArray>  activeContourVertIdx = vtkSmartPointer<vtkIntArray>::New();
     activeContourVertIdx = vtkIntArray::SafeDownCast( contourIdxArrayIn );
    std::vector<int> point_type = std::vector<int>(numVerts);
    std::list<int> interior_init = std::list<int>(0);
    std::list<int> exterior_init = std::list<int>(0);
    std::vector<int> visited = std::vector<int>(numVerts);
    for( int i = 0; i < numVerts; i++ ) {
      int val = activeContourVertIdx->GetValue( i );
      if( 1 == val ) {
         point_type[i] = 2;
         interior_init.push_back(i);
      }
      else {
         point_type[i] = -2;
         exterior_init.push_back(i);
      }
      visited[i] = 0;
    }

   
    int vprime_idx = exterior_init.front();
    std::list<int> visited_idx = std::list<int>(0);
    visited_idx.push_back( vprime_idx );



    int Ck = 1;     // first 'exterior class' of blobs
    vector<int> Ak; // area of each Ck blob
    vector<int> class_of_blob(numVerts);

    //visited[vprime_idx] = 1;
    //visited.push_back( vprime_idx );
    int num_ones_prev = 0;
    bool bFoundUnclassified = true;
    while( bFoundUnclassified ) {
        int j = 0;
        bool bFoundUnvisited = true;
        while( visited_idx.size() > 0) { 
        ::size_t len_vis = visited.size();
            // look at all neigbhors of all 
            // indices in the visited list
            
            int idx = visited_idx.front();
            std::vector<int> *neigh = &(myMeshData->adjimm[idx].myNeighbs);
            for( ::size_t k = 0; k < neigh->size(); k++ )  {
              int nid = (*neigh)[k];
              if( ( point_type[nid] == -2) && visited[nid] == 0 ) {
                visited[nid] = 1;
                visited_idx.push_back( nid );
                class_of_blob[nid] = Ck;
              }
            }
            visited_idx.pop_front();
            j++;
        }
        // everything part of that first 'blob' is in visited...
        int num_ones = count( visited.begin(), visited.end(), 1 );
        int numel = visited.size();
        int num_ones_delta = num_ones - num_ones_prev;
        Ak.push_back(num_ones_delta); // area of this blob....
        num_ones_prev = num_ones;
        
        // these Ones are either the entire exterior or some chunk of it.
        // go through vertices and see who is in neither this new list nor the interior
        // ie go thru 'exterior' list
        Ck++;
        bFoundUnclassified = false;
        for( int k = 0; k < exterior_init.size(); k++ ) {
          int idx = exterior_init.front();
          exterior_init.pop_front();
          exterior_init.push_back(idx);
          if( visited[idx] == 0 ) { // didn't hit this one during previous blob
            visited_idx.push_back( idx );
            bFoundUnclassified = true;
            break;
          }
          
        }
    }
  // Now: take everything smaller than some percent of the largest Ak and move it to interior
    int CkMax = 0;
    int AkMax = 0;
    for( int k =0; k < Ak.size(); k++ ) {
      if( Ak[k] > AkMax ) {
        AkMax = Ak[k];
        CkMax = k;
      }
    }
    for( int k = 0; k < numVerts; k++ ) {
        int Ck = class_of_blob[k]; // Ck=0 if this is interior point
        if( Ck > 0 ) {
          if( Ak[Ck-1] < AkMax/4 ) {
            interior_init.push_back( k );
            class_of_blob[k] = 0; // Interior!
          }
          else {
            class_of_blob[k] = 1; // Exterior!
          }
        }
    }
    
    
    // now compute the different layers.
    // L_z can contribute one or no plus/minus pts
    // this is the closest neighbor not having yet been assigned.
    // 1. compute first layers

   // L_z: the array of vertex indices that have both neighbor INSIDE and OUTSIDE
      std::vector<int> edgeVisited = std::vector<int>(numVerts);
#define INIT_VAL_EDGE -3
      for( int i = 0; i < numVerts; i++ ) {
          edgeVisited[i] = INIT_VAL_EDGE;
      }
      std::list<int> L_z = std::list<int>(0);
      for(int i=0; i<numVerts; i++){
        vector<int> neigh = myMeshData->adjimm[i].myNeighbs;
        if (class_of_blob[i] == 0) {
          for (int n=0; n<neigh.size(); n++){
            if (class_of_blob[neigh[n]] == 1 ){
              if( 0 == count(L_z.begin(),L_z.end(), i ) ) {
                L_z.push_back(i);
                this->Lz.push_back( i );
                edgeVisited[i] = 0;
              }
              //if (0 == count(L_z.begin(), L_z.end(), neigh[n])){
                //L_z.push_back(neigh[n]);
                //this->Lz.push_back( neigh[n] );
                //edgeVisited[neigh[n]] = 0;
              //}
            }
          }
        }
      }

    

      //std::vector<int> edgeVisited = std::vector<int>(numVerts);
      std::list<int> L_p1 = std::list<int>(0);
      std::list<int> L_n1 = std::list<int>(0);
      for(int i=0; i<numVerts; i++){
        vector<int> neigh = myMeshData->adjimm[i].myNeighbs;
        if (edgeVisited[i] == 0) {  // On L_z
          for (int n=0; n<neigh.size(); n++){
            if (edgeVisited[neigh[n]] == INIT_VAL_EDGE && class_of_blob[neigh[n]] == 0 ){
              if (0 == count(L_p1.begin(), L_p1.end(), neigh[n])){
                L_p1.push_back(neigh[n]);
                this->Lp1.push_back( neigh[n] );
                edgeVisited[neigh[n]] = 1;
              }
            }
            else if (edgeVisited[neigh[n]] == INIT_VAL_EDGE && class_of_blob[neigh[n]] == 1 ){
              if (0 == count(L_n1.begin(), L_n1.end(), neigh[n])){
                L_n1.push_back(neigh[n]);
                this->Ln1.push_back( neigh[n] );
                edgeVisited[neigh[n]] = -1;
              }
            }
          }
        }
      }
      std::list<int> L_p2 = std::list<int>(0);
      std::list<int> L_n2 = std::list<int>(0);
      for(int i=0; i<numVerts; i++){
        vector<int> neigh = myMeshData->adjimm[i].myNeighbs;
        if (edgeVisited[i] == 1) {  // On L_p1
          for (int n=0; n<neigh.size(); n++){
            if (edgeVisited[neigh[n]] == INIT_VAL_EDGE && class_of_blob[neigh[n]] == 0 ){
              if (0 == count(L_p2.begin(), L_p2.end(), neigh[n])){
                L_p2.push_back(neigh[n]);
                this->Lp2.push_back( neigh[n] );
                edgeVisited[neigh[n]] = 2;
              }
            }
          }
        }
        else if(edgeVisited[i] == -1) {  // On L_n1
          for (int n=0; n<neigh.size(); n++){
            if (edgeVisited[neigh[n]] == INIT_VAL_EDGE && class_of_blob[neigh[n]] == 1 ){
              if (0 == count(L_n2.begin(), L_n2.end(), neigh[n])){
                L_n2.push_back(neigh[n]);
                this->Ln2.push_back( neigh[n] );
                edgeVisited[neigh[n]] = -2;
              }
            }
          }
        }
      }
  
  Debug_Display( activeContourVertIdx, edgeVisited) ;
  output->GetPointData()->AddArray( activeContourVertIdx );
  this->map = edgeVisited;
  // ok now all points are either "interior" or "exterior".

  }
  else {
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
