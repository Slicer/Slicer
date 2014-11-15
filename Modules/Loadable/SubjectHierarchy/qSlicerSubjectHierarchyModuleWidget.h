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

protected:
  QScopedPointer<qSlicerSubjectHierarchyModuleWidgetPrivate> d_ptr;

  virtual void setup();
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyModuleWidget);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyModuleWidget);
};

#endif
