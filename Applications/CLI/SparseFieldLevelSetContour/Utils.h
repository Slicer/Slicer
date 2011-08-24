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

// using namespace std;
// typedef std::vector Stdvector
// typedef std::string Stdstring
// typedef std::valarray Stdvalarray
// typedef std::list Stdlist

void ProcessTriDataFiles( const std::vector<std::string>& filenames, std::valarray<int> & CurIdx,
                          std::valarray<double> & CurVertX, std::valarray<double> & CurVertY,
                          std::valarray<double> & CurVertZ );

void ReadNormals( const std::string& filename, std::valarray<double>& nx, std::valarray<double>& ny,
                  std::valarray<double>& nz );

void pkmult( const std::vector<double>& vecin, const std::vector<double>& mat, std::vector<double>& vecout );

void pkmult( const std::valarray<double>& vecin, const std::vector<double>& mat, std::valarray<double>& vecout );

void pkmult2( const std::valarray<double>& vecin, const std::valarray<double>& mat, std::valarray<double>& vecout );

void SelfUnion( std::vector<int>& vec );

void DropIdx( const std::vector<int>& L_zp, const std::vector<int>& L_zn, std::vector<int>& L_z );

void Crash();

// std::vector<int> ListToSTDVector( const std::list<int>& ListIn );
std::vector<int> ListToSTDVector( const std::list<int>& ListIn );
