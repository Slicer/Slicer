#ifndef __qSlicerMainWindowCorePrivate_p_h
#define __qSlicerMainWindowCorePrivate_p_h

// QT includes
#include <QObject>
#include <QStringList>
#include <QPointer>

// qCTK includes
#include <qCTKPimpl.h>

// SlicerQT includes
#include "qSlicerMainWindowCore.h"
#include "qSlicerMainWindow.h"

class qSlicerAbstractModule; 
class QSignalMapper;
class qCTKPythonShell; 

//-----------------------------------------------------------------------------
class qSlicerMainWindowCorePrivate: public QObject,
                                    public qCTKPrivate<qSlicerMainWindowCore>
{
  Q_OBJECT
  QCTK_DECLARE_PUBLIC(qSlicerMainWindowCore);
  
public:
  explicit qSlicerMainWindowCorePrivate();
  virtual ~qSlicerMainWindowCorePrivate(){}

public slots:
  // Description:
  // Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractModule* module);

  // Description:
  // Handle pre-unload operation
  void onModuleAboutToBeUnloaded(qSlicerAbstractModule* module);

public:
  QPointer<qSlicerMainWindow> ParentWidget;
  QSignalMapper*              ShowModuleActionMapper;
  qCTKPythonShell*            PythonShell;
  
  // For now, contains the list of module (title) we want to display in the ModuleToolbar
  QStringList ToolBarModuleList;
};

#endif
