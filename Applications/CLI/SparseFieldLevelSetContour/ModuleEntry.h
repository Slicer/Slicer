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

using std::vector;

namespace MeshContourEvolver {

// Input: mesh and indices of vertices for initialization
vtkPolyData* entry_main( vtkPolyData* inputMesh, vtkIntArray* initVertIdx );

// Input: mesh and 3D points for initialization. This is what you get
// when inputting 'fiducials' in Slicer GUI. The 3D points
// are not on the mesh, you need to first find closest points on the mesh.
vtkPolyData* entry_main( vtkPolyData* inputMesh, const vector< vector<float> >& initPoints3D);

// Input: mesh only. No initialization of points; either continue
// evolution of existing curve or only pre-compute geometry!
vtkPolyData* entry_main( vtkPolyData* inputMesh );



}
