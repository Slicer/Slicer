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
vtkImageSlicePaint::vtkImageSlicePaint()
{
  for (int i = 0; i < 3; i++)
    {
    this->TopLeft[i] = this->TopLeft[i] = this->BottomLeft[i] = this->BottomRight[i] = 0;
    }
  
  this->MaskImage = NULL;
  this->BackgroundImage = NULL;
  this->WorkingImage = NULL;
  this->ExtractImage = NULL;
  this->ReplaceImage = NULL;

  this->PaintLabel = 1;
  this->BrushCenter[0] = this->BrushCenter[1] = this->BrushCenter[2] = 0.0;
  this->BrushRadius = 0;

  this->BackgroundIJKToWorld = NULL;
  this->WorkingIJKToWorld = NULL;
  this->MaskIJKToWorld = NULL;

  this->ThresholdPaint = 0;
  this->ThresholdPaintRange[0] = 0;
  this->ThresholdPaintRange[1] = VTK_DOUBLE_MAX;
  this->PaintOver = 1;
}

//----------------------------------------------------------------------------
vtkImageSlicePaint::~vtkImageSlicePaint()
{
  this->SetMaskImage (NULL);
  this->SetBackgroundImage (NULL);
  this->SetWorkingImage (NULL);
  this->SetExtractImage (NULL);
  this->SetReplaceImage (NULL);

  this->SetBackgroundIJKToWorld (NULL);
  this->SetWorkingIJKToWorld  (NULL);
  this->SetMaskIJKToWorld  (NULL);
}


static
void transform3 (vtkMatrix4x4 *m, double *in, double *out)
{
  int i;
  if (m == NULL)
    { // treat NULL as Identity
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
void vtkImageSlicePaintPaint(vtkImageSlicePaint *self, T *ptr)
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

  // Get the pointer to the extact image if available
  vtkImageData *extractImage = self->GetExtractImage();
  T *extractPtr = NULL;
  int extracting = 0;
  if ( extractImage != NULL )
    {
    extractImage->SetDimensions(maxColumnDelta+1, maxRowDelta+1, 1);
    extractImage->SetScalarType( self->GetWorkingImage()->GetScalarType() );
    extractImage->AllocateScalars();
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
  vtkMatrix4x4 *backgroundWorldToIJK = vtkMatrix4x4::New();
  backgroundWorldToIJK->DeepCopy( backgroundIJKToWorld );
  backgroundWorldToIJK->Invert();
  vtkMatrix4x4 *maskWorldToIJK = vtkMatrix4x4::New();
  if ( self->GetMaskIJKToWorld() )
    {
    maskWorldToIJK->DeepCopy( self->GetMaskIJKToWorld() );
    maskWorldToIJK->Invert();
    }
  vtkImageData *background = self->GetBackgroundImage();
  double ijk[3], maskIJK[3];
  int intIJK[3], intMaskIJK[3];
  T *workingPtr;
  T oldValue;
  int paintOver = self->GetPaintOver();
  int thresholdPaint = self->GetThresholdPaint();
  double *thresholdPaintRange = self->GetThresholdPaintRange();

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

      workingPtr = (T *)(self->GetWorkingImage()->GetScalarPointer(intIJK));
      if ( workingPtr ) 
        {

        oldValue = *workingPtr;

        double workingWorld[3]; // world coordinates of current pixel
        transform3 (workingIJKToWorld, ijk, workingWorld);

        int pixelToPaint = 0;  // is this location something we should consider painting

        if ( self->GetReplaceImage() ) 
          {
          // with replace image, just copy value into the working label map
          T *replacePtr = NULL;
          replacePtr = (T *)(self->GetReplaceImage()->GetScalarPointer(column, row, 0));
          *workingPtr = *replacePtr;
          }
        else // no replace image, so paint with mask or brush
          {

          if ( self->GetMaskImage() == NULL )
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
            transform3(maskWorldToIJK, workingWorld, maskIJK);
            for (int i = 0; i < 2; i++) 
              { 
              intMaskIJK[i] = paintRound(maskIJK[i]); 
              if ( intMaskIJK[i] < 0 )
                {
                intMaskIJK[i] = 0;
                }
              }
            intMaskIJK[2] = 0;

            void *ptr = self->GetMaskImage()->GetScalarPointer ( 
                            intMaskIJK[0], intMaskIJK[1], intMaskIJK[2] );

            if ( ptr == NULL ) 
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
          // - apply threshold rule to only paint where backround fits constraints
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
              transform3(backgroundWorldToIJK, workingWorld, bgIJK);
              for (int i = 0; i < 3; i++) 
                { 
                intbgIJK[i] = paintRound(bgIJK[i]); 
                if ( intbgIJK[i] < 0 )
                  {
                  intbgIJK[i] = 0;
                  }
                }
              // TODO: bg may not be of type T
              T *bgPtr = (T *) background->GetScalarPointer(intbgIJK);

              if ( *bgPtr > thresholdPaintRange[0] && *bgPtr < thresholdPaintRange[1] )
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
      else  // working pointer was null
        {
        // TODO: this will leak matrices...
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
          // TODO: this will leak matrices...
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

  maskWorldToIJK->Delete();
  backgroundWorldToIJK->Delete();

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

