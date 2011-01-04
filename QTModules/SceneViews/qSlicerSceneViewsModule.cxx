
// Qt includes
#include <QtPlugin>

// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// AnnotationModule includes
#include "qSlicerSceneViewsModule.h"
#include "GUI/qSlicerSceneViewsModuleWidget.h"

#include <vtkSlicerSceneViewLogic.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSceneViewsModule, qSlicerSceneViewsModule);

//-----------------------------------------------------------------------------
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

}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSceneViewsModule::createWidgetRepresentation()
{
  return new qSlicerSceneViewsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSceneViewsModule::createLogic()
{

  return vtkSlicerSceneViewLogic::New();
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
QIcon qSlicerSceneViewsModule::icon() const
{
  return QIcon(":/Icons/SelectCameras.png");
}
