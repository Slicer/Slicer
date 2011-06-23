/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKANNOTATIONBIDIMENSIONALREPRESENTATION_H_
#define VTKANNOTATIONBIDIMENSIONALREPRESENTATION_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// VTK includes
#include <vtkBiDimensionalRepresentation2D.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkAnnotationBidimensionalRepresentation :
    public vtkBiDimensionalRepresentation2D
{
public:

  static vtkAnnotationBidimensionalRepresentation *New();
  vtkTypeRevisionMacro(vtkAnnotationBidimensionalRepresentation, vtkBiDimensionalRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetDistance1(double distance);
  void SetDistance2(double distance);

protected:

  vtkAnnotationBidimensionalRepresentation();
  virtual ~vtkAnnotationBidimensionalRepresentation();

  void BuildRepresentation();

private:

  vtkAnnotationBidimensionalRepresentation(const vtkAnnotationBidimensionalRepresentation&); /// Not implemented
  void operator=(const vtkAnnotationBidimensionalRepresentation&); /// Not Implemented

  double m_Distance1;
  double m_Distance2;

};

#endif /* VTKANNOTATIONRULERREPRESENTATION_H_ */
