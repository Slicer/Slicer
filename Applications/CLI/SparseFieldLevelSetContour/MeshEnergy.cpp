/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "MeshEnergy.h"

void MeshEnergy::GetNormals2( const vector<int>& C, const vector<double>& phi, 
                              valarray<double>& nx, valarray<double>& ny, valarray<double>& nz )
{ // quick thing to try: "gradient is direction of max increase"

  vtkPoints*    verts = meshdata->polydata->GetPoints();

  double thispt[3];
  double thatpt[3];
  for( ::size_t k = 0; k < C.size(); k++ )
    {
    int i = C[k];
    verts->GetPoint( i, thispt );
    vector<int> neigh = meshdata->adjimm[i].myNeighbs;
    valarray<double> val(neigh.size());
    valarray<double> xx(neigh.size());
    valarray<double> yy(neigh.size());
    valarray<double> zz(neigh.size());
    for( ::size_t j = 0; j < neigh.size(); j++ )
      {
      int idx = neigh[j];
      verts->GetPoint( idx, thatpt );
      val[j] = phi[idx];
      xx[j] = thatpt[0];
      yy[j] = thatpt[1];
      zz[j] = thatpt[2];
      }
// note: none of the neighbors of L_z should
// be more than 2 in absolute value...
    double maxE = val.max();
    int jMax;
    for( ::size_t ii = 0; ii < val.size(); ii++ )
      {
      if (val[ii] == maxE)
        {
        jMax = ii;
        break;
        }
      }
    double dx,dy,dz;
    double thenorm = sqrt( pow( thispt[0] - thatpt[0],2.0 ) + pow( thispt[1] - thatpt[1],2.0 ) + pow( thispt[2] - thatpt[2], 2.0 ) );
    if( thenorm < 1e-6 )
      {
      dx = 0.0;
      dy = 0.0;
      dz = 0.0;
      }
    else
      {
      dx = ( xx[jMax] - thispt[0] ) / thenorm;
      dy = ( yy[jMax] - thispt[1] ) / thenorm;
      dz = ( zz[jMax] - thispt[2] ) / thenorm;
      }
    nx[k] = dx;
    ny[k] = dy;
    nz[k] = dz;
    }

  for( ::size_t i = 0; i < C.size(); i++ )
    {
    double norm = sqrt( nx[i]*nx[i] + ny[i]*ny[i] + nz[i]*nz[i] );
    if( norm < 1e-6 )
      {
      continue;
      }
    nx[i] /= norm;
    ny[i] /= norm;
    nz[i] /= norm;
    }
}

void MeshEnergy::GetNormalsTangentPlane( const vector<int>& C, const vector<double>& phi,
                                         valarray<double>& ne1, valarray<double>& ne2, 
                                         MeshData* vtkNotUsed(meshdata))
{
  vtkPoints*    verts = meshdata->polydata->GetPoints();

  for( ::size_t k_ = 0; k_ < C.size(); k_++ )
    {
    int k = C[k_];
    vector<double> nhat(3);
    nhat[0] = meshdata->nx[k]; // these are normal to surface; the nx_ are the contour normals
    nhat[1] = meshdata->ny[k];
    nhat[2] = meshdata->nz[k];
// step 1. create the rotation matrix that orients the current normal as [0,0,1]'.

    double phiang = atan2( nhat[0], nhat[1] );
    vector<double> rotate1(9);
    rotate1[0] = cos(phiang); rotate1[1] = -sin(phiang); rotate1[2] = 0;
    rotate1[3] = sin(phiang); rotate1[4] = cos(phiang); rotate1[5] = 0;
    rotate1[6] = 0; rotate1[7] = 0; rotate1[8] = 1.0;
    vector<double> nhat_a(3);
    pkmult( nhat, rotate1, nhat_a );
    double ytilde = nhat_a[1];
    double theta = M_PI_2 - atan2(nhat[2],ytilde);
    vector<double> rotate2(9);
    rotate2[0] = 1.0; rotate2[1] = 0; rotate2[2] = 0;
    rotate2[3] = 0; rotate2[4] = cos(theta); rotate2[5] = -sin(theta);
    rotate2[6] = 0; rotate2[7] = sin(theta); rotate2[8] = cos(theta);
    vector<double> nhat_b(3);
    pkmult( nhat_a, rotate2, nhat_b );
// nhat_b should now be [0 0 1]'

    double thispt[3];
    verts->GetPoint( k, thispt );
// apply rotate2 * rotate1 to each *translated* neighbor of this k-th point
    ::size_t num_neigh = meshdata->adj[k].myNeighbs.size();
    double vec[3];
    vector<double> vv(3);
    vector<double> vv_(3);
    valarray<double> xdata(num_neigh);
    valarray<double> ydata(num_neigh);
    valarray<double> zdata(num_neigh);
// step 2. create temporary set of vectors as copies of neighboring points
// translated to origin
// step 3. apply the rotation to all these points
    for ( ::size_t i = 0; i < num_neigh; i++ )
      {
      int idx = meshdata->adj[k].myNeighbs[i];
      verts->GetPoint( idx, vec );
      vv[0] = vec[0] - thispt[0];
      vv[1] = vec[1] - thispt[1];
      vv[2] = vec[2] - thispt[2];
      pkmult( vv, rotate1, vv_ );
      pkmult( vv_, rotate2, vv );
      xdata[i] = vv[0];
      ydata[i] = vv[1];
      zdata[i] = phi[idx] - phi[k]; //vv[2];
// zero reference phi at the vertex where we are forming tangent plane
      }
/*if( abs(zdata).min() < 1e-6 )
continue;*/

// step 4. find least-squares fit for H(x,y) = ax + by
    valarray<double> RHS(2);
    valarray<double> ATA(4);
    ATA[0] = (xdata * xdata).sum();
    ATA[1] = (xdata * ydata).sum();
    ATA[2] = ATA[1];
    ATA[3] = (ydata * ydata).sum();

    RHS[0] = (xdata * zdata).sum();
    RHS[1] = (ydata * zdata).sum();

    int maxits = 1000;
    valarray<double> ab = RHS; // initial guess
    valarray<double> LHS(2);
    pkmult2( ab, ATA, LHS );
    double res = sqrt( ( (LHS - RHS)*(LHS - RHS) ).sum() );
    double tol = 1e-8;
    int iter = 0;
    while( iter < maxits && res > tol )
      {
      iter++;
      ab[0] = (RHS[0] - ( ab[1]*ATA[1] ) )/ ATA[0];
      ab[1] = (RHS[1] - ( ab[0]*ATA[2] ) )/ ATA[3];
      pkmult2( ab, ATA, LHS );
      res = sqrt( ( (LHS - RHS)*(LHS - RHS) ).sum() );
      }
    ne1[k_] = ab[0] / sqrt( (ab*ab).sum() );
    ne2[k_] = ab[1] / sqrt( (ab*ab).sum() );
// step 5. differentiate the plane along principal directions

    }

}

void MeshEnergy::GetKappa( const vector<int>& C, const vector<double>& phi,
                           valarray<double>& kappa)
{
// kappa: divergence of normal
// dy^2 * dxx - 2dxdydxy + dx^2dyy / ( dx^2 + dy^2 )^(3/2)

  vtkPoints*    verts = meshdata->polydata->GetPoints();

  for( ::size_t k_ = 0; k_ < C.size(); k_++ )\
    {
    int k = C[k_];
    vector<double> nhat(3);
    nhat[0] = meshdata->nx[k]; // these are normal to surface; the nx_ are the contour normals
    nhat[1] = meshdata->ny[k];
    nhat[2] = meshdata->nz[k];
// step 1. create the rotation matrix that orients the current normal as [0,0,1]'.

    double phiang = atan2( nhat[0], nhat[1] );
    vector<double> rotate1(9);
    rotate1[0] = cos(phiang); rotate1[1] = -sin(phiang); rotate1[2] = 0;
    rotate1[3] = sin(phiang); rotate1[4] = cos(phiang); rotate1[5] = 0;
    rotate1[6] = 0; rotate1[7] = 0; rotate1[8] = 1.0;
    vector<double> nhat_a(3);
    pkmult( nhat, rotate1, nhat_a );
    double ytilde = nhat_a[1];
    double theta = M_PI_2 - atan2(nhat[2],ytilde);
    vector<double> rotate2(9);
    rotate2[0] = 1.0; rotate2[1] = 0; rotate2[2] = 0;
    rotate2[3] = 0; rotate2[4] = cos(theta); rotate2[5] = -sin(theta);
    rotate2[6] = 0; rotate2[7] = sin(theta); rotate2[8] = cos(theta);
    vector<double> nhat_b(3);
    pkmult( nhat_a, rotate2, nhat_b );
// nhat_b should now be [0 0 1]'

    double thispt[3];
    verts->GetPoint( k, thispt );
// apply rotate2 * rotate1 to each *translated* neighbor of this k-th point
    ::size_t num_neigh = meshdata->adj[k].myNeighbs.size();
    double vec[3];
    vector<double> vv(3);
    vector<double> vv_(3);
    valarray<double> xdata(num_neigh);
    valarray<double> ydata(num_neigh);
    valarray<double> zdata(num_neigh);
// step 2. create temporary set of vectors as copies of neighboring points
// translated to origin
// step 3. apply the rotation to all these points
    for (::size_t i = 0; i < num_neigh; i++ )
      {
      int idx = meshdata->adj[k].myNeighbs[i];
      verts->GetPoint( idx, vec );
      vv[0] = vec[0] - thispt[0];
      vv[1] = vec[1] - thispt[1];
      vv[2] = vec[2] - thispt[2];
      pkmult( vv, rotate1, vv_ );
      pkmult( vv_, rotate2, vv );
      xdata[i] = vv[0];
      ydata[i] = vv[1];
      zdata[i] = phi[idx] - phi[k]; //vv[2];
// zero reference phi at the vertex where we are forming tangent plane
      }
/*if( abs(zdata).min() < 1e-6 )
continue;*/

// step 4. find first derivatives
    double phi_x = 0.0;
    double phi_y = 0.0;
    {
    valarray<double> RHS(2);
    valarray<double> ATA(4);
    ATA[0] = (xdata * xdata).sum();
    ATA[1] = (xdata * ydata).sum();
    ATA[2] = ATA[1];
    ATA[3] = (ydata * ydata).sum();

    RHS[0] = (xdata * zdata).sum();
    RHS[1] = (ydata * zdata).sum();

    int maxits = 1000;
    valarray<double> ab = RHS; // initial guess
    valarray<double> LHS(2);
    pkmult2( ab, ATA, LHS );
    double res = sqrt( ( (LHS - RHS)*(LHS - RHS) ).sum() );
    double tol = 1e-8;
    int iter = 0;
    while( iter < maxits && res > tol )
      {
      iter++;
      ab[0] = (RHS[0] - ( ab[1]*ATA[1] ) )/ ATA[0];
      ab[1] = (RHS[1] - ( ab[0]*ATA[2] ) )/ ATA[3];
      pkmult2( ab, ATA, LHS );
      res = sqrt( ( (LHS - RHS)*(LHS - RHS) ).sum() );
      }
    phi_x = ab[0];
    phi_y = ab[1];
    }

// step 4. find least-squares fit for phi(x,y) = ax^2 + bxy + cy^2
// to get second derivatives
    valarray<double> RHS(3); // A'z
    RHS[0] = ( xdata * xdata * zdata  ).sum();
    RHS[1] = ( xdata * ydata * zdata  ).sum();
    RHS[2] = ( ydata * ydata * zdata  ).sum();

    double tik_delta = 1e-1 * abs(RHS).min();

    vector<double> ATA(9); // A'A
    ATA[0] = tik_delta + (xdata * xdata * xdata * xdata).sum();
    ATA[1] = (xdata * xdata * xdata * ydata).sum();
    ATA[2] = (xdata * xdata * ydata * ydata).sum();
    ATA[3] = (xdata * ydata * xdata * xdata).sum();
    ATA[4] = tik_delta + (xdata * ydata * xdata * ydata).sum();
    ATA[5] = (xdata * ydata * ydata * ydata).sum();
    ATA[6] = (ydata * ydata * xdata * xdata).sum();
    ATA[7] = (ydata * ydata * xdata * ydata).sum();
    ATA[8] = tik_delta + (ydata * ydata * ydata * ydata).sum();

    int maxits = 1000;
    valarray<double> abc = RHS; // initial guess
    valarray<double> LHS(3);
    pkmult( abc, ATA, LHS );
    double res = sqrt( ( (LHS - RHS)*(LHS - RHS) ).sum() );
    double tol = 1e-8;
    int iter = 0;
    while( iter < maxits && res > tol )
      {
      iter++;
      abc[0] = (RHS[0] - ( abc[1]*ATA[1] + abc[2]*ATA[2] ) )/ ATA[0];
      abc[1] = (RHS[1] - ( abc[0]*ATA[3] + abc[2]*ATA[5] ) )/ ATA[4];
      abc[2] = (RHS[2] - ( abc[0]*ATA[6] + abc[1]*ATA[7] ) )/ ATA[8];
      pkmult( abc, ATA, LHS );
      res = sqrt( ( (LHS - RHS)*(LHS - RHS) ).sum() );
      }
// step 5. get the derivatives from quadratic form
    double phi_xx = 2*abc[0];
    double phi_xy = abc[1];
    double phi_yy = 2*abc[2];

    kappa[k_] = phi_y * phi_y * phi_xx - 2 * phi_x * phi_y * phi_xy + phi_x * phi_x * phi_yy;
    if( abs(phi_x) + abs(phi_y) > 1e-9 )
      {
      kappa[k_] /= pow( (phi_x*phi_x + phi_y*phi_y), 1.5 );
      }
    }
}
