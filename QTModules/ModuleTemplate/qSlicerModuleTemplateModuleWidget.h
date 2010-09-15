#ifndef __qSlicerModuleTemplateModuleWidget_h
#define __qSlicerModuleTemplateModuleWidget_h

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
  virtual ~qSlicerModuleTemplateModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerModuleTemplateModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerModuleTemplateModuleWidget);
  Q_DISABLE_COPY(qSlicerModuleTemplateModuleWidget);
};

#endif
