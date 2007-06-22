/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkHyperPointandArray.cxx,v $
  Date:      $Date: 2006/01/06 17:58:04 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include "vtkHyperPointandArray.h"

// BTX
vtkHyperPoint::vtkHyperPoint()
{
  this->V[0] = this->V0;
  this->V[1] = this->V1;
  this->V[2] = this->V2;
}

vtkHyperPoint& vtkHyperPoint::operator=(const vtkHyperPoint& hp)
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
    }
  this->CellId = hp.CellId;
  this->SubId = hp.SubId;
  this->S = hp.S;
  this->D = hp.D;

  return *this;
}
//ETX

vtkHyperArray::vtkHyperArray()
{
  this->MaxId = -1; 
  this->Array = new vtkHyperPoint[1000];
  this->Size = 1000;
  this->Extend = 5000;
  this->Direction = VTK_INTEGRATE_FORWARD;
}

vtkHyperPoint *vtkHyperArray::Resize(vtkIdType sz)
{
  vtkHyperPoint *newArray;
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

  newArray = new vtkHyperPoint[newSize];

  for (i=0; i<sz; i++)
    {
    newArray[i] = this->Array[i];
    }

  this->Size = newSize;
  delete [] this->Array;
  this->Array = newArray;

  return this->Array;
}

