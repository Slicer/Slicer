
// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>

// Slices QTModule includes
#include "qSlicerSliceControllerModule.h"
#include "qSlicerSliceControllerModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerSliceControllerModulePrivate: public ctkPrivate<qSlicerSliceControllerModule>
{
public:
};

//-----------------------------------------------------------------------------
qSlicerSliceControllerModule::qSlicerSliceControllerModule(QObject* _parent)
  :Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerSliceControllerModule);
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerSliceControllerModule::createWidgetRepresentation()
{
  return new qSlicerSliceControllerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerSliceControllerModule::createLogic()
{
  return 0;
}
