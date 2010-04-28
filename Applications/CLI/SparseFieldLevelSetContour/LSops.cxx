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
#include <list>

void SparseFieldLS::SelfUnion( vector<int>& vec )
{
  sort( vec.begin(), vec.end() );
  vec.erase( unique(vec.begin(), vec.end()),vec.end() );
  int breakhere = 1;
}

void SparseFieldLS::AppendIdx( const vector<int>& src, vector<int>& dst ) {
  for( int i = 0; i < src.size(); i++ )
    dst.push_back( src[i] );
  SelfUnion( dst );
}

void SparseFieldLS::DropIdx( const vector<int>& L_zp, const vector<int>& L_zn, vector<int>& L_z )
{
  vector<int> L_z_(0);
  for( int i = 0; i < L_z.size(); i++ ) {
    int num1 = count( L_zp.begin(), L_zp.end(), L_z[i] );
    int num2 = count( L_zn.begin(), L_zn.end(), L_z[i] );
    if( num1 + num2 == 0 )
      L_z_.push_back( L_z[i] );
  }
  L_z = L_z_;

}


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
  cerr<<"Error, self-intersecting initial curve detected. Path finding failed.\n";
  return 0;
  }

  vector<int> visited(0);
  int vprime_idx = -1;
  for( ::size_t i = 0; i < meshdata->adjimm[L_z[0]].myNeighbs.size(); i++ )
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
        cerr<<"Error in layer neighboring computation!! \n";
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
        cerr<<"Error in layer neighboring computation!! \n";
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
  scalars2->SetName("InitSphere");
  for( int i = 0; i < numverts; i++ )
    {
    scalars2->InsertTuple1(i, double(point_type[i] != 0) );
    }
  meshdata->polydata->GetPointData()->AddArray(scalars2); //SetScalars(scalars2);
  scalars2->Delete();
  meshdata->polydata->Update();


  return ret;
}

vector<int> SparseFieldLS::EvolveRecalc( ) {

   int numVerts = phi.size();
   vector<int> class_of_blob(numVerts);

   vector<int> Lz;
   vector<int> Ln1;
   vector<int> Lp1;
   vector<int> Ln2;
   vector<int> Lp2;


   for( int k = 0; k < numVerts; k++ ) {
        if( phi[k] > -0.5 ) {
            class_of_blob[k] = 0; // Interior!
          }
          else {
            class_of_blob[k] = 1; // Exterior!
          }
        }
    
        
    // now compute the different layers.
    // L__z can contribute one or no plus/minus pts
    // this is the closest neighbor not having yet been assigned.
    // 1. compute first layers

   // L__z: the array of vertex indices that have both neighbor INSIDE and OUTSIDE
      std::vector<int> edgeVisited = std::vector<int>(numVerts);
#define INIT_VAL__EDGE -3
      for( int i = 0; i < numVerts; i++ ) {
          edgeVisited[i] = INIT_VAL__EDGE;
      }
      std::list<int> L__z = std::list<int>(0);
      for(int i=0; i<numVerts; i++){
        vector<int> neigh = meshdata->adjimm[i].myNeighbs;
        if (class_of_blob[i] == 0) {
          for (int n=0; n<neigh.size(); n++){
            if (class_of_blob[neigh[n]] == 1 ){
              if( 0 == count(L__z.begin(),L__z.end(), i) ) {
                L__z.push_back(i);
                Lz.push_back( i );
                edgeVisited[i] = 0;
              }
             
            }
          }
        }
      }

      //std::vector<int> edgeVisited = std::vector<int>(numVerts);
      std::list<int> L__p1 = std::list<int>(0);
      std::list<int> L__n1 = std::list<int>(0);
      for(int i=0; i<numVerts; i++){
        vector<int> neigh = meshdata->adjimm[i].myNeighbs;
        if (edgeVisited[i] == 0) {  // On L__z
          for (int n=0; n<neigh.size(); n++){
            if (edgeVisited[neigh[n]] == INIT_VAL__EDGE && class_of_blob[neigh[n]] == 0 ){
              if (0 == count(L__p1.begin(), L__p1.end(), neigh[n])){
                L__p1.push_back(neigh[n]);
                Lp1.push_back( neigh[n] );
                edgeVisited[neigh[n]] = 1;
              }
            }
            else if (edgeVisited[neigh[n]] == INIT_VAL__EDGE && class_of_blob[neigh[n]] == 1 ){
              if (0 == count(L__n1.begin(), L__n1.end(), neigh[n])){
                L__n1.push_back(neigh[n]);
                Ln1.push_back( neigh[n] );
                edgeVisited[neigh[n]] = -1;
              }
            }
          }
        }
      }
      std::list<int> L__p2 = std::list<int>(0);
      std::list<int> L__n2 = std::list<int>(0);
      for(int i=0; i<numVerts; i++){
        vector<int> neigh = meshdata->adjimm[i].myNeighbs;
        if (edgeVisited[i] == 1) {  // On L__p1
          for (int n=0; n<neigh.size(); n++){
            if (edgeVisited[neigh[n]] == INIT_VAL__EDGE && class_of_blob[neigh[n]] == 0 ){
              if (0 == count(L__p2.begin(), L__p2.end(), neigh[n])){
                L__p2.push_back(neigh[n]);
                Lp2.push_back( neigh[n] );
                edgeVisited[neigh[n]] = 2;
              }
            }
          }
        }
        else if(edgeVisited[i] == -1) {  // On L__n1
          for (int n=0; n<neigh.size(); n++){
            if (edgeVisited[neigh[n]] == INIT_VAL__EDGE && class_of_blob[neigh[n]] == 1 ){
              if (0 == count(L__n2.begin(), L__n2.end(), neigh[n])){
                L__n2.push_back(neigh[n]);
                Ln2.push_back( neigh[n] );
                edgeVisited[neigh[n]] = -2;
              }
            }
          }
        }
      }
  this->L_z = Lz;
  this->L_n1 =  Ln1;
  this->L_p1 =  Lp1;
  this->L_n2 =  Ln2;
  this->L_p2 =  Lp2;


  return Lz;
}



vector<int> SparseFieldLS::Evolve(int its )
{
    double cfl = 0.15;
    int smooth_its = 1;
    for( int its_ = 0; its_ < its; its_++ ) {




    valarray<double> F = energy->getforce( L_z, L_p1, L_n1, phi );
    for( int i = 0; i < F.size(); i++ ) phi[ L_z[i] ] += dDirection*cfl * F[i];
    if( (its % 5 == 0) || (its_ % (its-1) == 0) ) {
    EvolveRecalc( ); // brute force update...
        continue;
    }
       


    vector<int> S0(0); // entering zero LS
    vector<int> Sp(0); // entering +1 level
    vector<int> Sn(0); // entering -1 level
    vector<int> Sp1(0); // entering +2 level
    vector<int> Sn1(0); // entering -2 level
    vector<int> Sp2(0); // to be dropped entirely
    vector<int> Sn2(0); // to be dropped entirely


    // 2. find where zero layer exceeds [-0.5,0.5]
    
    for( int i = 0; i < L_z.size(); i++ ) {
      int idx = L_z[i];
      if( phi[idx] >= 0.5 )
        Sp.push_back( idx );
      if( phi[idx] <= -0.5 )
        Sn.push_back( idx );
    }



    // 3. visit grid points in layers L_p1, L_n1 and update grid point
    // values by adding/subtracting one unit from next inner layer.
    // if multiple neighbors, use the closest one:
    
    

    for( int i = 0; i < L_p1.size(); i++ ) {
      int idx = L_p1[i];
      vector<int>* neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      valarray<double> vals(neigh->size()); // get neighbors of this point in L_p1
      for( int k = 0; k < vals.size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      bool bHasL_zneigh = false;
      for( int k = 0; k < vals.size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
        if( point_type[neighidx] == 0 ) // found it in L_z
          bHasL_zneigh = true;
      }
      if( !bHasL_zneigh ) { // i have no zero neighbors, // therefore i am no longer eligble for L_p1 membership
        Sp1.push_back( idx );
      }
      else {
        for( int k = 0; k < vals.size(); k++ ) {
          int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
          if( point_type[neighidx] != 0 )
            vals[k] = 1.0;
        }
        double oldval = phi[idx];
        double newval = vals.min() + 1.0;
        phi[idx] = newval;
        // if this new value is outside allowed range, move it to appropriate S layer
        if( newval <= 0.5 )
          S0.push_back(idx);
        else if( newval > 1.5 )
          Sp1.push_back(idx);
      }    
    }
    for( int i = 0; i < L_n1.size(); i++ ) {
      int idx = L_n1[i];
      vector<int>* neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      valarray<double> vals(neigh->size());
      for( int k = 0; k < vals.size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      bool bHasL_zneigh = false;
      for( int k = 0; k < vals.size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
        if( point_type[neighidx] == 0 ) // found it in L_z
          bHasL_zneigh = true;
      }
      if( !bHasL_zneigh ) { // i have no zero neighbors, 
        // therefore i am no longer eligble for L_n1 membership
        Sn1.push_back( idx );
      }
      else {
        for( int k = 0; k < vals.size(); k++ ) {
          int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
          if( point_type[neighidx] != 0 ) // found it in L_z
            vals[k] = -1.0;
        }
        double oldval = phi[idx];
        double newval = vals.max() - 1.0;
        phi[idx] = newval;
        // if this new value is outside allowed range, move it to appropriate S layer
        if( newval >= -0.5 )
          S0.push_back(idx);
        else if( newval < -1.5 )
          Sn1.push_back(idx);

      }
    }

    
    for( int i = 0; i < L_p2.size(); i++ ) {
      int idx = L_p2[i];
      vector<int>* neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      valarray<double> vals(neigh->size()); // get neighbors of this point in L_p1
      for( int k = 0; k < vals.size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      bool bHasL_p1neigh = false;
      for( int k = 0; k < vals.size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
        if( point_type[neighidx] == 1 ) // found it in L_z
          bHasL_p1neigh = true;
      }
      if( !bHasL_p1neigh ) { // i have no L_p1 neighbors therefore i am no longer eligble for L_p2 membership
        Sp2.push_back( idx );
      }
      else {
        for( int k = 0; k < vals.size(); k++ ) {
          int neighidx = (*neigh)[k]; // 
          if( point_type[neighidx] != 1 ) 
            vals[k] = 5.0;
        }
        double oldval = phi[idx];
        double newval = vals.min() + 1.0;
        phi[idx] = newval;
        // if this new value is outside allowed range, move it to appropriate S layer
        if( newval <= 1.5 )
          Sp.push_back(idx);
        else if( newval > 2.5 )
          Sp2.push_back(idx);
      }    
    }
    for( int i = 0; i < L_n2.size(); i++ ) {
      int idx = L_n2[i];
      vector<int>* neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      valarray<double> vals(neigh->size());
      for( int k = 0; k < vals.size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      bool bHasL_n1neigh = false;
      for( int k = 0; k < vals.size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_n1 don't use it for the max()
        if( point_type[neighidx] == -1 ) 
          bHasL_n1neigh = true; // found it in L_n1
          
      }
      if( !bHasL_n1neigh ) { // i have no L_n1 neighbors therefore i am no longer eligble for L_n2 membership
        Sn2.push_back( idx );
      }
      else {
        for( int k = 0; k < vals.size(); k++ ) {
          int neighidx = (*neigh)[k]; // if this is not in L_n1 don't use it for the max()
          if( point_type[neighidx] != -1 ) // did not find it in L_n1
            vals[k] = -5.0;
        }
        double oldval = phi[idx];
        double newval = vals.max() - 1.0;
        phi[idx] = newval;
        if( newval >= -1.5 )
          Sn.push_back(idx);
        else if( newval < -2.5 )
          Sn2.push_back(idx);
      }
    }
    
    // erase from L_z the points that just got stored
    
    DropIdx( Sp, Sn, L_z );
    AppendIdx( S0, L_z);

    DropIdx( Sp, Sp2, L_p2 );
    AppendIdx( Sp1, L_p2 );
    DropIdx( Sn, Sn2, L_n2 );
    AppendIdx( Sn1, L_n2 );

    DropIdx( S0, Sp1, L_p1 );
    AppendIdx( Sp, L_p1 );
    DropIdx( S0, Sn1, L_n1 );
    AppendIdx( Sn, L_n1 );

      // update point_type
    for( int i = 0; i < L_z.size(); i++ ) {
      point_type[ L_z[i] ] = 0;
    }
    for( int i = 0; i < L_p1.size(); i++ ) {
      point_type[ L_p1[i] ] = 1;
    }
    for( int i = 0; i < L_n1.size(); i++ ) {
      point_type[ L_n1[i] ] = -1;
    }
    for( int i = 0; i < L_p2.size(); i++ ) {
      point_type[ L_p2[i] ] = 2;
    }
    for( int i = 0; i < L_n2.size(); i++ ) {
      point_type[ L_n2[i] ] = -2;
    }
    for( int i = 0; i < Sp2.size(); i++ ) {
      point_type[ Sp2[i] ] = 3;
    }
    for( int i = 0; i < Sn2.size(); i++ ) {
      point_type[ Sn2[i] ] = -3;
    }

    // grab new points for outermost layers from neighbors of L_p1, L_n1
    for( int i = 0; i < L_p1.size(); i++ ) {
      int idx = L_p1[i];
      vector<int>* neigh = &(meshdata->adjimm[idx].myNeighbs);
      int numneigh = neigh->size();
      for( int j = 0; j < numneigh; j++ ) {
        int neighidx = (*neigh)[j]; // if its not in L_p1 or L_z grab it!
        if( point_type[neighidx] == 3 ) {
          L_p2.push_back( neighidx);
          point_type[neighidx] = 2;
          //double val = phi[idx]+1;
          //phi[ neighidx ] = val;
        }
      }
    }
    for( int i = 0; i < L_n1.size(); i++ ) {
      int idx = L_n1[i];
      vector<int>* neigh = &(meshdata->adjimm[idx].myNeighbs);
      int numneigh = neigh->size();
      for( int j = 0; j < numneigh; j++ ) {
        int neighidx = (*neigh)[j]; // if its not in L_n1 or L_z grab it!
        if( point_type[neighidx] == -3 ) {
          L_n2.push_back( neighidx);
          //double val = phi[idx]-1;
          //phi[ neighidx ] = val;
          point_type[neighidx] = -2;
        }
      }
    }

    // discard redundant verts
    SelfUnion( L_n2 );
    SelfUnion( L_p2 );
    SelfUnion( L_n1 );
    SelfUnion( L_p1 );
    SelfUnion( L_z );
  }
  

#ifdef DEF_USE_CV_SFLS
   vector<int>* neigh;

  vector<int> S0(0); // entering zero LS
  vector<int> Sp(0); // entering +1 level
  vector<int> Sn(0); // entering -1 level
  vector<int> Sp1(0); // entering +2 level
  vector<int> Sn1(0); // entering -2 level
  vector<int> Sp2(0); // to be dropped entirely
  vector<int> Sn2(0); // to be dropped entirely

  vector<double> placed = vector<double>(phi.size());
  
#define MAX_NEIGH 20
  valarray<double> vals(MAX_NEIGH); // big enough to hold all immediate neighbors
  int numVerts = phi.size(); 
  for( int i =0; i < numVerts; i++ ) {
    if( phi[i] < 0 )
      point_type[i] = -3;
    else
      point_type[i] = 3;
  }
  // cvCopy( myimg, output );

  int numel = this->meshdata->adjimm.size();
  for( int i = 0; i < numel; i++ ) {
    if( phi[i] < 0 )
      continue;
    bool bHasNegNeigh = false;
    neigh = &(meshdata->adjimm[i].myNeighbs);
    int numneigh = neigh->size();
    for( int j = 0; j < numneigh; j++ ) {
      if( phi[ (*neigh)[j] ] < 0 )
        bHasNegNeigh = true;
    }
    if( bHasNegNeigh ) {
      L_z.push_back(i);
      //// int col = i % output->width;
      //// int row = i / output->width;
      // CV( output, row, col, cvScalar(0) );
      placed[i] = 1.0;
      phi[i] = 0.001;
      point_type[i] = 0;
    }

  }

  // step 2. build L_p1, L_n1 the one-away layers from L_z
  for( int i = 0; i < L_z.size(); i++ ) {
    int idx = L_z[i];
    neigh = &(meshdata->adjimm[idx].myNeighbs);
    int numneigh = neigh->size();
    for( int j = 0; j < numneigh; j++ ) {
      int neighidx = (*neigh)[j];
      if( phi[ neighidx ] < 0 && placed[neighidx] < 0.5 ) {
        L_n1.push_back( neighidx);
        placed[neighidx] = 1.0;
        // int col = neighidx % output->width;
        // int row = neighidx / output->width;
        // CV( output, row, col, cvScalar(255) );
        phi[neighidx] = -1.0;
        point_type[i] = -1;
      }
      if( phi[ neighidx ] >= 0 && placed[neighidx] < 0.5 ) {
        L_p1.push_back( neighidx );
        placed[neighidx] = 1.0;
        // int col = neighidx % output->width;
        // int row = neighidx / output->width;
        // CV( output, row, col, cvScalar(255) );
        phi[neighidx] = 1.0;
        point_type[i] = 1;
      }
    }
  }

  // step 3. build L_p2, L_n2 the two-away layers from L_z
  for( int i = 0; i < L_p1.size(); i++ ) {
    int idx = L_p1[i];
    neigh = &(meshdata->adjimm[idx].myNeighbs);
    int numneigh = neigh->size();
    for( int j = 0; j < numneigh; j++ ) {
      int neighidx = (*neigh)[j];
      if( phi[ neighidx ] > 0 && placed[neighidx] < 0.5 ) {
        L_p2.push_back( neighidx);
        placed[neighidx] = 1.0;
        // int col = neighidx % output->width;
        // int row = neighidx / output->width;
        // CV( output, row, col, cvScalar(100) );
        phi[neighidx] = 2.0;
        point_type[i] = 2;
      }
      /*if( phi[ neighidx ] < 0 ) {
        int impossible = 1;
      }*/
    }
  }
  for( int i = 0; i < L_n1.size(); i++ ) {
    int idx = L_n1[i];
    neigh = &(meshdata->adjimm[idx].myNeighbs);
    int numneigh = neigh->size();
    for( int j = 0; j < numneigh; j++ ) {
      int neighidx = (*neigh)[j];
      if( phi[ neighidx ] < 0 && placed[neighidx] < 0.5 ) {
        L_n2.push_back( neighidx);
        placed[neighidx] = 1.0;
        // int col = neighidx % output->width;
        // int row = neighidx / output->width;
        // CV( output, row, col, cvScalar(100) );
        phi[neighidx] = -2.0;
        point_type[i] = -2;
      }
      /*if( phi[ neighidx ] > 0 ) {
        int impossible = 1;
      }*/
    }
  }

  for( int i = 0; i < numVerts; i++ ) {
    // int col = i % w;
    // int row = i / w;
    if( phi[i] > 0 && placed[i] < 0.5 ) {
      phi[i] = 3.0;
      placed[i] = 1.0;
      // CV( output, row, col, cvScalar(255) );
    }
    else if ( phi[i] < 0 && placed[i] < 0.5 ) {
      phi[i] = -3.0;
      placed[i] = 1.0;
      // CV( output, row, col, cvScalar(0) );
    }
  }




  double prev_energy = 0.0;
  for( int ii = 0; ii < its; ii++ ) {

    S0.clear();
    Sp.clear(); 
    Sn.clear(); 
    Sp1.clear();
    Sn1.clear();
    Sp2.clear();
    Sn2.clear();

        
    double current_energy = this->energy->eval_energy(L_z);
    double delta = abs( current_energy - prev_energy );
    //cout<<"delta is: "<<delta<<"\n";
    prev_energy = current_energy;
    /*if( delta < 1000 && ii > 100 )
      return delta;*/

    // 1. for each active grid point x_i:
    // a. compute local geometry of level set
    // b. compute net change of u_x from internal and external forces
    // (encapsulated in getforce() function )

    

    valarray<double> F = energy->getforce(L_z, L_p1, L_p2, phi);//  >GetSpeed( L_z, phi );
    
    double dDir = this->dDirection;
    double cfl = 0.4;
    for( int i = 0; i < F.size(); i++ ) phi[ L_z[i] ] += dDir*cfl * F[i];
    
    // 2. find where zero layer exceeds [-0.5,0.5]
    
    for( int i = 0; i < L_z.size(); i++ ) {
      int idx = L_z[i];
      if( phi[idx] >= 0.5 )
        Sp.push_back( idx );
      if( phi[idx] <= -0.5 )
        Sn.push_back( idx );
    }



    // 3. visit grid points in layers L_p1, L_n1 and update grid point
    // values by adding/subtracting one unit from next inner layer.
    // if multiple neighbors, use the closest one:
    
    

    for( int i = 0; i < L_p1.size(); i++ ) {
      int idx = L_p1[i];
      neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      //valarray<double> vals(neigh->size()); // get neighbors of this point in L_p1
      for( int k = 0; k < neigh->size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      for( int k = neigh->size(); k < vals.size(); k++ ) {
        vals[k] = vals[k-1];
      }
      bool bHasL_zneigh = false;
      for( int k = 0; k < neigh->size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
        if( point_type[neighidx] == 0 ) // found it in L_z
          bHasL_zneigh = true;
      }
      if( !bHasL_zneigh ) { // i have no zero neighbors, // therefore i am no longer eligble for L_p1 membership
        Sp1.push_back( idx );
      }
      else {
        for( int k = 0; k < neigh->size(); k++ ) {
          int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
          if( point_type[neighidx] != 0 )
            vals[k] = 1.0;
        }
        double oldval = phi[idx];
        double newval = vals.min() + 1.0;
        phi[idx] = newval;
        // if this new value is outside allowed range, move it to appropriate S layer
        if( newval <= 0.5 )
          S0.push_back(idx);
        else if( newval > 1.5 )
          Sp1.push_back(idx);
      }    
    }
    for( int i = 0; i < L_n1.size(); i++ ) {
      int idx = L_n1[i];
      neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      //valarray<double> vals(neigh->size());
      for( int k = 0; k < neigh->size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      for( int k = neigh->size(); k < vals.size(); k++ ) {
        vals[k] = vals[k-1];
      }
      bool bHasL_zneigh = false;
      for( int k = 0; k < neigh->size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
        if( point_type[neighidx] == 0 ) // found it in L_z
          bHasL_zneigh = true;
      }
      if( !bHasL_zneigh ) { // i have no zero neighbors, 
        // therefore i am no longer eligble for L_n1 membership
        Sn1.push_back( idx );
      }
      else {
        for( int k = 0; k < neigh->size(); k++ ) {
          int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
          if( point_type[neighidx] != 0 ) // found it in L_z
            vals[k] = -1.0;
        }
        double oldval = phi[idx];
        double newval = vals.max() - 1.0;
        phi[idx] = newval;
        // if this new value is outside allowed range, move it to appropriate S layer
        if( newval >= -0.5 )
          S0.push_back(idx);
        else if( newval < -1.5 )
          Sn1.push_back(idx);

      }
    }

    
    for( int i = 0; i < L_p2.size(); i++ ) {
      int idx = L_p2[i];
      neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      //valarray<double> vals(neigh->size()); // get neighbors of this point in L_p1
      
      for( int k = 0; k < neigh->size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      for( int k = neigh->size(); k < vals.size(); k++ ) {
        vals[k] = vals[k-1];
      }
      bool bHasL_p1neigh = false;
      for( int k = 0; k < neigh->size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_z don't use it for the max()
        if( point_type[neighidx] == 1 ) // found it in L_z
          bHasL_p1neigh = true;
      }
      if( !bHasL_p1neigh ) { // i have no L_p1 neighbors therefore i am no longer eligble for L_p2 membership
        Sp2.push_back( idx );
      }
      else {
        for( int k = 0; k < neigh->size(); k++ ) {
          int neighidx = (*neigh)[k]; // 
          if( point_type[neighidx] != 1 ) 
            vals[k] = 5.0;
        }
        double oldval = phi[idx];
        double newval = vals.min() + 1.0;
        phi[idx] = newval;
        // if this new value is outside allowed range, move it to appropriate S layer
        if( newval <= 1.5 )
          Sp.push_back(idx);
        else if( newval > 2.5 )
          Sp2.push_back(idx);
      }    
    }
    for( int i = 0; i < L_n2.size(); i++ ) {
      int idx = L_n2[i];
      neigh = &(meshdata->adjimm[ idx ].myNeighbs);
      //valarray<double> vals(neigh->size());
      for( int k = 0; k < neigh->size(); k++ ) { 
        vals[k] = phi[ (*neigh)[k] ];
      }
      for( int k = neigh->size(); k < vals.size(); k++ ) {
        vals[k] = vals[k-1];
      }
      bool bHasL_n1neigh = false;
      for( int k = 0; k < neigh->size(); k++ ) {
        int neighidx = (*neigh)[k]; // if this is not in L_n1 don't use it for the max()
        if( point_type[neighidx] == -1 ) 
          bHasL_n1neigh = true; // found it in L_n1
          
      }
      if( !bHasL_n1neigh ) { // i have no L_n1 neighbors therefore i am no longer eligble for L_n2 membership
        Sn2.push_back( idx );
      }
      else {
        for( int k = 0; k < neigh->size(); k++ ) {
          int neighidx = (*neigh)[k]; // if this is not in L_n1 don't use it for the max()
          if( point_type[neighidx] != -1 ) // did not find it in L_n1
            vals[k] = -5.0;
        }
        double oldval = phi[idx];
        double newval = vals.max() - 1.0;
        phi[idx] = newval;
        if( newval >= -1.5 )
          Sn.push_back(idx);
        else if( newval < -2.5 )
          Sn2.push_back(idx);
      }
    }
    
    // erase from L_z the points that just got stored
    
    DropIdx( Sp, Sn, L_z );
    AppendIdx( S0, L_z);

    DropIdx( Sp, Sp2, L_p2 );
    AppendIdx( Sp1, L_p2 );
    DropIdx( Sn, Sn2, L_n2 );
    AppendIdx( Sn1, L_n2 );

    DropIdx( S0, Sp1, L_p1 );
    AppendIdx( Sp, L_p1 );
    DropIdx( S0, Sn1, L_n1 );
    AppendIdx( Sn, L_n1 );

      // update point_type
    for( int i = 0; i < L_z.size(); i++ ) {
      point_type[ L_z[i] ] = 0;
    }
    for( int i = 0; i < L_p1.size(); i++ ) {
      point_type[ L_p1[i] ] = 1;
    }
    for( int i = 0; i < L_n1.size(); i++ ) {
      point_type[ L_n1[i] ] = -1;
    }
    for( int i = 0; i < L_p2.size(); i++ ) {
      point_type[ L_p2[i] ] = 2;
    }
    for( int i = 0; i < L_n2.size(); i++ ) {
      point_type[ L_n2[i] ] = -2;
    }
    for( int i = 0; i < Sp2.size(); i++ ) {
      point_type[ Sp2[i] ] = 3;
    }
    for( int i = 0; i < Sn2.size(); i++ ) {
      point_type[ Sn2[i] ] = -3;
    }

    // grab new points for outermost layers from neighbors of L_p1, L_n1
    for( int i = 0; i < L_p1.size(); i++ ) {
      int idx = L_p1[i];
      neigh = &(meshdata->adjimm[idx].myNeighbs);
      int numneigh = neigh->size();
      for( int j = 0; j < numneigh; j++ ) {
        int neighidx = (*neigh)[j]; // if its not in L_p1 or L_z grab it!
        if( point_type[neighidx] == 3 ) {
          L_p2.push_back( neighidx);
          point_type[neighidx] = 2;
          double val = phi[idx]+1;
          phi[ neighidx ] = val;
        }
      }
    }
    for( int i = 0; i < L_n1.size(); i++ ) {
      int idx = L_n1[i];
      neigh = &(meshdata->adjimm[idx].myNeighbs);
      int numneigh = neigh->size();
      for( int j = 0; j < numneigh; j++ ) {
        int neighidx = (*neigh)[j]; // if its not in L_n1 or L_z grab it!
        if( point_type[neighidx] == -3 ) {
          L_n2.push_back( neighidx);
          double val = phi[idx]-1;
          phi[ neighidx ] = val;
          point_type[neighidx] = -2;
        }
      }
    }

    // discard redundant verts
    SelfUnion( L_n2 );
    SelfUnion( L_p2 );
    SelfUnion( L_n1 );
    SelfUnion( L_p1 );
    SelfUnion( L_z );
  }
#endif 

  return L_z;

}
