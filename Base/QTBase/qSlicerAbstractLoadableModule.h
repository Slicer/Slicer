#ifndef __qSlicerAbstractLoadableModule_h
#define __qSlicerAbstractLoadableModule_h

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTBaseWin32Header.h"

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractLoadableModule(QWidget *parent=0);
  virtual ~qSlicerAbstractLoadableModule();

  virtual void printAdditionalInfo();

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText();
  virtual QString acknowledgementText();

protected:
  virtual void setup();

private:
  struct qInternal;
  qInternal* Internal;
};

Q_DECLARE_INTERFACE(qSlicerAbstractLoadableModule,
                     "org.slicer.QTModules.qSlicerAbstractLoadableModule/1.0");

#endif
