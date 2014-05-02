/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageSetTensorComponents.h,v $
  Date:      $Date: 2006/01/06 17:58:05 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkImageSetTensorComponents_h
#define __vtkImageSetTensorComponents_h

#include "vtkTeemConfigure.h"

#include "vtkThreadedImageAlgorithm.h"

/// \brief Six scalar components to tensor.
///
/// vtkImageSetTensorComponents build from the input with 6 scalar components
/// the full tensor as output. Components are assumed to be ordered as follows:
/// 1 2 3
/// 2 4 5
/// 3 5 6
///
/// \sa vtkImageGetTensorComponents
class VTK_TEEM_EXPORT vtkImageSetTensorComponents : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageSetTensorComponents *New();
  vtkTypeMacro(vtkImageSetTensorComponents,vtkThreadedImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Set/Get the components to extract.
  vtkGetVector3Macro(Components,int);

  ///
  /// Get the number of components to extract. This is set implicitly by the
  /// SetComponents() method.
  vtkGetMacro(NumberOfComponents,int);

protected:
  vtkImageSetTensorComponents();
  ~vtkImageSetTensorComponents() {};

  int NumberOfComponents;
  int Components[3];

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                       int ext[6], int id);

  /// We override this in order to allocate output tensors
  /// before threading happens.  This replaces the superclass
  /// vtkImageAlgorithm's Execute function.
  void ExecuteData(vtkDataObject *out);

//private:
///  vtkImageSetTensorComponents(const vtkImageSetTensorComponents&);  /// Not //implemented.
///  void operator=(const vtkImageSetTensorComponents&);  /// Not implemented.
};

#endif
