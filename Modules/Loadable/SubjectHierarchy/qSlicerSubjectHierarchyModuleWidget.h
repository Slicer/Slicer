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

#ifndef __qSlicerSubjectHierarchyModuleWidget_h
#define __qSlicerSubjectHierarchyModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerSubjectHierarchyModuleExport.h"

class vtkMRMLSubjectHierarchyNode;
class qSlicerSubjectHierarchyModuleWidgetPrivate;
class qMRMLSceneSubjectHierarchyModel;
class qSlicerSubjectHierarchyAbstractPlugin;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_QTMODULES_SUBJECTHIERARCHY_EXPORT qSlicerSubjectHierarchyModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSubjectHierarchyModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSubjectHierarchyModuleWidget();

  virtual void enter();
  virtual void exit();

public slots:
  /// Update widget GUI from parameter node
  void updateWidgetFromMRML();

protected slots:
  /// Show or hide MRML IDs
  void setMRMLIDsVisible(bool visible);

  /// Show or hide transforms
  void setTransformsVisible(bool visible);

public:
  /// Assessor function for subject hierarchy scene model (for python)
  Q_INVOKABLE qMRMLSceneSubjectHierarchyModel* subjectHierarchySceneModel()const;

  /// Assessor function for getting subject hierarchy plugin by name (for python)
  Q_INVOKABLE qSlicerSubjectHierarchyAbstractPlugin* subjectHierarchyPluginByName(QString name)const;

  /// Assessor function for getting currently selected subject hierarchy node (for python)
  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* currentSubjectHierarchyNode()const;

  /// Utility test function to be able to set currently selected subject hierarchy node from python
  /// Only used in python tests!
  Q_INVOKABLE void setCurrentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node);

public slots:
  /// Handles logic modified event, which happens on major changes in the
  /// scene (new scene, batch processing, import etc.)
  void onLogicModified();

protected:
  /// Add supported nodes to subject hierarchy.
  /// This method is called if auto-creation is enabled and a supported node is added to the
  /// scene, or if the user answers yes to the question that pops up upon entering subject
  /// hierarchy module if supported nodes are found that are not in the hierarchy.
  void addSupportedNodesToSubjectHierarchy();

  /// Determine whether there are supported nodes not in subject hierarchy
  bool isThereSupportedNodeOutsideSubjectHierarchy();

protected slots:
  /// Called when a node is added to the scene so that a plugin can claim it
  /// if it is a subject hierarchy node
  /// Note: Implemented in Module class not in Logic because this exercises the Subject
  ///   hierarchy plugin mechanism and those are Qt classes
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);

  /// Called when a node is removed to the scene so that the associated
  /// subject hierarchy node can be deleted too
  /// Note: Implemented in Module class not in Logic because this exercises the Subject
  ///   hierarchy plugin mechanism and those are Qt classes
  void onNodeAboutToBeRemoved(vtkObject* scene, vtkObject* nodeObject);

  /// Called when a subject hierarchy node is modified.
  /// A search for owner plugin is performed if the plugin is not overridden by the user,
  /// in case the changes in the subject hierarchy node leads to a new most suitable plugin.
  /// Note: Implemented in Module class not in Logic because this exercises the Subject
  ///   hierarchy plugin mechanism and those are Qt classes
  void onSubjectHierarchyNodeModified(vtkObject* nodeObject);

  /// Called when scene import is finished.
  /// Subject hierarchy nodes are created for supported data nodes if they have not
  /// been imported with the scene (backwards compatibility for older scenes)
  /// Note: Implemented in Module class not in Logic because this exercises the Subject
  ///   hierarchy plugin mechanism and those are Qt classes
  void onSceneImportEnded(vtkObject* sceneObject);

  /// Called when hierarchy modified event is fired for a data node
  /// This method ensures that the hierarchy nodes remain consistent and deterministic
  /// so that instead of having a data node directly associated to multiple hierarchy nodes,
  /// nested association is used (see vtkMRMLSubjectHierarchyNode header)
  void onMRMLNodeHierarchyModified(vtkObject* nodeObject);

protected:
  QScopedPointer<qSlicerSubjectHierarchyModuleWidgetPrivate> d_ptr;

  virtual void setup();
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyModuleWidget);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyModuleWidget);
};

#endif
