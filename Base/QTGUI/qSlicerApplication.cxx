#include "qSlicerApplication.h"

// SlicerQT includes
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerModulePanel.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// QT includes
#include <QRect>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontDatabase>
#include <QWidget>
#include <QMap>
#include <QDebug>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"

// VTK includes
#include "vtkSmartPointer.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
struct qSlicerApplication::qInternal
{
  qInternal(qSlicerApplication* backPointer):
    BackPointer(backPointer)
    {
    this->initFont();
    this->initPalette();
    this->loadStyleSheet();
    }

  // Description:
  // Initialize application Palette/Font
  void initPalette();
  void initFont();

  // Description:
  // Load application styleSheet
  void loadStyleSheet();

  vtkSmartPointer<vtkMRMLScene>               MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic>  AppLogic;

  qSlicerApplication*           BackPointer;
  QMap<QWidget*,bool>           TopLevelWidgetsSavedVisibilityState;
  QString                       SlicerHome;
  Qt::WindowFlags               DefaultWindowFlags;
};

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  this->Internal = new qInternal(this);
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
  // Make sure all modules are unparented
  qSlicerModuleManager::instance()->modulePanel()->removeAllModule();

  // Uninstantiate modules
  qSlicerModuleManager::instance()->factory()->uninstantiateAll();

  delete this->Internal;
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app = qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::initialize()
{
  // Create MRML scene
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtksys_stl::string root = vtksys::SystemTools::GetCurrentWorkingDirectory();
  scene->SetRootDirectory(root.c_str());
  vtkMRMLScene::SetActiveScene( scene );

  // Create the application Logic object,
  vtkSmartPointer<vtkSlicerApplicationLogic> appLogic =
    vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  appLogic->SetMRMLScene(scene);

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  appLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  appLogic->SetAndObserveMRMLScene(scene);
  appLogic->CreateProcessingThread();

  // --- First scene needs a crosshair to be added manually
  vtkSmartPointer<vtkMRMLCrosshairNode> crosshair = vtkSmartPointer<vtkMRMLCrosshairNode>::New();
  crosshair->SetCrosshairName("default");
  scene->AddNode( crosshair );

  this->setMRMLScene(scene);
  this->setAppLogic(appLogic);
}

//-----------------------------------------------------------------------------
qSlicerSetInternalCxxMacro(qSlicerApplication, Qt::WindowFlags, setDefaultWindowFlags, DefaultWindowFlags);
qSlicerGetInternalCxxMacro(qSlicerApplication, Qt::WindowFlags, defaultWindowFlags, DefaultWindowFlags);

//-----------------------------------------------------------------------------
void qSlicerApplication::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  if (this->Internal->MRMLScene == mrmlScene)
    {
    return;
    }
  this->Internal->MRMLScene = mrmlScene;
  emit this->currentMRMLSceneChanged(mrmlScene);
}

//-----------------------------------------------------------------------------
qSlicerGetInternalCxxMacro(qSlicerApplication, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
qSlicerSetInternalCxxMacro(qSlicerApplication, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);
qSlicerGetInternalCxxMacro(qSlicerApplication, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QString qSlicerApplication::slicerHome()
{
  // TODO Use QCoreApplication::applicationDirPath
  return this->Internal->SlicerHome;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setSlicerHome(const QString& slicerHome)
{
  this->Internal->SlicerHome = slicerHome;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::initializeLoadableModulesPaths()
{
  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  QString defaultQTModulePaths = this->slicerHome() + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR;

//   if (hasIntDir)
//     {
//     defaultQTModulePaths = defaultQTModulePaths + PathSep +
//       slicerHome + "/" + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR + "/" + intDir;
//     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QString qtModulePaths = /*userModulePaths + PathSep + */defaultQTModulePaths;
  this->addLibraryPath(defaultQTModulePaths);

//   foreach (QString path, this->libraryPaths())
//     {
//     qDebug() << "libraryPath:" << path;
//     }

  QStringList paths;
  paths << qtModulePaths;
  qSlicerModuleManager::instance()->factory()->setLoadableModuleSearchPaths(paths);
  //qDebug() << "initializeLoadableModulesPaths - qtModulePaths:" << qtModulePaths;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::initializeCmdLineModulesPaths()
{
  QString defaultCmdLineModulePaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultCmdLineModulePaths = this->slicerHome() + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR;
//   if (hasIntDir)
//     {
//     defaultQTModulePaths = defaultCmdLineModulePaths + PathSep +
//       slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "/" + intDir;
//     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QString cmdLineModulePaths = /*userModulePaths + PathSep +*/ defaultCmdLineModulePaths;
  this->addLibraryPath(defaultCmdLineModulePaths);

  QStringList paths;
  paths << cmdLineModulePaths;
  qSlicerModuleManager::instance()->factory()->setCmdLineModuleSearchPaths(paths);
  //cout << "cmdLineModulePaths:" << cmdLineModulePaths << endl;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetsVisible(bool visible)
{
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    // Store current visibility state
    if (!visible)
      {
      if (!this->Internal->TopLevelWidgetsSavedVisibilityState.contains(widget))
        {
        this->Internal->TopLevelWidgetsSavedVisibilityState[widget] = widget->isVisible();
        }
      widget->hide();
      }
    else
      {
      QMap<QWidget*,bool>::const_iterator it =
        this->Internal->TopLevelWidgetsSavedVisibilityState.find(widget);

      // If widget state was saved, restore it. Otherwise skip.
      if (it != this->Internal->TopLevelWidgetsSavedVisibilityState.end())
        {
        widget->setVisible(it.value());
        }
      }
    }

  // Each time widget are set visible. Internal Map can be cleared.
  if (visible)
    {
    this->Internal->TopLevelWidgetsSavedVisibilityState.clear();
    }
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetVisible(qSlicerWidget* widget, bool visible)
{
  if (!widget) { return; }
  Q_ASSERT(!widget->parent());
  // When internal Map is empty, it means top widget are visible
  if (this->Internal->TopLevelWidgetsSavedVisibilityState.empty())
    {
    widget->setVisible(visible);
    }
  else
    {
    this->Internal->TopLevelWidgetsSavedVisibilityState[widget] = visible;
    }
}

//-----------------------------------------------------------------------------
// Internal methods

//-----------------------------------------------------------------------------
void qSlicerApplication::qInternal::initPalette()
{
  QPalette p = this->BackPointer->palette();
  /* Old palette that makes a high contrast in Windows.
  p.setColor(QPalette::Window, Qt::white);
  p.setColor(QPalette::Base, Qt::white);
  p.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  p.setColor(QPalette::Button, Qt::white);
  */
  p.setColor(QPalette::Button, "#fcfcfc");
  p.setColor(QPalette::Light, "#c8c8c8");
  p.setColor(QPalette::Midlight, "#e6e6e6");
  p.setColor(QPalette::Dark, "#aaaaaa");
  p.setColor(QPalette::Mid, "#c8c8c8");
  p.setColor(QPalette::Base, Qt::white);
  p.setColor(QPalette::Window, Qt::white);
  p.setColor(QPalette::Shadow, "#5a5a5a");
  p.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));

  this->BackPointer->setPalette(p);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::qInternal::initFont()
{
  /*
  QFont f("Verdana", 9);
  QFontInfo ff(f);
  QFontDatabase database;
  foreach (QString family, database.families())
    {
    cout << family.toStdString() << endl;
    }

  cout << "Family: " << ff.family().toStdString() << endl;
  cout << "Size: " << ff.pointSize() << endl;
  this->BackPointer->setFont(f);
  */
}

//-----------------------------------------------------------------------------
void qSlicerApplication::qInternal::loadStyleSheet()
{
//   QString styleSheet =
//     "background-color: white;"
//     "alternate-background-color: #e4e4fe;";
//
//   this->BackPointer->setStyleSheet(styleSheet);
}
