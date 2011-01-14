#ifndef __qSlicerExtensionTestModuleWidget_h
#define __qSlicerExtensionTestModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerExtensionTestModuleExport.h"

class qSlicerExtensionTestModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_EXTENSIONTEST_EXPORT qSlicerExtensionTestModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerExtensionTestModuleWidget(QWidget *parent=0);
  virtual ~qSlicerExtensionTestModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerExtensionTestModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionTestModuleWidget);
  Q_DISABLE_COPY(qSlicerExtensionTestModuleWidget);
};

#endif
