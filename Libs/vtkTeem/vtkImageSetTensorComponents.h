/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageSetTensorComponents.h,v $
  Date:      $Date: 2006/01/06 17:58:05 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkImageSetTensorComponents - Six scalar components to tensor
// .SECTION Description
// vtkImageSetTensorComponents build from the input with 6 scalar components
// the full tensor as output. Components are assumed to be ordered as follows:
// 1 2 3
// 2 4 5
// 3 5 6

// .SECTION See Also
// vtkImageGetTensorComponents

#ifndef __vtkImageSetTensorComponents_h
#define __vtkImageSetTensorComponents_h

#include "vtkTeemConfigure.h"

#include "vtkImageToImageFilter.h"

class VTK_TEEM_EXPORT vtkImageSetTensorComponents : public vtkImageToImageFilter
{
public:
  static vtkImageSetTensorComponents *New();
  vtkTypeMacro(vtkImageSetTensorComponents,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the components to extract.
  vtkGetVector3Macro(Components,int);
  
  // Description:
  // Get the number of components to extract. This is set implicitly by the 
  // SetComponents() method.
  vtkGetMacro(NumberOfComponents,int);

protected:
  vtkImageSetTensorComponents();
  ~vtkImageSetTensorComponents() {};

  int NumberOfComponents;
  int Components[3];

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
                       int ext[6], int id);

  // We override this in order to allocate output tensors
  // before threading happens.  This replaces the superclass 
  // vtkImageMultipleInputFilter's Execute function.
  void ExecuteData(vtkDataObject *out);
               
//private:
//  vtkImageSetTensorComponents(const vtkImageSetTensorComponents&);  // Not //implemented.
//  void operator=(const vtkImageSetTensorComponents&);  // Not implemented.
};

#endif










