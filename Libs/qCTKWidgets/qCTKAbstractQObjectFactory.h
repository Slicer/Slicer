#ifndef __qCTKAbstractQObjectFactory_h
#define __qCTKAbstractQObjectFactory_h

#include "qCTKAbstractObjectFactory.h"

#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType, typename ClassType>
class qCTKFactoryQObjectItem : public qCTKFactoryObjectItem<BaseClassType, ClassType>
{
public:
  qCTKFactoryQObjectItem(const QString& key):qCTKFactoryObjectItem<BaseClassType, ClassType>(key){}

  virtual void uninstantiate()
    {
    if (!this->Instance)
      {
      return;
      }
    Q_ASSERT(!this->Instance->parent());
    if (this->Instance->parent())
      {
      return;
      }
    delete this->Instance;
    this->Instance = 0;
    }
};

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractQObjectFactory : protected qCTKAbstractObjectFactory<BaseClassType>
{
public:
  //-----------------------------------------------------------------------------
  // Description:
  // Constructor
  qCTKAbstractQObjectFactory(){}
  virtual ~qCTKAbstractQObjectFactory(){}

  //----------------------------------------------------------------------------
  // Description:
  // Create an instance of the object
  BaseClassType * instantiate(const QString& itemKey)
    {
    return this->qCTKAbstractObjectFactory<BaseClassType>::instantiate(itemKey);
    }

  //----------------------------------------------------------------------------
  // Description:
  // Uninstanciate the object
  void uninstantiate(const QString& itemKey)
    {
    this->qCTKAbstractObjectFactory<BaseClassType>::uninstantiate(itemKey);
    }

  //----------------------------------------------------------------------------
  // Description:
  // Get list of all registered item names
  QStringList names()
    {
    return this->qCTKAbstractObjectFactory<BaseClassType>::names();
    }

  //-----------------------------------------------------------------------------
  // Description:
  // Register an object in the factory
  template<typename ClassType>
  bool registerQObject()
    {
    QString key = QString::fromLatin1(ClassType::staticMetaObject.className());

    // Check if already registered
    if (this->getItem(key))
      {
      return false;
      }
    QSharedPointer<qCTKFactoryQObjectItem<BaseClassType, ClassType> > objectItem =
      QSharedPointer<qCTKFactoryQObjectItem<BaseClassType, ClassType> >(
        new qCTKFactoryQObjectItem<BaseClassType, ClassType>(key) );
    return this->registerItem(objectItem);
    }

private:
  qCTKAbstractQObjectFactory(const qCTKAbstractQObjectFactory &);  // Not implemented
  void operator=(const qCTKAbstractQObjectFactory&); // Not implemented
};

#endif
