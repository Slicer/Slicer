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
qSlicerCxxInternalConstructor1Macro(qSlicerAbstractCoreModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerAbstractCoreModule);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::initializer()
{
  Q_ASSERT(this->Internal != 0);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo(); 
}
