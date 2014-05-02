/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkPreciseHyperPoint.h,v $
  Date:      $Date: 2006/04/27 22:52:38 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
#ifndef __vtkPreciseHyperPoint_h
#define __vtkPreciseHyperPoint_h

#include "vtkTeemConfigure.h"

class VTK_Teem_EXPORT vtkPreciseHyperPoint { //;prevent man page generation
public:
    vtkPreciseHyperPoint(); /// method sets up storage
    vtkPreciseHyperPoint &operator=(const vtkPreciseHyperPoint& hp); //for resizing

    double   X[3];    /// position
    vtkIdType CellId;  /// cell
    int     SubId; /// cell sub id
    double   P[3];    /// parametric coords in cell
    double   W[3];    /// eigenvalues (sorted in decreasing value)
    double   *V[3];   /// pointers to eigenvectors (also sorted)
    double   V0[3];   /// storage for eigenvectors
    double   V1[3];
    double   V2[3];
    double   S;       /// scalar value
    double   D;       /// distance travelled so far
};

#endif
