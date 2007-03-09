/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageThermalMap.cxx,v $
  Date:      $Date: 2006/03/23 18:45:38 $
  Version:   $Revision: 1.1.2.7 $

=========================================================================auto=*/
#include "vtkImageThermalMap.h"
#include "vtkObjectFactory.h"
#include <math.h>


//------------------------------------------------------------------------------
vtkImageThermalMap* vtkImageThermalMap::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageThermalMap");
    if(ret)
    {
        return (vtkImageThermalMap*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkImageThermalMap;
}


//----------------------------------------------------------------------------
// Description:
// Constructor sets default values
vtkImageThermalMap::vtkImageThermalMap()
{
    // all inputs 
    this->NumberOfRequiredInputs = 4;
    this->NumberOfInputs = 0;
}


//----------------------------------------------------------------------------
vtkImageThermalMap::~vtkImageThermalMap()
{
}


//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
template <class IT, class OT>
static void vtkImageThermalMapExecute(
        vtkImageThermalMap *self,
        vtkImageData **inDatas, 
        IT **inPtrs,
        vtkImageData *outData,
        OT *outPtr,
        int outExt[6], 
        int id)
{
    int idxR, idxY, idxZ;
    int maxY, maxZ;
    int inIncX, inIncY, inIncZ;
    int outIncX, outIncY, outIncZ;
    int rowLength;
    unsigned long count = 0;
    unsigned long target;

    // Get increments to march through data 
    // all indatas are the same type, so use the same increments
    inDatas[0]->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
    outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

    // find the region to loop over
    rowLength = (outExt[1] - outExt[0]+1)*inDatas[0]->GetNumberOfScalarComponents();
    maxY = outExt[3] - outExt[2]; 
    maxZ = outExt[5] - outExt[4];
    target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
    target++;

    int numberOfInputs = self->GetNumberOfInputs(); 

    // Loop through input pixels
    for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
        for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
        {
            if (!id) 
            {
                if (!(count%target))
                {
                    self->UpdateProgress(count/(50.0*target));
                }
                count++;
            }
            for (idxR = 0; idxR < rowLength; idxR++)
            {
                // Pixel operation: compute thermal volume
                float t  = 0.0;
                float pi = 3.1415926535897931;

                float r1 = (*inPtrs[0]);
                float i1 = (*inPtrs[1]);
                float r2 = (*inPtrs[2]);
                float i2 = (*inPtrs[3]);
                float xx = r1 * r2 + i1 * i2;
                float yy = i1 * r2 - r1 * i2;

                if (xx != 0) 
                {
                    // atan2 the same as matlab:
                    t = atan2(yy,xx);

                    // get temperature
                    // add the base temperature of human body: 37
                    t = 37 + (t / 2.0 / pi / self->GetTE() / self->GetW0() / (-1 * self->GetTC()));
                } 
                else 
                {
                    if (yy > 0) 
                    {
                        t = pi / 2;
                    } 
                    else if (yy < 0) 
                    {
                        t = -1 * pi / 2;
                    }
                }

                *outPtr = (OT)t;
                outPtr++;

                for (int i = 0; i < numberOfInputs; i++)
                {
                    (inPtrs[i])++;
                }
            }
            outPtr += outIncY;
            for (int i = 0; i < numberOfInputs; i++)
            {
                inPtrs[i] += inIncY;
            }
        }
        outPtr += outIncZ;
        for (int i = 0; i < numberOfInputs; i++)
        {
            inPtrs[i] += inIncZ;
        }
    }
}


//----------------------------------------------------------------------------
// Description:
// This method is passed input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageThermalMap::ThreadedExecute(
        vtkImageData **inDatas, 
        vtkImageData *outData,
        int outExt[6], 
        int id)
{
    void **inPtrs = new void* [this->NumberOfInputs];
    for (int i = 0; i < this->NumberOfInputs; i++) 
    {
        inPtrs[i] = inDatas[i]->GetScalarPointerForExtent(outExt);
    }

    void *outPtr = outData->GetScalarPointerForExtent(outExt);

    switch (inDatas[0]->GetScalarType())
    {
        vtkTemplateMacro7(vtkImageThermalMapExecute,
                this, 
                inDatas, 
                (short **)inPtrs, 
                outData, 
                (short *)outPtr,
                outExt, 
                id);
            break;
        default:
            vtkErrorMacro(<< "Execute: Unknown input ScalarType");
            return;
    }

    delete [] inPtrs;
}


//----------------------------------------------------------------------------
// Make sure all the inputs are the same size. Doesn't really change 
// the output. Just performs a sanity check
void vtkImageThermalMap::ExecuteInformation(
        vtkImageData **inputs,
        vtkImageData *output)
{
    int *in1Ext, *in2Ext;

    // we require that all inputs have been set.
    if (this->NumberOfInputs < this->NumberOfRequiredInputs)
    {
        vtkErrorMacro(<< "ExecuteInformation: Expected " 
                      << this->NumberOfRequiredInputs 
                      << " inputs, got only " 
                      << this->NumberOfInputs);
        return;      
    }

    // Check that all extents are the same.
    in1Ext = inputs[0]->GetWholeExtent();
    for (int i = 1; i < this->NumberOfInputs; i++) 
    {
        in2Ext = inputs[i]->GetWholeExtent();

        if (in1Ext[0] != in2Ext[0] || 
            in1Ext[1] != in2Ext[1] || 
            in1Ext[2] != in2Ext[2] || 
            in1Ext[3] != in2Ext[3] || 
            in1Ext[4] != in2Ext[4] || 
            in1Ext[5] != in2Ext[5])
        {
            vtkErrorMacro(<< "ExecuteInformation: Inputs 0 and " 
                          << i 
                          << " are not the same size. " 
                          << in1Ext[0] << " " 
                          << in1Ext[1] << " " 
                          << in1Ext[2] << " " 
                          << in1Ext[3] << " vs: "
                          << in2Ext[0] << " " 
                          << in2Ext[1] << " " 
                          << in2Ext[2] << " " 
                          << in2Ext[3] );
            return;
        }
    }

    // we like longs
    output->SetNumberOfScalarComponents(1);
    output->SetScalarType(VTK_SHORT);
}


void vtkImageThermalMap::PrintSelf(
        ostream& os, 
        vtkIndent indent)
{
    vtkImageMultipleInputFilter::PrintSelf(os,indent);
}

