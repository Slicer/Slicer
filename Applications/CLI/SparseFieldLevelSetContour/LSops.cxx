/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "LSops.h"

void SparseFieldLS::CleanLZ()
{
  SelfUnion( L_z );

// if a there is a path from any two neighbors of an element of L_z without 
// touching L_z itself, that L_z point is degenerate

}
int ii = 1;
bool SparseFieldLS::InitSphere()
{
  bool ret = 1;

// we are given the zero-level set.
// need to compute what the other layers are.
// since we have some 3D mesh we can't do something simple
// like for an image with mask.

// algorithm is as follows.
// pick some L_z point, and pick one of its neigbhors v' not in L_z.
// all vertices in the mesh are in one of three classes.
// either on the L_z, in the same region as v', or the complement
// of the union of the first two regions.
// grow the region in which v' is located adding vertices to that class
// ie all neighbors not currently in a class get added to the v' class.

// assumption: mesh is homeomorphic to a sphere
// ie if this were a torus and the C encircled the torus, this would fail to
// divide the mesh into two sections

  vtkPoints*    verts = meshdata->polydata->GetPoints();
  int numverts = verts->GetNumberOfPoints();

  point_type = vector<int>(numverts);
  for( int i = 0; i < numverts; i++ )
    {
    point_type[i] = 2;
    phi[i] = 2.0;
    }
  for( ::size_t i = 0; i < L_z.size(); i++ )
    {
    point_type[ L_z[i] ] = 0;
    phi[L_z[i]] = 0.0;
    }

  if( L_z.size() <= 0 ) {
  cout<<"Error, self-intersecting initial curve detected. Path finding failed.\n";
  return 0;
  }

  vector<int> visited(0);
  int vprime_idx = -1;
  for( ::size_t i = 0; i < meshdata->adj[L_z[0]].myNeighbs.size(); i++ )
    {
    int idx = meshdata->adjimm[L_z[0]].myNeighbs[i];
    if( point_type[idx] == 2 )
      {// found first neighbor not in L_z
      point_type[idx] = -2;
      phi[i] = -2.0;
      vprime_idx = idx;
      break;
      }
    }
  if( vprime_idx == -1 ) return 0; // couldnt find? must be degenerate L_z
  visited.push_back( vprime_idx );

  bool bFoundUnvisited = true;
  int maxits = 10;
  int j = 0;
  while( j < maxits && bFoundUnvisited ) {
  ::size_t len_vis = visited.size();
  for( ::size_t i = 0; i < visited.size(); i++ )
    { // look at all neigbhors of all 
// indices in the visited list
    int idx = visited[i];
    vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
    for( ::size_t k = 0; k < neigh.size(); k++ )
      {
      int nid = neigh[k];
      if( nid != idx && point_type[nid] == 2 )
        {
        point_type[nid] = -2;
        phi[nid] = -2.0;
        visited.push_back(nid);
        }
      }
    }
  j++;
  bFoundUnvisited = (visited.size() != len_vis); // when we don't find any more, done
  }

// ok now all points are either "interior" or "exterior".
// now compute the different layers.
// L_z can contribute one or no plus/minus pts
// this is the closest neighbor not having yet been assigned.
// 1. compute first layers

  for( ::size_t i = 0; i < L_z.size(); i++ )
    {
    int idx = L_z[i];
    vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
    for( ::size_t k = 0; k < neigh.size(); k++ )
      {
      int nid = neigh[k];
      int val = point_type[ nid ];
      if( val < 0 )
        {
        point_type[ nid ] = -1;
        phi[ nid ] = -1.0;
        L_n1.push_back( nid );
        }
      if( val > 0 )
        {
        point_type[ nid ] = 1;
        phi[ nid ] = 1.0;
        L_p1.push_back( nid );
        }
      }
    }
// discard redundant verts
  SelfUnion( L_n1 );
  SelfUnion( L_p1 );

// don't need 2nd layer for this method actually
// 2. compute second layers
  for( ::size_t i = 0; i < L_n1.size(); i++ )
    {
    int idx = L_n1[i];
    vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
    for( ::size_t k = 0; k < neigh.size(); k++ )
      {
      int nid = neigh[k];
      int val = point_type[ nid ];
      if( val < -1 )
        {
        point_type[ nid ] = -2;
        phi[ nid ] = -2.0;
        L_n2.push_back( nid );
        }
      if( val > 0 )
        {
        cout<<"Error in layer neighboring computation!! \n";
        }
      }
    }
  for( ::size_t i = 0; i < L_p1.size(); i++ )
    {
    int idx = L_p1[i];
    vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
    for( ::size_t k = 0; k < neigh.size(); k++ )
      {
      int nid = neigh[k];
      int val = point_type[ nid ];
      if( val < 0 )
        {
        cout<<"Error in layer neighboring computation!! \n";
        }
      if( val > 1 )
        {
        point_type[ nid ] = 2;
        phi[ nid ] = 2.0;
        L_p2.push_back( nid );
        }
      }
    }

  SelfUnion( L_p2 );
  SelfUnion( L_n2 );



// assign some data from curvature computation to be the new colormap
  vtkFloatArray* scalars2 = vtkFloatArray::New();
  for( int i = 0; i < numverts; i++ )
    {
    scalars2->InsertTuple1(i, double(point_type[i] != 0) );
    }
  meshdata->polydata->GetPointData()->SetScalars(scalars2);
  scalars2->Delete();
  meshdata->polydata->Update();


  return ret;
}

vector<int> SparseFieldLS::Evolve(int its )
{
  ii *= -1;
  double cfl = 0.15;

  for( int its_ = 0; its_ < its; its_++ )
    {
    if( L_z.size() == 0 )
      {
      cout<<"Error, level set has vanished from surface \n";
      return L_z;
      }

// How can this possibly be right if there are isolated L_z points
// appearing !?!?!?

// 1.5 optional: smooth phi along L_z
    vector<double> phi2 = phi;
    for( int smooth_its = 0; smooth_its < 1; smooth_its++ )
      {
      valarray<double> phi_(L_z.size());
      valarray<double> phi_p1(L_p1.size());
      valarray<double> phi_n1(L_n1.size());
      for( ::size_t i = 0; i < L_z.size(); i++ )
        {
        int idx = L_z[i];
        vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
        double phiav = 0.0;
        int num = 0;
        for( ::size_t k = 0; k < neigh.size(); k++ )
          {
          int nid = neigh[k];
//if( point_type[nid] == 0 ) {
          phiav += phi[nid];
          num++;
//}
          }
        phiav = phiav / num; // num > 0 always
        phi_[i] = phiav;
        }

      for( ::size_t i = 0; i < L_p1.size(); i++ )
        {
        int idx = L_p1[i];
        vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
        double phiav = 0.0;
        int num = 0;
        for( ::size_t k = 0; k < neigh.size(); k++ )
          {
          int nid = neigh[k];
//if( point_type[nid] == 1 ) {
          phiav += phi[nid];
          num++;
//}
          }
        phiav = phiav / num; // num > 0 always
        phi_p1[i] = phiav;
        }
      for(::size_t i = 0; i < L_n1.size(); i++ )
        {
        int idx = L_n1[i];
        vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
        double phiav = 0.0;
        int num = 0;
        for(::size_t k = 0; k < neigh.size(); k++ )
          {
          int nid = neigh[k];
//if( point_type[nid] == 1 ) {
          phiav += phi[nid];
          num++;
//}
          }
        phiav = phiav / num; // num > 0 always
        phi_n1[i] = phiav;
        }
      for(::size_t i = 0; i < L_p1.size(); i++ )
        {
        phi2[ L_p1[i] ] = phi_p1[i];
        }
      for(::size_t i = 0; i < L_z.size(); i++ )
        {
        phi2[ L_z[i] ] = phi_[i];
        }
      for(::size_t i = 0; i < L_n1.size(); i++ )
        {
        phi2[ L_n1[i] ] = phi_n1[i];
        }
      }
//phi = phi2;

// 1. for each active grid point x_i:
// a. compute local geometry of level set
// b. compute net change of u_x from internal and external forces
// (encapsulated in getforce() function )
    valarray<double> F = energy->getforce( L_z, L_p1, L_n1, phi2 );
    for(::size_t i = 0; i < F.size(); i++ ) phi[ L_z[i] ] += cfl * F[i];

// 2. find where zero layer exceeds [-0.5,0.5]
    vector<int> Sp(0);
    vector<int> Sn(0);
    for(::size_t i = 0; i < L_z.size(); i++ )
      {
      int idx = L_z[i];
      if( phi[idx] >= 0.5 )
        {
        Sp.push_back( idx );
        }
      if( phi[idx] <= -0.5 )
        {
        Sn.push_back( idx );
        }
      }

// 3. visit grid points in layers L_p1, L_n1 and update grid point
// values by adding/subtracting one unit from next inner layer.
// if multiple neighbors, use the closest one:
// maximum for negative layer, minimum for positive layers
    vector<int> Sp1(0);
    vector<int> Sn1(0);
    vector<int> Szp(0);
    vector<int> Szn(0);
    for(::size_t i = 0; i < L_p1.size(); i++ )
      {
      int idx = L_p1[i];
      vector<int> neigh = meshdata->adjimm[ idx ].myNeighbs;
      valarray<double> vals(neigh.size());
      for(::size_t k = 0; k < vals.size(); k++ )
        {
        vals[k] = phi[ neigh[k] ];
        }
      phi[idx] = vals.min() + 1.0;
      double phi_ = phi[idx];
      if( vals.min() > 0.5 )
        { // i have no zero neighbors, 
// therefore i am no longer eligble for L_p1 membership
        point_type[idx] = 2;
//phi[idx] = 2.0;
        Sp1.push_back( idx );
        }
      if( phi_ < 0.5 )
        {
        Szp.push_back( idx );
        }
      }
    for(::size_t i = 0; i < L_n1.size(); i++ )
      {
      int idx = L_n1[i];
      vector<int> neigh = meshdata->adjimm[ idx ].myNeighbs;
      valarray<double> vals(neigh.size());
      for(::size_t k = 0; k < vals.size(); k++ )
        {
        vals[k] = phi[ neigh[k] ];
        }
      phi[idx] = vals.max() - 1.0;
      double phi_ = phi[idx];
      if( vals.max() < -0.5 )
        { // i have no zero neighbors, 
// therefore i am no longer eligble for L_n1 membership
        point_type[idx] = -2;
//phi[idx] = -2.0;
        Sn1.push_back( idx );
        }
      if( phi_ > -0.5 )
        {
        Szn.push_back( idx );
        }
      }

// 3. erase from L_z the points that just got stored
    DropIdx( Sp, Sn, L_z );


// update zero layer with incoming points
    for(::size_t i = 0; i < Szp.size(); i++ )
      {
      L_z.push_back( Szp[i] );
      point_type[ Szp[i] ] = 0;
      }
    for(::size_t i = 0; i < Szn.size(); i++ )
      {
      L_z.push_back( Szn[i] );
      point_type[ Szn[i] ] = 0;
      }

    DropIdx( Szp, Sp1, L_p1 );
    DropIdx( Szn, Sn1, L_n1 );

    for(::size_t i = 0; i < Sp.size(); i++ ){
    L_p1.push_back( Sp[i] );
    point_type[ Sp[i] ] = 1;
    }
    for(::size_t i = 0; i < Sn.size(); i++ ){
    L_n1.push_back( Sn[i] );
    point_type[ Sn[i] ] = -1;
    }

// find isolated L_p1 and L_n1 points in L_z
    vector<int> Lzip(0);
    vector<int> Lzin(0);
    for(::size_t i = 0; i < L_z.size(); i++ )
      {
      int idx = L_z[i];
      vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
      valarray<double> vals(neigh.size());
      for(::size_t j = 0; j < neigh.size(); j++ )
        {
        vals[j] = phi[neigh[j]];
        }
      double vmax = vals.max();
      double vmin = vals.min();
      if( vmax < 0.5 )
        {
        Lzin.push_back( idx );
        phi[idx] = -1.0;
        point_type[idx] = -1;
        }
      if( vmin > -0.5 )
        {
        Lzip.push_back( idx );
        phi[idx] = 1.0;
        point_type[idx] = 1;
        }
      }
    DropIdx( Lzin, Lzip, L_z );

    for(::size_t i = 0; i < L_z.size(); i++ )
      {
      int idx = L_z[i];
      vector<int> neigh = meshdata->adjimm[idx].myNeighbs;
      for(::size_t k = 0; k < neigh.size(); k++ )
        {
        int nid = neigh[k];
        int val = point_type[ nid ];
        if( val < 0 )
          {
          point_type[ nid ] = -1;
//phi[ nid ] = -1.0;
          L_n1.push_back( nid );
          }
        if( val > 0 )
          {
          point_type[ nid ] = 1;
//phi[ nid ] = 1.0;
          L_p1.push_back( nid );
          }
        }
      }
// discard redundant verts
    SelfUnion( L_n1 );
    SelfUnion( L_p1 );
    SelfUnion( L_z );
    }
// assign some data from curvature computation to be the new colormap
  vtkFloatArray* scalars2 = vtkFloatArray::New(); // colormap
  if( meshdata->showLS == 1 )
    {
    meshdata->mapper->SetScalarRange( -2.0, 2.0 );
    for(::size_t i = 0; i < phi.size(); i++ )
      {
      scalars2->InsertTuple1(i, phi[i]);
//scalars2->InsertTuple1(i, meshdata->dkdx[i]);
      }
    }
  else {
  meshdata->mapper->SetScalarRange( -1.0, 1.0 );
  for(::size_t i = 0; i < phi.size(); i++ )
    {
    scalars2->InsertTuple1(i, double(abs(phi[i]) < 1.0 ));
//scalars2->InsertTuple1(i, meshdata->dkdx[i]);
    }
  }
  meshdata->polydata->GetPointData()->SetScalars(scalars2);
  scalars2->Delete();
  meshdata->polydata->Update();

  return L_z;

}
