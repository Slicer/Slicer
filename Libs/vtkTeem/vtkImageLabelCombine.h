/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageLabelCombine.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkImageLabelCombine_h
#define __vtkImageLabelCombine_h

#include <vtkVersion.h>

#include "vtkTeemConfigure.h"

#include "vtkThreadedImageAlgorithm.h"

/// \brief Add, subtract, multiply, divide, invert, sin, cos, exp, log.
///
/// vtkImageLabelCombine implements basic mathematic operations SetOperation is
/// used to select the filters behavior.  The filter can take two or one
/// input.
class VTK_Teem_EXPORT vtkImageLabelCombine : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageLabelCombine *New();
  vtkTypeMacro(vtkImageLabelCombine,vtkThreadedImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Set/Get the Operation to perform.
  vtkSetMacro(OverwriteInput,int);
  vtkGetMacro(OverwriteInput,int);

  ///
  /// Set the two inputs to this filter
  virtual void SetInput1(vtkDataObject *in)
  {
      this->SetInputData(0,in);
  }
  virtual void SetInput2(vtkDataObject *in)
  {
      this->SetInputData(1,in);
  }

protected:
  vtkImageLabelCombine();
  ~vtkImageLabelCombine() override  = default;

  int OverwriteInput;

  int RequestInformation (vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *) override;

  void ThreadedRequestData(vtkInformation *request,
                                   vtkInformationVector **inputVector,
                                   vtkInformationVector *outputVector,
                                   vtkImageData ***inData,
                                   vtkImageData **outData,
                                   int extent[6], int threadId) override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkImageLabelCombine(const vtkImageLabelCombine&) = delete;
  void operator=(const vtkImageLabelCombine&) = delete;
};

#endif
