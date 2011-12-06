#include "ModuleEntry.h"

// Input: mesh and indices of vertices for initialization
void MeshContourEvolver::entry_main( vtkPolyData* inputMesh,
                                     vtkIntArray* initVertIdx,
                                     vtkPolyData* outputMesh,
                                     InitParam init,
                                     bool bForceRecompute)
{

  // instantiate output mesh
  // vtkPolyData * outputMesh = vtkPolyData::New();
  if( outputMesh == NULL )
    {
    return;
    }

  // put through triangle filter: ensure that we have a "polys" field
  vtkSmartPointer<vtkTriangleFilter> triangle_filter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInput(inputMesh);
  triangle_filter->Update();

  // algorithm #1: initialization of path
  vtkSmartPointer<vtkInitClosedPath> initPath = vtkSmartPointer<vtkInitClosedPath>::New();
  initPath->SetInitPointVertexIdx( initVertIdx );
  initPath->SetForceRecompute( bForceRecompute );
  initPath->SetInput( triangle_filter->GetOutput() );
  initPath->Update();

  // algorithm #2: computing the geometry (if the input data
  // does NOT already have a scalar data set containing it)
  vtkSmartPointer<vtkComputeLocalGeometry> computeGeometry = vtkSmartPointer<vtkComputeLocalGeometry>::New();

  // Pass in Initialization Parameters
  computeGeometry->Set_evolve_its(init.evolve_its);
  computeGeometry->Set_mesh_smooth_its(init.mesh_smooth_its);
  computeGeometry->Set_H_smooth_its(init.H_smooth_its);
  computeGeometry->Set_adj_levels(init.adj_levels);
  computeGeometry->Set_rightHandMesh(init.rightHandMesh);

  computeGeometry->SetInputConnection( initPath->GetOutputPort() );
  computeGeometry->Update();

  // algorithm #3: run curve evolution and update the scalar dataset for display
  vtkSmartPointer<vtkLevelSetMeshEvolver> evolver = vtkSmartPointer<vtkLevelSetMeshEvolver>::New();
  MeshData*                               data = computeGeometry->GetMeshData();
  evolver->SetMeshData( data );
  std::list<int>   L_z;  std::list<int> L_n1;  std::list<int> L_p1;  std::list<int> L_n2;  std::list<int> L_p2;
  std::vector<int> map;
  computeGeometry->GetLists( L_z, L_p1, L_n1, L_p2, L_n2, map );
  evolver->SetLists( L_z, L_p1, L_n1, L_p2, L_n2, map );

  evolver->SetInputConnection( computeGeometry->GetOutputPort() );
  // evolver->SetInputConnection( 1, initPath->GetOutputPort(1) );

  // Set Initial Active Contour before main algorithm
  evolver->Update();

  // register and return the result of the pipeline of algorithms
  vtkSmartPointer<vtkPolyData> result = evolver->GetOutput();
  outputMesh->DeepCopy( result );

  return; //  outputMesh;
}

void MeshContourEvolver::entry_main( vtkPolyData* inputMesh,
                                     std::vector<std::vector<float> >& initPoints3D,
                                     vtkPolyData* outputMesh,
                                     InitParam init,
                                     bool bForceRecompute )
{

  vtkSmartPointer<vtkTriangleFilter> triangle_filter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInput(inputMesh);
  triangle_filter->Update();
  vtkSmartPointer<vtkPolyData> tri_result = triangle_filter->GetOutput();
  inputMesh->DeepCopy(tri_result);

  vtkSmartPointer<vtkPoints>   verts = inputMesh->GetPoints();
  double                       thispt[3];
  vtkSmartPointer<vtkIntArray> initialPoints = vtkSmartPointer<vtkIntArray>::New();
  unsigned int                 iNumMeshVerts  = verts->GetNumberOfPoints();
  unsigned int                 iNumSeedPoints = initPoints3D.size();
  std::cout << "appending " << iNumSeedPoints << " points to list. \n";

  if( iNumSeedPoints < 2 )
    {
    std::cout << "Error, no fiducial points were read...check your input format\n";
    std::vector<float> X(3); X[0] = 20; X[1] = 0; X[2] = 0;
    std::vector<float> Y(3); Y[0] = 0; X[1] = 20; Y[2] = 0;
    std::vector<float> Z(3); Z[0] = 0; Z[1] = 0; Z[2] = 20;
    initPoints3D.push_back( X );
    initPoints3D.push_back( Y );
    initPoints3D.push_back( Z );
    }
  /* Find closest vertex to each input 3D point */
  for( unsigned int k = 0; k < initPoints3D.size(); k++ )
    {
    float        xcur = (initPoints3D[k])[0];
    float        ycur = (initPoints3D[k])[1];
    float        zcur = (initPoints3D[k])[2];
    float        fMinDist = 1e20;
    unsigned int iMinIdx = 0;
    for( unsigned int i = 0; i < iNumMeshVerts; i++ )
      {
      verts->GetPoint( i, thispt );
      float dist = sqrt( pow(xcur - thispt[0], 2) + pow(ycur - thispt[1], 2) + pow(zcur - thispt[2], 2) );
      if( dist < fMinDist )
        {
        fMinDist = dist;
        iMinIdx = i;
        }
      }
    std::cout << " " << iMinIdx << " ...";
    initialPoints->InsertNextValue( iMinIdx );
    }
  std::cout << "\n";

  entry_main( inputMesh, initialPoints, outputMesh, init, bForceRecompute );

  return;
}

// Input: mesh only. No initialization of points; either continue
// evolution of existing curve or only pre-compute geometry!
void MeshContourEvolver::entry_main( vtkPolyData* inputMesh, vtkPolyData* outputMesh, InitParam init )
{
  vtkIntArray* emptyIntVec = 0;

  entry_main( inputMesh, emptyIntVec, outputMesh, init );

  return;
}
