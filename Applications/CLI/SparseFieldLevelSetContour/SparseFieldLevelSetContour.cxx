/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "SparseFieldLevelSetContourCLP.h"
#include <iostream>
#include <vector>
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
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


// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {

MeshData* meshdata;
SparseFieldLS* sfls;
MeanCurvatureEnergy* energy;
vtkPolyDataWriter* writer;
int showLS = 0;
int evolve_its = 10; // how many evolution iterations to take
int mesh_smooth_its = 100; // how many times to smooth the input mesh
int H_smooth_its = 100; // how many times to average the curvature
int adj_levels = 1; // number of levels into the adjacency tree to look


/* An example calling convention from the commandline for this module:
--inputScene "C:\academic\computer_vision_research\level_set_brain_proj\ls_brain_vtk\bin\Release\LSBrainVTKOut1.vtk" -c "1.5,1.5,0" -c "-1.5,1.5,0" -c "-1.5,-1.5,0" -c "1.5,-1.5,0" --outputFilename "tempOut.vtk"
This gives it an initial geometry in the vtk file, four seed points in (x,y,z) coordinates, and where to write the output.
The resulting output contains a colormap that defines interior/exterior of the surface
as determined by evolving the curve into high mean curvature areas.
*/

} // end of anonymous namespace


int main(int argc, char* argv[] )
{
  std::cout<<"starting...\n";
  PARSE_ARGS;

  std::cout<<OutputFilename.c_str()<<"\n";
  std::cout<<"Length of contour seeds: "<<ContourSeedPts.size()<<"\n";

  vtkPolyDataReader* reader = vtkPolyDataReader::New();
  reader->SetFileName(InputSurface.c_str());
  reader->Update();

  vtkPolyData* polyDataInput = reader->GetOutput();

  vtkSmoothPolyDataFilter* smoother = vtkSmoothPolyDataFilter::New();
  std::cout<<"Smoothing the surface...";
  smoother->SetNumberOfIterations( mesh_smooth_its );

  smoother->SetInput( polyDataInput );
  smoother->Update();
  std::cout<<" done! \n ";

// Now we'll look at it.
  vtkPolyData* smooth_brain = smoother->GetOutput();
  //polyDataInput->Delete();
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
  for( ::size_t i = 0; i < meshdata->MeanCurv.size(); i++ )
    {
    scalars2->InsertTuple1(i, meshdata->MeanCurv[i] );
    }
  smooth_brain->GetPointData()->SetScalars(scalars2);
  scalars2->Delete();

  vtkPolyDataMapper* cubeMapper = vtkPolyDataMapper::New();
  meshdata->mapper = cubeMapper;

  cubeMapper->SetInput( smooth_brain );
  cubeMapper->SetScalarRange(meshdata->MeanCurv.min(), meshdata->MeanCurv.max() );

  // for reference: the seed points are declared as:
  // std::vector<std::vector<float> > ContourSeedPts;
  vector<int> init_pts; // vector of poly data vertex indices that are seeds
  
  // for each of the seed points, we need to determine an index in the mesh
  // that is closest to the (x,y,z) coordinate.
  // a straightforward but likely non-optimal way to do this is to search over the mesh
    
  vtkPoints*    verts = meshdata->polydata->GetPoints();
  double thispt[3];
  unsigned int iNumSeedPts = ContourSeedPts.size();
  unsigned int iNumMeshVerts = verts->GetNumberOfPoints();
  for( unsigned int k = 0; k < iNumSeedPts; k++ )
  {
    float xcur = (ContourSeedPts[k])[0];
    float ycur = (ContourSeedPts[k])[1];
    float zcur = (ContourSeedPts[k])[2];
    float fMinDist = 1e20;
    unsigned int iMinIdx = 0;
    for( unsigned int i = 0; i < iNumMeshVerts; i++ )
    {
      verts->GetPoint( i, thispt );
      float dist = sqrt( pow(xcur - thispt[0],2) + pow(ycur - thispt[1],2) + pow(zcur - thispt[2],2) );
      if( dist < fMinDist )
      {
        fMinDist = dist;
        iMinIdx = i;
      }
    }
    init_pts.push_back(iMinIdx);
  }

  // Problem: this routine can fail easily.
  // A better routine would devise an initial contour that can intersect, etc
  // or more generally be created from arbitrary seed points.
  // As it stands the points need to be *in order* along the contour
  // and the pathfinder is greedy. If it finds itself intersecting back on itself
  // it fails.
  vector<int> C = InitPath( meshdata, init_pts );

  // Great we were able to initialize a path. Now just do the curve evolution.
  // Potential Issue: the way that points are colored as inside/outside.
  // Depending on the shape, the contour may not define a partition of the object.
  // For example, drawing a circle around a Torus (Donut) does NOT partition it,
  // although drawing it on the surface without going around the loop of the donut does.
  sfls = new SparseFieldLS( meshdata, C, energy );
  C = sfls->Evolve(evolve_its);

  writer = vtkPolyDataWriter::New();
  writer->SetInput( meshdata->polydata );
  writer->SetFileName( OutputFilename.c_str() );
  writer->Write();
  // The result is contained in the scalar colormap of the output.

  delete energy;
  delete meshdata;
  reader->Delete();
  cubeMapper->Delete();  
  writer->Delete();
  smoother->Delete();
  //smooth_brain->Delete();
  delete sfls;

  return EXIT_SUCCESS;
}
