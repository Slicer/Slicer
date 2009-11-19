#include "qSlicerAbstractModule.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include "vtkMRMLScene.h"

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
qSlicerCxxInternalConstructor1Macro(qSlicerAbstractModule, QWidget*);

//-----------------------------------------------------------------------------
qSlicerAbstractModule::~qSlicerAbstractModule()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize(vtkSlicerApplicationLogic* appLogic)
{
  Q_ASSERT(appLogic);
  this->setAppLogic(appLogic);
  this->setup();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::name()const
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
