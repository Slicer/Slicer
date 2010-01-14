/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef MEANCURVATUREENERGY
#define MEANCURVATUREENERGY

#include "MeshEnergy.h"

class MeanCurvatureEnergy : public MeshEnergy
{
public:
MeanCurvatureEnergy( MeshData* data ) {meshdata = data;}
virtual ~MeanCurvatureEnergy( ) {}

double eval_energy( const vector<int>& C );
valarray<double> getforce( const vector<int>& C);
valarray<double> getforce( const vector<int>& C, 
const vector<int>& L_p1, const vector<int>& L_n1,
const vector<double>& phi);

};



#endif
