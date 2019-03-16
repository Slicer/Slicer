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
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLAnnotationRulerDisplayableManager(){this->m_Focus="vtkMRMLAnnotationRulerNode";}
  ~vtkMRMLAnnotationRulerDisplayableManager() override = default;

  /// Callback for click in RenderWindow
  void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) override;
  /// Create a widget.
  vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node) override;

  /// Gets called when widget was created
  void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node) override;

  /// Propagate properties of MRML node to widget.
  void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget) override;
  /// Propagate properties of widget to MRML node.
  void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node) override;

  // update the ruler end point positions from the MRML node
  void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) override;

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
  void OnMRMLSelectionNodeUnitModifiedEvent(vtkMRMLSelectionNode* selectionNode) override;

private:

  vtkMRMLAnnotationRulerDisplayableManager(const vtkMRMLAnnotationRulerDisplayableManager&) = delete;
  void operator=(const vtkMRMLAnnotationRulerDisplayableManager&) = delete;

};

#endif
