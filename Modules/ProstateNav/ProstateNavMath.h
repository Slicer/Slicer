/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/
// .NAME ProstateNavMath - performs common math operations
// .SECTION Description
// ProstateNavMath provides mathematical operations

#ifndef __ProstateNavMath_h
#define __ProstateNavMath_h

#include "vtkMath.h"
#include "itkPoint.h"

class vtkMatrix4x4;

class ProstateNavMath
{
public:

  // Compute the dimension order for the matrix, and look for flips.
  static void ComputePermutationFromOrientation(vtkMatrix4x4 *matrix,
    int permutation[3],
    int flip[3])
  {
    if (matrix == 0)
    {
      permutation[0] = 0;
      permutation[1] = 1;
      permutation[2] = 2;
      flip[0] = 0;
      flip[1] = 0;
      flip[2] = 0;

      return;
    }

    // There are 6 permutations for 3 dimensions.  In addition,
    // if each of those dimensions can be flipped, then there are
    // 8 (two to the power of three) possible flips.  That would
    // give 48 different possibilities, but since we don't consider
    // any combinations that result in left-handed rotations, the
    // total number of combinations that we test is 24.

    // Convert the matrix into three column vectors
    double vectors[3][4];
    int i = 0;
    int j = 0;
    for (i = 0; i < 3; i++)
    {
      double *v = vectors[i];
      for (j = 0; j < 4; j++)
      {
        v[j] = 0.0;
      }
      v[i] = 1.0;
      matrix->MultiplyPoint(v, v);
    }

    // Here's how the algorithm works.  We want to find a matrix
    // composed only of permutations and flips that has the closest
    // possible orientation (in terms of absolute orientation angle)
    // to our DirectionCosines.

    // The orientation angle for any matrix A is given by:
    //
    //   cos(angle/2) = sqrt(1 + trace(A))/2
    //
    // Therefore, the minimum angle occurs when the trace is
    // at its maximum.

    // So our method is to calculate the traces of all the various
    // permutations and flips, and just use the one with the largest
    // trace.

    // First check if the matrix includes an odd number of flips,
    // since if it does, it specifies a left-handed rotation.
    double d = vtkMath::Determinant3x3(vectors[0], vectors[1], vectors[2]);
    int oddPermutation = (d < 0);

    // Calculate all the traces, including any combination of
    // permutations and flips that represent right-handed
    // orientations.
    int imax = 0;
    int jmax = 0;
    int kmax = 0;
    int lmax = 0;
    double maxtrace = -1e30;

    for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 2; j++)
      {
        double xval = vectors[i][0];
        double yval = vectors[(i + 1 + j) % 3][1];
        double zval = vectors[(i + 2 - j) % 3][2];
        for (int k = 0; k < 2; k++)
        {
          for (int l = 0; l < 2; l++)
          {
            // The (1 - 2*k) gives a sign from a boolean.
            // For z, we want to set the sign that will
            // not change the handedness ("^" is XOR).
            double xtmp = xval * (1 - 2*k);
            double ytmp = yval * (1 - 2*l);
            double ztmp = zval * (1 - 2*(j ^ k ^ l ^ oddPermutation));

            double trace = xtmp + ytmp + ztmp;

            // Find maximum trace
            if (trace > maxtrace)
            {
              maxtrace = trace;
              imax = i;
              jmax = j;
              kmax = k;
              lmax = l;
            }
          }
        }
      }
    }

    // Find the permutation to map each column of the orientation
    // matrix to a spatial dimension x, y, or z.
    int xidx = imax;
    int yidx = (imax + 1 + jmax) % 3;
    int zidx = (imax + 2 - jmax) % 3;

    permutation[0] = xidx;
    permutation[1] = yidx;
    permutation[2] = zidx;

    flip[xidx] = kmax;
    flip[yidx] = lmax;
    flip[zidx] = (jmax ^ kmax ^ lmax ^ oddPermutation);
  }

  // Description
  // Calculate distance between a line (defined by two points) and a point
  static double ComputeDistanceLinePoint(const double x[3], // linepoint 1
                                                   const double y[3], // linepoint 2
                                                   const double z[3], // target point
                                                   double & apError, double & lrError, double & isError)
  {
    double u[3];
    double v[3];
    double w[3];

    u[0] = y[0] - x[0];
    u[1] = y[1] - x[1];
    u[2] = y[2] - x[2];

    vtkMath::Normalize(u);

    v[0] = z[0] - x[0];
    v[1] = z[1] - x[1];
    v[2] = z[2] - x[2];

    double dot = vtkMath::Dot(u,v);

    w[0] = v[0] - dot*u[0];
    w[1] = v[1] - dot*u[1];
    w[2] = v[2] - dot*u[2];

    // actual distance calculation ends here

    // additional code below is from Axel's MATLAB code
    double e[3];
    e[0] = -w[0];
    e[1] = -w[1];
    e[2] = -w[2];

    double r[3];
    double s[3];
    double temp[3] = {0,1,0};
    vtkMath::Cross(temp,u,r);
    vtkMath::Normalize(r);
    //double cre = vtkMath::Dot(e,r);

    vtkMath::Cross(r,u,s);
    vtkMath::Normalize(s);
    //double cse = vtkMath::Dot(e,s);

    // additional code ends here

    return sqrt(vtkMath::Dot(w,w));
  }

  // distance between 2 points
  static double VMagnitudeM( itk::Point<double,3> Point1, double Point2[3] )
  {
    double Vector[3];

    Vector[0] = Point2[0] - Point1[0];
    Vector[1] = Point2[1] - Point1[1];
    Vector[2] = Point2[2] - Point1[2];

    double d = sqrt( Vector[0] * Vector[0] + Vector[1] * Vector[1] + Vector[2] * Vector[2] );

    return d;
  }

  // distance between a line and a point
  static double PointDistanceFromLineM(itk::Point<double, 3> point, double L[3], double v[3])
  {
    double u;
    double Intersection[3];

    u = ( ( ( point[0] - L[0] ) * v[0] ) +
      ( ( point[1] - L[1] ) * v[1] ) +
      ( ( point[2] - L[2] ) * v[2] ) ) ;

    Intersection[0] = L[0] + u * v[0];
    Intersection[1] = L[1] + u * v[1];
    Intersection[2] = L[2] + u * v[2];

    return VMagnitudeM( point, Intersection );
  }

};

#endif
