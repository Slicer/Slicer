#ifndef __qSlicerCLIModuleWidget_h
#define __qSlicerCLIModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCLIWin32Header.h"

class ModuleDescription; 
class qSlicerCLIModuleWidgetPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCLIModuleWidget(ModuleDescription* desc, QWidget *parent=0);
  virtual ~qSlicerCLIModuleWidget(){}

protected:
  // Description:
  // Overloaded
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCLIModuleWidget);
};

#endif
