#include "qSlicerApplication.h"

// SlicerQT includes
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"

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
#include "vtkEventBroker.h"

// VTK includes
#include "vtkSmartPointer.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
struct qSlicerApplicationPrivate: public qCTKPrivate<qSlicerApplication>
{
  QCTK_DECLARE_PUBLIC(qSlicerApplication);
  qSlicerApplicationPrivate()
    {
    this->AppLogic = 0;
    this->MRMLScene = 0;
    
    this->initFont();
    this->initPalette();
    this->loadStyleSheet();
    }

  ~qSlicerApplicationPrivate()
    {
     this->AppLogic->Delete();
     this->MRMLScene->Delete();
    }

  // Description:
  // Initialize application Palette/Font
  void initPalette();
  void initFont();

  // Description:
  // Load application styleSheet
  void loadStyleSheet();

  // Description:
  // MRMLScene and AppLogic pointers
  // Note: Since the logic and the scene should be deleted before the EventBroker,
  // they are not SmartPointer.
  vtkMRMLScene*                        MRMLScene;
  vtkSlicerApplicationLogic*           AppLogic;
  
  vtkSmartPointer<vtkEventBroker>      EventBroker;

  QMap<QWidget*,bool>           TopLevelWidgetsSavedVisibilityState;
  QString                       SlicerHome;
  Qt::WindowFlags               DefaultWindowFlags;
};

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  QCTK_INIT_PRIVATE(qSlicerApplication);
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
  // Uninstantiate modules
  qSlicerModuleManager::instance()->factory()->uninstantiateAll();
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
  // Take ownership of the vtkEventBroker instance.
  // Note: Since EventBroker is a SmartPointer, the object will be deleted when
  // qSlicerApplicationPrivate will be deleted
  qctk_d()->EventBroker.TakeReference(vtkEventBroker::GetInstance());
  
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
QCTK_SET_CXX(qSlicerApplication, Qt::WindowFlags, setDefaultWindowFlags, DefaultWindowFlags);
QCTK_GET_CXX(qSlicerApplication, Qt::WindowFlags, defaultWindowFlags, DefaultWindowFlags);

//-----------------------------------------------------------------------------
void qSlicerApplication::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  QCTK_D(qSlicerApplication);
  if (d->MRMLScene == mrmlScene)
    {
    return;
    }
  d->MRMLScene = mrmlScene;
  d->MRMLScene->Register(NULL);
  emit this->currentMRMLSceneChanged(mrmlScene);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerApplication, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerApplication::setAppLogic(vtkSlicerApplicationLogic* appLogic)
{
  QCTK_D(qSlicerApplication);
  d->AppLogic = appLogic;
  d->AppLogic->Register(NULL);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerApplication, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QString qSlicerApplication::slicerHome()
{
  // TODO Use QCoreApplication::applicationDirPath
  return qctk_d()->SlicerHome;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerApplication, const QString&, setSlicerHome, SlicerHome);

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
  QCTK_D(qSlicerApplication);
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    // Store current visibility state
    if (!visible)
      {
      if (!d->TopLevelWidgetsSavedVisibilityState.contains(widget))
        {
        d->TopLevelWidgetsSavedVisibilityState[widget] = widget->isVisible();
        }
      widget->hide();
      }
    else
      {
      QMap<QWidget*,bool>::const_iterator it = d->TopLevelWidgetsSavedVisibilityState.find(widget);

      // If widget state was saved, restore it. Otherwise skip.
      if (it != d->TopLevelWidgetsSavedVisibilityState.end())
        {
        widget->setVisible(it.value());
        }
      }
    }

  // Each time widget are set visible. Internal Map can be cleared.
  if (visible)
    {
    d->TopLevelWidgetsSavedVisibilityState.clear();
    }
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetVisible(qSlicerWidget* widget, bool visible)
{
  if (!widget) { return; }
  Q_ASSERT(!widget->parent());
  QCTK_D(qSlicerApplication);
  // When internal Map is empty, it means top widget are visible
  if (d->TopLevelWidgetsSavedVisibilityState.empty())
    {
    widget->setVisible(visible);
    }
  else
    {
    d->TopLevelWidgetsSavedVisibilityState[widget] = visible;
    }
}

//-----------------------------------------------------------------------------
// qSlicerApplicationPrivate methods

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initPalette()
{
  QCTK_P(qSlicerApplication);
  QPalette palette = p->palette();
  /* Old palette that makes a high contrast in Windows.
  p.setColor(QPalette::Window, Qt::white);
  p.setColor(QPalette::Base, Qt::white);
  p.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  p.setColor(QPalette::Button, Qt::white);
  */
  palette.setColor(QPalette::Button, "#fcfcfc");
  palette.setColor(QPalette::Light, "#c8c8c8");
  palette.setColor(QPalette::Midlight, "#e6e6e6");
  palette.setColor(QPalette::Dark, "#aaaaaa");
  palette.setColor(QPalette::Mid, "#c8c8c8");
  palette.setColor(QPalette::Base, Qt::white);
  palette.setColor(QPalette::Window, Qt::white);
  palette.setColor(QPalette::Shadow, "#5a5a5a");
  palette.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));

  p->setPalette(palette);
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initFont()
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
void qSlicerApplicationPrivate::loadStyleSheet()
{
//   QString styleSheet =
//     "background-color: white;"
//     "alternate-background-color: #e4e4fe;";
//
//   this->BackPointer->setStyleSheet(styleSheet);
}
