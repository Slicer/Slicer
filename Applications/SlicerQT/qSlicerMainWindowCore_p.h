#ifndef __qSlicerMainWindowCorePrivate_p_h
#define __qSlicerMainWindowCorePrivate_p_h

// Qt includes
#include <QObject>
#include <QStringList>
#include <QPointer>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerMainWindowCore.h"
#include "qSlicerMainWindow.h"

class qSlicerAbstractModule; 
class QSignalMapper;
class ctkPythonShell;

//-----------------------------------------------------------------------------
class qSlicerMainWindowCorePrivate: public QObject,
                                    public ctkPrivate<qSlicerMainWindowCore>
{
  Q_OBJECT
  CTK_DECLARE_PUBLIC(qSlicerMainWindowCore);
  
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
  ctkPythonShell*             PythonShell;
  
  // For now, contains the list of module (title) we want to display in the ModuleToolbar
  QStringList ToolBarModuleList;
};

#endif
