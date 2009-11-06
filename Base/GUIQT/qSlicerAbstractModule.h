#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h 

#include "qSlicerWidget.h"

#define qSlicerGetModuleTitleMacro(_TITLE) \
  static QString staticModuleTitle() { return _TITLE; } \
  virtual QString moduleTitle() { return _TITLE; }

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerAbstractModule : public qSlicerWidget
{
  Q_OBJECT
  
public:
  
  typedef qSlicerWidget Superclass;
  qSlicerAbstractModule(QWidget *parent=0);
  virtual ~qSlicerAbstractModule();
  
  virtual void printAdditionalInfo(); 
  
  // Description:
  // All initialization code should be done in the initialize function
  void initialize();
  inline bool initialized() { return this->Initialized; }
  
  // Description:
  virtual QString moduleName();
  virtual QString moduleTitle() = 0;
  
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

protected:
  // Description:
  // All inialization code should be done in the initializer
  virtual void initializer() = 0; 

private:
  class qInternal;
  qInternal* Internal;
  
  // Description:
  // Indicate if the module has already been initialized
  bool Initialized;
};

#endif
