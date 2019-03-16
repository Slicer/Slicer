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

#ifndef __qMRMLTransformSliders_h
#define __qMRMLTransformSliders_h

// Qt includes
#include <QPair>

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
#include "qMRMLWidget.h"

class vtkMRMLNode;
class vtkMRMLTransformNode;
class vtkMatrix4x4;
class qMRMLTransformSlidersPrivate;

class QMRML_WIDGETS_EXPORT qMRMLTransformSliders : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString Title READ title WRITE setTitle)
  Q_PROPERTY(CoordinateReferenceType CoordinateReference READ coordinateReference WRITE setCoordinateReference)
  Q_ENUMS(CoordinateReferenceType)
  Q_PROPERTY(TransformType TypeOfTransform READ typeOfTransform WRITE setTypeOfTransform)
  Q_ENUMS(TransformType)
  Q_PROPERTY(QString LRLabel READ lrLabel WRITE setLRLabel)
  Q_PROPERTY(QString PALabel READ paLabel WRITE setPALabel)
  Q_PROPERTY(QString ISLabel READ isLabel WRITE setISLabel)
  Q_PROPERTY(double SingleStep READ singleStep WRITE setSingleStep)

  Q_PROPERTY(int decimals READ decimals WRITE setDecimals NOTIFY decimalsChanged)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(bool minMaxVisible READ isMinMaxVisible WRITE setMinMaxVisible)

public:
  /// Constructors
  typedef qMRMLWidget Superclass;
  explicit qMRMLTransformSliders(QWidget* parent);
  ~qMRMLTransformSliders() override;

  ///
  /// Set/Get Coordinate system
  /// By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  ///
  /// Set/Get TypeOfTransform
  /// By default, the selector coordinate system will be set to TRANSLATION
  enum TransformType { ROTATION, TRANSLATION };
  void setTypeOfTransform(TransformType typeOfTransform);
  TransformType typeOfTransform() const;

  ///
  /// Set/Get Title of the group box
  void setTitle(const QString& title);
  QString title()const;

  /// Return the decimals property value.
  /// \sa decimals
  int decimals()const;

  ///
  /// Get sliders range
  double minimum()const;
  double maximum()const;

  ///
  /// Set sliders range
  void setMinimum(double min);
  void setMaximum(double max);
  ///
  /// Utility function that set min/max in once
  void setRange(double min, double max);

  ///
  /// Set the visibility property of the minimum QSpinBox and maximum QSpinBox.
  /// Note: If the QSpinBoxes are hidden, you can still can setMinimum() and
  /// setMaximum() manually
  void setMinMaxVisible(bool visible);
  bool isMinMaxVisible()const;

  ///
  /// Set sliders single step
  double singleStep()const;
  void setSingleStep(double step);

  ///
  /// Get/Set slider's label
  QString lrLabel()const;
  QString paLabel()const;
  QString isLabel()const;
  void setLRLabel(const QString& label);
  void setPALabel(const QString& label);
  void setISLabel(const QString& label);

  ///
  /// Return the current MRML node of interest
  vtkMRMLTransformNode* mrmlTransformNode()const;

signals:
  ///
  /// Signal sent if at least one of the slider's value is updated
  void valuesChanged();

  /// Signal sent if the minimum or maximum slider's value is updated
  void rangeChanged(double newMinimum, double newMaximum);

  /// Signal sent when the number of decimals has changed.
  /// \sa decimals
  void decimalsChanged(int newDecimals);

public slots:
  ///
  /// Set the MRML node of interest
  void setMRMLTransformNode(vtkMRMLTransformNode* transformNode);
  void setMRMLTransformNode(vtkMRMLNode* node);

  ///
  /// Reset all sliders to their 0 position and value.
  /// Note that it can change the transform node values.
  void reset();

  ///
  /// Reset all sliders but the one that is currently active (i.e. the user
  /// is changing its value. The reset doesn't fire any event and the node
  /// is not modified. If no slider is active, then all sliders are reset.
  void resetUnactiveSliders();

  /// Set the decimals property value.
  /// \sa decimals
  void setDecimals(int newDecimals);

protected slots:
  void onSliderPositionChanged(double position);

  void onMinimumChanged(double min);
  void onMaximumChanged(double max);

  ///
  /// Triggered upon MRML node transform modified events
  void onMRMLTransformNodeModified(vtkObject* caller);

protected:
  QScopedPointer<qMRMLTransformSlidersPrivate> d_ptr;

  /// Extract the min/max values from the matrix and
  /// change the slider min/max values accordingly.
  /// Needed if the matrix changed externally (python, cli, etc.)
  void updateRangeFromTransform(vtkMRMLTransformNode* transformNode);

  ///
  /// Extract min/max translation values from the transform matrix.
  /// Parameter 'pad' allows to specify (using a value between 0 and 1)
  /// which percentage of the found range(max-min) value should be subtracted/added
  /// to the min/max value found.
  static QPair<double, double> extractMinMaxTranslationValue(vtkMatrix4x4 * mat,
                                                            double pad = 0);

private:
  Q_DECLARE_PRIVATE(qMRMLTransformSliders);
  Q_DISABLE_COPY(qMRMLTransformSliders);
};

#endif
