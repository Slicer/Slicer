#ifndef __qMRMLMatrixWidget_h
#define __qMRMLMatrixWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include <ctkVTKAbstractMatrixWidget.h>

#include "qMRMLWidgetsExport.h"
 
class vtkMRMLNode; 
class vtkMRMLLinearTransformNode; 
class vtkMatrix4x4;
class qMRMLMatrixWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLMatrixWidget : public ctkVTKAbstractMatrixWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(CoordinateReferenceType CoordinateReference READ coordinateReference WRITE setCoordinateReference)
  Q_ENUMS(CoordinateReferenceType)
  
public:
  
  /// Constructors
  typedef ctkVTKAbstractMatrixWidget Superclass;
  explicit qMRMLMatrixWidget(QWidget* parent);
  virtual ~qMRMLMatrixWidget(){}
  
  /// 
  /// Set/Get Coordinate system
  /// By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  vtkMRMLLinearTransformNode* mrmlTransformNode()const;
public slots:
  /// 
  /// Set the MRML node of interest
  void setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode); 
  void setMRMLTransformNode(vtkMRMLNode* node); 
  
protected slots:
  /// 
  /// Triggered upon MRML transform node updates
  void updateMatrix();

private:
  CTK_DECLARE_PRIVATE(qMRMLMatrixWidget);
}; 

#endif
