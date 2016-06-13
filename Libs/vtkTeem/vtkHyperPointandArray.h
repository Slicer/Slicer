/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkHyperPointandArray.h,v $
  Date:      $Date: 2006/01/06 17:58:05 $
  Version:   $Revision: 1.7 $

=========================================================================auto=*/
#ifndef __vtkHyperPointandArray_h
#define __vtkHyperPointandArray_h

#include "vtkTeemConfigure.h"

#include "vtkSystemIncludes.h"
#include "vtkHyperStreamline.h"


/// copied directly from vtkHyperStreamline.
/// this class was defined in the vtkHyperStreamline.cxx file.

//
/// Special classes for manipulating data
class VTK_Teem_EXPORT vtkHyperPoint { //;prevent man page generation
public:
    vtkHyperPoint(); /// method sets up storage
    vtkHyperPoint &operator=(const vtkHyperPoint& hp); //for resizing

    double   X[3];    /// position
    vtkIdType     CellId;  /// cell
    int     SubId; /// cell sub id
    double   P[3];    /// parametric coords in cell
    double   W[3];    /// eigenvalues (sorted in decreasing value)
    double      *V[3];   /// pointers to eigenvectors (also sorted)
    double   V0[3];   /// storage for eigenvectors
    double   V1[3];
    double   V2[3];
    double   S;       /// scalar value
    double   D;       /// distance travelled so far
};

class VTK_Teem_EXPORT vtkHyperArray { //;prevent man page generation
public:
  vtkHyperArray();
  ~vtkHyperArray()
    {
      if (this->Array)
        {
        delete [] this->Array;
        }
    };
  vtkIdType GetNumberOfPoints() {return this->MaxId + 1;};
  vtkHyperPoint *GetHyperPoint(vtkIdType i) {return this->Array + i;};
  vtkHyperPoint *InsertNextHyperPoint()
    {
    if ( ++this->MaxId >= this->Size )
      {
      this->Resize(this->MaxId);
      }
    return this->Array + this->MaxId;
    }
  vtkHyperPoint *Resize(vtkIdType sz); //reallocates data
  void Reset() {this->MaxId = -1;};

  vtkHyperPoint *Array;  /// pointer to data
  vtkIdType MaxId;             /// maximum index inserted thus far
  vtkIdType Size;              /// allocated size of data
  vtkIdType Extend;            /// grow array by this amount
  double Direction;       /// integration direction
};

#define VTK_START_FROM_POSITION 0
#define VTK_START_FROM_LOCATION 1

#endif
