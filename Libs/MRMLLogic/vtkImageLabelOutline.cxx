/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageLabelOutline.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/
#include "vtkImageLabelOutline.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkImageData.h"


//------------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkImageLabelOutline, "$Revision$");
vtkStandardNewMacro(vtkImageLabelOutline);

//----------------------------------------------------------------------------
// Description:
// Constructor sets default values
vtkImageLabelOutline::vtkImageLabelOutline()
{
  this->Outline = 1;
  this->Background = 0;
  this->HandleBoundaries = 1;
  this->SetNeighborTo8();
}


//----------------------------------------------------------------------------
vtkImageLabelOutline::~vtkImageLabelOutline()
{

}
//----------------------------------------------------------------------------


// Description:
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageLabelOutlineExecute(vtkImageLabelOutline *self,
                     vtkImageData *inData, T *inPtr,
                     vtkImageData *outData,
                     int outExt[6], int id)
{
  // For looping though output (and input) pixels.
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  int outIdx0, outIdx1, outIdx2;
  vtkIdType inInc0, inInc1, inInc2;
  vtkIdType outInc0, outInc1, outInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  T *outPtr0, *outPtr1, *outPtr2;
  // For looping through hood pixels
  int hoodMin0, hoodMax0, hoodMin1, hoodMax1, hoodMin2, hoodMax2;
  int hoodIdx0, hoodIdx1, hoodIdx2;
  T *hoodPtr0, *hoodPtr1, *hoodPtr2;
  // For looping through the mask.
  unsigned char *maskPtr, *maskPtr0, *maskPtr1, *maskPtr2;
  vtkIdType maskInc0, maskInc1, maskInc2;
  // The extent of the whole input image
  int inImageMin0, inImageMin1, inImageMin2;
  int inImageMax0, inImageMax1, inImageMax2;
  // Other
  T backgnd = (T)(self->GetBackground());
  T pix;
  T *outPtr = (T*)outData->GetScalarPointerForExtent(outExt);
  unsigned long count = 0;
  unsigned long target;

  // Get information to march through data
  
  inData->GetIncrements(inInc0, inInc1, inInc2);
  self->GetInput()->GetWholeExtent(inImageMin0, inImageMax0, inImageMin1,
    inImageMax1, inImageMin2, inImageMax2);  
  outData->GetIncrements(outInc0, outInc1, outInc2);
  outMin0 = outExt[0];   outMax0 = outExt[1];
  outMin1 = outExt[2];   outMax1 = outExt[3];
  outMin2 = outExt[4];   outMax2 = outExt[5];

  // Neighborhood around current voxel
  self->GetRelativeHoodExtent(hoodMin0, hoodMax0, hoodMin1,
    hoodMax1, hoodMin2, hoodMax2);

  // Set up mask info
  maskPtr = (unsigned char *)(self->GetMaskPointer());
  self->GetMaskIncrements(maskInc0, maskInc1, maskInc2);

  // in and out should be marching through corresponding pixels.
  inPtr = (T *)(inData->GetScalarPointer(outMin0, outMin1, outMin2));

  target = (unsigned long)((outMax2-outMin2+1)*(outMax1-outMin1+1)/50.0);
  target++;

  // loop through pixels of output
  outPtr2 = outPtr;
  inPtr2 = inPtr;
  for (outIdx2 = outMin2; outIdx2 <= outMax2; outIdx2++)
    {
    outPtr1 = outPtr2;
    inPtr1 = inPtr2;
    for (outIdx1 = outMin1;
      !self->AbortExecute && outIdx1 <= outMax1; outIdx1++)
      {
      if (!id)
        {
        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }
      outPtr0 = outPtr1;
      inPtr0 = inPtr1;
      for (outIdx0 = outMin0; outIdx0 <= outMax0; outIdx0++)
        {
        pix = *inPtr0;
        // Default output equal to backgnd
        *outPtr0 = backgnd;

        if (pix != backgnd)
          {
          // Loop through neighborhood pixels (kernel radius=1)
          // Note: input pointer marches out of bounds.
          //hoodPtr2 = inPtr0 - inInc0 - inInc1 - inInc2;
          hoodPtr2 = inPtr0 + inInc0*hoodMin0 + inInc1*hoodMin1
            + inInc2*hoodMin2;
          maskPtr2 = maskPtr;
          for (hoodIdx2 = hoodMin2; hoodIdx2 <= hoodMax2; ++hoodIdx2)
            {
            hoodPtr1 = hoodPtr2;
            maskPtr1 = maskPtr2;
            for (hoodIdx1 = hoodMin1; hoodIdx1 <= hoodMax1;    ++hoodIdx1)
              {
              hoodPtr0 = hoodPtr1;
              maskPtr0 = maskPtr1;
              for (hoodIdx0 = hoodMin0; hoodIdx0 <= hoodMax0; ++hoodIdx0)
                {
                if (*maskPtr0)
                  {
                  // handle boundaries
                  if (outIdx0 + hoodIdx0 >= inImageMin0 &&
                    outIdx0 + hoodIdx0 <= inImageMax0 &&
                    outIdx1 + hoodIdx1 >= inImageMin1 &&
                    outIdx1 + hoodIdx1 <= inImageMax1 &&
                    outIdx2 + hoodIdx2 >= inImageMin2 &&
                    outIdx2 + hoodIdx2 <= inImageMax2)
                    {
                    // If the neighbor not identical, use this pixel
                    // (set the output to foreground)
                    if (*hoodPtr0 != pix)
                      *outPtr0 = pix;
                    }
                  }
                hoodPtr0 += inInc0;
                maskPtr0 += maskInc0;
                }//for0
              hoodPtr1 += inInc1;
              maskPtr1 += maskInc1;
              }//for1
            hoodPtr2 += inInc2;
            maskPtr2 += maskInc2;
            }//for2
          }//if
        inPtr0 += inInc0;
        outPtr0 += outInc0;
        }//for0
      inPtr1 += inInc1;
      outPtr1 += outInc1;
      }//for1
    inPtr2 += inInc2;
    outPtr2 += outInc2;
    }//for2
}

//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageLabelOutline::ThreadedExecute(vtkImageData *inData,
  vtkImageData *outData,
  int outExt[6], int id)
{    int x1;

  // Single component input is required
  vtkImageData *input = this->GetInput();
  x1 = input->GetNumberOfScalarComponents();
  if (x1 != 1) {
    vtkErrorMacro(<<"Input has "<<x1<<" instead of 1 scalar component.");
    return;
  }


  void *inPtr = inData->GetScalarPointerForExtent(outExt);

  switch (inData->GetScalarType())
    {
  case VTK_DOUBLE:
    vtkImageLabelOutlineExecute(this, inData, (double *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_FLOAT:
    vtkImageLabelOutlineExecute(this, inData, (float *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_LONG:
    vtkImageLabelOutlineExecute(this, inData, (long *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_UNSIGNED_LONG:
    vtkImageLabelOutlineExecute(this, inData, (unsigned long *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_INT:
    vtkImageLabelOutlineExecute(this, inData, (int *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_UNSIGNED_INT:
    vtkImageLabelOutlineExecute(this, inData, (unsigned int *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_SHORT:
    vtkImageLabelOutlineExecute(this, inData, (short *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_UNSIGNED_SHORT:
    vtkImageLabelOutlineExecute(this, inData, (unsigned short *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_CHAR:
    vtkImageLabelOutlineExecute(this, inData, (char *)(inPtr),
      outData, outExt, id);
    break;
  case VTK_UNSIGNED_CHAR:
    vtkImageLabelOutlineExecute(this, inData, (unsigned char *)(inPtr),
      outData, outExt, id);
    break;
  default:
    vtkErrorMacro(<< "Execute: Unknown input ScalarType");
    return;
    }
}

//----------------------------------------------------------------------------
void vtkImageLabelOutline::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);

    os << indent << "Outline: " << this->Outline << "\n";
    os << indent << "Background: " << this->Background<< "\n";

    if (this->GetInput() != NULL)
      {
      os << indent << "Input0:\n";
      this->GetInput()->PrintSelf(os,indent.GetNextIndent());
      }
    if (this->GetOutput() != NULL)
      {
      os << indent << "Output0:\n";
      this->GetOutput()->PrintSelf(os,indent.GetNextIndent());
      }
}

