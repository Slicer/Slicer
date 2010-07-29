/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
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
std::vector<int> myNeighbs;
};


struct MeshData {
std::valarray<double> MeanCurv;
std::valarray<double> dkdx;
std::valarray<double> dkdy;
std::valarray<double> dkdz;
std::valarray<double> nx;
std::valarray<double> ny;
std::valarray<double> nz;

// derivatives w.r.t. intrinsic basis on manifold
std::valarray<double> dkde1;
std::valarray<double> dkde2;
std::valarray<double> dkmag;

// extra color maps to use for various purposes
std::valarray<double> cmap0;
std::valarray<double> cmap1;
std::valarray<double> cmap2;
std::valarray<double> cmap3;

vtkPolyData* polydata;
vtkPolyDataMapper *mapper;
std::vector<AdjData> adj; // neighbors N steps into tree
std::vector<AdjData> adjimm; // immediate neighbors; not yet computed

std::valarray<double> kappa;

int adj_levels;
int smoothH_its;
int showLS;
int rightHandMesh;
};

void ComputeCurvatureData( MeshData* meshdata ); // main function

// d'oh, make MeshData a class and these guys as methods

// subroutines:

void ComputeAdjacency( MeshData* meshdata );

void ComputeNormals( MeshData* meshdata );

void ComputeCurvature( MeshData* meshdata );

int CountVertsOnMesh( vtkPolyData* poly );

void SmoothCurvature( MeshData* meshdata );

void ComputeGradCurvatureTangentPlane( MeshData* meshdata );

std::vector<int> InitPath( MeshData* meshdata, std::vector<int> pts);

#endif
