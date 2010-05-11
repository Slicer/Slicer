#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerBaseQTCoreWin32Header.h"

class qSlicerModuleManagerInitialize;

class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleManager : public QObject
{
  Q_OBJECT
public:
  typedef qSlicerModuleManager Self;

  // Description:
  static qSlicerModuleManager* instance();


protected:
  qSlicerModuleManager();
  ~qSlicerModuleManager(){}

  static qSlicerModuleManager* Instance;

  // Description:
  static void classInitialize();
  static void classFinalize();

  friend class qSlicerModuleManagerInitialize;

private:
  qSlicerModuleManager(const qSlicerModuleManager&);  // Not implemented.
  void operator=(const qSlicerModuleManager&);  // Not implemented.

  CTK_DECLARE_PRIVATE(qSlicerModuleManager);
};


// Utility class to make sure qSlicerModuleManager is initialized before it is used.
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleManagerInitialize
{
public:
  typedef qSlicerModuleManagerInitialize Self;

  qSlicerModuleManagerInitialize();
  ~qSlicerModuleManagerInitialize();
private:
  static unsigned int Count;
};

// This instance will show up in any translation unit that uses
// qSlicerModuleManager.  It will make sure qSlicerModuleManager is initialized
// before it is used.
static qSlicerModuleManagerInitialize qSlicerModuleManagerInitializer;

#endif
