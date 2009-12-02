#include "qSlicerAbstractCoreModule.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerAbstractCoreModulePrivate: public qCTKPrivate<qSlicerAbstractCoreModule>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractCoreModule, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setup()
{
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
