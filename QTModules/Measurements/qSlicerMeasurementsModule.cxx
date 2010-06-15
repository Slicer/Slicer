// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include "qSlicerMeasurementsModule.h"
#include "qSlicerMeasurementsModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
class qSlicerMeasurementsModulePrivate: public ctkPrivate<qSlicerMeasurementsModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMeasurementsModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerMeasurementsModule::createWidgetRepresentation()
{
  return new qSlicerMeasurementsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerMeasurementsModule::createLogic()
{
  //return new qSlicerMeasurementsModuleLogic;
  return 0; 
}
