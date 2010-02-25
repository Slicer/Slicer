#include "qSlicerMainWindow.h" 

#include "ui_qSlicerMainWindow.h" 

// SlicerQT includes
#include "qSlicerApplication.h"
#include "qSlicerModulePanel.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"
#include "qSlicerModuleSelectorWidget.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerMainWindowPrivate: public qCTKPrivate<qSlicerMainWindow>, public Ui_qSlicerMainWindow
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerMainWindow);
  qSlicerMainWindowPrivate()
    {
    this->Core = 0;
    this->ModuleSelector = 0; 
    }
  void setupUi(QMainWindow * mainWindow);
  qSlicerMainWindowCore*       Core;
  qSlicerModuleSelectorWidget* ModuleSelector; 
};

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *_parent):Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qSlicerMainWindow);
  QCTK_D(qSlicerMainWindow);
  d->setupUi(this);
  
  // Main window core helps to coordinate various widgets and panels
  d->Core = new qSlicerMainWindowCore(this);
  
  this->setupMenuActions();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindow, qSlicerMainWindowCore*, core, Core);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindow, QToolBar*, moduleToolBar, ModuleToolBar);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindow, qSlicerModulePanel*, modulePanel, ModulePanel);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindow, qSlicerModuleSelectorWidget*, moduleSelector, ModuleSelector);

//-----------------------------------------------------------------------------
// Helper macro allowing to connect the MainWindow action with the corresponding
// slot in MainWindowCore
#define qSlicerMainWindow_connect(ACTION_NAME)   \
  this->connect(                                 \
    d->action##ACTION_NAME, SIGNAL(triggered()), \
    this->core(),                                \
    SLOT(on##ACTION_NAME##ActionTriggered()));

//-----------------------------------------------------------------------------
void qSlicerMainWindow::setupMenuActions()
{
  QCTK_D(qSlicerMainWindow);
  
  this->connect(
    d->actionFileExit, SIGNAL(triggered()),
    qSlicerApplication::instance(), SLOT(quit()));

  qSlicerMainWindow_connect(EditRedo);
  qSlicerMainWindow_connect(EditUndo);
  qSlicerMainWindow_connect(WindowPythonInteractor);
    
}

#undef qSlicerMainWindow_connect

//-----------------------------------------------------------------------------
// qSlicerMainWindowPrivate methods

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  QCTK_P(qSlicerMainWindow);
  
  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  this->ModuleSelector = new qSlicerModuleSelectorWidget(p);
  this->MainToolBar->addWidget(this->ModuleSelector);

  // Connect the selector with the module panel
  QObject::connect(this->ModuleSelector, SIGNAL(moduleSelected(const QString&)),
                   this->ModulePanel, SLOT(setModule(const QString&)));
}
