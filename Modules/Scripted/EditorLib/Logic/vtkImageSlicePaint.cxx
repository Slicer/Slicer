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

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkVersion.h>


vtkStandardNewMacro(vtkImageSlicePaint);

//----------------------------------------------------------------------------
vtkImageSlicePaint::vtkImageSlicePaint()
{
  for (size_t i = 0; i < 3; ++i)
    {
    this->TopLeft[i] = this->TopRight[i] = this->BottomLeft[i] = this->BottomRight[i] = 0;
    }

  this->MaskImage = nullptr;
  this->BackgroundImage = nullptr;
  this->WorkingImage = nullptr;
  this->ExtractImage = nullptr;
  this->ReplaceImage = nullptr;

  this->PaintLabel = 1;
  this->BrushCenter[0] = this->BrushCenter[1] = this->BrushCenter[2] = 0.0;
  this->BrushRadius = 0;

  this->BackgroundIJKToWorld = vtkMatrix4x4::New();
  this->BackgroundIJKToWorld->Identity();
  this->WorkingIJKToWorld = nullptr;
  this->MaskIJKToWorld = nullptr;

  this->ThresholdPaint = 0;
  this->ThresholdPaintRange[0] = 0;
  this->ThresholdPaintRange[1] = VTK_DOUBLE_MAX;
  this->PaintOver = 1;
}

//----------------------------------------------------------------------------
vtkImageSlicePaint::~vtkImageSlicePaint()
{
  this->SetMaskImage (nullptr);
  this->SetBackgroundImage (nullptr);
  this->SetWorkingImage (nullptr);
  this->SetExtractImage (nullptr);
  this->SetReplaceImage (nullptr);

  this->SetBackgroundIJKToWorld (nullptr);
  this->SetWorkingIJKToWorld  (nullptr);
  this->SetMaskIJKToWorld  (nullptr);
}


static
void transform3 (vtkMatrix4x4 *m, double *in, double *out)
{
  int i;
  if (m == nullptr)
    { // treat nullptr as Identity
    for (i = 0; i < 3; i++) { out[i] = in[i]; }
    return;
    }

  // otherwise to a homogeneous transform
  double in4[4], out4[4];
  for (i = 0; i < 3; i++) { in4[i] = in[i]; }
  in4[3] = 1.0;
  m->MultiplyPoint(in4, out4);
  for (i = 0; i < 3; i++) { out[i] = out4[i]; }
}

static
int paintRound (double in)
{
  if ( in - floor(in) > 0.5 ) { return (static_cast <int> (floor(in) + 1)); }
  return (static_cast<int> (floor(in)));
}

// static  UNUSED
// int paintFloor (double in)
// {
  // return (static_cast<int> (floor(in)));
// }


template <class T>
void vtkImageSlicePaintPaint(vtkImageSlicePaint *self, T *vtkNotUsed(ptr))
{
  int deltaTopRow[3];
  int deltaBottomRow[3];
  int deltaRightColumn[3];
  int deltaLeftColumn[3];
  int maxRowDelta = 0, maxColumnDelta = 0;

  // first get the width and height of the extracted region
  // as the maximum distance along any of the edges
  // and also calculate the deltas along the edges
  for (int i = 0; i < 3; i++)
    {
    deltaTopRow[i] = self->GetTopRight()[i] - self->GetTopLeft()[i];
    if ( abs(deltaTopRow[i]) > maxColumnDelta )
      {
      maxColumnDelta = abs(deltaTopRow[i]);
      }
    deltaBottomRow[i] = self->GetBottomRight()[i] - self->GetBottomLeft()[i];
    if ( abs(deltaBottomRow[i]) > maxColumnDelta )
      {
      maxColumnDelta = abs(deltaBottomRow[i]);
      }
    deltaLeftColumn[i] = self->GetBottomLeft()[i] - self->GetTopLeft()[i];
    if ( abs(deltaLeftColumn[i]) > maxRowDelta )
      {
      maxRowDelta = abs(deltaLeftColumn[i]);
      }
    deltaRightColumn[i] = self->GetBottomRight()[i] - self->GetTopRight()[i];
    if ( abs(deltaRightColumn[i]) > maxRowDelta )
      {
      maxRowDelta = abs(deltaRightColumn[i]);
      }
    }

  if ( maxRowDelta == 0 || maxColumnDelta == 0 )
    {
    // the region is a singularity - can't draw anything
    vtkDebugWithObjectMacro (self, << "a delta is zero: maxRowDelta = " << maxRowDelta << ", maxColumnDelta = " << maxColumnDelta << "\n" );
    return;
    }

  // Get the pointer to the extract image if available
  vtkImageData *extractImage = self->GetExtractImage();
  T *extractPtr = nullptr;
  int extracting = 0;
  if ( extractImage != nullptr )
    {
    extractImage->SetDimensions(maxColumnDelta+1, maxRowDelta+1, 1);
    extractImage->AllocateScalars(self->GetWorkingImage()->GetScalarType(), 1);
    extracting = 1;
    }


  double radiusSquared = self->GetBrushRadius() * self->GetBrushRadius();
  double *brushCenter = self->GetBrushCenter();
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

  //
  // Now loop across the columns and across the rows
  // and fill in each pixel as we go
  // - get matrices to relate the label IJK to the background IJK
  //   by going first to world, then back to ijk.
  //   (not that this is linear, so deltas could be precomputed for
  //   efficiency, but probably aren't needed)
  // - cache some values to avoid call overhead per pixel
  //
  vtkMatrix4x4 *workingIJKToWorld = self->GetWorkingIJKToWorld();
  vtkMatrix4x4 *backgroundIJKToWorld = self->GetBackgroundIJKToWorld();
  vtkNew<vtkMatrix4x4> backgroundWorldToIJK;
  backgroundWorldToIJK->DeepCopy(backgroundIJKToWorld);
  backgroundWorldToIJK->Invert();
  vtkNew<vtkMatrix4x4> maskWorldToIJK;
  if ( self->GetMaskIJKToWorld() )
    {
    maskWorldToIJK->DeepCopy( self->GetMaskIJKToWorld() );
    maskWorldToIJK->Invert();
    }
  vtkImageData *background = self->GetBackgroundImage();
  double ijk[3], maskIJK[3];
  int intIJK[3], intMaskIJK[3];
  T *workingPtr;
  T oldValue = 0;
  int paintOver = self->GetPaintOver();
  int thresholdPaint = self->GetThresholdPaint();
  double *thresholdPaintRange = self->GetThresholdPaintRange();
  const int *workingExtent = self->GetWorkingImage()->GetExtent();

  // row loop
  for (int row = 0; row <= maxRowDelta; row++)
    {

    for (int i = 0; i < 3; i++)
      {
      dIJKdC[i] = (rowEnd[i] - rowStart[i]) / maxColumnDelta;
      ijk[i] = rowStart[i];
      }

    // column loop
    for (int column = 0; column <= maxColumnDelta; column++)
      {
      // get coordinates in Working IJK space (intIJK)
      for (int i = 0; i < 3; i++)
        {
        intIJK[i] = paintRound (ijk[i]);
        if ( intIJK[i] < 0 )
          {
          intIJK[i] = 0;
          }
        }

      // check to see if we are inside working volume
      if ( ijk[0] < workingExtent[0] || ijk[0] > workingExtent[1] ||
           ijk[1] < workingExtent[2] || ijk[1] > workingExtent[3] ||
           ijk[2] < workingExtent[4] || ijk[2] > workingExtent[5] )
        {
        workingPtr = nullptr;
        }
      else
        {
        workingPtr = (T *)(self->GetWorkingImage()->GetScalarPointer(intIJK));
        }

      if ( workingPtr )
        {

        oldValue = *workingPtr;

        double workingWorld[3]; // world coordinates of current pixel
        transform3 (workingIJKToWorld, ijk, workingWorld);

        int pixelToPaint = 0;  // is this location something we should consider painting

        if ( self->GetReplaceImage() )
          {
          // with replace image, just copy value into the working label map
          T *replacePtr = nullptr;
          replacePtr = (T *)(self->GetReplaceImage()->GetScalarPointer(column, row, 0));
          *workingPtr = *replacePtr;
          }
        else // no replace image, so paint with mask or brush
          {

          if ( self->GetMaskImage() == nullptr )
            {
            // no mask, so check brush radius
            double distSquared = ((workingWorld[0] - brushCenter[0]) *
                                  (workingWorld[0] - brushCenter[0])) +
                                 ((workingWorld[1] - brushCenter[1]) *
                                  (workingWorld[1] - brushCenter[1])) +
                                 ((workingWorld[2] - brushCenter[2]) *
                                  (workingWorld[2] - brushCenter[2]));
            if ( distSquared < radiusSquared )
              {
              pixelToPaint = 1; // Now we're inside the brush
              }
            }
          else
            {
            // check the mask image
            // note: k will always be zero for the mask since it is 2D...
            transform3(maskWorldToIJK.GetPointer(), workingWorld, maskIJK);
            for (int i = 0; i < 2; i++)
              {
              intMaskIJK[i] = paintRound(maskIJK[i]);
              if ( intMaskIJK[i] < 0 )
                {
                intMaskIJK[i] = 0;
                }
              }
            intMaskIJK[2] = 0;

            void *ptr1 = self->GetMaskImage()->GetScalarPointer (
                            intMaskIJK[0], intMaskIJK[1], intMaskIJK[2] );

            if ( ptr1 == nullptr )
              {
              int dimensions[3];
              self->GetMaskImage()->GetDimensions(dimensions);
              // TODO: this will leak matrices...
              vtkErrorWithObjectMacro ( self, << "vtkImageSlicePaintPaint:\nCannot get mask pointer for pixel\n"
                << "workingWorld = " <<
                workingWorld[0] << " " <<  workingWorld[1] << " " << workingWorld[2] << "\n"
                << "intMaskIJK = " <<
                intMaskIJK[0] << " " <<  intMaskIJK[1] << " " << intMaskIJK[2] << "\n"
                << "maskDimensions = " <<
                dimensions[0] << " " <<  dimensions[1] << " " << dimensions[2] << "\n"
                );
              return;
              }

            double maskValue = self->GetMaskImage()->GetScalarComponentAsDouble (
                                    intMaskIJK[0], intMaskIJK[1], intMaskIJK[2], 0 );
            if ( maskValue )
              {
              pixelToPaint = 1; // mask is non-zero, so paint
              }
            }

          }

        if ( pixelToPaint )
          {
          // get here if there's no replace image and the pixel is in the brush radius
          // or corresponds to a non-zero mask value
          // - apply paintOver rule to avoid overwriting existing data
          // - apply threshold rule to only paint where background fits constraints
          if ( !paintOver && oldValue != 0 )
            {
            // don't write
            }
          else // okay to paint
            {
            if ( thresholdPaint )
              {
              double bgIJK[3];
              int intbgIJK[3];

              // get the background pixel
              transform3(backgroundWorldToIJK.GetPointer(), workingWorld, bgIJK);
              for (int i = 0; i < 3; i++)
                {
                intbgIJK[i] = paintRound(bgIJK[i]);
                if ( intbgIJK[i] < 0 )
                  {
                  intbgIJK[i] = 0;
                  }
                }
              double bgValue = background->GetScalarComponentAsDouble(intbgIJK[0], intbgIJK[1], intbgIJK[2], 0);

              if ( bgValue >= thresholdPaintRange[0] && bgValue <= thresholdPaintRange[1] )
                {
                *workingPtr = label; // TODO: need to work on multicomponent images
                }
              }
            else // Not in thresholdPaint
              {
              *workingPtr = label; // TODO: need to work on multicomponent images
              }
            }
          }
        }

      if ( extracting )
        {
        extractPtr = (T *)(self->GetExtractImage()->GetScalarPointer(column, row, 0));
        if ( extractPtr )
          {
          *extractPtr = oldValue;
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
  void *ptr = nullptr;

  if ( this->GetWorkingImage() == nullptr )
    {
    vtkErrorMacro (<< "Working image cannot be NULL\n");
    return;
    }

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

