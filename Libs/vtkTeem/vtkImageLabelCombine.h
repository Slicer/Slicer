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
// .NAME vtkImageLabelCombine - Add, subtract, multiply, divide, invert, sin, cos, exp, log.
// .SECTION Description
// vtkImageLabelCombine implements basic mathematic operations SetOperation is
// used to select the filters behavior.  The filter can take two or one
// input.


#ifndef __vtkImageLabelCombine_h
#define __vtkImageLabelCombine_h

#include "vtkTeemConfigure.h"

#include "vtkThreadedImageAlgorithm.h"

class VTK_TEEM_EXPORT vtkImageLabelCombine : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageLabelCombine *New();
  vtkTypeRevisionMacro(vtkImageLabelCombine,vtkThreadedImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the Operation to perform.
  vtkSetMacro(OverwriteInput,int);
  vtkGetMacro(OverwriteInput,int);

  // Description:
  // Set the two inputs to this filter
  virtual void SetInput1(vtkDataObject *in) { this->SetInput(0,in); }
  virtual void SetInput2(vtkDataObject *in) { this->SetInput(1,in); }

protected:
  vtkImageLabelCombine();
  ~vtkImageLabelCombine() {};

  int OverwriteInput;
  
  virtual int RequestInformation (vtkInformation *, 
                                  vtkInformationVector **,
                                  vtkInformationVector *);
  
  virtual void ThreadedRequestData(vtkInformation *request, 
                                   vtkInformationVector **inputVector, 
                                   vtkInformationVector *outputVector,
                                   vtkImageData ***inData, 
                                   vtkImageData **outData,
                                   int extent[6], int threadId);

  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkImageLabelCombine(const vtkImageLabelCombine&);  // Not implemented.
  void operator=(const vtkImageLabelCombine&);  // Not implemented.
};

#endif













