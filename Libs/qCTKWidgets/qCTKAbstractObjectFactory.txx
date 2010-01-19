#ifndef __qCTKAbstractObjectFactory_txx
#define __qCTKAbstractObjectFactory_txx

#include "qCTKAbstractObjectFactory.h"

#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType, typename ClassType>
qCTKFactoryObjectItem<BaseClassType,ClassType>::qCTKFactoryObjectItem(const QString& _key)
  :qCTKAbstractFactoryItem<BaseClassType>(_key)
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename ClassType>
bool qCTKFactoryObjectItem<BaseClassType,ClassType>::load()
{
  this->instantiateObjectFunc = &instantiateObject<BaseClassType, ClassType>;
  return true;
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename ClassType>
BaseClassType* qCTKFactoryObjectItem<BaseClassType,ClassType>::instanciator()
{
  return this->instantiateObjectFunc();
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractObjectFactory<BaseClassType>::qCTKAbstractObjectFactory()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractObjectFactory<BaseClassType>::~qCTKAbstractObjectFactory()
{
}

//-----------------------------------------------------------------------------
template<typename BaseClassType>
template<typename ClassType>
bool qCTKAbstractObjectFactory<BaseClassType>::registerObject(const QString& key)
{
  // Check if already registered
  if (this->item(key))
    {
    return false;
    }
  QSharedPointer<qCTKFactoryObjectItem<BaseClassType, ClassType> > objectItem =
    QSharedPointer<qCTKFactoryObjectItem<BaseClassType, ClassType> >(
      new qCTKFactoryObjectItem<BaseClassType, ClassType>(key) );
  return this->registerItem(objectItem);
}

#endif
