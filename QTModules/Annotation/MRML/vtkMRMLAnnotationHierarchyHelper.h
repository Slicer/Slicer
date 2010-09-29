/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationClickCounter.h,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKMRMLANNOTATIONHIERARCHYHELPER_H_
#define VTKMRMLANNOTATIONHIERARCHYHELPER_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationHierarchyNode.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK include
#include <vtkObject.h>

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationHierarchyHelper :
    public vtkObject
{
public:

  static vtkMRMLAnnotationHierarchyHelper *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationHierarchyHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetMRMLScene(vtkMRMLScene* scene);

  //
  /// Set the pointer to the active (eq. currently selected) hierarchy node.
  /// This gets used to set the parent for new hierarchy nodes.
  void SetActiveHierarchyNode(vtkMRMLAnnotationHierarchyNode* hierarchyNode);

  //
  /// Add a new annotation hierarchy node under the active hierarchy node. If there is no
  /// active hierarchy node, use the top-level annotation hierarchy node as the parent.
  /// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
  /// a parent to the new hierarchy node. Return the new hierarchy node.
  vtkMRMLAnnotationHierarchyNode* AddNewHierarchyNode();

  //
  /// Return the top level annotation hierarchy node in the scene. If there is none, create a new one.
  vtkMRMLAnnotationHierarchyNode* GetTopLevelHierarchyNode();

protected:

  vtkMRMLAnnotationHierarchyHelper();
  virtual ~vtkMRMLAnnotationHierarchyHelper();

private:

  vtkMRMLAnnotationHierarchyHelper(const vtkMRMLAnnotationHierarchyHelper&); /// Not implemented
  void operator=(const vtkMRMLAnnotationHierarchyHelper&); /// Not Implemented

  vtkMRMLScene* m_scene;
  vtkMRMLAnnotationHierarchyNode* m_activeHierarchyNode;

};

#endif /* VTKMRMLANNOTATIONHIERARCHYHELPER_H_ */
