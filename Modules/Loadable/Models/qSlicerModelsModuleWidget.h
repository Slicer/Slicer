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

#ifndef __qSlicerModelsModuleWidget_h
#define __qSlicerModelsModuleWidget_h

// CTK includes
#include "ctkVTKObject.h"

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerModelsModuleExport.h"

class qSlicerModelsModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLSelectionNode;

/// \ingroup Slicer_QtModules_Models
class Q_SLICER_QTMODULES_MODELS_EXPORT qSlicerModelsModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerModelsModuleWidget(QWidget *parent=nullptr);
  ~qSlicerModelsModuleWidget() override;

  void enter() override;
  void exit() override;
  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:
  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Set current subject hierarchy item IDs.
  /// The current node (the properties of which the widget displays) will be the one associated
  /// to the first selected subject hierarchy item.
  /// When a property changes, then it is applied to all the models in the selection
  void setDisplaySelectionFromSubjectHierarchyItems(QList<vtkIdType> itemIDs);

  void onClippingConfigurationButtonClicked();
  void onDisplayNodeChanged();
  void onClipSelectedModelToggled(bool);

  static void onMRMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                               void* client_data, void* call_data);

  /// hide/show all the models in the scene
  void hideAllModels();
  void showAllModels();

protected slots:
  /// Called when a subject hierarchy item is modified.
  /// Updates current item selection to reflect changes in item (such as display node creation)
  void onSubjectHierarchyItemModified(vtkObject* caller, void* callData);

  /// Called when the information collapsible button collapsed state is changed.
  void onInformationSectionCollapsed(bool);

protected:
  void setup() override;

protected:
  QScopedPointer<qSlicerModelsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModelsModuleWidget);
  Q_DISABLE_COPY(qSlicerModelsModuleWidget);
};

#endif
