/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKMRMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_
#define VTKMRMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

#include "vtkMRMLAnnotationNode.h"

// VTK include
#include <vtkObject.h>
#include <vtkAbstractWidget.h>

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationDisplayableManagerHelper :
    public vtkObject
{
public:

  static vtkMRMLAnnotationDisplayableManagerHelper *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationDisplayableManagerHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  void UpdateLocked(vtkMRMLAnnotationNode* node);
  void UpdateVisible(vtkMRMLAnnotationNode* node);
  void UpdateWidget(vtkMRMLAnnotationNode* node);

  /// Get a vtkAbstractWidget* given a node
  vtkAbstractWidget * GetWidget(vtkMRMLAnnotationNode * node);
  void RemoveWidget(vtkMRMLAnnotationNode *node);

  /// List of Nodes managed by the DisplayableManager
  std::vector<vtkMRMLAnnotationNode*> AnnotationNodeList;

  /// .. and its associated convenient typedef
  typedef std::vector<vtkMRMLAnnotationNode*>::iterator AnnotationNodeListIt;

  /// Map of vtkWidget indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget *> Widgets;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget *>::iterator WidgetsIt;


protected:

  vtkMRMLAnnotationDisplayableManagerHelper();
  virtual ~vtkMRMLAnnotationDisplayableManagerHelper();

private:

  vtkMRMLAnnotationDisplayableManagerHelper(const vtkMRMLAnnotationDisplayableManagerHelper&); /// Not implemented
  void operator=(const vtkMRMLAnnotationDisplayableManagerHelper&); /// Not Implemented

};

#endif /* VTKMRMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_ */
