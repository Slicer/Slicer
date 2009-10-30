#ifndef __qSlicerAbstractLoadableModule_h
#define __qSlicerAbstractLoadableModule_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLScene; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerAbstractLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT
  
public:
  
  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractLoadableModule(QWidget *parent=0);
  virtual ~qSlicerAbstractLoadableModule();
  
  virtual void printAdditionalInfo(); 

  // Description:
  // Return help/about text
  virtual QString helpText();
  virtual QString aboutText(); 
  
private:
  class qInternal;
  qInternal* Internal;
};

#endif
