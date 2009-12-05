#include "qSlicerAbstractCoreModule.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerAbstractCoreModulePrivate: public qCTKPrivate<qSlicerAbstractCoreModule>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractCoreModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setup()
{
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::name()const
{
  return this->metaObject()->className();
}
