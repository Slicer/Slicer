#include "qSlicerModuleLogic.h" 
#include "qSlicerMacros.h"

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerModuleLogic::qInternal
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerModuleLogic, QObject*);
qSlicerCxxDestructorMacro(qSlicerModuleLogic);

//-----------------------------------------------------------------------------
void qSlicerModuleLogic::printAdditionalInfo()
{
  //this->Superclass::printAdditionalInfo(); 
}
