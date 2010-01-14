#include "ModuleEntry.h"

// Input: mesh and indices of vertices for initialization
vtkPolyData* MeshContourEvolver::entry_main( vtkPolyData* inputMesh, const vector<int>& initPointVertexIndices )
{
  // instantiate output mesh
  vtkPolyData* outputMesh = vtkPolyData::New();

  // algorithm #1: initialization of path
  vtkSmartPointer<vtkInitClosedPath> initPath = vtkSmartPointer<vtkInitClosedPath>::New();
  initPath->SetInput( inputMesh );
    
  // algorithm #2: computing the geometry (if the input data 
  // does NOT already have a scalar data set containing it)
  vtkSmartPointer<vtkComputeLocalGeometry> computeGeometry = vtkSmartPointer<vtkComputeLocalGeometry>::New();
  computeGeometry->SetInputConnection( initPath->GetOutputPort() );
    
  // algorithm #3: run curve evolution and update the scalar dataset for display
  vtkSmartPointer<vtkLevelSetMeshEvolver> evolver = vtkSmartPointer<vtkLevelSetMeshEvolver>::New();
  evolver->SetInputConnection( computeGeometry->GetOutputPort() );
    evolver->Update();

  // register and return the result of the pipeline of algorithms
  vtkSmartPointer<vtkPolyData> result = evolver->GetOutput();
  outputMesh->DeepCopy( result );

  return outputMesh;
}

vtkPolyData* MeshContourEvolver::entry_main( vtkPolyData* inputMesh, const vector< vector<float> >& initPoints3D )
{
  vtkSmartPointer<vtkPoints>    verts = inputMesh->GetPoints();
  double thispt[3];
  vector<int> initialPoints;
  unsigned int iNumMeshVerts  = verts->GetNumberOfPoints();
  unsigned int iNumSeedPoints = initPoints3D.size();  std::cout<<"appending "<<iNumSeedPoints<<" points to list. \n";
  for( unsigned int k = 0; k < iNumSeedPoints; k++ ) {
    float xcur = (initPoints3D[k])[0];
    float ycur = (initPoints3D[k])[1];
    float zcur = (initPoints3D[k])[2];
    float fMinDist = 1e20;
    unsigned int iMinIdx = 0;
    for( unsigned int i = 0; i < iNumMeshVerts; i++ ) {
      verts->GetPoint( i, thispt );
      float dist = sqrt( pow(xcur - thispt[0],2) + pow(ycur - thispt[1],2) + pow(zcur - thispt[2],2) );
      if( dist < fMinDist ) {
      fMinDist = dist;
      iMinIdx = i;
      }
    }
    std::cout<<" "<<iMinIdx<<" ...";
    initialPoints.push_back(iMinIdx);
  }
  std::cout<<"\n";
  vtkPolyData* outputMesh = entry_main( inputMesh, initialPoints );

  return outputMesh;
}


// Input: mesh only. No initialization of points; either continue
// evolution of existing curve or only pre-compute geometry!
vtkPolyData* MeshContourEvolver::entry_main( vtkPolyData* inputMesh )
{  
  vector<int> emptyIntVec(0);
  vtkPolyData* outputMesh;// = entry_main( inputMesh, emptyIntVec );

  return outputMesh;
}
