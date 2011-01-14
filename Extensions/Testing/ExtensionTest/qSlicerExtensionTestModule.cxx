
// Qt includes
#include <QtPlugin>

// ExtensionTest Logic includes
#include <vtkSlicerExtensionTestLogic.h>

// ExtensionTest includes
#include "qSlicerExtensionTestModule.h"
#include "qSlicerExtensionTestModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerExtensionTestModule, qSlicerExtensionTestModule);

//-----------------------------------------------------------------------------
class qSlicerExtensionTestModulePrivate
{
public:
  qSlicerExtensionTestModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerExtensionTestModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerExtensionTestModulePrivate::qSlicerExtensionTestModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerExtensionTestModule methods

//-----------------------------------------------------------------------------
qSlicerExtensionTestModule::qSlicerExtensionTestModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionTestModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerExtensionTestModule::~qSlicerExtensionTestModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionTestModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionTestModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerExtensionTestModule::icon()const
{
  return QIcon(":/Icons/ExtensionTest.png");
}

//-----------------------------------------------------------------------------
void qSlicerExtensionTestModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerExtensionTestModule::createWidgetRepresentation()
{
  return new qSlicerExtensionTestModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerExtensionTestModule::createLogic()
{
  return vtkSlicerExtensionTestLogic::New();
}
