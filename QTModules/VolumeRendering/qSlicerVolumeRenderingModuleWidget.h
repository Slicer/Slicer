#ifndef __qSlicerVolumeRenderingModuleWidget_h
#define __qSlicerVolumeRenderingModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumeRenderingModuleExport.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLVolumePropertyNode.h"

class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeRenderingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVolumeRenderingModuleWidget();

public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setMRMLParameterNode(vtkMRMLNode* node);

  void setMRMLROINode(vtkMRMLNode* node);

  void setMRMLVolumePropertyNode(vtkMRMLNode* node);

  vtkMRMLVolumeRenderingParametersNode* createParameterNode();

  vtkMRMLVolumePropertyNode* createVolumeProprtyNode();

protected:
  QScopedPointer<qSlicerVolumeRenderingModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModuleWidget);
};

#endif
