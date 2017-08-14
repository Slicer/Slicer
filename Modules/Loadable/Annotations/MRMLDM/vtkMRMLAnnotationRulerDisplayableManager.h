/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationRulerDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationRulerDisplayableManager_h
#define __vtkMRMLAnnotationRulerDisplayableManager_h

// Annotation includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLAnnotationRulerNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationRulerDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLSelectionNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationRulerDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationRulerDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAnnotationRulerDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

protected:

  vtkMRMLAnnotationRulerDisplayableManager(){this->m_Focus="vtkMRMLAnnotationRulerNode";}
  virtual ~vtkMRMLAnnotationRulerDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) VTK_OVERRIDE;
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node) VTK_OVERRIDE;

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node) VTK_OVERRIDE;

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget) VTK_OVERRIDE;
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node) VTK_OVERRIDE;

  // update the ruler end point positions from the MRML node
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) VTK_OVERRIDE;

  // Get the label from the node and unit node
  std::string GetLabelFormat(vtkMRMLAnnotationRulerNode* rulerNode);

  /// Compute the distance in mm between 2 world coordinates points
  /// \sa ApplyUnit()
  double GetDistance(const double* wc1, const double* wc2);
  /// Apply the current unit to a length in mm.
  /// \sa GetDistance()
  double ApplyUnit(double lengthInMM);

  /// When the unit has changed, modify the ruler nodes to refresh the label.
  /// \sa AddObserversToSelectionNode(), RemoveObserversFromSelectionNode()
  virtual void OnMRMLSelectionNodeUnitModifiedEvent(vtkMRMLSelectionNode* selectionNode) VTK_OVERRIDE;

private:

  vtkMRMLAnnotationRulerDisplayableManager(const vtkMRMLAnnotationRulerDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationRulerDisplayableManager&); /// Not Implemented

};

#endif
