/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkAnnotationBidimensionalWidget,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKANNOTATIONBIDIMENSIONALWIDGET_H_
#define VTKANNOTATIONBIDIMENSIONALWIDGET_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkBiDimensionalWidget.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkAnnotationBidimensionalWidget :
    public vtkBiDimensionalWidget
{
public:

  static vtkAnnotationBidimensionalWidget *New();
  vtkTypeRevisionMacro(vtkAnnotationBidimensionalWidget, vtkBiDimensionalWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  void CreateDefaultRepresentation();
  void CreateDefaultRepresentation3D();

protected:

  vtkAnnotationBidimensionalWidget();
  virtual ~vtkAnnotationBidimensionalWidget();

private:

  vtkAnnotationBidimensionalWidget(const vtkAnnotationBidimensionalWidget&); /// Not implemented
  void operator=(const vtkAnnotationBidimensionalWidget&); /// Not Implemented

};

#endif /* VTKANNOTATIONBIDIMENSIONALWIDGET_H_ */
