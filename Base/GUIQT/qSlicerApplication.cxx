#include "qSlicerApplication.h" 

#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"

#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontDatabase>
#include <QWidget>
#include <QMap>
#include <QDebug>

#include "vtkMRMLScene.h"

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
class qSlicerApplication::qInternal
{
public:
  qInternal()
    {
    this->MRMLScene = 0;
    }
  vtkMRMLScene * MRMLScene; 
  QMap<QWidget*,bool> TopLevelWidgetsSavedVisibilityState; 
  QString             SlicerHome;
};

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  this->Internal = new qInternal;
  this->initFont(); 
  this->initPalette(); 
  this->loadStyleSheet(); 
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app =
    qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

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
void qSlicerApplication::initPalette()
{
  QPalette p = this->palette();
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

  this->setPalette(p);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::initFont()
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
  this->setFont(f);
  */
}

//-----------------------------------------------------------------------------
void qSlicerApplication::loadStyleSheet()
{
//   QString styleSheet = 
//     "background-color: white;"
//     "alternate-background-color: #e4e4fe;";
//   
//   this->setStyleSheet(styleSheet);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetsVisible(bool visible)
{
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    // Store current visibility state
    if (!visible)
      {
      this->Internal->TopLevelWidgetsSavedVisibilityState[widget] = widget->isVisible(); 
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
void qSlicerApplication::setMRMLScene(vtkMRMLScene * scene)
{
  this->Internal->MRMLScene = scene; 
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerApplication::getMRMLScene()
{
  return this->Internal->MRMLScene; 
}
