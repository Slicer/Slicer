#include "qSlicerMainWindowCore.h" 

// QT includes
#include <QMainWindow>
#include <QPointer>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerMainWindowCorePrivate: public qCTKPrivate<qSlicerMainWindowCore>
{
  QCTK_DECLARE_PUBLIC(qSlicerMainWindowCore);
  qSlicerMainWindowCorePrivate()
    {
    }
  QPointer<QMainWindow> ParentWidget;
};

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::qSlicerMainWindowCore(QMainWindow* parent):Superclass(parent)
{
  QCTK_INIT_PRIVATE(qSlicerMainWindowCore);
  qctk_d()->ParentWidget = parent;
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindowCore, QMainWindow*, widget, ParentWidget);
