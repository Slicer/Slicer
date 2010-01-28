/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractModule;
class qSlicerModuleFactoryManager; 

class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleManager : public QObject
{
  Q_OBJECT
public:
  qSlicerModuleManager();
  virtual ~qSlicerModuleManager();

  // Description:
  virtual void printAdditionalInfo();

  // Description:
  // Return a pointer to the current module factory manager
  qSlicerModuleFactoryManager * factoryManager()const;

  // Description:
  bool loadModule(const QString& name);

  // Description:
  bool unLoadModule(const QString& name);

  // Description:
  bool isLoaded(const QString& name)const;

  // Description:
  qSlicerAbstractModule* module(const QString& name);

  // Description:
  // Convenient method to get module title given its name
  // Deprecated
  QString moduleTitle(const QString& name) const;

  // Description:
  // Convenient method to get module name given its title
  // Deprecated
  QString moduleName(const QString& title) const;

signals:
  void moduleLoaded(qSlicerAbstractModule* module);
  void moduleAboutToBeUnloaded(qSlicerAbstractModule* module);

private:
  qSlicerModuleManager(const qSlicerModuleManager&);  // Not implemented.
  void operator=(const qSlicerModuleManager&);  // Not implemented.

  QCTK_DECLARE_PRIVATE(qSlicerModuleManager);
};

#endif
