#include "qSlicerAbstractModule.h" 

#include "vtkMRMLScene.h"

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerAbstractModule::qInternal
{
public:
  qInternal()
    {
    this->ModuleEnabled = false; 
    }
  ~qInternal()
    {
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
