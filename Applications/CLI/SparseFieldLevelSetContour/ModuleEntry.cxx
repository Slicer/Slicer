#include "ModuleEntry.h"

// Input: mesh and indices of vertices for initialization
vtkPolyData* MeshContourEvolver::entry_main( vtkPolyData* inputMesh, 
                                            vtkIntArray* initVertIdx )
{
  // instantiate output mesh
  vtkPolyData* outputMesh = vtkPolyData::New();

  // put through triangle filter: ensure that we have a "polys" field
  vtkSmartPointer<vtkTriangleFilter> triangle_filter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInput(inputMesh);
  triangle_filter->Update();


  // algorithm #1: initialization of path
  vtkSmartPointer<vtkInitClosedPath> initPath = vtkSmartPointer<vtkInitClosedPath>::New();
  initPath->SetInitPointVertexIdx( initVertIdx );
  initPath->SetInput( triangle_filter->GetOutput() );
  initPath->Update();
  
  // algorithm #2: computing the geometry (if the input data 
  // does NOT already have a scalar data set containing it)
  vtkSmartPointer<vtkComputeLocalGeometry> computeGeometry = vtkSmartPointer<vtkComputeLocalGeometry>::New();
  computeGeometry->SetInputConnection( initPath->GetOutputPort() );
    
  // algorithm #3: run curve evolution and update the scalar dataset for display
  vtkSmartPointer<vtkLevelSetMeshEvolver> evolver = vtkSmartPointer<vtkLevelSetMeshEvolver>::New();
  evolver->SetInputConnection( computeGeometry->GetOutputPort() );
 // evolver->SetInputConnection( 1, initPath->GetOutputPort(1) );

  // Set Initial Active Contour before main algorithm
  evolver->Update();

  // register and return the result of the pipeline of algorithms
  vtkSmartPointer<vtkPolyData> result = evolver->GetOutput();
  outputMesh->DeepCopy( result );

  
  return outputMesh;
}

vtkPolyData* MeshContourEvolver::entry_main( vtkPolyData* inputMesh, 
                                            const vector< vector<float> >& initPoints3D )
{ 
  vtkSmartPointer<vtkPoints>    verts = inputMesh->GetPoints();
  double thispt[3];
  vtkSmartPointer<vtkIntArray> initialPoints = vtkSmartPointer<vtkIntArray>::New();
  unsigned int iNumMeshVerts  = verts->GetNumberOfPoints();
  unsigned int iNumSeedPoints = initPoints3D.size();
  std::cout<<"appending "<<iNumSeedPoints<<" points to list. \n";

  /* Find closest vertex to each input 3D point */
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
    //initialPoints->
    //initialPoints.push_back(iMinIdx);
  }
  std::cout<<"\n";
  vtkPolyData* outputMesh = entry_main( inputMesh, initialPoints );

  return outputMesh;
}


// Input: mesh only. No initialization of points; either continue
// evolution of existing curve or only pre-compute geometry!
vtkPolyData* MeshContourEvolver::entry_main( vtkPolyData* inputMesh )
{  
  vtkIntArray* emptyIntVec;
  vtkPolyData* outputMesh = entry_main( inputMesh, emptyIntVec );

  return outputMesh;
}
