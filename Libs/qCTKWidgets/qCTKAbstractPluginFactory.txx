#ifndef __qCTKAbstractPluginFactory_txx
#define __qCTKAbstractPluginFactory_txx

#include "qCTKAbstractPluginFactory.h"

#include <QPluginLoader>
#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKFactoryPluginItem<BaseClassType>::qCTKFactoryPluginItem(const QString& _key,
                                                            const QString& _path)
  :qCTKAbstractFactoryItem<BaseClassType>(_key),Path(_path)
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
bool qCTKFactoryPluginItem<BaseClassType>::load()
{
  this->Loader.setFileName(this->path());
  return this->Loader.load();
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
QString qCTKFactoryPluginItem<BaseClassType>::path()const
{ 
  return this->Path; 
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
QString qCTKFactoryPluginItem<BaseClassType>::loadErrorString()const
{ 
  return this->Loader.errorString();
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
BaseClassType* qCTKFactoryPluginItem<BaseClassType>::instanciator()
{
  //qDebug() << "PluginItem::instantiate - name:" << this->path();
  QObject * object = this->Loader.instance();
  if (!object)
    {
    qWarning() << "Failed to instantiate plugin:" << this->path();
    return 0;
    }
  BaseClassType* castedObject = qobject_cast<BaseClassType*>(object);
  if (!castedObject)
    {
    qWarning() << "Failed to access interface [" << BaseClassType::staticMetaObject.className()
               << "] in plugin:" << this->path();
    delete object; // Clean memory
    return 0;
    }
  return castedObject;
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
qCTKAbstractPluginFactory<BaseClassType, FactoryItemType>::qCTKAbstractPluginFactory():qCTKAbstractFactory<BaseClassType>()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
qCTKAbstractPluginFactory<BaseClassType, FactoryItemType>::~qCTKAbstractPluginFactory()
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
bool qCTKAbstractPluginFactory<BaseClassType, FactoryItemType>::registerLibrary(const QFileInfo& file, QString& key)
{
  key = file.fileName();
  // Check if already registered
  if (this->item(key))
    {
    return false;
    }
  QSharedPointer<FactoryItemType> _item =
    QSharedPointer<FactoryItemType>(new FactoryItemType(key, file.filePath()));
  return this->registerItem(_item);
}

#endif
