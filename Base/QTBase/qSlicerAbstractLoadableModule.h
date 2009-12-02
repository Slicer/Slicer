#ifndef __qSlicerAbstractLoadableModule_h
#define __qSlicerAbstractLoadableModule_h

#include "qSlicerAbstractModule.h"

#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseWin32Header.h"

class qSlicerAbstractLoadableModulePrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractLoadableModule(QWidget *parent=0);

  virtual void printAdditionalInfo();

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText();
  virtual QString acknowledgementText();

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractLoadableModule);
};

Q_DECLARE_INTERFACE(qSlicerAbstractLoadableModule,
                     "org.slicer.QTModules.qSlicerAbstractLoadableModule/1.0");

#endif
