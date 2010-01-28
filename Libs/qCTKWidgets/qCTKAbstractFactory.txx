#ifndef __qCTKAbstractFactory_txx
#define __qCTKAbstractFactory_txx

#include "qCTKAbstractFactory.h"
#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractFactoryItem<BaseClassType>::qCTKAbstractFactoryItem(const QString& _key)
  :Instance()
  ,Key(_key)
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
QString qCTKAbstractFactoryItem<BaseClassType>::loadErrorString()const
{ 
  return QString(); 
}

//----------------------------------------------------------------------------
template<typename BaseClassType>  
BaseClassType* qCTKAbstractFactoryItem<BaseClassType>::instantiate()
{
  if (this->Instance)
    {
    return this->Instance;
    }
  this->Instance = this->instanciator();
  return this->Instance;
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
bool qCTKAbstractFactoryItem<BaseClassType>::instantiated() 
{
  return (this->Instance != 0); 
}


//----------------------------------------------------------------------------
template<typename BaseClassType>
QString qCTKAbstractFactoryItem<BaseClassType>::key() 
{ 
  return this->Key; 
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
void qCTKAbstractFactoryItem<BaseClassType>::uninstantiate()
{
  if (!this->Instance)
    {
    return;
    }
  delete this->Instance;
  // Make sure the pointer is set to 0. Doing so, Will prevent attempt to
  // delete unextising object if uninstantiate() methods is called multiple times.
  this->Instance = 0; 
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractFactory<BaseClassType>::qCTKAbstractFactory()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractFactory<BaseClassType>::~qCTKAbstractFactory()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
void qCTKAbstractFactory<BaseClassType>::printAdditionalInfo()
{
  qDebug() << "qCTKAbstractFactory<BaseClassType> (" << this << ")";
  // TODO
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
BaseClassType* qCTKAbstractFactory<BaseClassType>::instantiate(const QString& itemKey)
{
  qCTKAbstractFactoryItem<BaseClassType>* _item = this->item(itemKey);
  return (_item ? _item->instantiate() : 0);
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
void qCTKAbstractFactory<BaseClassType>::uninstantiate(const QString& itemKey)
{
  qCTKAbstractFactoryItem<BaseClassType> * _item = this->item(itemKey);
  if (!_item)
    {
    return;
    }
  _item->uninstantiate();
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
QStringList qCTKAbstractFactory<BaseClassType>::names() const
{
  // Since by construction, we checked if a name was already in the QHash,
  // there is no need to call 'uniqueKeys'
  return this->RegisteredItemMap.keys();
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
bool qCTKAbstractFactory<BaseClassType>::registerItem(
  const QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > & _item)
{
  // Sanity checks
  if (!_item || _item->key().isEmpty() || this->item(_item->key()))
    {
    return false;
    }
  
  // Attempt to load it
  if (!_item->load())
    {
    QString errorStr;
    if (!_item->loadErrorString().isEmpty())
      {
      errorStr = " - " + _item->loadErrorString();
      }
    qCritical() << "Failed to load object:" << _item->key() << errorStr ;
    return false;
    }
  
  // Store its reference using a QSharedPointer
  this->RegisteredItemMap[_item->key()] = _item;
  return true;
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKAbstractFactoryItem<BaseClassType> * qCTKAbstractFactory<BaseClassType>::item(const QString& itemKey)const
{
  ConstIterator iter = this->RegisteredItemMap.find(itemKey);
  if ( iter == this->RegisteredItemMap.constEnd())
    {
    return 0;
    }
  return iter.value().data();
}

#endif
