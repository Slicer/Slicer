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
qSlicerWidgetCxxZeroInitConstructorMacro(qSlicerAbstractCoreModule);
qSlicerCxxDestructorMacro(qSlicerAbstractCoreModule);

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
