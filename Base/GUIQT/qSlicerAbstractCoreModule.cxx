#include "qSlicerAbstractCoreModule.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerAbstractCoreModule::qInternal
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule::qSlicerAbstractCoreModule(QWidget *parent)
 :Superclass(parent)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule::~qSlicerAbstractCoreModule()
{
  if (this->initialized()) { delete this->Internal; }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::initializer()
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo(); 
}
