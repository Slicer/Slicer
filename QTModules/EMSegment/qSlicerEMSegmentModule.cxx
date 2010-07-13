
// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>

// EMSegment Logic includes
#include <vtkSlicerEMSegmentLogic.h>

// EMSegment QTModule includes
#include "qSlicerEMSegmentModule.h"
#include "qSlicerEMSegmentModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerEMSegmentModule, qSlicerEMSegmentModule);

//-----------------------------------------------------------------------------
class qSlicerEMSegmentModulePrivate: public ctkPrivate<qSlicerEMSegmentModule>
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentModule::qSlicerEMSegmentModule(QObject* _parent)
  :Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentModule);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEMSegmentModule::createWidgetRepresentation()
{
  return new qSlicerEMSegmentModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerEMSegmentModule::createLogic()
{
  return vtkSlicerEMSegmentLogic::New();
}
