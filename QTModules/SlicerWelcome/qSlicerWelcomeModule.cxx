#include "qSlicerWelcomeModule.h"

// SlicerQT includes
#include "qSlicerWelcomeModuleWidget.h"

// QT includes
#include <QtPlugin>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWelcomeModule, qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
struct qSlicerWelcomeModulePrivate: public qCTKPrivate<qSlicerWelcomeModule>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerWelcomeModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerWelcomeModule::createWidgetRepresentation()
{
  return new qSlicerWelcomeModuleWidget;
}
