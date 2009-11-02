#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h 

#include "qSlicerWidget.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerAbstractModule : public qSlicerWidget
{
  Q_OBJECT
  
public:
  
  typedef qSlicerWidget Superclass;
  qSlicerAbstractModule(QWidget *parent=0);
  virtual ~qSlicerAbstractModule();
  
  virtual void printAdditionalInfo(); 
  
  virtual QString moduleTitle() = 0; 

  // Description:
  virtual QString moduleName();
  
  // Description:
  virtual void populateToolbar(){}
  virtual void unPopulateToolbar(){}
  
  // Description:
  virtual void populateApplicationSettings(){}
  virtual void unPopulateApplicationSettings(){}
  
  // Description:
  // Return help/about text
  virtual QString helpText() {return "";}
  virtual QString aboutText() { return "";} 

public slots:

  // Description:
  // Set/Get module enabled
  bool moduleEnabled(); 
  virtual void setModuleEnabled(bool value); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif
