#ifndef __qSlicerVolumesModuleWidget_h
#define __qSlicerVolumesModuleWidget_h


// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumesModuleExport.h"

class qSlicerVolumesModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumesModuleWidget(QWidget *parent=0);

public slots:

  void setActiveVolumeNode(vtkMRMLNode* node);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerVolumesModuleWidget);
};

#endif
