/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationFiducialDisplayableManager.h,v $
 Date:      $Date: 2010/07/27 04:48:05 $
 Version:   $Revision: 1.1 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationFiducialDisplayableManager_h
#define __vtkMRMLAnnotationFiducialDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManager.h"

class vtkMRMLAnnotationFiducialNode;
class vtkSlicerViewerWidget;
//class vtkMRMLAnnotationFiducialDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkSeedWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationFiducialDisplayableManager :
    public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationFiducialDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationFiducialDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationFiducialDisplayableManager(){}
  virtual ~vtkMRMLAnnotationFiducialDisplayableManager(){}

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Propagate properties of MRML node to widgets.
  virtual void SetWidget(vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationFiducialDisplayableManager(const vtkMRMLAnnotationFiducialDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationFiducialDisplayableManager&); /// Not Implemented

};

#endif

