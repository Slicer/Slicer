/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKMRMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_
#define VTKMRMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_

// Annotations includes
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

// Annotations MRML includes
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationNode;

// MRML includes
class vtkMRMLInteractionNode;

// VTK includes
#include <vtkHandleWidget.h>
#include <vtkLineWidget2.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationDisplayableManagerHelper
  : public vtkObject
{
public:

  static vtkMRMLAnnotationDisplayableManagerHelper *New();
  vtkTypeMacro(vtkMRMLAnnotationDisplayableManagerHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Lock/Unlock all widgets based on the state of the nodes
  void UpdateLockedAllWidgetsFromNodes();
  /// Lock/Unlock all widgets from interaction node
  void UpdateLockedAllWidgetsFromInteractionNode(vtkMRMLInteractionNode* interactionNode);
  /// Lock/Unlock all widgets
  void UpdateLockedAllWidgets(bool locked);
  /// Lock/Unlock a widget
  void UpdateLocked(vtkMRMLAnnotationNode* node);
  /// Hide/Show a widget according to node's visible flag and if it can be
  /// displayed in this viewer
  void UpdateVisible(vtkMRMLAnnotationNode* node, bool displayableInViewer = true);
  /// Update lock and visibility of a widget
  void UpdateWidget(vtkMRMLAnnotationNode* node);

  /// Get a vtkAbstractWidget* given a node
  vtkAbstractWidget * GetWidget(vtkMRMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice intersection representation
  vtkAbstractWidget * GetIntersectionWidget(vtkMRMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice projection representation
  vtkAbstractWidget * GetOverLineProjectionWidget(vtkMRMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice projection representation
  vtkAbstractWidget * GetUnderLineProjectionWidget(vtkMRMLAnnotationNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice projection representation
  vtkAbstractWidget * GetPointProjectionWidget(vtkMRMLAnnotationNode * node);
  /// Remove all widgets, intersection widgets, nodes
  void RemoveAllWidgetsAndNodes();
  /// Remove a node, its widget and its intersection widget
  void RemoveWidgetAndNode(vtkMRMLAnnotationNode *node);


  /// Search the annotation node list and return the annotation node that has this display node
  vtkMRMLAnnotationNode * GetAnnotationNodeFromDisplayNode(vtkMRMLAnnotationDisplayNode *displayNode);

  //----------------------------------------------------------------------------------
  // The Lists!!
  //
  // An annotation which is managed by a displayableManager consists of
  //   a) the Annotation MRML Node (AnnotationNodeList)
  //   b) the vtkWidget to show this annotation (Widgets)
  //   c) a vtkWidget to represent sliceIntersections in the slice viewers (WidgetIntersections)
  //

  /// List of Nodes managed by the DisplayableManager
  std::vector<vtkMRMLAnnotationNode*> AnnotationNodeList;

  /// .. and its associated convenient typedef
  typedef std::vector<vtkMRMLAnnotationNode*>::iterator AnnotationNodeListIt;

  /// Map of vtkWidget indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*> Widgets;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetsIt;

  /// Map of vtkWidgets to reflect the Slice intersections indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*> WidgetIntersections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetIntersectionsIt;

  /// Map of vtkWidgets to reflect the Slice projection indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*> WidgetOverLineProjections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetOverLineProjectionsIt;

  /// Map of vtkWidgets to reflect the Slice projection indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*> WidgetUnderLineProjections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetUnderLineProjectionsIt;

  /// Map of vtkWidgets to reflect the Slice projection indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*> WidgetPointProjections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget*>::iterator WidgetPointProjectionsIt;

  //
  // End of The Lists!!
  //
  //----------------------------------------------------------------------------------


  /// Placement of seeds for widget placement
  void PlaceSeed(double x, double y, vtkRenderWindowInteractor * interactor, vtkRenderer * renderer);

  /// Get a placed seed
  vtkHandleWidget * GetSeed(int index);

  /// Remove all placed seeds
  void RemoveSeeds();


protected:

  vtkMRMLAnnotationDisplayableManagerHelper();
  ~vtkMRMLAnnotationDisplayableManagerHelper() override;

private:

  vtkMRMLAnnotationDisplayableManagerHelper(const vtkMRMLAnnotationDisplayableManagerHelper&) = delete;
  void operator=(const vtkMRMLAnnotationDisplayableManagerHelper&) = delete;

  /// SeedWidget for point placement
  vtkSmartPointer<vtkSeedWidget> SeedWidget;
  /// List of Handles for the SeedWidget
  std::vector<vtkSmartPointer<vtkHandleWidget> > HandleWidgetList;
  /// .. and its associated convenient typedef
  typedef std::vector<vtkSmartPointer<vtkHandleWidget> >::iterator HandleWidgetListIt;
};

#endif /* VTKMRMLANNOTATIONDISPLAYABLEMANAGERHELPER_H_ */
