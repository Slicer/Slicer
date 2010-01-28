#ifndef __qMRMLLinearTransformSlider_h
#define __qMRMLLinearTransformSlider_h

// qVTK includes
#include <qVTKObject.h>

// qCTK includes
#include <qCTKSliderSpinBoxLabel.h>
#include <qCTKPimpl.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLLinearTransformNode;
class vtkMatrix4x4;
class qMRMLLinearTransformSliderPrivate;

class QMRML_WIDGETS_EXPORT qMRMLLinearTransformSlider : public qCTKSliderSpinBoxLabel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(TransformType TypeOfTransform READ typeOfTransform WRITE setTypeOfTransform)
  Q_ENUMS(TransformType)
  Q_PROPERTY(CoordinateReferenceType CoordinateReference READ coordinateReference WRITE setCoordinateReference)
  Q_ENUMS(CoordinateReferenceType)

public:
  // Constructors
  typedef qCTKSliderSpinBoxLabel Superclass;
  explicit qMRMLLinearTransformSlider(QWidget* parent);
  virtual ~qMRMLLinearTransformSlider(){}

  // Description:
  // Set/Get Transform type
  // By default, the slider transform type will be set to TRANSLATION_LR
  // X axis:LR, Y axis:PA, Z axis:IS
  enum TransformType
  {
    TRANSLATION_LR, TRANSLATION_PA, TRANSLATION_IS,
    ROTATION_LR, ROTATION_PA, ROTATION_IS
  };
  void setTypeOfTransform(TransformType typeOfTransform);
  TransformType typeOfTransform() const;

  // Description:
  // Convenience method allowing to get which familly of transform is set
  bool isRotation();
  bool isTranslation();

  // Description:
  // Set/Get Coordinate system
  // By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  // Description:
  // Apply the appropriate rotation/translation according to the typeOfTransform of the slider.
  void applyTransformation(double sliderPosition);

  // Description:
  // Return the current transform node
  vtkMRMLLinearTransformNode* mrmlTransformNode()const;
public slots:
  // Description:
  // Set the MRML node of interest
  // Note that setting transformNode to 0 will disable the widget
  void setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode);

protected slots:
  // Description:
  // Triggered upon MRML scene updates
  void onMRMLTransformNodeModified(vtkObject* caller);

private:
  QCTK_DECLARE_PRIVATE(qMRMLLinearTransformSlider);

};

#endif
