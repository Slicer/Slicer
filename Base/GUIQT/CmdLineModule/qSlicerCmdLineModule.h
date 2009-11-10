#ifndef __qSlicerCmdLineModule_h
#define __qSlicerCmdLineModule_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerCmdLineModule : public qSlicerAbstractModule
{
  Q_OBJECT
  
public:
  
  typedef qSlicerAbstractModule Superclass;
  qSlicerCmdLineModule(QWidget *parent=0);
  virtual ~qSlicerCmdLineModule();
  
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
  class qInternal;
  qInternal* Internal;
};

#endif
