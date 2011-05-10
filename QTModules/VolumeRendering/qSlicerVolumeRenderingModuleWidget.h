#ifndef __qSlicerVolumeRenderingModuleWidget_h
#define __qSlicerVolumeRenderingModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeRenderingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVolumeRenderingModuleWidget();

  vtkMRMLScalarVolumeNode* mrmlVolumeNode()const;
  vtkMRMLVolumeRenderingDisplayNode* mrmlDisplayNode()const;
  vtkMRMLViewNode* mrmlViewNode()const;

public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setMRMLDisplayNode(vtkMRMLNode* node);

  void setMRMLROINode(vtkMRMLNode* node);

  void setMRMLVolumePropertyNode(vtkMRMLNode* node);

  void setMRMLViewNode(vtkMRMLNode* node);

  vtkMRMLVolumeRenderingDisplayNode* createVolumeRenderingDisplayNode();

protected:
  QScopedPointer<qSlicerVolumeRenderingModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModuleWidget);
};

#endif
