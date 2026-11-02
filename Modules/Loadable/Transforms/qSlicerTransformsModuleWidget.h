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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerTransformsModuleWidget_h
#define __qSlicerTransformsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// Transforms includes
#include "qSlicerTransformsModuleExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTransformsModuleWidgetPrivate;

class Q_SLICER_QTMODULES_TRANSFORMS_EXPORT qSlicerTransformsModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTransformsModuleWidget(QWidget* parent = nullptr);
  ~qSlicerTransformsModuleWidget() override;

  /// Reimplemented for internal reasons
  void setMRMLScene(vtkMRMLScene* scene) override;

  void enter() override;
  void exit() override;
  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:

  /// Set the transform to identity. Only for linear transforms.
  /// The sliders are reset to the position 0.
  void identity();

  /// Invert the transform.
  void invert();

  /// Split composite transform to its components
  void split();

protected:
  void setup() override;

protected slots:

  /// Show the color legend of the displacement magnitude that the transform is colored by.
  /// The legend node is created the first time the section is opened.
  void updateColorLegendFromMRML();
  void colorLegendCollapsibleButtonCollapsed(bool collapsed);

  /// Called when a subject hierarchy item is modified.
  /// Updates current item selection to reflect changes in item (such as display node creation)
  void onSubjectHierarchyItemModified(vtkObject* caller, void* callData);

  void onTranslateFirstButtonPressed(bool checked);
  void onNodeSelected(vtkMRMLNode* node);
  void onMRMLTransformNodeModified(vtkObject* caller);

  void copyTransform();
  void pasteTransform();

  void updateCenterOfTransformationWidgets();
  void onCenterOfTransformationChanged();
  void resetCenterOfTransformation();

  void transformSelectedNodes();
  void untransformSelectedNodes();
  void hardenSelectedNodes();

  void onDisplaySectionClicked(bool);
  void onTransformableSectionClicked(bool);

  /// Show the inputs of the chosen operation and type, and enable Apply when they are set
  void updateExportImportWidgets();
  /// Export the transform to the output node, or import it from the input node
  void applyExportImport();

  /// Show the source model and displacement array of a scattered transform
  void updateScatteredTransformWidgets();
  void onScatteredSourceModelChanged(vtkMRMLNode* node);
  void onScatteredDisplacementArrayChanged(int index);
  /// Set the fitting parameters and the displacement scale of a scattered transform
  void onScatteredFittingParameterChanged();

protected:
  ///
  /// Convenient method to return the coordinate system currently selected
  int coordinateReference() const;

protected:
  QScopedPointer<qSlicerTransformsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformsModuleWidget);
  Q_DISABLE_COPY(qSlicerTransformsModuleWidget);
};

#endif
