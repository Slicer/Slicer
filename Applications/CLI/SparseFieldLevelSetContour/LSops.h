/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef LSOPS
#define LSOPS

#include "MeshOps.h"
#include "MeshEnergy.h"

class SparseFieldLS
{
public:
SparseFieldLS( MeshData* data, const vector<int>& C, MeshEnergy* mergy )
{
L_z = C;
meshdata = data;
energy = mergy;
phi = vector<double>(data->MeanCurv.size());
CleanLZ();
InitSphere();
dDirection = -1.0;
}
vector<int> Evolve( int iterations = 1 );


private:
// the different layers for sparse-field method
MeshEnergy* energy;
MeshData* meshdata;
vector<int> L_z;
vector<int> L_n1;
vector<int> L_p1;
vector<int> L_n2;
vector<int> L_p2;
double dDirection;

vector<int> point_type;
vector<double> phi;

public:
void SwitchDirection()    { dDirection *= -1.0; }
vector<double>* GetPhi()  { return &phi; };
vector<int>*  GetPtrLZ()  { return &L_z; };

// "sphere": assume that the mesh is homeomorphic to a sphere
bool InitSphere(); // determine what the other layers are on the mesh given L_z
bool InitFast(); // TODO: a fast way of doing this (local search...)
void CleanLZ(); // delete extraneous LZ points
};

#endif
