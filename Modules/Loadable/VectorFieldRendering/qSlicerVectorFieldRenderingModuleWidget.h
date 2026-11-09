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

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __qSlicerVectorFieldRenderingModuleWidget_h
#define __qSlicerVectorFieldRenderingModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVectorFieldRenderingModuleExport.h"

class QListWidgetItem;
class qSlicerVectorFieldRenderingModuleWidgetPrivate;
class vtkMRMLDisplayableNode;
class vtkMRMLNode;
class vtkMRMLVectorFieldDisplayNode;

/// \brief Selects a model, vector volume or transform, lists its vector field displays, and edits the selected one.
class Q_SLICER_QTMODULES_VECTORFIELDRENDERING_EXPORT qSlicerVectorFieldRenderingModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVectorFieldRenderingModuleWidget(QWidget* parent = nullptr);
  ~qSlicerVectorFieldRenderingModuleWidget() override;

  /// Select a model, vector volume or transform, or a vector field display node (its node and the display node)
  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

  /// The selected model, vector volume or transform
  vtkMRMLDisplayableNode* currentNode() const;
  /// The selected vector field display node of the current node
  vtkMRMLVectorFieldDisplayNode* currentDisplayNode() const;

public slots:
  /// Select a model, vector volume or transform
  void setCurrentNode(vtkMRMLNode* node);
  /// Select a vector field display node of the current node
  void setCurrentDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode);
  /// Add a vector field display node to the current node and select it
  void addDisplayNode();
  /// Remove the selected vector field display node
  void removeCurrentDisplayNode();

protected slots:
  void onSubjectHierarchyItemsChanged(QList<vtkIdType> itemIDs);
  void updateDisplayNodesList();
  void onDisplayNodeSelectionChanged();
  void onDisplayNodeItemChanged(QListWidgetItem* item);
  void updateColorMapFromMRML();
  void onColorMapModified();

protected:
  void setup() override;

  QScopedPointer<qSlicerVectorFieldRenderingModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVectorFieldRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVectorFieldRenderingModuleWidget);
};

#endif
