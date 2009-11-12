#ifndef __qSlicerCLIModuleUILoader_h
#define __qSlicerCLIModuleUILoader_h


#include "qSlicerBaseQTCLIWin32Header.h"

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleUILoader
{
public:

  qSlicerCLIModuleUILoader();
  virtual ~qSlicerCLIModuleUILoader();

  virtual void printAdditionalInfo();


private:
  struct qInternal;
  qInternal* Internal;
};

#endif
