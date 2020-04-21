/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/


// Qt includes
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QTimer>

#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"

#include "vtkMRMLScene.h"

// Slicer includes
#include "qSlicerIOManager.h"
#include "qSlicerNodeWriter.h"

// Sequence Logic includes
#include <vtkSlicerSequencesLogic.h>

// Sequence includes
#include "vtkMRMLSequenceBrowserNode.h"
#include "qMRMLSequenceBrowserToolBar.h"
#include "qSlicerSequencesModule.h"
#include "qSlicerSequencesModuleWidget.h"
#include "qSlicerSequencesReader.h"

static const double UPDATE_VIRTUAL_OUTPUT_NODES_PERIOD_SEC = 0.020; // refresh output with a maximum of 50FPS

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerSequencesModule, qSlicerSequencesModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSequencesModulePrivate
{
public:
  qSlicerSequencesModulePrivate();

  /// Adds sequence browser toolbar to the application GUI (toolbar and menu)
  virtual void addToolBar();

  virtual ~qSlicerSequencesModulePrivate();
  QTimer UpdateAllVirtualOutputNodesTimer;
  qMRMLSequenceBrowserToolBar* ToolBar;
  bool SequenceBrowserModuleOwnsToolBar{true};
  bool AutoShowToolBar{true};

};

//-----------------------------------------------------------------------------
// qSlicerSequencesModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSequencesModulePrivate::qSlicerSequencesModulePrivate()
{
  this->ToolBar = new qMRMLSequenceBrowserToolBar;
  this->ToolBar->setWindowTitle(QObject::tr("Sequence browser"));
  this->ToolBar->setObjectName("SequenceBrowser");
  this->ToolBar->setVisible(false);
}

//-----------------------------------------------------------------------------
qSlicerSequencesModulePrivate::~qSlicerSequencesModulePrivate()
{
  if (this->SequenceBrowserModuleOwnsToolBar)
    {
    // the toolbar has not been added to the main window
    // so it is still owned by this class, therefore
    // we are responsible for deleting it
    delete this->ToolBar;
    this->ToolBar = nullptr;
    }
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModulePrivate::addToolBar()
{
  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  if (mainWindow==nullptr)
    {
    qDebug("qSlicerSequencesModulePrivate::addToolBar: no main window is available, toolbar is not added");
    return;
    }

  this->ToolBar->setWindowTitle("Sequence browser");
  this->ToolBar->setObjectName("SequenceBrowserToolBar");
  mainWindow->addToolBar(this->ToolBar);
  this->SequenceBrowserModuleOwnsToolBar = false;
  foreach (QMenu* toolBarMenu,mainWindow->findChildren<QMenu*>())
    {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
      {
      toolBarMenu->addAction(this->ToolBar->toggleViewAction());
      break;
      }
    }

  // Main window takes care of saving and restoring toolbar geometry and state.
  // However, when state is restored the sequence browser toolbar was not created yet.
  // We need to restore the main window state again, now, that the Sequences toolbar is available.
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
  {
    mainWindow->restoreState(settings.value("windowState").toByteArray());
  }
  settings.endGroup();
}


//-----------------------------------------------------------------------------
// qSlicerSequencesModule methods

//-----------------------------------------------------------------------------
qSlicerSequencesModule::qSlicerSequencesModule(QObject* _parent)
: Superclass(_parent)
, d_ptr(new qSlicerSequencesModulePrivate)
{
  Q_D(qSlicerSequencesModule);

  d->UpdateAllVirtualOutputNodesTimer.setSingleShot(true);
  connect(&d->UpdateAllVirtualOutputNodesTimer, SIGNAL(timeout()), this, SLOT(updateAllVirtualOutputNodes()));

  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
  if (scene)
    {
    // Need to listen for any new sequence browser nodes being added to start/stop timer
    this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));
    this->qvtkConnect(scene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));
    }
}



//-----------------------------------------------------------------------------
qSlicerSequencesModule::~qSlicerSequencesModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerSequencesModule::helpText()const
{
  return "This is a module for creating, recording, and replaying node sequences."
    " See more information in the  <a href=\"http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/Sequences\">"
    " online documentation</a>.";
}

//-----------------------------------------------------------------------------
QString qSlicerSequencesModule::acknowledgementText()const
{
  return "This work was funded by CCO ACRU and OCAIRO grants.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequencesModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's), Matthew Holden (PerkLab, Queen's), Kevin Wang (PMH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSequencesModule::icon()const
{
  return QIcon(":/Icons/Sequences.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequencesModule::categories() const
{
  return QStringList() << "Sequences";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequencesModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModule::setup()
{
  Q_D(qSlicerSequencesModule);
  this->Superclass::setup();
  d->addToolBar();
  // Register IOs
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(this->logic());
  ioManager->registerIO(new qSlicerNodeWriter("Sequences", QString("SequenceFile"), QStringList() << "vtkMRMLSequenceNode", true, this));
  ioManager->registerIO(new qSlicerSequencesReader(sequencesLogic, this));
  ioManager->registerIO( new qSlicerNodeWriter( "Sequences", QString( "VolumeSequenceFile" ), QStringList() << "vtkMRMLSequenceNode", true, this ) );
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerSequencesModule
::createWidgetRepresentation()
{
  return new qSlicerSequencesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSequencesModule::createLogic()
{
  return vtkSlicerSequencesLogic::New();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModule::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerSequencesModule);

  vtkMRMLScene* oldScene = this->mrmlScene();
  this->Superclass::setMRMLScene(scene);

  if (scene == nullptr)
    {
    return;
    }

  // Need to listen for any new sequence browser nodes being added to start/stop timer
  this->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));
  this->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  d->ToolBar->setMRMLScene(scene);
}

// --------------------------------------------------------------------------
void qSlicerSequencesModule::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSequencesModule);

  vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
  if (sequenceBrowserNode)
    {
    // If the timer is not active, so it should be turned on
    if (!d->UpdateAllVirtualOutputNodesTimer.isActive())
      {
      d->UpdateAllVirtualOutputNodesTimer.start(UPDATE_VIRTUAL_OUTPUT_NODES_PERIOD_SEC*1000.0);
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerSequencesModule::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSequencesModule);

  vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
  if (sequenceBrowserNode)
    {
    // Check if there is any other sequence browser node left in the Scene
    vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
    if (scene)
      {
      vtkMRMLNode* node;
      node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLSequenceBrowserNode");
      if (!node)
        {
        // The last sequence browser was removed, so
        // turn off timer refresh and stop any pending timers
        d->UpdateAllVirtualOutputNodesTimer.stop();
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModule::updateAllVirtualOutputNodes()
{
  Q_D(qSlicerSequencesModule);

  vtkMRMLAbstractLogic* l = this->logic();
  vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(l);
  if (sequencesLogic)
    {
    qSlicerApplication* application = qSlicerApplication::application();
    if (application)
      {
      application->pauseRender();
      }
    // update proxies then request another singleShot timer
    sequencesLogic->UpdateAllProxyNodes();
    d->UpdateAllVirtualOutputNodesTimer.start(UPDATE_VIRTUAL_OUTPUT_NODES_PERIOD_SEC*1000.0);
    if (application)
      {
      application->resumeRender();
      }
    }
}

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserToolBar* qSlicerSequencesModule::toolBar()
{
  Q_D(qSlicerSequencesModule);
  return d->ToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModule::setToolBarVisible(bool visible)
{
  Q_D(qSlicerSequencesModule);
  d->ToolBar->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerSequencesModule::isToolBarVisible()
{
  Q_D(qSlicerSequencesModule);
  return d->ToolBar->isVisible();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModule::setToolBarActiveBrowserNode(vtkMRMLSequenceBrowserNode* browserNode)
{
  Q_D(qSlicerSequencesModule);
  return d->ToolBar->setActiveBrowserNode(browserNode);
}

//-----------------------------------------------------------------------------
bool qSlicerSequencesModule::autoShowToolBar()
{
  Q_D(qSlicerSequencesModule);
  return d->AutoShowToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModule::setAutoShowToolBar(bool autoShow)
{
  Q_D(qSlicerSequencesModule);
  d->AutoShowToolBar = autoShow;
}

//-----------------------------------------------------------------------------
bool  qSlicerSequencesModule::showSequenceBrowser(vtkMRMLSequenceBrowserNode* browserNode)
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (!app
    || !app->moduleManager()
    || !dynamic_cast<qSlicerSequencesModule*>(app->moduleManager()->module("Sequences")) )
    {
    qCritical("Sequences module is not available");
    return false;
    }
  qSlicerSequencesModule* sequenceBrowserModule = dynamic_cast<qSlicerSequencesModule*>(app->moduleManager()->module("Sequences"));
  if (sequenceBrowserModule->autoShowToolBar())
    {
    sequenceBrowserModule->setToolBarActiveBrowserNode(browserNode);
    sequenceBrowserModule->setToolBarVisible(true);
    }
  return true;
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequencesModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkMRMLSequenceNode"
    << "vtkMRMLSequenceBrowserNode";
}
