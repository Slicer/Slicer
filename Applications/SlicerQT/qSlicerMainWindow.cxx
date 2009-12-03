#include "qSlicerMainWindow.h" 

#include "ui_qSlicerMainWindow.h" 

// SlicerQT includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerMainWindowPrivate: public qCTKPrivate<qSlicerMainWindow>, public Ui_qSlicerMainWindow
{
  QCTK_DECLARE_PUBLIC(qSlicerMainWindow);
  qSlicerMainWindowPrivate()
    {
    this->Core = 0;
    }
  qSlicerMainWindowCore* Core;
};

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *parent)
 :Superclass(parent)
{
  QCTK_INIT_PRIVATE(qSlicerMainWindow);
  QCTK_D(qSlicerMainWindow);
  d->setupUi(this);
  
  // Main window core helps to coordinate various widgets and panels
  d->Core = new qSlicerMainWindowCore(this);
  
  this->setupMenuActions();

  qSlicerModuleManager::instance()->setModuleToolBar(d->ModuleToolBar);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindow, qSlicerMainWindowCore*, core, Core);

//-----------------------------------------------------------------------------
void qSlicerMainWindow::setupMenuActions()
{
  this->connect(
    qctk_d()->actionFileExit, SIGNAL(triggered()),
    qSlicerApplication::instance(), SLOT(quit()));
}
