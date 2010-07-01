// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerMainWindow.h" 
#include "ui_qSlicerMainWindow.h" 
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModulePanel.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"
#include "qSlicerModuleSelectorWidget.h"

//-----------------------------------------------------------------------------
class qSlicerMainWindowPrivate: public ctkPrivate<qSlicerMainWindow>, public Ui_qSlicerMainWindow
{
public:
  CTK_DECLARE_PUBLIC(qSlicerMainWindow);
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
// qSlicerMainWindowPrivate methods

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  CTK_P(qSlicerMainWindow);
  
  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  this->ModuleSelector = new qSlicerModuleSelectorWidget(p);
  this->MainToolBar->addWidget(this->ModuleSelector);

  // Connect the selector with the module panel
  QObject::connect(this->ModuleSelector, SIGNAL(moduleSelected(const QString&)),
                   this->ModulePanel, SLOT(setModule(const QString&)));

  // Instanciate and assign the layout manager to the slicer application
  qSlicerLayoutManager* layoutManager = new qSlicerLayoutManager(this->CentralWidget);
  qSlicerApplication::application()->setLayoutManager(layoutManager);

  // Layout manager should also listen the MRML scene
  layoutManager->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   layoutManager,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
}

//-----------------------------------------------------------------------------
// qSlicerMainWindow methods

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *_parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerMainWindow);
  CTK_D(qSlicerMainWindow);
  d->setupUi(this);
  
  // Main window core helps to coordinate various widgets and panels
  d->Core = new qSlicerMainWindowCore(this);
  
  this->setupMenuActions();
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerMainWindow, qSlicerMainWindowCore*, core, Core);

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerMainWindow, QToolBar*, moduleToolBar, ModuleToolBar);

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerMainWindow, qSlicerModulePanel*, modulePanel, ModulePanel);

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerMainWindow, qSlicerModuleSelectorWidget*, moduleSelector, ModuleSelector);

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
  CTK_D(qSlicerMainWindow);
  
  this->connect(
    d->actionFileExit, SIGNAL(triggered()),
    qSlicerApplication::instance(), SLOT(quit()));

  qSlicerMainWindow_connect(FileAddData);
  qSlicerMainWindow_connect(FileImportScene);
  qSlicerMainWindow_connect(FileLoadScene);
  
  qSlicerMainWindow_connect(EditRedo);
  qSlicerMainWindow_connect(EditUndo);

  qSlicerMainWindow_connect(ViewLayoutConventional);
  qSlicerMainWindow_connect(ViewLayoutFourUp);
  qSlicerMainWindow_connect(ViewLayoutDual3D);
  qSlicerMainWindow_connect(ViewLayoutOneUp3D);
  qSlicerMainWindow_connect(ViewLayoutOneUpRedSlice);
  qSlicerMainWindow_connect(ViewLayoutOneUpYellowSlice);
  qSlicerMainWindow_connect(ViewLayoutOneUpGreenSlice);
  qSlicerMainWindow_connect(ViewLayoutTabbed3D);
  qSlicerMainWindow_connect(ViewLayoutTabbedSlice);
  qSlicerMainWindow_connect(ViewLayoutCompare);
  qSlicerMainWindow_connect(ViewLayoutSideBySideCompare);
  
  qSlicerMainWindow_connect(WindowPythonInteractor);
    
}

#undef qSlicerMainWindow_connect
