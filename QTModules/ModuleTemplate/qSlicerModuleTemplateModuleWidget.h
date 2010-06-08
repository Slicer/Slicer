#ifndef __qSlicerModuleTemplateModuleWidget_h
#define __qSlicerModuleTemplateModuleWidget_h


// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerModuleTemplateModuleExport.h"

class qSlicerModuleTemplateModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_MODULETEMPLATE_EXPORT qSlicerModuleTemplateModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerModuleTemplateModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

public slots:

  void setActiveVolumeNode(vtkMRMLNode* node);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerModuleTemplateModuleWidget);
};

#endif
