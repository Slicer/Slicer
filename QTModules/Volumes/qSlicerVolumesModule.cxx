
// QT includes
#include <QtPlugin>

// Volumes Logic includes
#include <vtkSlicerVolumesLogic.h>

// Volumes QTModule includes
#include "qSlicerVolumesModule.h"
#include "qSlicerVolumesModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumesModule, qSlicerVolumesModule);

//-----------------------------------------------------------------------------
class qSlicerVolumesModulePrivate: public qCTKPrivate<qSlicerVolumesModule>
{
public:
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerVolumesModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerVolumesModule::createWidgetRepresentation()
{
  return new qSlicerVolumesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerVolumesModule::createLogic()
{
  return vtkSlicerVolumesLogic::New();
}
