/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkAnnotationBidimensionalWidget,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef __vtkAnnotationBidimensionalWidget_h
#define __vtkAnnotationBidimensionalWidget_h

// Annotations includes
#include "vtkSlicerAnnotationsModuleVTKWidgetsExport.h"

// VTK includes
#include <vtkBiDimensionalWidget.h>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_VTKWIDGETS_EXPORT
vtkAnnotationBidimensionalWidget
  : public vtkBiDimensionalWidget
{
public:

  static vtkAnnotationBidimensionalWidget *New();
  vtkTypeMacro(vtkAnnotationBidimensionalWidget, vtkBiDimensionalWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void CreateDefaultRepresentation() override;

protected:

  vtkAnnotationBidimensionalWidget();
  ~vtkAnnotationBidimensionalWidget() override;

private:

  vtkAnnotationBidimensionalWidget(const vtkAnnotationBidimensionalWidget&) = delete;
  void operator=(const vtkAnnotationBidimensionalWidget&) = delete;

};

#endif /* __vtkAnnotationBidimensionalWidget_h */
