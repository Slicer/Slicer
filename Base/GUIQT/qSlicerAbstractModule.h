#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h 

#include "qSlicerWidget.h"

#include "qSlicerBaseGUIQTWin32Header.h"

// Macro allowing to declare:
//     - the static method 'moduleTitle()'
//     - the member 'ModuleTitle'
#define qSlicerGetModuleTitleDeclarationMacro() \
  public: \
  static const QString moduleTitle(); \
  static QString ModuleTitle; 

// Macro allowing to define
//    - the static method 'moduleTitle()'
//    - the associated module title
#define qSlicerGetModuleTitleDefinitionMacro(_CLASSTYPE, _TITLE) \
  const QString _CLASSTYPE::moduleTitle(){ return _CLASSTYPE::ModuleTitle; } \
  QString _CLASSTYPE::ModuleTitle = _TITLE; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerAbstractModule : public qSlicerWidget
{
  Q_OBJECT
  
public:
  
  typedef qSlicerWidget Superclass;
  qSlicerAbstractModule(QWidget *parent=0);
  virtual ~qSlicerAbstractModule();
  
  virtual void printAdditionalInfo(); 

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
  virtual QString helpText() = 0;
  virtual QString aboutText() = 0; 

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
