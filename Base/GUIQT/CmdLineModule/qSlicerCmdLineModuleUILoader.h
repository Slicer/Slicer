#ifndef __qSlicerCmdLineModuleUILoader_h
#define __qSlicerCmdLineModuleUILoader_h 


#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerCmdLineModuleUILoader
{ 
public:
  
  qSlicerCmdLineModuleUILoader();
  virtual ~qSlicerCmdLineModuleUILoader(); 
  
  virtual void printAdditionalInfo();


private:
  class qInternal;
  qInternal* Internal;
};

#endif
