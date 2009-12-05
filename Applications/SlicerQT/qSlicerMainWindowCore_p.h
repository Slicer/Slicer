#ifndef __qSlicerMainWindowCorePrivate_p_h
#define __qSlicerMainWindowCorePrivate_p_h

#include "qSlicerMainWindowCore.h"

// SlicerQT includes
#include "qSlicerMainWindow.h"

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>
#include <QStringList>
#include <QPointer>

class qSlicerAbstractModule; 
class QSignalMapper; 

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
  
  // For now, contains the list of module (title) we want to display in the ModuleToolbar
  QStringList ToolBarModuleList;
};

#endif
