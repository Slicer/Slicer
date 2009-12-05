#include "qSlicerMeasurementsModule.h"

// SlicerQT includes
#include "qSlicerMeasurementsModuleWidget.h"

// QT includes
#include <QtPlugin>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
struct qSlicerMeasurementsModulePrivate: public qCTKPrivate<qSlicerMeasurementsModule>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMeasurementsModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerMeasurementsModule::createWidgetRepresentation()
{
  return new qSlicerMeasurementsModuleWidget;
}
