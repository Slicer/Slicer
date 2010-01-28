#ifndef __qMRMLTransformSliders_h
#define __qMRMLTransformSliders_h


// qVTK includes
#include <qVTKObject.h>

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QWidget>

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
  Q_PROPERTY(QString LRLabel READ getLRLabel WRITE setLRLabel)
  Q_PROPERTY(QString PALabel READ getPALabel WRITE setPALabel)
  Q_PROPERTY(QString ISLabel READ getISLabel WRITE setISLabel)
  Q_PROPERTY(double SingleStep READ singleStep WRITE setSingleStep)

  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(bool minMaxVisible READ isMinMaxVisible WRITE setMinMaxVisible)

public:
  // Constructors
  typedef QWidget Superclass;
  explicit qMRMLTransformSliders(QWidget* parent);
  virtual ~qMRMLTransformSliders(){}

  // Description:
  // Set/Get Coordinate system
  // By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  // Description:
  // Set/Get TypeOfTransform
  // By default, the selector coordinate system will be set to TRANSLATION
  enum TransformType { ROTATION, TRANSLATION };
  void setTypeOfTransform(TransformType typeOfTransform);
  TransformType typeOfTransform() const;

  // Description:
  // Set/Get Title of the group box
  void setTitle(const QString& title);
  QString title()const;

  // Description:
  // Get sliders range
  double minimum()const;
  double maximum()const;

  // Description:
  // Set sliders range
  void setMinimum(double min);
  void setMaximum(double max);
  // Description:
  // Utility function that set min/max in once
  void setRange(double min, double max);

  // Description:
  // Set the visibility property of the minimum QSpinBox and maximum QSpinBox.
  // Note: If the QSpinBoxes are hidden, you can still can setMinimum() and
  // setMaximum() manually
  void setMinMaxVisible(bool visible);
  bool isMinMaxVisible()const;

  // Description:
  // Set sliders single step
  double singleStep()const;
  void setSingleStep(double step);

  // Description:
  // Get/Set slider's label
  QString getLRLabel()const;
  QString getPALabel()const;
  QString getISLabel()const;
  void setLRLabel(const QString& label);
  void setPALabel(const QString& label);
  void setISLabel(const QString& label);

  // Description:
  // Return the current MRML node of interest
  vtkMRMLLinearTransformNode* mrmlTransformNode()const;

signals:
  // Description:
  // Signal sent if at least one of the slider's position is updated
  void sliderMoved();

public slots:
  // Description:
  // Set the MRML node of interest
  void setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode);
  void setMRMLTransformNode(vtkMRMLNode* node);

  // Description:
  // Reset all sliders to their 0 position and value
  void reset();

protected slots:
  void onLRSliderPositionChanged(double position);
  void onPASliderPositionChanged(double position);
  void onISSliderPositionChanged(double position);

  void onMinimumChanged(double min);
  void onMaximumChanged(double max);

private:
  QCTK_DECLARE_PRIVATE(qMRMLTransformSliders);
};

#endif
