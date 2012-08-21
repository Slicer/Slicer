
// Qt includes
#include <QtPlugin>

// QTGUI includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include <qSlicerNodeWriter.h>

// SceneViewsModule includes
#include "qSlicerSceneViewsModule.h"

#include <qSlicerSceneViewsModuleWidget.h>
#include <vtkSlicerSceneViewsModuleLogic.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSceneViewsModule, qSlicerSceneViewsModule);

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
qSlicerSceneViewsModule::~qSlicerSceneViewsModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModule::setup()
{
  qSlicerCoreIOManager* ioManager =
    qSlicerApplication::application()->coreIOManager();
  ioManager->registerIO(new qSlicerNodeWriter(
    "SceneViews", QString("SceneViewFile"),
    QStringList() << "vtkMRMLSceneViewNode", this));
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
  return "The SceneViews module. Still under heavy development. For feedback please contact the Slicer mailing list (slicer-users@bwh.harvard.edu).";
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
