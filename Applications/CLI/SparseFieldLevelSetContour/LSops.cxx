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

void SparseFieldLS::SelfUnion( std::list<int>& vec )
{
  vec.sort();
  vec.erase( std::unique(vec.begin(), vec.end()),vec.end() );
}

void SparseFieldLS::SelfUnion( std::vector<int>& vec )
{ 
  std::sort( vec.begin(), vec.end() );
  vec.erase( std::unique(vec.begin(), vec.end()),vec.end() );
}

void SparseFieldLS::AppendIdx( const std::vector<int>& src, std::vector<int>& dst ) {
  for(unsigned int i = 0; i < src.size(); i++ )
    {
    dst.push_back( src[i] );
    }
  SelfUnion( dst );
}

void SparseFieldLS::DropIdx( const std::vector<int>& L_zp, const std::vector<int>& L_zn, std::vector<int>& L_z )
{
  std::vector<int> L_z_(0);
  for(unsigned int i = 0; i < L_z.size(); i++ ) {
    int num1 = std::count( L_zp.begin(), L_zp.end(), L_z[i] );
    int num2 = std::count( L_zn.begin(), L_zn.end(), L_z[i] );
    if( num1 + num2 == 0 )
      L_z_.push_back( L_z[i] );
  }
  L_z = L_z_;

}

void SparseFieldLS::DropIdx( const std::list<int>& L_zp, const std::list<int>& L_zn, std::list<int>& L_z )
{
  std::list<int> L_z_(0);
  while( 0 < L_z.size() ) {
    int idx  = L_z.front();
    L_z.pop_front( );
    int num1 = std::count( L_zp.begin(), L_zp.end(), idx );
    int num2 = std::count( L_zn.begin(), L_zn.end(), idx );
    if( num1 + num2 == 0 )
      L_z_.push_back( idx );
  }
  L_z = L_z_;

}


int ii = 1;
bool SparseFieldLS::InitSphere()
{
  return false;
}


std::vector<int> SparseFieldLS::Evolve(int its )
{
    double cfl = 0.2;
    
    std::vector<int> Lz  ;
    std::vector<int> Lp1 ;
    std::vector<int> Ln1 ;
    std::vector<int> Lp2 ;
    std::vector<int> Ln2 ;

    std::list<int> Sn1;
    std::list<int> Sp1;
    std::list<int> Sn2;
    std::list<int> Sp2;
    std::list<int> Sz;

    for( int its_ = 0; its_ < its; its_++ ) {  
      Lz = ListToSTDVector( L_z );
      Lp1 = ListToSTDVector( L_p1 );
      Ln1 = ListToSTDVector( L_n1 );
      Lp2 = ListToSTDVector( L_p2 );
      Ln2 = ListToSTDVector( L_n2 );// update val along LZ
      
      if( Lz.size() < 1 )
        return Lz;
      //cout<<Lz.size()<<" \n";

      Sn1.clear();
      Sp1.clear();
      Sz.clear();
      Sp2.clear();
      Sn2.clear();

      std::valarray<double> F = energy->getforce( L_z, L_p1, L_n1, phi );
      for( ::size_t i = 0; i < F.size(); i++ ) phi[ Lz[i] ] += dDirection*cfl * F[i] ;
     
      /*
       Sn1 = Lz( ( phi( Lz ) <= -0.5 ) );
       Sp1 = Lz( ( phi( Lz ) >= 0.5 ) ); 
       Lz( phi(Lz)<= -0.5 ) = [];
       Lz( phi(Lz)>= 0.5 ) = []; 
     */
      ::size_t LzN = L_z.size();
      for( ::size_t i = 0; i < LzN; i++ ) {
        L_z.pop_front( );
        if( phi[ Lz[i] ] <= -0.5 ) {
          Sn1.push_back( Lz[i] );
          //point_type[ Lz[i] ] = -1;
        }
        else if( phi[ Lz[i] ] >= 0.5 ) {
          Sp1.push_back( Lz[i] ); 
          //point_type[ Lz[i] ] = 1;
        }
        else 
          L_z.push_back( Lz[i] );
      }

      ::size_t Ln1N = L_n1.size();
     
      for( ::size_t k = 0; k < Ln1N; k++ ) {
        L_n1.pop_front( );
        std::vector<int>* neigh = &(meshdata->adjimm[ Ln1[k] ].myNeighbs);
        int maxid     = -1; int maxlabel  = -1; int maxphiid  = -1; double phimax = 0.0;
        for( ::size_t i = 0 ; i < neigh->size(); i++ ) {
          if( this->point_type[ (*neigh)[i] ] > maxlabel ) {
            maxlabel = this->point_type[ (*neigh)[i] ];
            maxid    = (*neigh)[i]; }
          if( this->phi[ (*neigh)[i] ] > phimax ) {
            phimax   = this->phi[ (*neigh)[i] ];
            maxphiid = (*neigh)[i]; }
        }
        if( maxlabel < 0 ) {
          Sn2.push_back( Ln1[k] );
          //point_type[ Ln1[k] ] = -2;
        }
        else {
          phi[ Ln1[k] ] = phimax - 1.0;
          if( phi[Ln1[k]] >= -0.5 )
            Sz.push_back(Ln1[k]);
          else if( phi[Ln1[k]] < -1.5 ) {
            //point_type[ Ln1[k] ] = -2;
            Sn2.push_back( Ln1[k] );
          }
          else
            L_n1.push_back(Ln1[k]);
        }
      }
      
      ::size_t Lp1N = L_p1.size();
      for( ::size_t k = 0; k < Lp1N; k++ ) {
        L_p1.pop_front( );
        std::vector<int>* neigh = &(meshdata->adjimm[ Lp1[k] ].myNeighbs);
       int minid     = 4; int minlabel  = 4; int minphiid  = -1; double phimin = 4.0;
        for( ::size_t i = 0 ; i < neigh->size(); i++ ) {
          if( this->point_type[ (*neigh)[i] ] < minlabel ) {
            minlabel = this->point_type[ (*neigh)[i] ];
            minid    = (*neigh)[i]; }
          if( this->phi[ (*neigh)[i] ] < phimin ) {
            phimin   = this->phi[ (*neigh)[i] ];
            minphiid = (*neigh)[i]; }
        }
        if( minlabel > 0 )
          Sp2.push_back( Lp1[k] );
        else {
          phi[ Lp1[k] ] = phimin + 1.0;
          if( phi[Lp1[k]] <= 0.5 ) {
            Sz.push_back(Lp1[k]);
            //point_type[ Lp1[k] ] = 0;
          }
          else if( phi[Lp1[k]] >= 1.5 ) {
            Sp2.push_back( Lp1[k] );
            //point_type[ Lp1[k] ] = 2;
          }
          else
            L_p1.push_back(Lp1[k]);
        }
      }



        // 2nd-Layers:
      ::size_t Ln2N = L_n2.size();
     for( ::size_t k = 0; k < Ln2N; k++ ) {
        L_n2.pop_front( );
        std::vector<int>* neigh = &(meshdata->adjimm[ Ln2[k] ].myNeighbs);
        int maxid     = -4; int maxlabel  = -4; int maxphiid  = -4; double phimax = -4.0;
        for( ::size_t i = 0 ; i < neigh->size(); i++ ) {
          if( this->point_type[ (*neigh)[i] ] > maxlabel ) {
            maxlabel = this->point_type[ (*neigh)[i] ];
            maxid    = (*neigh)[i]; }
          if( this->phi[ (*neigh)[i] ] > phimax ) {
            phimax   = this->phi[ (*neigh)[i] ];
            maxphiid = (*neigh)[i]; }
        }
        if( maxlabel < -1 ) {
          phi[ Ln2[k] ] = -3.0;
          point_type[ Ln2[k] ] = -3;
        }
        else {
          phi[ Ln2[k] ] = phimax - 1.0;
          if( phi[Ln2[k]] >= -1.5 )
            Sn1.push_back(Ln2[k]);
          else if( phi[Ln2[k]] < -2.5 ) {
            phi[  Ln2[k] ] = -3.0;
            point_type[  Ln2[k] ] = -3;
          }
          else
            L_n2.push_back(Ln2[k]);
        }
      }
      
       ::size_t Lp2N = L_p2.size();
     for( ::size_t k = 0; k < Lp2N; k++ ) {
        L_p2.pop_front( );
        std::vector<int>* neigh = &(meshdata->adjimm[ Lp2[k] ].myNeighbs);
        int minid     = 4; int minlabel  = 4; int minphiid  = -1; double phimin = 4.0;
        for( ::size_t i = 0 ; i < neigh->size(); i++ ) {
          if( this->point_type[ (*neigh)[i] ] < minlabel ) {
            minlabel = this->point_type[ (*neigh)[i] ];
            minid    = (*neigh)[i]; }
          if( this->phi[ (*neigh)[i] ] < phimin ) {
            phimin   = this->phi[ (*neigh)[i] ];
            minphiid = (*neigh)[i]; }
        }
        if( minlabel > 1 ) {
          phi[ Lp2[k] ] = 3.0;
          point_type[ Lp2[k] ] = 3;
        }
        else {
          phi[ Lp2[k] ] = phimin + 1.0;
          if( phi[Lp2[k]] <= 1.5 )
            Sp1.push_back(Lp2[k]);
          else if( phi[Lp2[k]] > 2.5 ) {
            phi[  Lp2[k] ] = 3.0;
            point_type[  Lp2[k] ] = 3;
          }
          else
            L_p2.push_back(Lp2[k]);
        }
      }

      // The 'changing type' points have been removed!
      Lz  = ListToSTDVector( L_z );
      Lp1 = ListToSTDVector( L_p1 );
      Ln1 = ListToSTDVector( L_n1 );
      Lp2 = ListToSTDVector( L_p2 );
      Ln2 = ListToSTDVector( L_n2 );

      for( unsigned int k = 0; k < Lz.size(); k++ )
        {
        point_type[Lz[k]] = 0;
        }
     
      if( its_%50 == 0 )
        std::cout<<"LZ state: "<<Sz.size()<<","<<L_z.size()<<"\n";
      while( Sz.size() > 0 ) {
        int idx = Sz.front();
        L_z.push_back( idx );
        Sz.pop_front();
        point_type[idx] = 0;
      }
      
     // cout<<Sp1.size()<<","<<L_p1.size()<<"\n";
      //cout<<Sn1.size()<<","<<L_n1.size()<<"\n";
      while( Sn1.size() > 0 ) {
        int idx = Sn1.front();
        Sn1.pop_front( );
        std::vector<int>* neigh = &(meshdata->adjimm[ idx ].myNeighbs);
        for(unsigned int i = 0; i < neigh->size(); i++ ) {
          if( point_type[(*neigh)[i]] == -3 ) {
            phi[ (*neigh)[i] ] =  phi[idx] - 1.0;
            Sn2.push_back( (*neigh)[i] );
          }
        }
        L_n1.push_back(idx);
        point_type[idx] = -1;
      }
      while( Sp1.size() > 0 ) {
        int idx = Sp1.front();
        Sp1.pop_front( );
        std::vector<int>* neigh = &(meshdata->adjimm[ idx ].myNeighbs);
        for(unsigned int i = 0; i < neigh->size(); i++ ) {
          if( point_type[(*neigh)[i]] == 3 ) {
            phi[ (*neigh)[i] ] =  phi[idx] + 1.0;
            Sp2.push_back( (*neigh)[i] );
          }
        }
        L_p1.push_back(idx);
        point_type[idx] = 1;
      }
      
      DropIdx( Sp2, Sp2, L_p1 );
      DropIdx( Sn2, Sn2, L_n1 );

     // cout<<Sp2.size()<<","<<L_p2.size()<<"\n";
     // cout<<Sn2.size()<<","<<L_n2.size()<<"\n";
      while( Sn2.size() > 0 ) {
        int idx = Sn2.front();
        Sn2.pop_front( );
        L_n2.push_back(idx);
        point_type[idx] = -2;
      }
      while( Sp2.size() > 0 ) {
        int idx = Sp2.front();
        Sp2.pop_front( );
        L_p2.push_back(idx);
        point_type[idx] = 2;
      }

      
      
      
          
      SelfUnion( L_z );
      SelfUnion( L_n1 );
      SelfUnion( L_p1 );
      SelfUnion( L_n2 );
      SelfUnion( L_p2 );

    }
    
    return Lz;
}
