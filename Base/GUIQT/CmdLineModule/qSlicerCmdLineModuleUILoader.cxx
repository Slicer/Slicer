#include "qSlicerCmdLineModuleUILoader.h" 

#include "qSlicerMacros.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerCmdLineModuleUILoader::qInternal
{
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructorMacro(qSlicerCmdLineModuleUILoader);
qSlicerCxxDestructorMacro(qSlicerCmdLineModuleUILoader);

//-----------------------------------------------------------------------------
void qSlicerCmdLineModuleUILoader::printAdditionalInfo()
{
  //this->Superclass::dumpObjectInfo(); 
}
