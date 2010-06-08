
// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>

// ModuleTemplate Logic includes
#include <vtkSlicerModuleTemplateLogic.h>

// ModuleTemplate QTModule includes
#include "qSlicerModuleTemplateModule.h"
#include "qSlicerModuleTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerModuleTemplateModule, qSlicerModuleTemplateModule);

//-----------------------------------------------------------------------------
class qSlicerModuleTemplateModulePrivate: public ctkPrivate<qSlicerModuleTemplateModule>
{
public:
};

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModule::qSlicerModuleTemplateModule(QObject* _parent)
  :Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerModuleTemplateModule);
}

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerModuleTemplateModule::createWidgetRepresentation()
{
  return new qSlicerModuleTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerModuleTemplateModule::createLogic()
{
  return vtkSlicerModuleTemplateLogic::New();
}
