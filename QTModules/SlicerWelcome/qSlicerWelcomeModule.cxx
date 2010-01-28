#include "qSlicerWelcomeModule.h"

// SlicerQT includes
#include "qSlicerWelcomeModuleWidget.h"

// QT includes
#include <QtPlugin>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWelcomeModule, qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
class qSlicerWelcomeModulePrivate: public qCTKPrivate<qSlicerWelcomeModule>
{
public:
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerWelcomeModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerWelcomeModule::createWidgetRepresentation()
{
  return new qSlicerWelcomeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerWelcomeModule::createLogic()
{
  return 0;
}
