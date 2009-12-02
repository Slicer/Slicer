#ifndef __qMRMLMatrixWidget_h
#define __qMRMLMatrixWidget_h

// qMRML includes
#include "qVTKObject.h"

// qCTK includes
#include <qCTKMatrixWidget.h>
#include <qCTKPimpl.h>

#include "qMRMLWidgetsWin32Header.h"
 
class vtkMRMLNode; 
class vtkMRMLLinearTransformNode; 
class vtkMatrix4x4;
class qMRMLMatrixWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLMatrixWidget : public qCTKMatrixWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(CoordinateReferenceType CoordinateReference READ coordinateReference WRITE setCoordinateReference)
  Q_ENUMS(CoordinateReferenceType)
  
public:
  
  // Self/Superclass typedef
  typedef qMRMLMatrixWidget  Self;
  typedef qCTKMatrixWidget   Superclass;
  
  // Constructors
  qMRMLMatrixWidget(QWidget* parent);
  
  // Description:
  // Set/Get Coordinate system
  // By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  vtkMRMLLinearTransformNode* mrmlTransformNode()const;
public slots:
  // Description:
  // Set the MRML node of interest
  void setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode); 
  void setMRMLTransformNode(vtkMRMLNode* node); 
  
protected slots:
  // Description:
  // Triggered upon MRML transform node updates
  void onMRMLTransformNodeModified(void* call_data, vtkObject* caller);

private:
  QCTK_DECLARE_PRIVATE(qMRMLMatrixWidget);
}; 

#endif
