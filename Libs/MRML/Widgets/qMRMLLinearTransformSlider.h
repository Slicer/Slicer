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

#ifndef __qMRMLLinearTransformSlider_h
#define __qMRMLLinearTransformSlider_h

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
#include "qMRMLSliderWidget.h"

class vtkMRMLTransformNode;
class vtkMatrix4x4;
class qMRMLLinearTransformSliderPrivate;

class QMRML_WIDGETS_EXPORT qMRMLLinearTransformSlider : public qMRMLSliderWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(TransformType TypeOfTransform READ typeOfTransform WRITE setTypeOfTransform)
  Q_ENUMS(TransformType)
  Q_PROPERTY(CoordinateReferenceType CoordinateReference READ coordinateReference WRITE setCoordinateReference)
  Q_ENUMS(CoordinateReferenceType)

public:
  /// Constructors
  typedef qMRMLSliderWidget Superclass;
  explicit qMRMLLinearTransformSlider(QWidget* parent);
  ~qMRMLLinearTransformSlider() override;

  ///
  /// Set/Get Transform type
  /// By default, the slider transform type will be set to TRANSLATION_LR
  /// X axis:LR, Y axis:PA, Z axis:IS
  enum TransformType
  {
    TRANSLATION_LR, TRANSLATION_PA, TRANSLATION_IS,
    ROTATION_LR, ROTATION_PA, ROTATION_IS
  };
  void setTypeOfTransform(TransformType typeOfTransform);
  TransformType typeOfTransform() const;

  ///
  /// Convenience method allowing to get which family of transform is set
  bool isRotation()const;
  bool isTranslation()const;

  ///
  /// Set/Get Coordinate system
  /// By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  ///
  /// Return the current transform node
  vtkMRMLTransformNode* mrmlTransformNode()const;

public slots:
  ///
  /// Set the MRML node of interest
  /// Note that setting transformNode to 0 will disable the widget
  void setMRMLTransformNode(vtkMRMLTransformNode* transformNode);

  ///
  /// Apply the appropriate rotation/translation according to the typeOfTransform of the slider.
  void applyTransformation(double sliderPosition);

protected slots:
  ///
  /// Triggered upon MRML scene updates
  void onMRMLTransformNodeModified(vtkObject* caller);

protected:
  QScopedPointer<qMRMLLinearTransformSliderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLLinearTransformSlider);
  Q_DISABLE_COPY(qMRMLLinearTransformSlider);

};

#endif
