/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationTextDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationTextDisplayableManager_h
#define __vtkMRMLAnnotationTextDisplayableManager_h

// Annotation includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLAnnotationTextNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationTextDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationTextDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationTextDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationTextDisplayableManager(){this->m_Focus="vtkMRMLAnnotationTextNode";}
  virtual ~vtkMRMLAnnotationTextDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);
  // update just the position, called from PropagateMRMLToWidget and in
  // response to slice node modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

  /// Examine nodes in the scene and try to pick a caption coordinate that
  /// doesn't conflict.
  void BestGuessForNewCaptionCoordinates(double bestGuess[2]);
private:

  vtkMRMLAnnotationTextDisplayableManager(const vtkMRMLAnnotationTextDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationTextDisplayableManager&); /// Not Implemented

};

#endif
