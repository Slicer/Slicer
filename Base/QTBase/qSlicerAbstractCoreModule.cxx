#include "qSlicerAbstractCoreModule.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerAbstractCoreModule::qInternal
{
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerAbstractCoreModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerAbstractCoreModule);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setup()
{
  Q_ASSERT(this->Internal != 0);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
