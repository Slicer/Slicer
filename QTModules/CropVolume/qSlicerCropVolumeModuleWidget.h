#ifndef __qSlicerCropVolumeModuleWidget_h
#define __qSlicerCropVolumeModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerCropVolumeModuleExport.h"

class qSlicerCropVolumeModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_CROPVOLUME_EXPORT qSlicerCropVolumeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCropVolumeModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCropVolumeModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerCropVolumeModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerCropVolumeModuleWidget);
  Q_DISABLE_COPY(qSlicerCropVolumeModuleWidget);

  void initializeNode(vtkMRMLNode*);
};

#endif
