/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageSlicePaint.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageSlicePaint.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageSlicePaint, "$Revision: 1.22 $");
vtkStandardNewMacro(vtkImageSlicePaint);

//----------------------------------------------------------------------------
void vtkImageSlicePaint::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "TopLeft : " << this->TopLeft[0] << " " << this->TopLeft[1] << " " << this->TopLeft[2] << "\n";
  os << indent << "TopRight : " << this->TopRight[0] << " " << this->TopRight[1] << " " << this->TopRight[2] << "\n";
  os << indent << "BottomLeft : " << this->BottomLeft[0] << " " << this->BottomLeft[1] << " " << this->BottomLeft[2] << "\n";
  os << indent << "BottomRight : " << this->BottomRight[0] << " " << this->BottomRight[1] << " " << this->BottomRight[2] << "\n";

  os << indent << "ExtractImage: " << this->GetReplaceImage() << "\n";
  os << indent << "ReplaceImage: " << this->GetReplaceImage() << "\n";
}

//----------------------------------------------------------------------------
vtkImageSlicePaint::vtkImageSlicePaint()
{
  for (int i = 0; i < 3; i++)
    {
    this->TopLeft[i] = this->TopLeft[i] = this->BottomLeft[i] = this->BottomRight[i] = 0;
    }
  
  this->WorkingImage = NULL;
  this->ExtractImage = NULL;
  this->ReplaceImage = NULL;
}



template <class T>
void vtkImageSlicePaintExtract(vtkImageSlicePaint *self, T *ptr)
{
  int deltaTopRow[3];
  int deltaBottomRow[3];
  int deltaRightColumn[3];
  int deltaLeftColumn[3];
  int maxRowDelta = 0, maxColumnDelta = 0;
  int maxRowIndex, maxColumnIndex;

  // first get the width and height of the extracted region
  // as the maximum distance along any of the edges
  // and also calculate the deltas along the edges
  for (int i = 0; i < 3; i++)
    {
    deltaTopRow[i] = self->GetTopRight()[i] - self->GetTopLeft()[i];
    if ( abs(deltaTopRow[i]) > maxRowDelta )
      {
      maxRowDelta = abs(deltaTopRow[i]);
      maxRowIndex = i;
      }
    deltaBottomRow[i] = self->GetBottomRight()[i] - self->GetBottomLeft()[i];
    if ( abs(deltaBottomRow[i]) > maxRowDelta )
      {
      maxRowDelta = abs(deltaBottomRow[i]);
      maxRowIndex = i;
      }
    deltaLeftColumn[i] = self->GetBottomLeft()[i] - self->GetTopLeft()[i];
    if ( abs(deltaLeftColumn[i]) > maxColumnDelta )
      {
      maxColumnDelta = abs(deltaLeftColumn[i]);
      maxColumnIndex = i;
      }
    deltaRightColumn[i] = self->GetBottomRight()[i] - self->GetTopRight()[i];
    if ( abs(deltaRightColumn[i]) > maxColumnDelta )
      {
      maxColumnDelta = abs(deltaRightColumn[i]);
      maxColumnIndex = i;
      }
    }

  // Get the point to the extact image if available
  vtkImageData *extractImage = self->GetExtractImage();
  T *extractPtr = NULL;
  if ( extractImage != NULL )
    {
    extractImage->SetDimensions(maxColumnDelta, maxRowDelta, 1);
    extractImage->SetScalarType( self->GetWorkingImage()->GetScalarType() );
    extractImage->AllocateScalars();
    extractPtr = (T *)extractImage->GetScalarPointer();
    }

  double dIJKdRStart[3];
  double dIJKdREnd[3];
  double dIJKdC[3];
  double rowStart[3];
  double rowEnd[3];
  
  // now calculate the change in IJK coordinates per row step
  for (int i = 0; i < 3; i++)
    {
    rowStart[i] = self->GetTopLeft()[i];
    rowEnd[i] = self->GetTopRight()[i];
    dIJKdRStart[i] = deltaLeftColumn[i] / maxColumnDelta;
    dIJKdREnd[i] = deltaRightColumn[i] / maxColumnDelta;
    }

  // Now loop across the columns and across the rows
  // and fill in each pixel as we go
  double ijk[3];
  int intIJK[3];
  T *workingPtr;
  for (int row = 0; row <= maxRowDelta; row++)
    {
    for (int i = 0; i < 3; i++)
      {
      dIJKdC[i] = (rowEnd[i] - rowStart[i]) / maxColumnDelta;
      ijk[i] = rowStart[i];
      }
    for (int column = 0; column <= maxColumnDelta; column++)
      {
      extractPtr = (T *)(self->GetWorkingImage()->GetScalarPointer(column, row, 0));
      if ( extractPtr ) 
        {
        *extractPtr = (T) 100; // FOR NOW...
        //*extractPtr = (T)(value);
        }
      for (int i = 0; i < 3; i++)
        {
        if ( ijk[i] - floor(ijk[i]) > 0.5 )
          {
          intIJK[i] = static_cast <int> (floor(ijk[i]) + 1);
          }
        else
          {
          intIJK[i] = static_cast<int> (floor(ijk[i]));
          }
        }

      workingPtr = (T *)(self->GetWorkingImage()->GetScalarPointer(intIJK));
      if ( workingPtr ) 
        {
        *workingPtr = (T) 100; // FOR NOW...
        }

      for (int i = 0; i < 3; i++)
        {
        ijk[i] += dIJKdC[i];
        }
      }
    for (int i = 0; i < 3; i++)
      {
      rowStart[i] += dIJKdRStart[i];
      rowEnd[i] += dIJKdREnd[i];
      }
    
    }
  self->Modified();
}

//----------------------------------------------------------------------------
void vtkImageSlicePaint::Extract()
{
  void *ptr = NULL;
  
  if ( this->GetWorkingImage() == NULL )
    {
    vtkErrorMacro (<< "Working image cannot be NULL\n");
    return;
    }
  this->GetWorkingImage()->Update();

  switch (this->GetWorkingImage()->GetScalarType())
    {
    vtkTemplateMacro( 
      vtkImageSlicePaintExtract (this, (VTK_TT *)ptr ) );
    default: 
      {
      vtkErrorMacro(<< "Execute: Unknown ScalarType\n");
      return;
      }
    }
  return;
}

//----------------------------------------------------------------------------
void vtkImageSlicePaint::Replace()
{
}


