/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
#include "vtkImageLabelChange.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkImageLabelChange);


//----------------------------------------------------------------------------
// Description:
// Constructor sets default values
vtkImageLabelChange::vtkImageLabelChange()
{
    this->InputLabel = 0;
    this->OutputLabel = 0;
}

//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageLabelChangeExecute(vtkImageLabelChange *self,
                     vtkImageData *vtkNotUsed(inData), T *inPtr,
                     vtkImageData *outData,
                     int outExt[6], int vtkNotUsed(id))
{
    T *outPtr = (T *)outData->GetScalarPointerForExtent(outExt);
    // looping
    vtkIdType outIncX, outIncY, outIncZ;
    int idxX, idxY, idxZ, maxX, maxY, maxZ;
    // Other
    T inLabel = (T) self->GetInputLabel();
    T outLabel = (T) self->GetOutputLabel();

    // Get increments to march through data
    outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
    maxX = outExt[1] - outExt[0];
    maxY = outExt[3] - outExt[2];
    maxZ = outExt[5] - outExt[4];

    // Loop through output pixels
    for (idxZ = 0; idxZ <= maxZ; idxZ++) {
        for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++) {
            for (idxX = 0; idxX <= maxX; idxX++) {
                if (*inPtr == inLabel)
                    *outPtr = outLabel;
                else
                    *outPtr = *inPtr;
                outPtr++;
                inPtr++;
            }
            outPtr += outIncY;
            inPtr += outIncY;
        }
        outPtr += outIncZ;
        inPtr += outIncZ;
    }
}


//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageLabelChange::ThreadedExecute(vtkImageData *inData,
                    vtkImageData *outData,
                    int outExt[6], int id)
{
    // Check Single component
    int x1;
    x1 = inData->GetNumberOfScalarComponents();
    if (x1 != 1) {
    vtkErrorMacro(<<"Input has "<<x1<<" instead of 1 scalar component.");
        return;
    }

    void *inPtr = inData->GetScalarPointerForExtent(outExt);

    switch (inData->GetScalarType())
    {
        case VTK_DOUBLE:
        vtkImageLabelChangeExecute(this, inData, (double *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_FLOAT:
        vtkImageLabelChangeExecute(this, inData, (float *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_LONG:
        vtkImageLabelChangeExecute(this, inData, (long *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_UNSIGNED_LONG:
        vtkImageLabelChangeExecute(this, inData, (unsigned long *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_INT:
        vtkImageLabelChangeExecute(this, inData, (int *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_UNSIGNED_INT:
        vtkImageLabelChangeExecute(this, inData, (unsigned int *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_SHORT:
        vtkImageLabelChangeExecute(this, inData, (short *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_UNSIGNED_SHORT:
        vtkImageLabelChangeExecute(this, inData, (unsigned short *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_CHAR:
        vtkImageLabelChangeExecute(this, inData, (char *)(inPtr),
            outData, outExt, id);
        break;
    case VTK_UNSIGNED_CHAR:
        vtkImageLabelChangeExecute(this, inData, (unsigned char *)(inPtr),
            outData, outExt, id);
        break;
    default:
        vtkErrorMacro(<< "Execute: Unknown input ScalarType");
        return;
    }
}

//----------------------------------------------------------------------------
void vtkImageLabelChange::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);

    os << indent << "InputLabel: " << this->InputLabel << "\n";
    os << indent << "OutputLabel: " << this->OutputLabel << "\n";
}

