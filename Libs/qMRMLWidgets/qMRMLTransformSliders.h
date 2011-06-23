/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLLinearTransformNode;
class vtkMatrix4x4;
class qMRMLTransformSlidersPrivate;

class QMRML_WIDGETS_EXPORT qMRMLTransformSliders : public QWidget
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

  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(bool minMaxVisible READ isMinMaxVisible WRITE setMinMaxVisible)

public:
  /// Constructors
  typedef QWidget Superclass;
  explicit qMRMLTransformSliders(QWidget* parent);
  virtual ~qMRMLTransformSliders();

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
  vtkMRMLLinearTransformNode* mrmlTransformNode()const;

signals:
  /// 
  /// Signal sent if at least one of the slider's value is updated
  void valuesChanged();

  ///
  /// Signal sent if the minimum or maximum slider's value is updated
  void rangeChanged(double newMinimum, double newMaximum);

public slots:
  /// 
  /// Set the MRML node of interest
  void setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode);
  void setMRMLTransformNode(vtkMRMLNode* node);

  /// 
  /// Reset all sliders to their 0 position and value
  void reset();

  /// Reset all sliders but the one that are currently active (i.e. the user
  /// is changing its value.
  void resetUnactiveSliders();

protected slots:
  void onLRSliderPositionChanged(double position);
  void onPASliderPositionChanged(double position);
  void onISSliderPositionChanged(double position);

  void onMinimumChanged(double min);
  void onMaximumChanged(double max);

protected:
  QScopedPointer<qMRMLTransformSlidersPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTransformSliders);
  Q_DISABLE_COPY(qMRMLTransformSliders);
};

#endif
