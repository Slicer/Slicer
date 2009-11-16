#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h 

#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerVolumesModuleWin32Header.h"

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModule : 
  public qSlicerAbstractLoadableModule
{ 
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule)

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerVolumesModule(QWidget *parent=0);
  virtual ~qSlicerVolumesModule(); 
  
  virtual void printAdditionalInfo();
  
  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  virtual void initializer();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
