#include "qSlicerAbstractModule.h"

//-----------------------------------------------------------------------------
struct qSlicerAbstractModule::qInternal
{
  qInternal()
    {
    this->ModuleEnabled = false;
    }
  bool                    ModuleEnabled;
};

//-----------------------------------------------------------------------------
qSlicerAbstractModule::qSlicerAbstractModule(QWidget *parent)
 :Superclass(parent)
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule::~qSlicerAbstractModule()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize()
{
  this->initializer();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::moduleName()
{
  return this->metaObject()->className();
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractModule::moduleEnabled()
{
  return this->Internal->ModuleEnabled;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setModuleEnabled(bool value)
{
  this->Internal->ModuleEnabled = value;
}
