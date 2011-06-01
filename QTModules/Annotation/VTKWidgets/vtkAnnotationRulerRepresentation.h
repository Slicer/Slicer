/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKANNOTATIONRULERREPRESENTATION_H_
#define VTKANNOTATIONRULERREPRESENTATION_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// VTK includes
#include <vtkDistanceRepresentation2D.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkAnnotationRulerRepresentation :
    public vtkDistanceRepresentation2D
{
public:

  static vtkAnnotationRulerRepresentation *New();
  vtkTypeRevisionMacro(vtkAnnotationRulerRepresentation, vtkDistanceRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetDistance(double distance);

  // Description:
  // Return the property of the axis actor
  virtual vtkProperty2D *GetLineProperty();

protected:

  vtkAnnotationRulerRepresentation();
  virtual ~vtkAnnotationRulerRepresentation();

  void BuildRepresentation();

private:

  vtkAnnotationRulerRepresentation(const vtkAnnotationRulerRepresentation&); /// Not implemented
  void operator=(const vtkAnnotationRulerRepresentation&); /// Not Implemented

  double m_Distance;

};

#endif /* VTKANNOTATIONRULERREPRESENTATION_H_ */
