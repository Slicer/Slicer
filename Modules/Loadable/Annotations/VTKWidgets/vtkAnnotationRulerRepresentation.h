/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef __vtkAnnotationRulerRepresentation_h
#define __vtkAnnotationRulerRepresentation_h

// Annotations includes
#include "vtkSlicerAnnotationsModuleVTKWidgetsExport.h"

// VTK includes
#include <vtkDistanceRepresentation2D.h>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_VTKWIDGETS_EXPORT
vtkAnnotationRulerRepresentation
  : public vtkDistanceRepresentation2D
{
public:

  static vtkAnnotationRulerRepresentation *New();
  vtkTypeMacro(vtkAnnotationRulerRepresentation, vtkDistanceRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetDistance(double distance);

  // Description:
  // Return the property of the axis actor
  virtual vtkProperty2D *GetLineProperty();

protected:

  vtkAnnotationRulerRepresentation();
  ~vtkAnnotationRulerRepresentation() override;

  void BuildRepresentation() override;

private:

  vtkAnnotationRulerRepresentation(const vtkAnnotationRulerRepresentation&) = delete;
  void operator=(const vtkAnnotationRulerRepresentation&) = delete;

  double m_Distance;

};

#endif /* __vtkAnnotationRulerRepresentation_h */
