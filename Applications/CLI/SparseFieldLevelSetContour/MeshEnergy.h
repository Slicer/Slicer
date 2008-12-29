/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef ENERGY
#define ENERGY

#include "MeshOps.h"

class MeshEnergy
{
public:
MeshEnergy() { }
MeshEnergy( const MeshEnergy & );
virtual ~MeshEnergy() { }
// evaluate energy at every point indexed by C
virtual double eval_energy( const vector<int>& C ) = 0;

// get the force F at the vertices for sparse field method
virtual valarray<double> getforce( const vector<int>& C ) = 0;
virtual valarray<double> getforce( const vector<int>& C, const vector<double>& phi ) = 0;
virtual valarray<double> getforce( const vector<int>& C, 
const vector<int>& L_p1, const vector<int>& L_n1,
const vector<double>& phi) = 0;

public:
MeshData* meshdata;

protected:
void GetNormals( const vector<int>& C, const vector<double>& phi, 
valarray<double>& nx, valarray<double>& ny, valarray<double>& nz );
void GetKappa( const vector<int>& C, const vector<double>& phi,
valarray<double>& kappa);

void GetNormalsTangentPlane( const vector<int>& C, const vector<double>& phi,
valarray<double>& ne1, valarray<double>& ne2, MeshData* meshdata );

// bogus
void GetNormals2( const vector<int>& C, const vector<double>& phi, 
valarray<double>& nx, valarray<double>& ny, valarray<double>& nz );

};

#endif

