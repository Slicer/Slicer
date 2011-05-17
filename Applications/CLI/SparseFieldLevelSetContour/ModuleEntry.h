// this file defines a function that communicates
// with the application using the module.

// input function requires a vtkPolyData and 
// possible also vtkPoints as initializers.

// It then returns a vtkPolyData.

// the application is responsible for reading or creating
// data files to send as vtkPolyData,
// accepting user input for initial points, 
// and handling the output polyData (either to display or
// write to file).

#include "vtkComputeLocalGeometry.h"
#include "vtkInitClosedPath.h"
#include "vtkLevelSetMeshEvolver.h"

#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkFloatArray.h"
#include "vtkTriangleFilter.h"

#include <vector>

#if defined(WIN32) && !defined(SparseFieldLevelSetContour_STATIC)
 #if defined(SparseFieldLevelSetContour_EXPORTS)
  #define SPARSEFIELDLEVELSETCONOUR_EXPORT __declspec( dllexport ) 
 #else
  #define SPARSEFIELDLEVELSETCONOUR_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define SPARSEFIELDLEVELSETCONOUR_EXPORT
#endif

namespace MeshContourEvolver {

struct InitParam {
int evolve_its;
int mesh_smooth_its;
int H_smooth_its;
int adj_levels;
int rightHandMesh;
};

// Input: mesh and indices of vertices for initialization
//vtkPolyData* entry_main( vtkPolyData* inputMesh, vtkIntArray* initVertIdx, bool bForceRecompute = false);
void SPARSEFIELDLEVELSETCONOUR_EXPORT entry_main( vtkPolyData* inputMesh, vtkIntArray* initVertIdx, vtkPolyData *outputMesh, InitParam init, bool bForceRecompute = false );


// Input: mesh and 3D points for initialization. This is what you get
// when inputting 'fiducials' in Slicer GUI. The 3D points
// are not on the mesh, you need to first find closest points on the mesh.
void SPARSEFIELDLEVELSETCONOUR_EXPORT entry_main( vtkPolyData* inputMesh, std::vector< std::vector<float> >& initPoints3D, vtkPolyData* outputMesh, InitParam init, bool bForceRecompute = false);

// Input: mesh only. No initialization of points; either continue
// evolution of existing curve or only pre-compute geometry!
void SPARSEFIELDLEVELSETCONOUR_EXPORT entry_main( vtkPolyData* inputMesh, vtkPolyData* outputMesh, InitParam init);

}

