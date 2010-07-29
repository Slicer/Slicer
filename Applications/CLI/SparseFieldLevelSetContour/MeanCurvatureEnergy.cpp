/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "MeanCurvatureEnergy.h"

double MeanCurvatureEnergy::eval_energy(const std::vector<int> &C)
{
  double E = 0.0;

  return E;
}

std::valarray<double> MeanCurvatureEnergy::getforce( const std::list<int>& C_, 
                                                const std::list<int>& L_p1, const std::list<int>& L_n1,
                                                const std::vector<double>& phi)
{

  std::valarray<double> force( C_.size() );
// exp( -lambda * H ) * ( nhat dot gradH + kappa )

  std::valarray<double> ne1(C_.size());
  std::valarray<double> ne2(C_.size());
  std::valarray<double> kappa(C_.size());
  
  std::vector<int> C = ListToSTDVector( C_ );

  GetNormalsTangentPlane( C, phi, ne1, ne2, this->meshdata );
  GetKappa( C, phi, kappa );
  ::size_t CN = C.size();
  for( ::size_t i = 0; i < CN; i++ )
    {
    int idx = C[i];
    double val = meshdata->dkde1[idx] * ne1[i] + meshdata->dkde2[idx] * ne2[i];
    force[i] = val;
    }
  meshdata->kappa = kappa;

  double alpha = 0.5;
  double skap = abs(kappa).max();
  if( skap > 1e-6 )
    {
      force = tanh(force);
      kappa = tanh(kappa);
      force = ((1-alpha)*force / (1e-9+abs(force).max()) + alpha*kappa / (1e-2+abs(kappa).max()));
      
      return force;
    }
  else
    {
    return force / (abs(force)).max();
    }
}

std::valarray<double> MeanCurvatureEnergy::getforce( const std::list<int>& C)
{
  std::cout<<"Err!\n";
  return std::valarray<double>(0);
}
