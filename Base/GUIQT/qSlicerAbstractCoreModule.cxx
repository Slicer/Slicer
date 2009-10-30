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
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule::~qSlicerAbstractCoreModule()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::dumpObjectInfo()
{
  this->Superclass::dumpObjectInfo(); 
}
