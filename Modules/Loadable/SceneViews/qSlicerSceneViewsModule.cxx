
// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerIOManager.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerMainWindow.h>
#include <qSlicerNodeWriter.h>

#include <QMenuBar>

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
  qMRMLCaptureToolBar* CaptureToolBar{ nullptr };
};

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
  ioManager->registerIO(
    new qSlicerNodeWriter("SceneViews", QString("SceneViewFile"), QStringList() << "vtkMRMLSceneViewNode", true, this));

  qSlicerMainWindow* mainWindow = qobject_cast<qSlicerMainWindow*>(qSlicerApplication::application()->mainWindow());
  if (mainWindow)
  {
    //----------------------------------------------------------------------------
    // Capture tool bar
    //----------------------------------------------------------------------------
    // Capture bar needs to listen to the MRML scene and the layout
    d->CaptureToolBar = new qMRMLCaptureToolBar;
    d->CaptureToolBar->setObjectName("CaptureToolBar");
    d->CaptureToolBar->setWindowTitle("Capture/Restore");
    d->CaptureToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
    QObject::connect(qSlicerApplication::application(),
                     SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                     d->CaptureToolBar,
                     SLOT(setMRMLScene(vtkMRMLScene*)));
    d->CaptureToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());

    QObject::connect(d->CaptureToolBar,
                     SIGNAL(screenshotButtonClicked()),
                     qSlicerApplication::application()->ioManager(),
                     SLOT(openScreenshotDialog()));

    // to get the scene views module dialog to pop up when a button is pressed
    // in the capture tool bar, we emit a signal, and the
    // io manager will deal with the sceneviews module
    QObject::connect(d->CaptureToolBar,
                     SIGNAL(sceneViewButtonClicked()),
                     qSlicerApplication::application()->ioManager(),
                     SLOT(openSceneViewsDialog()));

    // if testing is enabled on the application level, add a time out to the pop ups
    if (qSlicerApplication::application()->testAttribute(qSlicerCoreApplication::AA_EnableTesting))
    {
      d->CaptureToolBar->setPopupsTimeOut(true);
    }

    mainWindow->addToolBar(d->CaptureToolBar);
    // Capture tool bar needs to listen to the layout manager
    QObject::connect(qSlicerApplication::application()->layoutManager(),
                     SIGNAL(activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode*)),
                     d->CaptureToolBar,
                     SLOT(setActiveMRMLThreeDViewNode(vtkMRMLViewNode*)));
    d->CaptureToolBar->setActiveMRMLThreeDViewNode(
      qSlicerApplication::application()->layoutManager()->activeMRMLThreeDViewNode());
  }
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
  return "This module was developed by Daniel Haehn and Kilian Pohl. The research was funded by an ARRA supplement to "
         "NIH NCRR (P41 RR13218).";
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
