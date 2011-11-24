/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAction>
#include <QDebug>
#include <QMainWindow>

// CTK includes
#include <ctkColorDialog.h>
#include <ctkConfirmExitDialog.h>
#include <ctkErrorLogModel.h>
#include <ctkSettings.h>
#include <ctkToolTipTrapper.h>

// QTGUI includes
#include "qSlicerAbstractModule.h"
#include "qSlicerApplication.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreApplication_p.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerPythonManager.h"
#endif
#include "qSlicerSettingsGeneralPanel.h"
#include "qSlicerSettingsModulesPanel.h"

// qMRMLWidget includes
#include "qMRMLColorPickerWidget.h"
#include "qMRMLEventBrokerConnection.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLNode.h>

//-----------------------------------------------------------------------------
class qSlicerApplicationPrivate : public qSlicerCoreApplicationPrivate
{
  Q_DECLARE_PUBLIC(qSlicerApplication);
protected:
  qSlicerApplication* const q_ptr;
public:
  typedef qSlicerCoreApplicationPrivate Superclass;

  qSlicerApplicationPrivate(qSlicerApplication& object, qSlicerCommandOptions * commandOptions, qSlicerIOManager * ioManager);
  virtual ~qSlicerApplicationPrivate();

  /// Convenient method regrouping all initialization code
  virtual void init();

  /// Initialize application style
  void initStyle();

  qSlicerLayoutManager*   LayoutManager;
  ctkToolTipTrapper*      ToolTipTrapper;
  QSharedPointer<qMRMLColorPickerWidget> ColorDialogPickerWidget;
  ctkSettingsDialog*      SettingsDialog;
};


//-----------------------------------------------------------------------------
// qSlicerApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::qSlicerApplicationPrivate(
  qSlicerApplication& object, qSlicerCommandOptions * commandOptions, qSlicerIOManager * ioManager)
  : qSlicerCoreApplicationPrivate(object, commandOptions, ioManager), q_ptr(&object)
{
  this->LayoutManager = 0;
  this->ToolTipTrapper = 0;
  this->SettingsDialog = 0;
}

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::~qSlicerApplicationPrivate()
{
  delete this->SettingsDialog;
  this->SettingsDialog = 0;
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::init()
{
  Q_Q(qSlicerApplication);

  ctkVTKConnectionFactory::setInstance(new qMRMLConnectionFactory);

  this->ColorDialogPickerWidget =
    QSharedPointer<qMRMLColorPickerWidget>(new qMRMLColorPickerWidget(0));
  ctkColorDialog::addDefaultTab(this->ColorDialogPickerWidget.data(),
                                "Labels", SIGNAL(colorSelected(QColor)));
  ctkColorDialog::setDefaultTab(1);

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // Note: qSlicerCoreApplication class takes ownership of the pythonManager and
    // will be responsible to delete it
    q->setCorePythonManager(new qSlicerPythonManager());
    }
#endif

  this->Superclass::init();

  this->initStyle();

  this->ToolTipTrapper = new ctkToolTipTrapper(q);
  this->ToolTipTrapper->setEnabled(false);

  //----------------------------------------------------------------------------
  // Settings Dialog
  //----------------------------------------------------------------------------
  this->SettingsDialog = new ctkSettingsDialog(0);
  this->SettingsDialog->addPanel("General settings", new qSlicerSettingsGeneralPanel);
  qSlicerSettingsModulesPanel * settingsModulesPanel = new qSlicerSettingsModulesPanel;
  this->SettingsDialog->addPanel("Modules settings", settingsModulesPanel);

  settingsModulesPanel->setRestartRequested(false);
  QObject::connect(this->SettingsDialog, SIGNAL(accepted()),
                   q, SLOT(onSettingDialogAccepted()));
}
/*
#if !defined (QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loaderV2,
    (QIconEngineFactoryInterfaceV2_iid, QLatin1String("/iconengines"), Qt::CaseInsensitive))
#endif
*/
//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initStyle()
{
  Q_Q(qSlicerApplication);

  // Force showing the icons in the menus despite the native OS style
  // discourages it
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

  // Init the style of the icons
  // The plugin qSlicerIconEnginePlugin is located in <Slicer_LIB_DIR>/iconengines
  // By adding the path to the lib dir, Qt will automatically load the icon engine plugin.
  q->addLibraryPath(q->slicerHome() + "/" + Slicer_LIB_DIR);
}

//-----------------------------------------------------------------------------
// qSlicerApplication methods

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &_argc, char **_argv)
  : Superclass(new qSlicerApplicationPrivate(*this, new qSlicerCommandOptions, 0), _argc, _argv)
{
  Q_D(qSlicerApplication);
  d->init();
  // Note: Since QWidget/QDialog requires a QApplication to be successfully instantiated,
  //       qSlicerIOManager is not added to the constructor initialization list.
  //       Indeed, internally qSlicerIOManager registers qSlicerDataDialog, ...
  d->CoreIOManager = QSharedPointer<qSlicerIOManager>(new qSlicerIOManager);
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app = qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
qSlicerCommandOptions* qSlicerApplication::commandOptions()
{
  qSlicerCommandOptions* _commandOptions =
    dynamic_cast<qSlicerCommandOptions*>(this->coreCommandOptions());
  Q_ASSERT(_commandOptions);
  return _commandOptions;
}

//-----------------------------------------------------------------------------
qSlicerIOManager* qSlicerApplication::ioManager()
{
  qSlicerIOManager* _ioManager = dynamic_cast<qSlicerIOManager*>(this->coreIOManager());
  Q_ASSERT(_ioManager);
  return _ioManager;
}

#ifdef Slicer_USE_PYTHONQT
//-----------------------------------------------------------------------------
qSlicerPythonManager* qSlicerApplication::pythonManager()
{
  qSlicerPythonManager* _pythonManager = 0;
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    _pythonManager = qobject_cast<qSlicerPythonManager*>(this->corePythonManager());
    Q_ASSERT(_pythonManager);
    }

  return _pythonManager;
}
#endif

//-----------------------------------------------------------------------------
void qSlicerApplication::setLayoutManager(qSlicerLayoutManager* layoutManager)
{
  Q_D(qSlicerApplication);
  d->LayoutManager = layoutManager;
  if (this->applicationLogic())
    {
    this->applicationLogic()->SetSliceLogics(
      layoutManager? layoutManager->mrmlSliceLogics() : 0);
    }
}

//-----------------------------------------------------------------------------
qSlicerLayoutManager* qSlicerApplication::layoutManager()const
{
  Q_D(const qSlicerApplication);
  return d->LayoutManager;
}

//-----------------------------------------------------------------------------
QMainWindow* qSlicerApplication::mainWindow()const
{
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    QMainWindow* window = qobject_cast<QMainWindow*>(widget);
    if (window)
      {
      return window;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::handleCommandLineArguments()
{
  qSlicerCommandOptions* options = this->commandOptions();
  Q_ASSERT(options);

  if (options->noMainWindow() || options->disableMessageHandlers())
    {
    // If no UI is expected, it doesn't make sens to use registered handlers.
    // Let's disable them.
    this->errorLogModel()->disableAllMsgHandler();
    }

  this->Superclass::handleCommandLineArguments();

  this->setToolTipsEnabled(!options->disableToolTips());
}

//-----------------------------------------------------------------------------
QSettings* qSlicerApplication::newSettings(const QString& fileName)
{
  if (!fileName.isEmpty())
    {
    // Special case for tmp settings
    return new ctkSettings(fileName, QSettings::defaultFormat(), this);
    }
  return new ctkSettings(this);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setToolTipsEnabled(bool enable)
{
  Q_D(qSlicerApplication);
  d->ToolTipTrapper->setEnabled(!enable);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::confirmRestart(const QString& reason)
{
  QString reasonText(reason);
  if (reasonText.isEmpty())
    {
    reasonText = tr("Are you sure you want to restart?");
    }

  bool restart = true;
  QSettings settings;
  bool confirm = settings.value("MainWindow/ConfirmRestart", true).toBool();
  if (confirm)
    {
    ctkConfirmExitDialog dialog;
    dialog.setText(reasonText);
    restart = (dialog.exec() == QDialog::Accepted);
    settings.setValue("MainWindow/ConfirmRestart", !dialog.dontShowAnymore());
    }
  if (restart)
    {
    this->restart();
    }
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setMRMLScene(vtkMRMLScene* newMRMLScene)
{
  Q_D(qSlicerApplication);
  // we do it now because Superclass::setMRMLScene() emits the signal
  // mrmlSceneChanged and we don't want the set the scene after the signal
  // is sent.
  d->ColorDialogPickerWidget->setMRMLScene(newMRMLScene);
  this->Superclass::setMRMLScene(newMRMLScene);
}

//-----------------------------------------------------------------------------
QString qSlicerApplication::nodeModule(vtkMRMLNode* node)const
{
  QString nodeClassName = node->GetClassName();
  if (node->IsA("vtkMRMLCameraNode") ||
      node->IsA("vtkMRMLViewNode"))
    {
    return "Cameras";
    }
  else if (node->IsA("vtkMRMLSliceNode") ||
           node->IsA("vtkMRMLSliceCompositeNode") ||
           node->IsA("vtkMRMLSliceLayerNode"))
    {
    return "SliceController";
    }
  else if (node->IsA("vtkMRMLAnnotationNode") ||
           node->IsA("vtkMRMLAnnotationDisplayNode") ||
           node->IsA("vtkMRMLAnnotationStorageNode") ||
           node->IsA("vtkMRMLAnnotationHierarchyNode"))
    {
    return "Annotations";
    }
  else if (node->IsA("vtkMRMLTransformNode") ||
           node->IsA("vtkMRMLTransformStorageNode"))
    {
    return "Transforms";
    }
  else if (node->IsA("vtkMRMLColorNode"))
    {
    return "Colors";
    }
  else if (nodeClassName.contains("vtkMRMLFiberBundle"))
    {
    return "TractographyDisplay";
    }
  else if (node->IsA("vtkMRMLModelNode") ||
           node->IsA("vtkMRMLModelDisplayNode") ||
           node->IsA("vtkMRMLModelHierarchyNode") ||
           node->IsA("vtkMRMLModelStorageNode"))
    {
    return "Models";
    }
  else if (node->IsA("vtkMRMLSceneViewNode") ||
           node->IsA("vtkMRMLSceneViewStorageNode"))
    {
    return "SceneViews";
    }
  else if (node->IsA("vtkMRMLVolumeNode") ||
           node->IsA("vtkMRMLVolumeDisplayNode") ||
           node->IsA("vtkMRMLVolumeArchetypeStorageNode") ||
           node->IsA("vtkMRMLVolumeHeaderlessStorageNode"))
    {
    return "Volumes";
    }
  else if (node->IsA("vtkMRMLVolumePropertyNode") ||
           node->IsA("vtkMRMLVolumePropertyStorageNode") ||
           node->IsA("vtkMRMLVolumeRenderingDisplayNode"))
    {
    return "VolumeRendering";
    }
  qWarning() << "Couldn't find a module for node class" << node->GetClassName();
  return "data";
}

//-----------------------------------------------------------------------------
void qSlicerApplication::openNodeModule(vtkMRMLNode* node)
{
  QString moduleName = this->nodeModule(node);
  qSlicerAbstractCoreModule* module = this->moduleManager()->module(moduleName);
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (moduleWithAction)
    {
    moduleWithAction->action()->trigger();
    }
}

// --------------------------------------------------------------------------
ctkSettingsDialog* qSlicerApplication::settingsDialog()const
{
  Q_D(const qSlicerApplication);
  return d->SettingsDialog;
}

// --------------------------------------------------------------------------
void qSlicerApplication::onSettingDialogAccepted()
{
  Q_D(qSlicerApplication);
  qSlicerSettingsModulesPanel *settingsModulesPanel =
    qobject_cast<qSlicerSettingsModulesPanel*>(
      d->SettingsDialog->panel("Modules settings"));
  Q_ASSERT(settingsModulesPanel);
  if (settingsModulesPanel->restartRequested())
    {
    QString reason = tr("Since the module paths have been updated, the application should be restarted.\n\n"
                        "Do you want to restart now?\n");
    this->confirmRestart(reason);
    }
}
