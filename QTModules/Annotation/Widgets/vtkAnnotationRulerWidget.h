/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKANNOTATIONRULERWIDGET_H_
#define VTKANNOTATIONRULERWIDGET_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkAbstractWidget.h>
#include <vtkDistanceWidget.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkAnnotationRulerWidget :
    public vtkDistanceWidget
{
public:

  static vtkAnnotationRulerWidget *New();
  vtkTypeRevisionMacro(vtkAnnotationRulerWidget, vtkDistanceWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  void CreateDefaultRepresentation();
  void CreateDefaultRepresentation3D();

protected:

  vtkAnnotationRulerWidget();
  virtual ~vtkAnnotationRulerWidget();

private:

  vtkAnnotationRulerWidget(const vtkAnnotationRulerWidget&); /// Not implemented
  void operator=(const vtkAnnotationRulerWidget&); /// Not Implemented

};

#endif /* VTKANNOTATIONRULERWIDGET_H_ */
