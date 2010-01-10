#ifndef __qCTKAbstractQObjectFactory_txx
#define __qCTKAbstractQObjectFactory_txx

#include "qCTKAbstractObjectFactory.h"

#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractQObjectFactory<BaseClassType>::qCTKAbstractQObjectFactory()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractQObjectFactory<BaseClassType>::~qCTKAbstractQObjectFactory()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
BaseClassType* qCTKAbstractQObjectFactory<BaseClassType>::instantiate(const QString& itemKey)
{
  return this->qCTKAbstractObjectFactory<BaseClassType>::instantiate(itemKey);
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
void qCTKAbstractQObjectFactory<BaseClassType>::uninstantiate(const QString& itemKey)
{
  this->qCTKAbstractObjectFactory<BaseClassType>::uninstantiate(itemKey);
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
template<typename ClassType>
bool qCTKAbstractQObjectFactory<BaseClassType>::registerQObject(QString& key)
{
  key = QString::fromLatin1(ClassType::staticMetaObject.className());
  return this->qCTKAbstractObjectFactory<BaseClassType>::template registerObject<ClassType>(key);
}

#endif
