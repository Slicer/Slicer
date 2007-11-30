/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTractographyPointAndArray.h,v $
  Date:      $Date: 2007/02/20 20:44:22 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#ifndef __vtkTractographyPointAndArray_h
#define __vtkTractographyPointAndArray_h

#include "vtkSystemIncludes.h"
#include "vtkStreamer.h"
#include "vtkTeemConfigure.h"

// copied directly from vtkTractographyStreamline.
// this class was defined in the vtkTractographyStreamline.cxx file.

//
// Special classes for manipulating data
//BTX
class VTK_TEEM_EXPORT vtkTractographyPoint { //;prevent man page generation
public:
    vtkTractographyPoint(); // method sets up storage
    vtkTractographyPoint &operator=(const vtkTractographyPoint& hp); //for resizing
    
    vtkFloatingPointType   X[3];    // position 
    vtkIdType     CellId;  // cell
    int     SubId; // cell sub id
    vtkFloatingPointType   P[3];    // parametric coords in cell 
    vtkFloatingPointType   W[3];    // eigenvalues (sorted in decreasing value)
    vtkFloatingPointType      *V[3];   // pointers to eigenvectors (also sorted)
    vtkFloatingPointType   V0[3];   // storage for eigenvectors
    vtkFloatingPointType   V1[3];
    vtkFloatingPointType   V2[3];
    vtkFloatingPointType   S;       // scalar value 
    vtkFloatingPointType   D;       // distance travelled so far 
    vtkFloatingPointType      *T[3];   // pointers to tensor
    vtkFloatingPointType   T0[3];   // storage for tensor
    vtkFloatingPointType   T1[3];
    vtkFloatingPointType   T2[3];
};
//ETX

//BTX
class VTK_TEEM_EXPORT vtkTractographyArray { //;prevent man page generation
public:
  vtkTractographyArray();
  ~vtkTractographyArray()
    {
      if (this->Array)
        {
        delete [] this->Array;
        }
    };
  vtkIdType GetNumberOfPoints() {return this->MaxId + 1;};
  vtkTractographyPoint *GetTractographyPoint(vtkIdType i) {return this->Array + i;};
  vtkTractographyPoint *InsertNextTractographyPoint() 
    {
    if ( ++this->MaxId >= this->Size )
      {
      this->Resize(this->MaxId);
      }
    return this->Array + this->MaxId;
    }
  vtkTractographyPoint *Resize(vtkIdType sz); //reallocates data
  void Reset() {this->MaxId = -1;};

  vtkTractographyPoint *Array;  // pointer to data
  vtkIdType MaxId;             // maximum index inserted thus far
  vtkIdType Size;              // allocated size of data
  vtkIdType Extend;            // grow array by this amount
  vtkFloatingPointType Direction;       // integration direction
};
//ETX

#define VTK_START_FROM_POSITION 0
#define VTK_START_FROM_LOCATION 1

#endif
