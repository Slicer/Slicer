#ifndef __qSlicerModuleFactory_h
#define __qSlicerModuleFactory_h 

#include <qCTKAbstractObjectFactory.h>

#include "qSlicerBaseGUI2Win32Header.h"

class qSlicerAbstractModule; 

class Q_SLICER_BASE_GUI2_EXPORT qSlicerModuleFactory : 
  protected qCTKAbstractObjectFactory<qSlicerAbstractModule, QString>
{ 
public:
  
  typedef qCTKAbstractObjectFactory<qSlicerAbstractModule, QString> Superclass;
  qSlicerModuleFactory();
  virtual ~qSlicerModuleFactory(); 
  
  virtual void dumpObjectInfo();
  
  // Description:
  QString getModuleName(const QString & moduleTitle);
  
  // Description:
  QString getModuleTitle(const QString & moduleName); 
  
  // Description:
  qSlicerAbstractModule* createModule(const QString& moduleName); 

protected:

  // Description:
  template<typename ClassType>
  void registerModule(const QString& moduleTitle); 


private:
  class qInternal;
  qInternal* Internal;
};

#endif
