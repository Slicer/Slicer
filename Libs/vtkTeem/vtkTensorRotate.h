/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTensorRotate.h,v $
  Date:      $Date: 2006/06/27 20:53:19 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkTensorRotate_h
#define __vtkTensorRotate_h

#include "vtkTeemConfigure.h"
#include "vtkThreadedImageAlgorithm.h"
#include "vtkTransform.h"

class vtkFloatArray;
class vtkImageData;

/// \brief  flip Y axis and negate xy and zy terms.
///
/// Make tend estim generated vtk files compatible with slicer
///
/// \warning The filter will always output floating point (loose precision)
/// explicit use of vtkFloatArray.
class VTK_Teem_EXPORT vtkTensorRotate : public vtkThreadedImageAlgorithm
{
public:
  static vtkTensorRotate *New();
  vtkTypeRevisionMacro(vtkTensorRotate,vtkThreadedImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Set the tensor type for the filter
  void SetTensorTypeToFloat(){this->SetTensorType(VTK_FLOAT);};
  void SetTensorTypeToDouble(){this->SetTensorType(VTK_DOUBLE);};
  vtkSetMacro(TensorType, int);
  vtkGetMacro(TensorType, int);

  /// Set/Get transformation to use to rotate the tensor
  vtkSetObjectMacro(Transform,vtkTransform);
  vtkGetObjectMacro(Transform,vtkTransform);

protected:
  vtkTensorRotate();
  ~vtkTensorRotate();
  vtkTensorRotate(const vtkTensorRotate&);
  void operator=(const vtkTensorRotate&);

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
        int extent[6], int id);

  /// This also copies other arrays from point and cell data from input to output.
  vtkImageData *AllocateOutputData(vtkDataObject *out);
  virtual void AllocateOutputData(vtkImageData *out, int *uExtent)
    { Superclass::AllocateOutputData(out, uExtent); }
  void AllocateTensors(vtkImageData *data);

  int TensorType;

  vtkTransform *Transform;

};

#endif
