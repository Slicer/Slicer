#ifndef __qSlicerWelcomeModule_h
#define __qSlicerWelcomeModule_h

#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerWelcomeModuleWin32Header.h"

class Q_SLICER_QTMODULES_WELCOME_EXPORT qSlicerWelcomeModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerWelcomeModule(QWidget *parent=0);
  virtual ~qSlicerWelcomeModule();

  virtual void printAdditionalInfo();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  virtual void setup();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
