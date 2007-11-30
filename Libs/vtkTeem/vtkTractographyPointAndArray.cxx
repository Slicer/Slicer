/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTractographyPointAndArray.cxx,v $
  Date:      $Date: 2007/02/23 19:04:54 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
#include "vtkTractographyPointAndArray.h"

// BTX
vtkTractographyPoint::vtkTractographyPoint()
{
  this->V[0] = this->V0;
  this->V[1] = this->V1;
  this->V[2] = this->V2;

  this->T[0] = this->T0;
  this->T[1] = this->T1;
  this->T[2] = this->T2;

  // init cell ID to 0 because this is the marker 
  // for points unused during tractography.
  this->CellId = 0;
  this->S = 0;
  this->D = 0;

}

vtkTractographyPoint& vtkTractographyPoint::operator=(const vtkTractographyPoint& hp)
{
  int i, j;

  for (i=0; i<3; i++) 
    {
    this->X[i] = hp.X[i];
    this->P[i] = hp.P[i];
    this->W[i] = hp.W[i];
    for (j=0; j<3; j++)
      {
      this->V[j][i] = hp.V[j][i];
      }
    for (j=0; j<3; j++)
      {
      this->T[j][i] = hp.T[j][i];
      }
    }
  this->CellId = hp.CellId;
  this->SubId = hp.SubId;
  this->S = hp.S;
  this->D = hp.D;

  return *this;
}
//ETX

vtkTractographyArray::vtkTractographyArray()
{
  this->MaxId = -1; 
  this->Array = new vtkTractographyPoint[1000];
  this->Size = 1000;
  this->Extend = 5000;
  this->Direction = VTK_INTEGRATE_FORWARD;
}

vtkTractographyPoint *vtkTractographyArray::Resize(vtkIdType sz)
{
  vtkTractographyPoint *newArray;
  vtkIdType newSize, i;

  if (sz >= this->Size)
    {
    newSize = this->Size + 
      this->Extend*(((sz-this->Size)/this->Extend)+1);
    }
  else
    {
    newSize = sz;
    }

  newArray = new vtkTractographyPoint[newSize];

  for (i=0; i<sz; i++)
    {
    newArray[i] = this->Array[i];
    }

  this->Size = newSize;
  delete [] this->Array;
  this->Array = newArray;

  return this->Array;
}

