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

class Q_SLICER_QTMODULES_TRANSFORMS_EXPORT qSlicerTransformsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTransformsModuleWidget(QWidget *parent=nullptr);
  ~qSlicerTransformsModuleWidget() override;

  /// Reimplemented for internal reasons
  void setMRMLScene(vtkMRMLScene* scene) override;

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

  void onTranslateFirstButtonPressed(bool checked);
  void onNodeSelected(vtkMRMLNode* node);
  void onMRMLTransformNodeModified(vtkObject* caller);

  void copyTransform();
  void pasteTransform();

  void transformSelectedNodes();
  void untransformSelectedNodes();
  void hardenSelectedNodes();

  void onDisplaySectionClicked(bool);
  void onTransformableSectionClicked(bool);

  void updateConvertButtonState();
  void convert();

protected:
  ///
  /// Convenient method to return the coordinate system currently selected
  int coordinateReference()const;

protected:
  QScopedPointer<qSlicerTransformsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformsModuleWidget);
  Q_DISABLE_COPY(qSlicerTransformsModuleWidget);
};

#endif
