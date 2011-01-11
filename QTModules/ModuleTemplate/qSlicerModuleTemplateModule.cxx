
// Qt includes
#include <QtPlugin>

// ModuleTemplate Logic includes
#include <vtkSlicerModuleTemplateLogic.h>

// ModuleTemplate includes
#include "qSlicerModuleTemplateModule.h"
#include "qSlicerModuleTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerModuleTemplateModule, qSlicerModuleTemplateModule);

//-----------------------------------------------------------------------------
class qSlicerModuleTemplateModulePrivate
{
public:
  qSlicerModuleTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModulePrivate::qSlicerModuleTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModule::qSlicerModuleTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModuleTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModule::~qSlicerModuleTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerModuleTemplateModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerModuleTemplateModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerModuleTemplateModule::icon()const
{
  return QIcon(":/Icons/ModuleTemplate.png");
}

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerModuleTemplateModule::createWidgetRepresentation()
{
  return new qSlicerModuleTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerModuleTemplateModule::createLogic()
{
  return vtkSlicerModuleTemplateLogic::New();
}
