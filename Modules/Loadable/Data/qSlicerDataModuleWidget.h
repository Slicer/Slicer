/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerDataModuleWidget_h
#define __qSlicerDataModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerDataModuleExport.h"

class vtkMRMLNode;
class qSlicerDataModuleWidgetPrivate;
class qMRMLSubjectHierarchyModel;

class Q_SLICER_QTMODULES_DATA_EXPORT qSlicerDataModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerDataModuleWidget(QWidget *parentWidget = nullptr);
  ~qSlicerDataModuleWidget() override;

  void enter() override;

  enum
    {
    TabIndexSubjectHierarchy = 0,
    TabIndexTransformHierarchy,
    TabIndexAllNodes
    };

public slots:
  /// Reimplemented for internal reasons
  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Change visibility of the MRML node ID columns
  void setMRMLIDsVisible(bool visible);

  /// Show or hide transforms
  void setTransformsVisible(bool visible);

  /// Set data node associated to the selected subject hierarchy item to the data node inspector
  void setDataNodeFromSubjectHierarchyItem(vtkIdType itemID);
  /// Set subject hierarchy item information to the label
  void setInfoLabelFromSubjectHierarchyItem(vtkIdType itemID);
  /// Handle subject hierarchy item modified event (update item info label if needed)
  void onSubjectHierarchyItemModified(vtkIdType itemID);

  /// Insert new transform node
  void insertTransformNode();
  /// Harden transform on current node
  void hardenTransformOnCurrentNode();

public:
  /// Assessor function for subject hierarchy model (for python)
  Q_INVOKABLE qMRMLSubjectHierarchyModel* subjectHierarchySceneModel()const;

protected:
  static void onSubjectHierarchyItemEvent(vtkObject* caller, unsigned long event, void* clientData, void* callData);

protected slots:
  void onCurrentNodeChanged(vtkMRMLNode* newCurrentNode);
  void onCurrentTabChanged(int tabIndex);
  void onHelpButtonClicked();

protected:
  void setup() override;

protected:
  QScopedPointer<qSlicerDataModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDataModuleWidget);
  Q_DISABLE_COPY(qSlicerDataModuleWidget);
};

#endif
