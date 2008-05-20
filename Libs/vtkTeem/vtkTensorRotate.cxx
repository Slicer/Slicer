/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTensorRotate.cxx,v $
  Date:      $Date: 2006/06/27 20:53:19 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
#include "vtkTensorRotate.h"

#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkMatrix4x4.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"


vtkCxxRevisionMacro(vtkTensorRotate, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkTensorRotate);


//----------------------------------------------------------------------------
vtkTensorRotate::vtkTensorRotate()
{
  this->TensorType = VTK_VOID;
  this->SetTensorType(VTK_FLOAT);
  this->Transform = vtkTransform::New();
}

vtkTensorRotate::~vtkTensorRotate()
{
  if (this->Transform != NULL) {
    this->Transform->Delete();
  }
}

//----------------------------------------------------------------------------
//
void vtkTensorRotate::ExecuteInformation(vtkImageData *inData,
                                       vtkImageData *outData)
{
  int ext[6];

  vtkDataArray *tensorArray = inData->GetPointData()->GetTensors();
  // Make sure the Input has been set.
  if ( tensorArray == NULL)
    {
    vtkErrorMacro(<< "ExecuteInformation: Input does not contain a Tensor field.");
    return;
    }

  inData->GetWholeExtent(ext);
  outData->SetWholeExtent(ext);
}

//----------------------------------------------------------------------------
vtkImageData *vtkTensorRotate::AllocateOutputData(vtkDataObject *out)
{ 
  vtkImageData *output = vtkImageData::SafeDownCast(out);
  vtkImageData *input = this->GetInput();
  int inExt[6];
  int outExt[6];
  vtkDataArray *inArray;
  vtkDataArray *inTensor;
  vtkDataArray *outArray;

  input->GetExtent(inExt);
  output->SetExtent(output->GetUpdateExtent());
  output->GetExtent(outExt);

  // Do not copy the array we will be generating.
  inArray = input->GetPointData()->GetScalars(this->InputScalarsSelection);
  inTensor = input->GetPointData()->GetTensors();

  // Conditionally copy point and cell data.
  // Only copy if corresponding indexes refer to identical points.
  double *oIn = input->GetOrigin();
  double *sIn = input->GetSpacing();
  double *oOut = output->GetOrigin();
  double *sOut = output->GetSpacing();
  if (oIn[0] == oOut[0] && oIn[1] == oOut[1] && oIn[2] == oOut[2] &&
      sIn[0] == sOut[0] && sIn[1] == sOut[1] && sIn[2] == sOut[2])   
    {
    output->GetPointData()->CopyAllOn();
    output->GetCellData()->CopyAllOn();
    // Scalar copy flag trumps the array copy flag.
    if (inArray == input->GetPointData()->GetScalars())
      {
      output->GetPointData()->CopyScalarsOff();
      }
    else
      {
      output->GetPointData()->CopyFieldOff(this->InputScalarsSelection);
      }
    if (inTensor == input->GetPointData()->GetTensors())
      {
      output->GetPointData()->CopyTensorsOff();
      }

    // If the extents are the same, then pass the attribute data for efficiency.
    if (inExt[0] == outExt[0] && inExt[1] == outExt[1] &&
        inExt[2] == outExt[2] && inExt[3] == outExt[3] &&
        inExt[4] == outExt[4] && inExt[5] == outExt[5])
      {// Pass
      output->GetPointData()->PassData(input->GetPointData());
      output->GetCellData()->PassData(input->GetCellData());
      }
    else
      {// Copy
       // Since this can be expensive to copy all of these values,
       // lets make sure there are arrays to copy (other than the scalars)
      if (input->GetPointData()->GetNumberOfArrays() > 1)
        {
        // Copy the point data.
        // CopyAllocate frees all arrays.
        // Keep the old scalar array (not being copied).
        // This is a hack, but avoids reallocation ...
        vtkDataArray *tmp = NULL;
        vtkDataArray *tmp2 = NULL;
        if ( ! output->GetPointData()->GetCopyScalars() )
          {
          tmp = output->GetPointData()->GetScalars();
          }
        if ( ! output->GetPointData()->GetCopyTensors() )
          {
          tmp2 = output->GetPointData()->GetTensors();
          }
        output->GetPointData()->CopyAllocate(input->GetPointData(), 
                                             output->GetNumberOfPoints());
        if (tmp)
          { // Restore the array.
          output->GetPointData()->SetScalars(tmp);
          }
        if (tmp2)
          {
          output->GetPointData()->SetTensors(tmp2);
          }
        // Now Copy The point data, but only if output is a subextent of the input.  
        if (outExt[0] >= inExt[0] && outExt[1] <= inExt[1] &&
            outExt[2] >= inExt[2] && outExt[3] <= inExt[3] &&
            outExt[4] >= inExt[4] && outExt[5] <= inExt[5])
          {
          output->GetPointData()->CopyStructuredData(input->GetPointData(),
                                                     inExt, outExt);
          }
        }

      if (input->GetCellData()->GetNumberOfArrays() > 0)
        {
        output->GetCellData()->CopyAllocate(input->GetCellData(), 
                                            output->GetNumberOfCells());  
        // Cell extent is one less than point extent.
        // Conditional to handle a colapsed axis (lower dimensional cells).
        if (inExt[0] < inExt[1]) {--inExt[1];}
        if (inExt[2] < inExt[3]) {--inExt[3];}
        if (inExt[4] < inExt[5]) {--inExt[5];}
        // Cell extent is one less than point extent.
        if (outExt[0] < outExt[1]) {--outExt[1];}
        if (outExt[2] < outExt[3]) {--outExt[3];}
        if (outExt[4] < outExt[5]) {--outExt[5];}
        // Now Copy The cell data, but only if output is a subextent of the input.  
        if (outExt[0] >= inExt[0] && outExt[1] <= inExt[1] &&
            outExt[2] >= inExt[2] && outExt[3] <= inExt[3] &&
            outExt[4] >= inExt[4] && outExt[5] <= inExt[5])
          {
          output->GetCellData()->CopyStructuredData(input->GetCellData(),
                                                    inExt, outExt);
          }
        }
      }
    }
  
  // Now create the scalars and tensors array that will hold the output data.
  this->ExecuteInformation();
  output->AllocateScalars();
  this->AllocateTensors(output);

  outArray = output->GetPointData()->GetScalars();
  if (inArray)
    {
    outArray->SetName(inArray->GetName());
    }

  outArray = output->GetPointData()->GetTensors();
  if (inArray)
    {
    outArray->SetName(inArray->GetName());
    }

  return output;
}

//----------------------------------------------------------------------------
void vtkTensorRotate::AllocateTensors(vtkImageData *data)
{
  vtkDataArray *tensors;
  

  if (data != NULL)
    {
    vtkErrorMacro("Input has not been assigned.");
    return;
    }

  // if the scalar type has not been set then we have a problem
  if (this->TensorType == VTK_VOID)
    {
    vtkErrorMacro("Attempt to allocate tensors before scalar type was set!.");
    return;
    }

  int ext[6];
  data->GetExtent(ext);

  // if we currently have scalars then just adjust the size
  tensors = data->GetPointData()->GetTensors();
  if (tensors && tensors->GetDataType() == this->TensorType
      && tensors->GetReferenceCount() == 1) 
    {
    tensors->SetNumberOfComponents(9);
    tensors->SetNumberOfTuples((ext[1] - ext[0] + 1)*
                               (ext[3] - ext[2] + 1)*
                               (ext[5] - ext[4] + 1));
    // Since the execute method will be modifying the scalars
    // directly.
    tensors->Modified();
    return;
    }
  
  // allocate the new scalars
  switch (this->TensorType)
    {
    case VTK_DOUBLE:
      tensors = vtkDoubleArray::New();
      break;
    case VTK_FLOAT:
      tensors = vtkFloatArray::New();
      break;
    default:
      vtkErrorMacro("Could not allocate data type.");
      return;
    }
  
  tensors->SetNumberOfComponents(9);

  // allocate enough memory
  tensors->
    SetNumberOfTuples((ext[1] - ext[0] + 1)*
                      (ext[3] - ext[2] + 1)*
                      (ext[5] - ext[4] + 1));

  data->GetPointData()->SetTensors(tensors);
  tensors->Delete();
}


//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// this is for when only tensors are present
template <class D>
static void vtkTensorRotateExecute(vtkTensorRotate *self, int outExt[6],
                       vtkImageData *inData,
                       vtkImageData *outData, D *outPtr, int id)
{
//  int num0, num1, num2;
//  int idx0, idx1, idx2;
  vtkIdType outInc0, outInc1, outInc2;
  unsigned long count = 0;
  unsigned long target;

  int maxY, maxZ;
  vtkIdType inInc[3];
  int inFullUpdateExt[6];

  int idxZ, idxY, idxR;

  vtkDataArray *inTensors = NULL;
  vtkDataArray *outTensors = NULL;
  double inT[3][3];
  double outT[3][3];
  vtkMatrix4x4 *inTM = vtkMatrix4x4::New();
  vtkMatrix4x4 *outTM = vtkMatrix4x4::New();
  vtkMatrix4x4 *rotMatrix = vtkMatrix4x4::New();
  vtkMatrix4x4 *rotMatrixT = vtkMatrix4x4::New();

  int ptId;

  // input tensors come from the inData
  inTensors = inData->GetPointData()->GetTensors();
  // output tensors are an ivar that gets put on the output later (avoids default overwrite by pipeline)
  outTensors = (vtkDataArray *) outData->GetPointData()->GetTensors();

  // rotation matrix to apply to the input tensor
  rotMatrix->DeepCopy(self->GetTransform()->GetMatrix());
  // Set translation part to zero (we only want to rotate the tensor)
  for (int k=0; k<3; k++)
    {
    rotMatrix->SetElement(k,3,0.0);
    }

  //Rotation matrix transpose
  rotMatrix->Transpose(rotMatrix,rotMatrixT);


  inData->GetIncrements(inInc);
  inData->GetUpdateExtent(inFullUpdateExt); //We are only working over the update extent
  ptId = ((outExt[0] - inFullUpdateExt[0]) * inInc[0]
        + (outExt[2] - inFullUpdateExt[2]) * inInc[1]
        + (outExt[4] - inFullUpdateExt[4]) * inInc[2]);

  // Get the full size of the output so we can mirror the Y axis
  int *outFullDims = outData->GetDimensions();
//  int ptIdOut;

  // Get information to march through data
  outData->GetContinuousIncrements(outExt, outInc0, outInc1, outInc2);
 
  // find the output region to loop over
  int rowLength = (outExt[1] - outExt[0]+1);
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;

  // Loop through ouput pixels
 for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; idxY <= maxY; idxY++)
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
        inTensors->GetTuple(ptId,(double *) inT);

        // inT is a [3][3] array with the tensor

        for (int j=0; j<3; j++)
          {
          for (int i=0; i<3; i++)
            {
            inTM->SetElement(i,j,inT[i][j]);
            }
          }
        // Perform rotation outT = R T R^(t)
       rotMatrix->Multiply4x4(inTM,rotMatrixT,outTM);
       rotMatrix->Multiply4x4(rotMatrix,outTM,outTM);

       // Save tensor
       for (int j=0; j<3; j++)
          {
          for (int i=0; i<3; i++)
            {
            outT[i][j]=outTM->GetElement(i,j);
            }
          }

        // set the output tensor to the calculated one
        outTensors->SetTuple(ptId,(double *) outT);

        ptId += 1;
        }
      ptId += outInc1;
      }
    ptId += outInc2;
    }

// Delete helper objects
rotMatrix->Delete();
rotMatrixT->Delete();
inTM->Delete();
outTM->Delete();

}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkTensorRotate::ThreadedExecute(vtkImageData *inData,
                      vtkImageData *outData,
                      int outExt[6], int id)
{
  void *outPtr;

  vtkDebugMacro(<< "Execute: inData = " << inData
        << ", outData = " << outData);

  if (inData == NULL)
    {
      vtkErrorMacro(<< "Input must be specified.");
      return;
    }

  outPtr = outData->GetScalarPointerForExtent(outExt);

  switch (outData->GetScalarType())
    {
      // we set the output data scalar type depending on the op
      // already.  And we only access the input tensors
      // which are float.  So this switch statement on output
      // scalar type is sufficient.
      vtkTemplateMacro6(vtkTensorRotateExecute,
                this, outExt, inData, outData,
                (VTK_TT *)(outPtr), id);
      default:
        vtkErrorMacro(<< "Execute: Unknown ScalarType");
        break;
    }
    return;
}

//----------------------------------------------------------------------------
void vtkTensorRotate::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
