#ifndef __qSlicerAbstractModuleWidget_h
#define __qSlicerAbstractModuleWidget_h

#include "qSlicerWidget.h"

#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseWin32Header.h"

// class vtkSlicerApplicationLogic;
class qSlicerAbstractModule; 
class QAction; 
class qSlicerAbstractModuleWidgetPrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractModuleWidget : public qSlicerWidget
{
  Q_OBJECT
public:

  typedef qSlicerWidget Superclass;
  qSlicerAbstractModuleWidget(QWidget *parent=0);

  // Description:
  // All initialization code should be done in the initialize function
  void initialize(/*vtkSlicerApplicationLogic* appLogic*/);

  // Description:
  // Return the action allowing to show the module
  virtual QAction* showModuleAction()  { return 0; }

  // Description:
  // Set/Get module name
  void setName(const QString& moduleName);
  QString name()const;

protected:
  // Description:
  // All inialization code should be done in the setup
  virtual void setup() = 0;

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractModuleWidget);
};

#endif
