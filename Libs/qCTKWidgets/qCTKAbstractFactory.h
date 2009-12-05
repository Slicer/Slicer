#ifndef __qCTKAbstractFactory_h
#define __qCTKAbstractFactory_h

#include <QString>
#include <QHash>
#include <QSharedPointer>
#include <QStringList>
#include <QDebug>

// Forward declaration
//template <typename X> class qCTKAbstractFactory;

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractFactoryItem
{
  //template <typename X> friend class qCTKAbstractFactory;

public:
  qCTKAbstractFactoryItem(const QString& key):Instance(),Key(key){}

  virtual QString loadErrorString() { return QString(); }
  virtual bool load() = 0;
  BaseClassType* instantiate()
    {
    if (this->Instance)
      {
      return this->Instance;
      }
    this->Instance = this->instanciator();
    return this->Instance;
    }
  bool instantiated() { return (this->Instance != 0); }
  QString key() { return this->Key; }
  virtual void uninstantiate()
    {
    if (!this->Instance)
      {
      return;
      }
    delete this->Instance;
    }

protected:
  virtual BaseClassType* instanciator() = 0;
  BaseClassType* Instance;
private:
  QString Key;
};

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractFactory
{
protected:
  typedef typename QHash<QString, QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > >::const_iterator ConstIterator;
  typedef typename QHash<QString, QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > >::iterator       Iterator;

public:
  qCTKAbstractFactory(){}
  virtual ~qCTKAbstractFactory(){}

  //----------------------------------------------------------------------------
  // Description:
  // Create an instance of the object
  BaseClassType * instantiate(const QString& itemKey)
    {
    qCTKAbstractFactoryItem<BaseClassType> * item = this->getItem(itemKey);
    return (item ? item->instantiate() : 0);
    }

  //----------------------------------------------------------------------------
  // Description:
  // Uninstanciate the object
  void uninstantiate(const QString& itemKey)
    {
    qCTKAbstractFactoryItem<BaseClassType> * item = this->getItem(itemKey);
    if (!item)
      {
      return;
      }
    item->uninstantiate();
    }

  //----------------------------------------------------------------------------
  // Description:
  // Get an object given its itemKey. Return 0 if any or if not instantiated
//   BaseClassType * get(const QString& itemKey)
//     {
//     qCTKAbstractFactoryItem<BaseClassType> * item = getItem(itemKey);
//     return (item ? item->instance() : 0);
//     }

  //----------------------------------------------------------------------------
  // Description:
  // Get list of all registered item names
  QStringList names() const
    {
    // Since by construction, we checked if a name was already in the QHash,
    // there is no need to call 'uniqueKeys'
    return this->RegisteredItemMap.keys();
    }

protected:
    //----------------------------------------------------------------------------
  // Description:
  // Call the load method associated with the item.
  // If succesfully loaded, add it to the internal map.
  bool registerItem(const QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > & item)
    {
    // Sanity checks
    if (!item || item->key().isEmpty() || this->getItem(item->key()))
      {
      return false;
      }

    // Attempt to load it
    if (!item->load())
      {
      QString errorStr;
      if (!item->loadErrorString().isEmpty())
        {
        errorStr = " - " + item->loadErrorString();
        }
      qCritical() << "Failed to load object:" << item->key()
                  << errorStr ;
      return false;
      }

    // Store its reference using a QSharedPointer
    this->RegisteredItemMap[item->key()] = item;
    return true;
    }

  //----------------------------------------------------------------------------
  // Description:
  // Get a Factory item given its itemKey. Return 0 if any.
  qCTKAbstractFactoryItem<BaseClassType> * getItem(const QString& itemKey)
    {
    ConstIterator iter = this->RegisteredItemMap.find(itemKey);

    if ( iter == this->RegisteredItemMap.constEnd())
      {
      return 0;
      }
    return iter.value().data();
    }

private:
  qCTKAbstractFactory(const qCTKAbstractFactory &); // Not implemented
  void operator=(const qCTKAbstractFactory&); // Not implemented

  QHash<QString, QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > > RegisteredItemMap;
};

#endif
