/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationTextDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationTextDisplayableManager_h
#define __vtkMRMLAnnotationTextDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAnnotationDisplayableManager.h>

class vtkMRMLAnnotationTextNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationTextDisplayableManager :
    public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationTextDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationTextDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationTextDisplayableManager(){}
  virtual ~vtkMRMLAnnotationTextDisplayableManager(){}

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Propagate properties of MRML node to widgets.
  virtual void SetWidget(vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationTextDisplayableManager(const vtkMRMLAnnotationTextDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationTextDisplayableManager&); /// Not Implemented

};

#endif

