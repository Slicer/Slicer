
// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerIOManager.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerMainWindow.h>
#include <qSlicerNodeWriter.h>

#include <QMenuBar>
#include <QSettings>

// SceneViewsModule includes
#include "qSlicerSceneViewsModule.h"

// SceneViewsModule GUI includes
#include <qMRMLCaptureToolBar.h>
#include <qSlicerSceneViewsModuleWidget.h>

// SceneViewsModule Logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

//-----------------------------------------------------------------------------
class qSlicerSceneViewsModulePrivate
{
public:
  qSlicerSceneViewsModulePrivate();
  virtual ~qSlicerSceneViewsModulePrivate();
  qMRMLCaptureToolBar* CaptureToolBar;
  bool SceneViewsModuleOwnsToolBar{ true };
  void addToolBar();
};

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSceneViewsModulePrivate::qSlicerSceneViewsModulePrivate()
{
  this->CaptureToolBar = new qMRMLCaptureToolBar;
  this->CaptureToolBar->setWindowTitle(QObject::tr("Capture/Restore"));
  this->CaptureToolBar->setObjectName("CaptureToolBar");
  this->CaptureToolBar->setVisible(false);
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModulePrivate::~qSlicerSceneViewsModulePrivate()
{
  if (this->SceneViewsModuleOwnsToolBar)
  {
    // the toolbar has not been added to the main window
    // so it is still owned by this class, therefore
    // we are responsible for deleting it
    delete this->CaptureToolBar;
    this->CaptureToolBar = nullptr;
  }
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModule::qSlicerSceneViewsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSceneViewsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModule::~qSlicerSceneViewsModule() = default;

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModule::setup()
{
  Q_D(qSlicerSceneViewsModule);

  qSlicerCoreIOManager* ioManager = qSlicerApplication::application()->coreIOManager();
  ioManager->registerIO(new qSlicerNodeWriter("SceneViews", QString("SceneViewFile"), QStringList() << "vtkMRMLSceneViewNode", true, this));

  d->addToolBar();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModulePrivate::addToolBar()
{
  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  if (mainWindow == nullptr)
  {
    qDebug("qSlicerSceneViewsModulePrivate::addToolBar: no main window is available, toolbar is not added");
    return;
  }

  // Capture bar needs to listen to the MRML scene and the layout
  this->CaptureToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(), SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->CaptureToolBar, SLOT(setMRMLScene(vtkMRMLScene*)));

  QObject::connect(this->CaptureToolBar, SIGNAL(screenshotButtonClicked()), qSlicerApplication::application()->ioManager(), SLOT(openScreenshotDialog()));

  // to get the scene views module dialog to pop up when a button is pressed
  // in the capture tool bar, we emit a signal, and the
  // io manager will deal with the sceneviews module
  QObject::connect(this->CaptureToolBar, SIGNAL(sceneViewButtonClicked()), qSlicerApplication::application()->ioManager(), SLOT(openSceneViewsDialog()));

  // if testing is enabled on the application level, add a time out to the pop ups
  if (qSlicerApplication::application()->testAttribute(qSlicerCoreApplication::AA_EnableTesting))
  {
    this->CaptureToolBar->setPopupsTimeOut(true);
  }

  mainWindow->addToolBar(Qt::TopToolBarArea, this->CaptureToolBar);
  this->SceneViewsModuleOwnsToolBar = false;
  for (QMenu* const toolBarMenu : mainWindow->findChildren<QMenu*>())
  {
    if (toolBarMenu->objectName() == QString("WindowToolBarsMenu"))
    {
      toolBarMenu->addAction(this->CaptureToolBar->toggleViewAction());
      break;
    }
  }

  // Main window takes care of saving and restoring toolbar geometry and state.
  // However, when state is restored the capture toolbar was not created yet.
  // We need to restore the main window state again, now, that the Capture toolbar is available.
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
  {
    mainWindow->restoreState(settings.value("windowState").toByteArray());
  }
  settings.endGroup();

  // Capture tool bar needs to listen to the layout manager
  QObject::connect(qSlicerApplication::application()->layoutManager(),
                   SIGNAL(activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode*)),
                   this->CaptureToolBar,
                   SLOT(setActiveMRMLThreeDViewNode(vtkMRMLViewNode*)));
  this->CaptureToolBar->setActiveMRMLThreeDViewNode(qSlicerApplication::application()->layoutManager()->activeMRMLThreeDViewNode());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSceneViewsModule::createWidgetRepresentation()
{
  return new qSlicerSceneViewsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSceneViewsModule::createLogic()
{

  return vtkSlicerSceneViewsModuleLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerSceneViewsModule::helpText() const
{
  QString help = "The SceneViews module. Create, edit, restore, delete scene views. Scene "
                 "views capture the state of the MRML scene at a given point. The "
                 "recommended way to use them is to load all of your data and then adjust "
                 "visibility of the elements and capture interesting scene views. "
                 "Unexpected behavior may occur if you add or delete data from the scene "
                 "while saving and restoring scene views.\n";
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerSceneViewsModule::acknowledgementText() const
{
  return "This module was developed by Daniel Haehn and Kilian Pohl. The research was funded by an ARRA supplement to NIH NCRR (P41 RR13218).";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneViewsModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (SPL, BWH)");
  moduleContributors << QString("Wendy Plesniak (SPL, BWH)");
  moduleContributors << QString("Daniel Haehn (UPenn)");
  moduleContributors << QString("Kilian Pohl (UPenn)");
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSceneViewsModule::icon() const
{
  return QIcon(":/Icons/SelectCameras.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneViewsModule::categories() const
{
  return QStringList() << "";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneViewsModule::dependencies() const
{
  return QStringList() << QString("Sequences");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModule::showSceneViewDialog()
{
  Q_ASSERT(this->widgetRepresentation());
  dynamic_cast<qSlicerSceneViewsModuleWidget*>(this->widgetRepresentation())->showSceneViewDialog();
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneViewsModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLSceneViewNode";
}
