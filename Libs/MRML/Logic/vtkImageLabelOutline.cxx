/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageLabelOutline.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/
#include "vtkImageLabelOutline.h"

// VTK includes
#include <vtkInformation.h>
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

//------------------------------------------------------------------------------
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
vtkImageLabelOutline::~vtkImageLabelOutline() = default;
//----------------------------------------------------------------------------


// Description:
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageLabelOutlineExecute(vtkImageLabelOutline *self,
                     vtkImageData *inData, T *vtkNotUsed(inPtr),
                     vtkImageData *outData,
                     int outExt[6], int id)
{
  int *kernelMiddle, *kernelSize;
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
  // The extent of the whole input image
  int inImageMin0, inImageMin1, inImageMin2;
  int inImageMax0, inImageMax1, inImageMax2;
  // Other
  T backgroundLabelValue = (T)(self->GetBackground());
  T inLabelValue;
  T *outPtr = (T*)outData->GetScalarPointerForExtent(outExt);
  unsigned long count = 0;
  unsigned long target;

  // Get information to march through data

  inData->GetIncrements(inInc0, inInc1, inInc2);
  int inExt[6];
  self->GetInputInformation()->Get(
        vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
  inImageMin0 = inExt[0]; inImageMax0 = inExt[1];
  inImageMin1 = inExt[2]; inImageMax1 = inExt[3];
  inImageMin2 = inExt[4]; inImageMax2 = inExt[5];
  outData->GetIncrements(outInc0, outInc1, outInc2);
  outMin0 = outExt[0];   outMax0 = outExt[1];
  outMin1 = outExt[2];   outMax1 = outExt[3];
  outMin2 = outExt[4];   outMax2 = outExt[5];

  // Neighborhood around current voxel
  kernelSize = self->GetKernelSize();
  kernelMiddle = self->GetKernelMiddle();
  hoodMin0 = kernelMiddle[0] - kernelSize[0]/2; // truncate on purpose
  hoodMin1 = kernelMiddle[1] - kernelSize[1]/2; // to round down odd sizes
  hoodMin2 = kernelMiddle[2] - kernelSize[2]/2;
  hoodMax0 = hoodMin0 + kernelSize[0];
  hoodMax1 = hoodMin1 + kernelSize[1];
  hoodMax2 = hoodMin2 + kernelSize[2];

  int outline = self->GetOutline();
  hoodMin0 = - outline;
  hoodMin1 = - outline;
  hoodMin2 = 0;
  hoodMax0 = outline;
  hoodMax1 = outline;
  hoodMax2 = 0;

  // in and out should be marching through corresponding pixels.
  target = (unsigned long)((outMax2-outMin2+1)*(outMax1-outMin1+1)/50.0);
  target++;

  // loop through pixels of output
  outPtr2 = outPtr;
  inPtr2 = (T*)(inData->GetScalarPointer(outMin0, outMin1, outMin2));
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
        inLabelValue = *inPtr0;
        // Default output equal to backgroundLabelValue
        // on the assumption this is not an outline pixel
        *outPtr0 = backgroundLabelValue;

        // look at neighborhood around non-background
        // pixels to see if there is a transition.
        // If there is, then this is an outline pixel
        if (inLabelValue != backgroundLabelValue)
          {
          // Loop through neighborhood pixels
          // Note: hood pointer marches out of bounds.
          // start at lower left pixel
          hoodPtr2 = inPtr0 + hoodMin0 * inInc0
                            + hoodMin1 * inInc1
                            + hoodMin2 * inInc2;

          for (hoodIdx2 = hoodMin2; hoodIdx2 <= hoodMax2; ++hoodIdx2)
            {
            hoodPtr1 = hoodPtr2;
            for (hoodIdx1 = hoodMin1; hoodIdx1 <= hoodMax1; ++hoodIdx1)
              {
              hoodPtr0 = hoodPtr1;
              for (hoodIdx0 = hoodMin0; hoodIdx0 <= hoodMax0; ++hoodIdx0)
                {
                // handle boundaries
                if (outIdx0 + hoodIdx0 >= inImageMin0 &&
                    outIdx0 + hoodIdx0 <= inImageMax0 &&
                    outIdx1 + hoodIdx1 >= inImageMin1 &&
                    outIdx1 + hoodIdx1 <= inImageMax1 &&
                    outIdx2 + hoodIdx2 >= inImageMin2 &&
                    outIdx2 + hoodIdx2 <= inImageMax2)
                  {
                  // If the neighbor value is not the same label value
                  // that means this is an outline pixel
                  // (border is within neighborhood).
                  // so set the output to foreground
                  if (*hoodPtr0 != inLabelValue)
                    {
                    *outPtr0 = inLabelValue;
                    }
                  }
                else
                  {
                  // neighborhood reaches outside of the input
                  // domain, so this is also an outline pixel
                  *outPtr0 = inLabelValue;
                  }
                hoodPtr0 += inInc0;
                }//for0
              hoodPtr1 += inInc1;
              }//for1
            hoodPtr2 += inInc2;
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
  vtkImageData *input = this->GetImageDataInput(0);
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

    if (this->GetInput() != nullptr)
      {
      os << indent << "Input0:\n";
      this->GetInput()->PrintSelf(os,indent.GetNextIndent());
      }
    if (this->GetOutput() != nullptr)
      {
      os << indent << "Output0:\n";
      this->GetOutput()->PrintSelf(os,indent.GetNextIndent());
      }
}

//----------------------------------------------------------------------------
void vtkImageLabelOutline::SetOutline(int outline)
{
  if (this->Outline == outline)
    {
    return;
    }

  this->Outline = outline;

  // also set the kernel size for 2D
  int kernelSize = (outline * 2) + 1;
  this->SetKernelSize(kernelSize, kernelSize, 1);

  this->Modified();
}
