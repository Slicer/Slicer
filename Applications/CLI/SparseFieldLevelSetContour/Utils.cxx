/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "Utils.h"

void Crash()
{
  void* foo = 0;
  free( foo );
}

void pkmult( const vector<double>& vecin, const vector<double>& mat, vector<double>& vecout )
{
  vecout[0] = mat[0] * vecin[0] + mat[1] * vecin[1] + mat[2] * vecin[2];
  vecout[1] = mat[3] * vecin[0] + mat[4] * vecin[1] + mat[5] * vecin[2];
  vecout[2] = mat[6] * vecin[0] + mat[7] * vecin[1] + mat[8] * vecin[2];
}

void pkmult( const valarray<double>& vecin, const vector<double>& mat, valarray<double>& vecout )
{
  vecout[0] = mat[0] * vecin[0] + mat[1] * vecin[1] + mat[2] * vecin[2];
  vecout[1] = mat[3] * vecin[0] + mat[4] * vecin[1] + mat[5] * vecin[2];
  vecout[2] = mat[6] * vecin[0] + mat[7] * vecin[1] + mat[8] * vecin[2];
}

void pkmult2( const valarray<double>& vecin, const valarray<double>& mat, valarray<double>& vecout )
{
  vecout[0] = mat[0] * vecin[0] + mat[1] * vecin[1] ;
  vecout[1] = mat[2] * vecin[0] + mat[3] * vecin[1] ;
}

void SelfUnion( vector<int>& vec )
{
  sort( vec.begin(), vec.end() );
  vec.erase( unique(vec.begin(), vec.end()),vec.end() );
}

std::vector<int> ListToSTDVector( const std::list<int>& ListIn_ )
{
  std::list<int> ListIn = ListIn_;
  int Len = ListIn.size();
  std::vector<int> res = std::vector<int>(Len);
  for( int i = 0; i < Len; i++ ) {
    int val = ListIn.front();
    ListIn.pop_front();
    res[i] = val;
  }
  return res;
}

void DropIdx( const vector<int>& L_zp, const vector<int>& L_zn, vector<int>& L_z )
{
  vector<int> L_z_(0);
  for( ::size_t i = 0; i < L_z.size(); i++ )
    {
    int num1 = count( L_zp.begin(), L_zp.end(), L_z[i] );
    int num2 = count( L_zn.begin(), L_zn.end(), L_z[i] );
    if( num1 + num2 == 0 )
      {
      L_z_.push_back( L_z[i] );
      }
    }
  L_z = L_z_;
}

void ProcessTriDataFiles( const vector<string>& filenames, valarray<int> &CurIdx,
                          valarray<double> &CurVertX, valarray<double> &CurVertY, valarray<double> &CurVertZ )
{
  string fileNameFaces = filenames[0];
  string fileNameVerts = filenames[1];

    cout<<"Reading from "<<fileNameFaces<<", "<<fileNameVerts<<"\n";

// read a comma separted value file that defines the vertices
// and indices into vertices for all faces

  char line[512];
  ifstream fin( fileNameFaces.c_str() );
  int numFaces = 0;

//cout<<"Start reading faces data..."<<fileNameFaces.c_str()<<"\n";
// TODO: check for endless loop here if file is not found or is messed up
  
  while( !fin.eof() )
    {
    fin>>line;
    numFaces++;
//cout<<numFaces;
//cout<<line<<"\n";
    }
  fin.close();

  cout<<"Done Reading faces data...";

  CurIdx   = valarray<int>( (numFaces-1)*3 );
  ifstream fin2( fileNameFaces.c_str() );

  string csTemp;

//cout<<"Reading face data...\n";
//cout<<"Creating object with "<<numFaces<<" triangles\n";
  int k = 0;
  int idx;
  while( !fin2.eof() )
    {
    fin2>>line;
    csTemp = line;
    if( csTemp.length() < 2 )
      break;
////cout<<csTemp.c_str()<<"\n";
    int comma1_idx = csTemp.find_first_of(',');
    string s1 = csTemp.substr(0,comma1_idx);
    idx = atoi(s1.c_str());
    (CurIdx)[ 3*k + 0 ] = idx-1;
    string rest1 = csTemp.substr(comma1_idx+1,csTemp.length());
    int comma2_idx = rest1.find_first_of(',');
    string s2 = rest1.substr(0,comma2_idx);
    idx = atoi(s2.c_str());
    (CurIdx)[ 3*k + 1 ] = idx-1;
    string rest2 = rest1.substr(comma2_idx+1,rest1.length());
    idx = atoi(rest2.c_str());
    (CurIdx)[ 3*k + 2 ] = idx-1;
    k++;
    }

  fin2.close();
  ifstream vin( fileNameVerts.c_str() );
//cout<<"Counting vertices...\n";
  int numVerts = 0;
  k = 0;
  while( !vin.eof() )
    {
    vin>>line;
    numVerts++;
    }
//numVerts--; // last line is blank
  vin.close();
  cout<<"Done counting vertices \n";
//double* tempnorms = new double[3*numVerts];

  CurVertX = valarray<double>(numVerts-1);
  CurVertY = valarray<double>(numVerts-1);
  CurVertZ = valarray<double>(numVerts-1);

  ifstream vin2( fileNameVerts.c_str() );
//double* tempverts = new double[3*numVerts];
//cout<<"Making object with "<<3*numVerts<<" vertices and normals...\n";
//double val;
  k = 0;
  double val;
  while( !vin2.eof() )
    {
    vin2>>line;
    csTemp = line;
    if( csTemp.length() < 2 )
      break;
////cout<<csTemp.c_str()<<"\n";
    int comma1_idx = csTemp.find_first_of(',');
    string s1 = csTemp.substr(0,comma1_idx);
    val = atof(s1.c_str());
    CurVertX[ k ] = val;
    string rest1 = csTemp.substr(comma1_idx+1,csTemp.length());
    int comma2_idx = rest1.find_first_of(',');
    string s2 = rest1.substr(0,comma2_idx);
    val = atof(s2.c_str());
    CurVertY[ k ] = val;
    string rest2 = rest1.substr(comma2_idx+1,rest1.length());
    val = atof(rest2.c_str());
    CurVertZ[ k ] = val;
    k++;
    }
  vin2.close();

/*::size_t i;
for( i = 0; i < (*CurVert).size(); i++ )
cout<< (*CurVert)[i] <<", ";*/

  numFaces--;
  numVerts--;

}

void ReadNormals( const string& filename, valarray<double>& nx, valarray<double>& ny, valarray<double>& nz )
{
// don't need this, the normals are pre-allocated before getting here
  ifstream vin( filename.c_str() );
  int numVerts = 0;
  char line[512];
  string csTemp;
  while( !vin.eof() )
    {
    vin>>line;
    numVerts++;
    }
  vin.close();
  nx = valarray<double>(numVerts-1);
  ny = valarray<double>(numVerts-1);
  nz = valarray<double>(numVerts-1);
  ifstream vin2( filename.c_str() );
  double val;
  int k = 0;
  while( !vin2.eof() )
    {
    vin2>>line;
    csTemp = line;
    if( csTemp.length() < 2 )
      break;
////cout<<csTemp.c_str()<<"\n";
    int comma1_idx = csTemp.find_first_of(',');
    string s1 = csTemp.substr(0,comma1_idx);
    val = atof(s1.c_str());
    nx[ k ] = val;
    string rest1 = csTemp.substr(comma1_idx+1,csTemp.length());
    int comma2_idx = rest1.find_first_of(',');
    string s2 = rest1.substr(0,comma2_idx);
    val = atof(s2.c_str());
    ny[ k ] = val;
    string rest2 = rest1.substr(comma2_idx+1,rest1.length());
    val = atof(rest2.c_str());
    nz[ k ] = val;
    k++;
    }
  vin2.close();

// now normalize the normals
//(for example since matlab apparently doesn't give them unit length)
  for( ::size_t i = 0; i < nx.size(); i++ )
    {
    double arg = nx[i]*nx[i] + ny[i]*ny[i] + nz[i]*nz[i];
    double norm = sqrt( arg );
    if( norm == 0.0 )
      {
      Crash();
      }
    nx[i] /= norm;
    ny[i] /= norm;
    nz[i] /= norm;
    }
  
}
