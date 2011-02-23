#ifndef __qSlicerVolumeRenderingModuleWidget_h
#define __qSlicerVolumeRenderingModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLVolumeRenderingDisplayableManager;

class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeRenderingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVolumeRenderingModuleWidget();

//   vtkMRMLVolumeRenderingDisplayableManager* getVolumeRenderingDisplayableManager();

public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

protected:
  QScopedPointer<qSlicerVolumeRenderingModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

  // Note: Member should start with an upper-case later and should be added to the Pimpl
  // vtkMRMLVolumeRenderingDisplayableManager* volumeRenderingDisplayableManager;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModuleWidget);
};

#endif
