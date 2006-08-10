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

  os << indent << "PaintLabel: " << this->GetPaintLabel() << "\n";
  os << indent << "BrushCenter : " << this->BrushCenter[0] << " " << this->BrushCenter[1] << " " << this->BrushCenter[2] << "\n";
  os << indent << "BrushRadius: " << this->GetBrushRadius() << "\n";

  os << indent << "BackgroundImage: " << this->GetBackgroundImage() << "\n";
  os << indent << "WorkingImage: " << this->GetWorkingImage() << "\n";
  os << indent << "ExtractImage: " << this->GetExtractImage() << "\n";
  os << indent << "ReplaceImage: " << this->GetReplaceImage() << "\n";

  os << indent << "BackgroundIJKToWorld: " << this->GetBackgroundIJKToWorld() << "\n";
  os << indent << "WorkingIJKToWorld: " << this->GetWorkingIJKToWorld() << "\n";

  os << indent << "ThresholdPaint: " << this->GetThresholdPaint() << "\n";
  os << indent << "ThresholdPaintRange: " << this->GetThresholdPaintRange()[0] << ", " <<  this->GetThresholdPaintRange()[1] << "\n";
  os << indent << "PaintOver: " << this->GetPaintOver() << "\n";
}

//----------------------------------------------------------------------------
vtkImageSlicePaint::vtkImageSlicePaint()
{
  for (int i = 0; i < 3; i++)
    {
    this->TopLeft[i] = this->TopLeft[i] = this->BottomLeft[i] = this->BottomRight[i] = 0;
    }
  
  this->BackgroundImage = NULL;
  this->WorkingImage = NULL;
  this->ExtractImage = NULL;
  this->ReplaceImage = NULL;

  this->PaintLabel = 1;
  this->BrushCenter[0] = this->BrushCenter[1] = this->BrushCenter[2] = 0.0;
  this->BrushRadius = 0;

  this->BackgroundIJKToWorld = NULL;
  this->WorkingIJKToWorld = NULL;

  this->ThresholdPaint = 0;
  this->ThresholdPaintRange[0] = 0;
  this->ThresholdPaintRange[1] = VTK_DOUBLE_MAX;
  this->PaintOver = 1;
}



template <class T>
void vtkImageSlicePaintPaint(vtkImageSlicePaint *self, T *ptr)
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

  if ( maxRowDelta == 0 || maxColumnDelta == 0 )
    {
    // the region is a singularity - can't draw anything
    //vtkErrorWithObjectMacro (self, << "a delta is zero: maxRowDelta = " << maxRowDelta << ", maxColumnDelta = " << maxColumnDelta << "\n" );
    return;
    }

  // Get the pointer to the extact image if available
  vtkImageData *extractImage = self->GetExtractImage();
  T *extractPtr = NULL;
  int extracting = 0;
  if ( extractImage != NULL )
    {
    extractImage->SetDimensions(maxColumnDelta, maxRowDelta, 1);
    extractImage->SetScalarType( self->GetWorkingImage()->GetScalarType() );
    extractImage->AllocateScalars();
    extracting = 1;
    }


  double radiusSquared = self->GetBrushRadius() * self->GetBrushRadius();
  double dIJKdRStart[3];
  double dIJKdREnd[3];
  double dIJKdC[3];
  double rowStart[3];
  double rowEnd[3];
  T label = (T) self->GetPaintLabel();
  
  // now calculate the change in IJK coordinates per row step
  for (int i = 0; i < 3; i++)
    {
    rowStart[i] = self->GetTopLeft()[i];
    rowEnd[i] = self->GetTopRight()[i];
    dIJKdRStart[i] = deltaLeftColumn[i] / maxRowDelta;
    dIJKdREnd[i] = deltaRightColumn[i] / maxRowDelta;
    }

  // Now loop across the columns and across the rows
  // and fill in each pixel as we go
  double ijk[3];
  int intIJK[3];
  T *workingPtr;
  T oldValue;
  for (int row = 0; row <= maxRowDelta; row++)
    {
    for (int i = 0; i < 3; i++)
      {
      dIJKdC[i] = (rowEnd[i] - rowStart[i]) / maxColumnDelta;
      ijk[i] = rowStart[i];
      }
    for (int column = 0; column <= maxColumnDelta; column++)
      {

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
        oldValue = *workingPtr;


        *workingPtr = label; // TODO: need to work on multicomponent images


        }
      else
        {
        vtkErrorWithObjectMacro (self, 
          << "can't get working image pointer for " 
          << intIJK[0] << " " << intIJK[1] << " " << intIJK[2] << "\n");
        return; // we got out of the region somehow and there will be error messages...
        }

      if ( extracting )
        {
        extractPtr = (T *)(self->GetExtractImage()->GetScalarPointer(column, row, 0));
        if ( extractPtr ) 
          {
          *extractPtr = oldValue;
          }
        else
          {
          vtkErrorWithObjectMacro (self, 
            << "can't get extract image pointer for " << row << " " << column << "\n");
          return; // we got out of the region somehow and there will be error messages...
          }

        }


      // update the ijk index for the next column
      for (int i = 0; i < 3; i++)
        {
        ijk[i] += dIJKdC[i];
        }
      }

    // update the ijk index for the next row
    for (int i = 0; i < 3; i++)
      {
      rowStart[i] += dIJKdRStart[i];
      rowEnd[i] += dIJKdREnd[i];
      }
    }

  self->GetWorkingImage()->Modified();
  if (self->GetExtractImage())
    {
    self->GetExtractImage()->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImageSlicePaint::Paint()
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
      vtkImageSlicePaintPaint (this, (VTK_TT *)ptr ) );
    default: 
      {
      vtkErrorMacro(<< "Execute: Unknown ScalarType\n");
      return;
      }
    }
  return;
}

//----------------------------------------------------------------------------
void vtkImageSlicePaint::Extract()
{
  // TODO: this will pull out the image to save
}

//----------------------------------------------------------------------------
void vtkImageSlicePaint::Replace()
{
  // TODO: this will put the image back into the volume
}

