/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageGetTensorComponents.h,v $
  Date:      $Date: 2006/01/06 17:58:05 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkImageGetTensorComponents_h
#define __vtkImageGetTensorComponents_h

#include "vtkTeemConfigure.h"

#include "vtkThreadedImageAlgorithm.h"

/// \brief Six scalar components from tensor.
///
/// vtkImageGetTensorComponents gets from the tensor the 6 components and
/// exports them as scalar components, ordered as follows:
/// 1 2 3
/// 2 4 5
/// 3 5 6
///
/// \sa vtkImageSetTensorComponents
class VTK_Teem_EXPORT vtkImageGetTensorComponents : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageGetTensorComponents *New();
  vtkTypeMacro(vtkImageGetTensorComponents,vtkThreadedImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Set/Get the components to extract.
  vtkGetVector3Macro(Components,int);

  ///
  /// Get the number of components to extract. This is set implicitly by the
  /// SetComponents() method.
  vtkGetMacro(NumberOfComponents,int);

protected:
  vtkImageGetTensorComponents();
  ~vtkImageGetTensorComponents() {};

  int NumberOfComponents;
  int Components[3];

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                       int ext[6], int id);
private:
  vtkImageGetTensorComponents(const vtkImageGetTensorComponents&);  /// Not implemented.
  void operator=(const vtkImageGetTensorComponents&);  /// Not implemented.
};

#endif
