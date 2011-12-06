/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationClickCounter.h,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKMRMLANNOTATIONCLICKCOUNTER_H_
#define VTKMRMLANNOTATIONCLICKCOUNTER_H_

// AnnotationModule includes
#include "qSlicerAnnotationsModuleExport.h"

// VTK include
#include <vtkObject.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationClickCounter :
    public vtkObject
{
public:

  static vtkMRMLAnnotationClickCounter *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationClickCounter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Check if enough clicks are counted
  bool HasEnoughClicks(int clicks);

  /// Reset the click counter
  void Reset();

protected:

  vtkMRMLAnnotationClickCounter();
  virtual ~vtkMRMLAnnotationClickCounter();

private:

  vtkMRMLAnnotationClickCounter(const vtkMRMLAnnotationClickCounter&); /// Not implemented
  void operator=(const vtkMRMLAnnotationClickCounter&); /// Not Implemented

  int m_Clicks;

};

#endif /* VTKMRMLANNOTATIONCLICKCOUNTER_H_ */
