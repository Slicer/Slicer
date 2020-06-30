
// QTGUI includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include <qSlicerNodeWriter.h>

// SceneViewsModule includes
#include "qSlicerSceneViewsModule.h"

#include <qSlicerSceneViewsModuleWidget.h>
#include <vtkSlicerSceneViewsModuleLogic.h>

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchySceneViewsPlugin.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SceneViews
class qSlicerSceneViewsModulePrivate
{
  public:
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
  qSlicerCoreIOManager* ioManager =
    qSlicerApplication::application()->coreIOManager();
  ioManager->registerIO(new qSlicerNodeWriter(
    "SceneViews", QString("SceneViewFile"),
    QStringList() << "vtkMRMLSceneViewNode", true, this));

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchySceneViewsPlugin());
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
  QString help =
    "The SceneViews module. Create, edit, restore, delete scene views. Scene "
    "views capture the state of the MRML scene at a given point. The "
    "recommended way to use them is to load all of your data and then adjust "
    "visibility of the elements and capture interesting scene views. "
    "Unexpected behavior may occur if you add or delete data from the scene "
    "while saving and restoring scene views.\n"
    "For more information see the <a href=\"%1/Documentation/%2.%3/Modules/SceneViews\">"
    "online documentation</a>.\n";

  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
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
void qSlicerSceneViewsModule::showSceneViewDialog()
{
  Q_ASSERT(this->widgetRepresentation());
  dynamic_cast<qSlicerSceneViewsModuleWidget*>(this->widgetRepresentation())
    ->showSceneViewDialog();
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneViewsModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLSceneViewNode";
}
