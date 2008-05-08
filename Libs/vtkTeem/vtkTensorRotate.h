/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTensorRotate.h,v $
  Date:      $Date: 2006/06/27 20:53:19 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkTensorRotate - flip Y axis and negate xy and zy terms
// .SECTION Description
// Make tend estim generated vtk files compatible with slicer
// .SECTION Warning
// The filter will always output floating point (loose precision)
// explicit use of vtkFloatArray


#ifndef __vtkTensorRotate_h
#define __vtkTensorRotate_h

#include "vtkTeemConfigure.h"
#include "vtkImageToImageFilter.h"
#include "vtkTransform.h"

class vtkFloatArray;
class vtkImageData;
class VTK_TEEM_EXPORT vtkTensorRotate : public vtkImageToImageFilter
{
public:
  static vtkTensorRotate *New();
  vtkTypeRevisionMacro(vtkTensorRotate,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Set the tensor type for the filter
  void SetTensorTypeToFloat(){this->SetTensorType(VTK_FLOAT);};
  void SetTensorTypeToDouble(){this->SetTensorType(VTK_DOUBLE);};
  vtkSetMacro(TensorType, int);
  vtkGetMacro(TensorType, int);

  // Set/Get transformation to use to rotate the tensor
  vtkSetObjectMacro(Transform,vtkTransform);
  vtkGetObjectMacro(Transform,vtkTransform);

protected:
  vtkTensorRotate();
  ~vtkTensorRotate();
  vtkTensorRotate(const vtkTensorRotate&);
  void operator=(const vtkTensorRotate&);

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->Superclass::ExecuteInformation();};
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
        int extent[6], int id);


  // This also copies other arrays from point and cell data from input to output.
  vtkImageData *AllocateOutputData(vtkDataObject *out);
  void AllocateTensors(vtkImageData *data);

  int TensorType;

  vtkTransform *Transform;


};

#endif













