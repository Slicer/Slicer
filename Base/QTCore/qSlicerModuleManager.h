#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerBaseQTCoreWin32Header.h"

class qSlicerAbstractModule;
class qSlicerModuleFactory;

class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleManager : public QObject
{
  Q_OBJECT
public:
  typedef qSlicerModuleManager Self;
  qSlicerModuleManager();
  virtual ~qSlicerModuleManager(){}

  // Description:
  static qSlicerModuleManager* instance();

  virtual void printAdditionalInfo();

  // Description:
  // Return a pointer to the current module factory
  qSlicerModuleFactory* factory();

  // Description:
  bool loadModule(const QString& moduleTitle);
  bool loadModuleByName(const QString& moduleName);

  // Description:
  bool unLoadModule(const QString& moduleTitle);
  bool unLoadModuleByName(const QString& moduleName);

  // Description:
  bool isLoaded(const QString& moduleTitle);

  // Description:
  qSlicerAbstractModule* getModule(const QString& moduleTitle);
  qSlicerAbstractModule* getModuleByName(const QString& moduleName);

  // Description:
  // Convenient method to get module title given its name
  QString moduleTitle(const QString& moduleName) const;

  // Description:
  // Convenient method to get module name given its title
  QString moduleName(const QString& moduleTitle) const;

signals:
  void moduleLoaded(qSlicerAbstractModule* module);
  void moduleAboutToBeUnloaded(qSlicerAbstractModule* module);

private:
  qSlicerModuleManager(const qSlicerModuleManager&);  // Not implemented.
  void operator=(const qSlicerModuleManager&);  // Not implemented.

  QCTK_DECLARE_PRIVATE(qSlicerModuleManager);
};

#endif
