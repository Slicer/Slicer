/*=========================================================================

  Program:   Extract Skeleton
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef COORD_TYPES_H
#define COORD_TYPES_H

#include "math.h"

#include "misc.h"

class Coord3i
{
  int v[3];
public:
  Coord3i()
  {
    v[0] = v[1] = v[2] = -1;
  }
  inline int & operator[](int i)
  {
    return v[i];
  }
  inline void conv(double * i)
  {
    i[0] = v[0]; i[1] = v[1]; i[2] = v[2];
  };
};

class Coord3f
{
  float v[3];
public:
  inline float & operator[](int i)
  {
    return v[i];
  }
  inline void conv(float * i)
  {
    i[0] = v[0]; i[1] = v[1]; i[2] = v[2];
  };
  inline void conv(double * i)
  {
    i[0] = v[0]; i[1] = v[1]; i[2] = v[2];
  };
};

class Coord3d
{
  double v[3];
public:
  inline double & operator[](int i)
  {
    return v[i];
  };
  inline void conv(int * i)
  {
    i[0] = (int) v[0]; i[1] = (int) v[1]; i[2] = (int) v[2];
  };
  inline void conv(float * i)
  {
    i[0] = static_cast<float>(v[0]); i[1] = static_cast<float>(v[1]); i[2] = static_cast<float>(v[2]);
  };
  inline void conv(double * i)
  {
    i[0] = v[0]; i[1] = v[1]; i[2] = v[2];
  };
};

// Euclidean distance between two points
// TODO: use image pixel spacing
inline double pointdistance(Coord3i &p1, Coord3i &p2)
{
  return sqrt((double)sqr(p1[0] - p2[0]) + sqr(p1[1] - p2[1]) + sqr(p1[2] - p2[2]));
}

inline void normcrossprod(double *v1, double *v2, double *norm)
// calculate normalized crossproduct
{
  double absval;

  norm[0] = v1[1] * v2[2] - v1[2] * v2[1];
  norm[1] = v1[2] * v2[0] - v1[0] * v2[2];
  norm[2] = v1[0] * v2[1] - v1[1] * v2[0];

  absval = sqrt(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]);
  norm[0] /= absval;
  norm[1] /= absval;
  norm[2] /= absval;
}

// calculate angle between two vectors (0..M_PI), you might want to adjust to 0..M_PI/2
// range after call via 'if (angle > M_PI/2) angle = M_PI-angle;'
inline double vectorangle(double *v1, double *v2)
{
  double prod = 0, length1 = 0, length2 = 0;

  for( int k = 0; k < 3; k++ )
    {
    prod += v1[k] * v2[k];
    length1 += v1[k] * v1[k];
    length2 += v2[k] * v2[k];
    }
  return acos(prod / sqrt(length1 * length2) );
}

// calculate angle between two vectors (0..M_PI), you might want to adjust to 0..M_PI/2
// range after call via 'if (angle > M_PI/2) angle = M_PI-angle;'
inline double vectorangle(Coord3d v1, Coord3d v2)
{
  double prod = 0, length1 = 0, length2 = 0;

  for( int k = 0; k < 3; k++ )
    {
    prod += v1[k] * v2[k];
    length1 += v1[k] * v1[k];
    length2 += v2[k] * v2[k];
    }
  return acos(prod / sqrt(length1 * length2) );
}

inline double vec_length(Coord3d x)
{
  return sqrt(sqr(x[0]) + sqr(x[1]) + sqr(x[2]) );
}

inline double vec_length(double *x)
{
  return sqrt(sqr(x[0]) + sqr(x[1]) + sqr(x[2]) );
}

inline double vec_length(double *x, double *y)
{
  return sqrt(sqr(x[0] - y[0]) + sqr(x[1] - y[1]) + sqr(x[2] - y[2]) );
}

// transform and check index, returns 0 on success and 1 if corrected location
inline int transWorldToImage(Coord3d loc_world, int *loc_img,
                             double *origin, int *dims, double voxelsize)
{
  int adjust = 0;

  for( int i = 0; i < 3; i++ )
    {
    loc_img[i] = (int) ( (loc_world[i] - origin[i]) / voxelsize);
    if( loc_img[i] < 0 )
      {
      adjust = 1; loc_img[i] = 0;
      }
    if( loc_img[i] >= dims[i] )
      {
      loc_img[i] = dims[i] - 1; adjust = 1;
      }
    }

  return adjust;
}

inline int transWorldToImage(double *loc_world, int *loc_img,
                             double *origin, int *dims, double voxelsize)
// transform and check index, returns 0 on success and 1 if corrected location
{
  int adjust = 0;

  for( int i = 0; i < 3; i++ )
    {
    loc_img[i] = (int) ( (loc_world[i] - origin[i]) / voxelsize);
    if( loc_img[i] < 0 )
      {
      adjust = 1; loc_img[i] = 0;
      }
    if( loc_img[i] >= dims[i] )
      {
      loc_img[i] = dims[i] - 1; adjust = 1;
      }
    }

  return adjust;
}

#endif
