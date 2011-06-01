/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKANNOTATIONRULERREPRESENTATION3D_H_
#define VTKANNOTATIONRULERREPRESENTATION3D_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// VTK includes
#include <vtkDistanceRepresentation3D.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkAnnotationRulerRepresentation3D :
    public vtkDistanceRepresentation3D
{
public:

  static vtkAnnotationRulerRepresentation3D *New();
  vtkTypeRevisionMacro(vtkAnnotationRulerRepresentation3D, vtkDistanceRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetDistance(double distance);

protected:

  vtkAnnotationRulerRepresentation3D();
  virtual ~vtkAnnotationRulerRepresentation3D();

  void BuildRepresentation();

private:

  vtkAnnotationRulerRepresentation3D(const vtkAnnotationRulerRepresentation3D&); /// Not implemented
  void operator=(const vtkAnnotationRulerRepresentation3D&); /// Not Implemented

  double m_Distance;

};

#endif /* VTKANNOTATIONRULERREPRESENTATION3D_H_ */
