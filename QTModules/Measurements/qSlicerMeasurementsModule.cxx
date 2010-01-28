#include "qSlicerMeasurementsModule.h"

// SlicerQT includes
#include "qSlicerMeasurementsModuleWidget.h"

// QT includes
#include <QtPlugin>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
class qSlicerMeasurementsModulePrivate: public qCTKPrivate<qSlicerMeasurementsModule>
{
public:
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMeasurementsModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerMeasurementsModule::createWidgetRepresentation()
{
  return new qSlicerMeasurementsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerMeasurementsModule::createLogic()
{
  //return new qSlicerMeasurementsModuleLogic;
  return 0; 
}
