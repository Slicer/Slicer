#ifndef __qSlicerMainWindowCorePrivate_p_h
#define __qSlicerMainWindowCorePrivate_p_h

// Qt includes
#include <QObject>
#include <QPointer>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerMainWindowCore.h"
#include "qSlicerMainWindow.h"

class qSlicerAbstractModule;
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

public:
  QPointer<qSlicerMainWindow> ParentWidget;
  ctkPythonShell*             PythonShell;
};

#endif
