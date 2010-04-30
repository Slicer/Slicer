/*=========================================================================

  Program:   SparseFieldLevelSetContour
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include <stdlib.h>
#include <valarray>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <list>

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

using namespace std;

void ProcessTriDataFiles( const vector<string>& filenames, valarray<int> &CurIdx,
valarray<double> &CurVertX, valarray<double> &CurVertY, valarray<double> &CurVertZ );

void ReadNormals( const string& filename, valarray<double>& nx, valarray<double>& ny, valarray<double>& nz );

void pkmult( const vector<double>& vecin, const vector<double>& mat, vector<double>& vecout );
void pkmult( const valarray<double>& vecin, const vector<double>& mat, valarray<double>& vecout );
void pkmult2( const valarray<double>& vecin, const valarray<double>& mat, valarray<double>& vecout );

void SelfUnion( vector<int>& vec );

void DropIdx( const vector<int>& L_zp, const vector<int>& L_zn, vector<int>& L_z );

void Crash();


std::vector<int> ListToSTDVector( const std::list<int>& ListIn );
