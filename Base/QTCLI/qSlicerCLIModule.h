#ifndef __qSlicerCLIModule_h
#define __qSlicerCLIModule_h

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTCLIWin32Header.h"

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModule : public qSlicerAbstractModule
{
  //Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerCLIModule(QWidget *parent=0);
  virtual ~qSlicerCLIModule();

  virtual void printAdditionalInfo();

  void setXmlModuleDescription(const char* xmlModuleDescription);

  virtual QString moduleTitle();

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText();
  virtual QString acknowledgementText();

protected:
  virtual void initializer();

private:
  struct qInternal;
  qInternal* Internal;
};

#endif
