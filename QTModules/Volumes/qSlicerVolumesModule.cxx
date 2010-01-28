#include "qSlicerVolumesModule.h"
#include "ui_qSlicerVolumesModule.h"

// SlicerQT includes
#include "qSlicerVolumesModuleWidget.h"

// QT includes
#include <QtPlugin>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumesModule, qSlicerVolumesModule);

//-----------------------------------------------------------------------------
class qSlicerVolumesModulePrivate: public qCTKPrivate<qSlicerVolumesModule>,
                                    public Ui_qSlicerVolumesModule
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
  return 0; 
}
