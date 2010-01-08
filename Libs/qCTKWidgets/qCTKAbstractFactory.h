#ifndef __qCTKAbstractFactory_h
#define __qCTKAbstractFactory_h

#include <QString>
#include <QHash>
#include <QSharedPointer>
#include <QStringList>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractFactoryItem
{
public:
  explicit qCTKAbstractFactoryItem(const QString& key);
  
  virtual QString loadErrorString();
  virtual bool load() = 0;
  BaseClassType* instantiate();
  bool instantiated();
  QString key();
  virtual void uninstantiate();
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
  // Description:
  // Constructor/Desctructor
  explicit qCTKAbstractFactory();
  virtual ~qCTKAbstractFactory();

  //----------------------------------------------------------------------------
  // Description:
  // Create an instance of the object
  BaseClassType * instantiate(const QString& itemKey);

  //----------------------------------------------------------------------------
  // Description:
  // Uninstanciate the object
  void uninstantiate(const QString& itemKey);

  //----------------------------------------------------------------------------
  // Description:
  // Get list of all registered item names
  QStringList names() const;

protected:
  //----------------------------------------------------------------------------
  // Description:
  // Call the load method associated with the item.
  // If succesfully loaded, add it to the internal map.
  bool registerItem(const QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > & item);

  //----------------------------------------------------------------------------
  // Description:
  // Get a Factory item given its itemKey. Return 0 if any.
  qCTKAbstractFactoryItem<BaseClassType> * getItem(const QString& itemKey);

private:
  qCTKAbstractFactory(const qCTKAbstractFactory &); // Not implemented
  void operator=(const qCTKAbstractFactory&); // Not implemented

  QHash<QString, QSharedPointer<qCTKAbstractFactoryItem<BaseClassType> > > RegisteredItemMap;
};

#include "qCTKAbstractFactory.txx"

#endif
