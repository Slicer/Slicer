#ifndef __qSlicerWelcomeModule_h
#define __qSlicerWelcomeModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerWelcomeModuleWin32Header.h"

class qSlicerWelcomeModulePrivate;

class Q_SLICER_QTMODULES_WELCOME_EXPORT qSlicerWelcomeModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerWelcomeModule(QWidget *parent=0);

  virtual void printAdditionalInfo();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerWelcomeModule);
};

#endif
