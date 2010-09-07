/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationSliceViewDisplayableManager.h.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.1 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationSliceViewDisplayableManager_h
#define __vtkMRMLAnnotationSliceViewDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractSliceViewDisplayableManager.h>


class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSliceViewDisplayableManager :
    public vtkMRMLAbstractSliceViewDisplayableManager
{
public:

  static vtkMRMLAnnotationSliceViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationSliceViewDisplayableManager, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationSliceViewDisplayableManager();
  virtual ~vtkMRMLAnnotationSliceViewDisplayableManager();

  virtual void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// Could be overloaded if DisplayableManager subclass
  virtual void OnMRMLSliceNodeModifiedEvent();

private:

  vtkMRMLAnnotationSliceViewDisplayableManager(const vtkMRMLAnnotationSliceViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationSliceViewDisplayableManager&); /// Not Implemented

};

#endif

