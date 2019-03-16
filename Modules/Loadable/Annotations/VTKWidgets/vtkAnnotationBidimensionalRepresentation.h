/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef __vtkAnnotationBidimensionalRepresentation_h
#define __vtkAnnotationBidimensionalRepresentation_h

// Annotations includes
#include "vtkSlicerAnnotationsModuleVTKWidgetsExport.h"

// VTK includes
#include <vtkBiDimensionalRepresentation2D.h>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_VTKWIDGETS_EXPORT
vtkAnnotationBidimensionalRepresentation
  : public vtkBiDimensionalRepresentation2D
{
public:

  static vtkAnnotationBidimensionalRepresentation *New();
  vtkTypeMacro(vtkAnnotationBidimensionalRepresentation, vtkBiDimensionalRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetDistance1(double distance);
  void SetDistance2(double distance);

protected:

  vtkAnnotationBidimensionalRepresentation();
  ~vtkAnnotationBidimensionalRepresentation() override;

  void BuildRepresentation() override;

private:

  vtkAnnotationBidimensionalRepresentation(const vtkAnnotationBidimensionalRepresentation&) = delete;
  void operator=(const vtkAnnotationBidimensionalRepresentation&) = delete;

  double m_Distance1;
  double m_Distance2;

};

#endif /* __vtkAnnotationBidimensionalRepresentation_h */
