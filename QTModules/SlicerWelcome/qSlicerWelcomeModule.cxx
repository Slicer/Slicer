#include "qSlicerWelcomeModule.h"

// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include "qSlicerWelcomeModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWelcomeModule, qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
class qSlicerWelcomeModulePrivate: public ctkPrivate<qSlicerWelcomeModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerWelcomeModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerWelcomeModule::createWidgetRepresentation()
{
  return new qSlicerWelcomeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerWelcomeModule::createLogic()
{
  return 0;
}
