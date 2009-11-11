#ifndef __qCTKAbstractFactory_h
#define __qCTKAbstractFactory_h 

#include <QString>
#include <QHash>
#include <QSharedPointer>
#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractFactoryItem
{
public:
  qCTKAbstractFactoryItem(const QString& key):Key(key),Instance(){}
  
  virtual bool load() = 0;
  BaseClassType* instanciate()
    {
    if (this->Instance)
      {
      return this->Instance; 
      }
    this->Instance = this->instanciator();
    return this->Instance; 
    }
  bool instanciated() { return (this->Instance != 0); }
  QString key() { return this->Key; }
protected:
  virtual BaseClassType* instanciator() = 0;
private:
  QString Key; 
  BaseClassType* Instance; 
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
  // Call the load method associated with the item. 
  // If succesfully loaded, add it to the internal map.
  bool registerItem(const QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > & item)
    {
    // Sanity checks
    if (!item || item->key().isEmpty() || this->get(item->key()))
      {
      return false; 
      }
      
    // Attempt to load it
    if (!item->load())
      {
      qWarning() << "Failed to load object:" << item->key();
      return false;
      }
      
    // Store its reference using a QSharedPointer
    this->RegisteredItemMap[item->key()] = item;
    return true;
    }
  
  //----------------------------------------------------------------------------
  // Description:
  // Create an instance of the object
  BaseClassType * instanciate(const QString& itemKey)
    {
    qCTKAbstractFactoryItem<BaseClassType> * item = this->get(itemKey);
    return (item ? item->instanciate() : 0);
    }
  
  //----------------------------------------------------------------------------
  // Description:
  // Get a Factory item given its itemKey. Return 0 if any.
  qCTKAbstractFactoryItem<BaseClassType> * get(const QString& itemKey)
    {
    ConstIterator iter = this->RegisteredItemMap.find(itemKey); 
    
    if ( iter == this->RegisteredItemMap.constEnd())
      {
      return 0;
      }
    return iter.value().data();
    }

  //----------------------------------------------------------------------------
  // Description:
  // Get list of all registered item names
  QStringList names() 
    {
    // Since by construction, we checked if a name has already in the QHash.
    // No need to call 'uniqueKeys'
    return this->RegisteredItemMap.keys();
    }

private:
  qCTKAbstractFactory(const qCTKAbstractFactory &); // Not implemented
  void operator=(const qCTKAbstractFactory&); // Not implemented
  
  QHash<QString, QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > > RegisteredItemMap;
}; 

#endif
