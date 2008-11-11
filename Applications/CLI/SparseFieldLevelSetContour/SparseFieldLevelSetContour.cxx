#include "SparseFieldLevelSetContourCLP.h"
#include <iostream>
#include <vector>
#include "vtkXMLPolyDataReader.h"
#include "vtkCellData.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

#include "Utils.h"
#include "MeshOps.h"
#include "LSops.h"
#include "MeanCurvatureEnergy.h"

using namespace std;

MeshData* meshdata;
SparseFieldLS* sfls;
MeanCurvatureEnergy* energy;
bool LSready = false;
bool done_init = false;
int showLS = 1;
int evolve_its = 1000;
int mesh_smooth_its = 100;
int H_smooth_its = 100;
int adj_levels = 1;

int main(int argc, char* argv[] )
{
cout<<"starting...\n";
PARSE_ARGS;

cout<<OutputFilename.c_str()<<"\n";
cout<<"Length of contour seeds: "<<ContourSeedPts.size()<<"\n";

vtkXMLPolyDataReader* reader = vtkXMLPolyDataReader::New();
reader->SetFileName(InputSurface.c_str());
reader->Update();

vtkPolyData* polyDataInput = reader->GetOutput();

vtkSmoothPolyDataFilter* smoother = vtkSmoothPolyDataFilter::New();
cout<<"Smoothing the surface...";
smoother->SetNumberOfIterations( mesh_smooth_its );

smoother->SetInput( polyDataInput );
smoother->Update();
cout<<" done! \n ";

// Now we'll look at it.
vtkPolyData* smooth_brain = smoother->GetOutput();

// do the curvature computations / pre-processing
meshdata = new MeshData();
meshdata->polydata = smooth_brain;
meshdata->smoothH_its = H_smooth_its;
meshdata->adj_levels = adj_levels;
meshdata->showLS = showLS;
ComputeCurvatureData( meshdata );
energy = new MeanCurvatureEnergy( meshdata );
// assign some data from curvature computation to be the new colormap
vtkFloatArray *scalars2 = vtkFloatArray::New();
for( int i = 0; i < meshdata->MeanCurv.size(); i++ ) {
scalars2->InsertTuple1(i, meshdata->MeanCurv[i] );
//scalars2->InsertTuple1(i, meshdata->dkde2[i] );
}
smooth_brain->GetPointData()->SetScalars(scalars2);
scalars2->Delete();


vtkPolyDataMapper* cubeMapper = vtkPolyDataMapper::New();
meshdata->mapper = cubeMapper;

cubeMapper->SetInput( smooth_brain );
double dmin = meshdata->dkde2.min();
double dmax = meshdata->dkde2.max();
dmin = -2.0;
dmax = 2.0;
//cubeMapper->SetScalarRange(dmin, dmax );
cubeMapper->SetScalarRange(meshdata->MeanCurv.min(), meshdata->MeanCurv.max() );

vector<int> init_pts; // vector of poly data vertex indices that are seeds
init_pts.push_back(1255);
init_pts.push_back(1291);
init_pts.push_back(4087);
init_pts.push_back(4028);


vector<int> C = InitPath( meshdata, init_pts );
sfls = new SparseFieldLS( meshdata, C, energy );
sfls->Evolve(evolve_its);
int breakhere = 1;

/*vtkMRMLScene* myScene = vtkMRMLScene::New();
myScene->SetURL( OutputFilename.c_str() );
int iResult = myScene->Connect();
vtkCollection* modelCollection = vtkCollection::New();
vector<vtkMRMLNode*> modelNodes;
myScene->GetNodesByClass( "vtkMRMLModelNode", modelNodes );
if( modelNodes.size() < 1 )
return EXIT_FAILURE;

vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( modelNodes[0] );
if( NULL == modelNode )
return EXIT_FAILURE;
vtkPolyData* polyData = modelNode->GetPolyData();*/


return EXIT_SUCCESS;
}
