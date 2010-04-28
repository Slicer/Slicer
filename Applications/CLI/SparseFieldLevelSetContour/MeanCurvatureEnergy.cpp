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

valarray<double> MeanCurvatureEnergy::getforce( const vector<int>& C, 
                                                const vector<int>& L_p1, const vector<int>& L_n1,
                                                const vector<double>& phi)
{

  valarray<double> force( C.size() );
// exp( -lambda * H ) * ( nhat dot gradH + kappa )

  valarray<double> ne1(C.size());
  valarray<double> ne2(C.size());
//valarray<double> nz(C.size());
  valarray<double> kappa(C.size());
//GetNormals2( C, phi, nx, ny, nz );
  GetNormalsTangentPlane( C, phi, ne1, ne2, this->meshdata );
  GetKappa( C, phi, kappa );
//kappa = ne1 * 0.0;
//double lambda = 1;
  for( ::size_t i = 0; i < C.size(); i++ )
    {
    int idx = C[i];
    double val = meshdata->dkde1[idx] * ne1[i] + meshdata->dkde2[idx] * ne2[i];
    force[i] = -val;
//force[i] = nx[i];
//force[i] = meshdata->dkdx[idx];
//force[i] = -exp( -lambda * meshdata->MeanCurv[idx] ) * val;
//force[i] = -meshdata->MeanCurv[idx];
//force[i] = sqrt( nx[i] * nx[i] + ny[i]*ny[i] + nz[i]*nz[i] );
//force[i] = exp( meshdata->MeanCurv[idx] ) * sqrt( nx[i] * nx[i] + ny[i]*ny[i] + nz[i]*nz[i] );
    }
  meshdata->kappa = kappa;

  double alpha = 0.5;
  double skap = abs(kappa).max();
  if( skap > 1e-6 )
    {
    return (1-alpha)*force / (abs(force)).max() + alpha*kappa / (abs(kappa)).max();
    }
  else
    {
    return force / (abs(force)).max();
    }
}

valarray<double> MeanCurvatureEnergy::getforce( const vector<int>& C)
{
  std::cout<<"Error, TODO function being called!\n";
  return valarray<double>(0);
}
