#ifndef __qCTKAbstractPluginFactory_h
#define __qCTKAbstractPluginFactory_h

#include "qCTKAbstractFactory.h"

#include <QPluginLoader>
#include <QFileInfo>
#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKFactoryPluginItem : public qCTKAbstractFactoryItem<BaseClassType>
{
public:
  explicit qCTKFactoryPluginItem(const QString& key, const QString& path):
    qCTKAbstractFactoryItem<BaseClassType>(key),Path(path){}
  virtual bool load()
    {
    this->Loader.setFileName(this->path());
    return this->Loader.load();
    }
  QString path() { return this->Path; }
  virtual QString loadErrorString() { return this->Loader.errorString();}

protected:
  virtual BaseClassType* instanciator()
    {
    qDebug() << "PluginItem::instantiate - name:" << this->path();
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

private:
  QPluginLoader    Loader;
  QString          Path;
};

//----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType = qCTKFactoryPluginItem<BaseClassType> >
class qCTKAbstractPluginFactory : public qCTKAbstractFactory<BaseClassType>
{
public:
  // Description:
  // Constructor
  explicit qCTKAbstractPluginFactory():qCTKAbstractFactory<BaseClassType>(){}
  virtual ~qCTKAbstractPluginFactory(){}

  // Description:
  // Register a plugin in the factory
  virtual bool registerLibrary(const QString& key, const QString& path)
    {
    // Check if already registered
    if (this->getItem(key))
      {
      return false;
      }
    QSharedPointer<FactoryItemType> item =
      QSharedPointer<FactoryItemType>(new FactoryItemType(key, path));
    return this->registerItem(item);
    }


private:
  qCTKAbstractPluginFactory(const qCTKAbstractPluginFactory &);  // Not implemented
  void operator=(const qCTKAbstractPluginFactory&); // Not implemented
};

#endif
