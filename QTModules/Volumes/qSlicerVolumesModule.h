#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h 

#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerVolumesModule : public qSlicerAbstractLoadableModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerVolumesModule(QWidget *parent=0);
  virtual ~qSlicerVolumesModule(); 
  
  virtual void printAdditionalInfo();
  
  qSlicerGetModuleTitleDeclarationMacro();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
