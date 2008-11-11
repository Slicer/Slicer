#ifndef MESHOPS
#define MESHOPS

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <valarray>
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"
#include "vtkDebugLeaks.h"
#include "vtkTestUtilities.h"
#include "vtkFieldData.h"
#include "Utils.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataMapper.h"
#include <vector>
#include <algorithm>

struct AdjData {
int myIdx;
vector<int> myNeighbs;
};


struct MeshData {
valarray<double> MeanCurv;
valarray<double> dkdx;
valarray<double> dkdy;
valarray<double> dkdz;
valarray<double> nx;
valarray<double> ny;
valarray<double> nz;

// derivatives w.r.t. intrinsic basis on manifold
valarray<double> dkde1;
valarray<double> dkde2;
valarray<double> dkmag;

// extra color maps to use for various purposes
valarray<double> cmap0;
valarray<double> cmap1;
valarray<double> cmap2;
valarray<double> cmap3;

vtkPolyData* polydata;
vtkPolyDataMapper *mapper;
vector<AdjData> adj; // neighbors N steps into tree
vector<AdjData> adjimm; // immediate neighbors; not yet computed

valarray<double> kappa;

int adj_levels;
int smoothH_its;
int showLS;
};

void ComputeCurvatureData( MeshData* meshdata ); // main function

// d'oh, make MeshData a class and these guys as methods

// subroutines:

void ComputeAdjacency( MeshData* meshdata );

void ComputeNormals( MeshData* meshdata );

void ComputeCurvature( MeshData* meshdata );

int CountVertsOnMesh( vtkPolyData* poly );

void SmoothCurvature( MeshData* meshdata );

void SmoothGradCurvature( MeshData* meshdata );

void ComputeGradCurvature( MeshData* meshdata );

void ComputeGradCurvatureTangentPlane( MeshData* meshdata );

vector<int> InitPath( MeshData* meshdata, vector<int> pts);

#endif
