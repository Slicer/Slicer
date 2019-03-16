/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationClickCounter.h,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationClickCounter_h
#define __vtkMRMLAnnotationClickCounter_h

// Annotation includes
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

// VTK include
#include <vtkObject.h>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationClickCounter
  : public vtkObject
{
public:

  static vtkMRMLAnnotationClickCounter *New();
  vtkTypeMacro(vtkMRMLAnnotationClickCounter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Increase the click counter and return the number of clicks.
  /// \sa HasEnoughClicks()
  int Click();

  /// Check if enough clicks are counted and reset the click number if it
  /// is equal to \a clicks
  /// \sa Click(), Reset()
  bool HasEnoughClicks(int clicks);

  /// Reset the click counter
  void Reset();

protected:

  vtkMRMLAnnotationClickCounter();
  ~vtkMRMLAnnotationClickCounter() override;

private:

  vtkMRMLAnnotationClickCounter(const vtkMRMLAnnotationClickCounter&) = delete;
  void operator=(const vtkMRMLAnnotationClickCounter&) = delete;

  int m_Clicks;

};

#endif /* __vtkMRMLAnnotationClickCounter_h */
