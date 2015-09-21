/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qMRMLSubjectHierarchyTreeView_h
#define __qMRMLSubjectHierarchyTreeView_h

// qMRML includes
#include "qMRMLTreeView.h"

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class vtkMRMLSubjectHierarchyNode;
class qMRMLSubjectHierarchyTreeViewPrivate;
class vtkSlicerSubjectHierarchyModuleLogic;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyTreeView : public qMRMLTreeView
{
  Q_OBJECT

  /// Flag determining whether to highlight nodes referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH node instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  Q_PROPERTY (bool highlightReferencedNodes READ highlightReferencedNodes WRITE setHighlightReferencedNodes)

public:
  typedef qMRMLTreeView Superclass;
  qMRMLSubjectHierarchyTreeView(QWidget *parent=0);
  virtual ~qMRMLSubjectHierarchyTreeView();

public:
  bool highlightReferencedNodes()const;
  void setHighlightReferencedNodes(bool highlightOn);

protected:
  /// Toggle visibility
  virtual void toggleVisibility(const QModelIndex& index);

  /// Populate context menu for current node
  virtual void populateContextMenuForCurrentNode();

  /// Handle mouse press event (facilitates timely update of context menu)
  virtual void mousePressEvent(QMouseEvent* event);

  /// Apply highlight for nodes referenced by argument nodes by DICOM
  /// \sa highlightReferencedNodes
  void applyReferenceHighlightForNode(QList<vtkMRMLSubjectHierarchyNode*> nodes);

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Handle expand node requests in the subject hierarchy tree
  virtual void expandNode(vtkMRMLSubjectHierarchyNode* node);

  /// Handle manual selection of a plugin as the new owner of a subject hierarchy node
  virtual void selectPluginForCurrentNode();

  /// Update select plugin actions. Is called when the plugin selection sub-menu is opened,
  /// and when the user manually changes the owner plugin of a node. It sets checked state
  /// and update confidence values in the select plugin actions in the node context menu
  /// for the currently selected node.
  virtual void updateSelectPluginActions();

  /// Remove current node from subject hierarchy on context menu choice
  virtual void removeCurrentNodeFromSubjectHierarchy();

  /// Edit properties of current node
  virtual void editCurrentSubjectHierarchyNode();

  /// Delete selected subject hierarchy node(s)
  virtual void deleteSelectedNodes();

  /// Set multi-selection
  virtual void setMultiSelection(bool multiSelectionOn);

protected slots:
  /// Expand tree to depth specified by the clicked context menu action
  virtual void expandToDepthFromContextMenu();

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyTreeView);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyTreeView);
};

#endif
