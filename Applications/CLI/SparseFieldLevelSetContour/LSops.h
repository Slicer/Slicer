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
}
void Evolve( int iterations = 1 );


private:
// the different layers for sparse-field method
MeshEnergy* energy;
MeshData* meshdata;
vector<int> L_z;
vector<int> L_n1;
vector<int> L_p1;
vector<int> L_n2;
vector<int> L_p2;

vector<int> point_type;
vector<double> phi;

// "sphere": assume that the mesh is homeomorphic to a sphere
bool InitSphere(); // determine what the other layers are on the mesh given L_z
bool InitFast(); // TODO: a fast way of doing this (local search...)
void CleanLZ(); // delete extraneous LZ points
};

#endif
